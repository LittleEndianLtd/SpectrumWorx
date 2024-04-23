////////////////////////////////////////////////////////////////////////////////
///
/// sampleMac.cpp
/// -------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "file.hpp"

#include "le/math/conversion.hpp"
#include "le/utility/filesystem.hpp"
#include "le/utility/trace.hpp"

#include "boost/utility/string_ref.hpp"

#include "AudioToolbox/AudioToolbox.h"
#if !__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__
#include "CoreServices/CoreServices.h"
#endif // !__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__
#include "CoreFoundation/CoreFoundation.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

// http://izotope.fileburst.com/guides/iZotope_iOS_Audio_Programming_Guide.pdf
// https://developer.apple.com/library/mac/documentation/musicaudio/reference/ExtendedAudioFileServicesReference/Reference/reference.html

namespace
{
    class ExtAudioFileGuard
    {
    public:
        ExtAudioFileGuard() : file_( nullptr ) {}
        ExtAudioFileGuard( ExtAudioFileGuard && other ) : file_( other.file_ ) { other.file_ = nullptr; }
        ~ExtAudioFileGuard() { close(); }

        ExtAudioFileRef   operator*() const { BOOST_ASSERT(  file_ ); return  file_; }
        ExtAudioFileRef * operator&()       { BOOST_ASSERT( !file_ ); return &file_; }

        LE_NOTHROW LE_COLD void close() { BOOST_VERIFY( ::ExtAudioFileDispose( file_ ) == noErr || file_ == nullptr ); file_ = nullptr; }

    private:
        ExtAudioFileRef file_;
    }; // class ExtAudioFileGuard
} // anonymous namespace


class FileImpl
{
public:
    FileImpl() = default;
    FileImpl( FileImpl && other ) : abl_( other.abl_ ), file_( std::move( other.file_ ) ) {}

    template <Utility::SpecialLocations parentDirectory>
    char const * LE_COLD open( char const * const fileName )
    {
        return openFullPath( Utility::fullPath<parentDirectory>( fileName ) );
    }

    LE_NOTHROW
    char const * LE_COLD openFullPath( char const * const sampleFileName )
    {
        BOOST_ASSERT_MSG( ::access( sampleFileName, R_OK ) != -1, "Sample file does not exist or is not accessible." );

        OSStatus error;
        {
            ExtAudioFileRef file;

            auto const sampleURL( ::CFURLCreateFromFileSystemRepresentation( nullptr, reinterpret_cast<UInt8 const *>( sampleFileName ), std::strlen( sampleFileName ), false ) );
            error = ::ExtAudioFileOpenURL( sampleURL, &file );
            ::CFRelease( sampleURL );
            if ( error != noErr )
                return "Unable to open file.";

            file_.close();
            *&file_ = file;

            // https://developer.apple.com/library/ios/qa/qa1678/_index.html
            BOOST_ASSERT( position() == 0 );
        }

        auto const inputFormat  ( format()    );
        auto       desiredFormat( inputFormat );
        desiredFormat.mFormatID        = kAudioFormatLinearPCM;
        desiredFormat.mFormatFlags     = kLinearPCMFormatFlagIsFloat;
        desiredFormat.mFramesPerPacket = 1;
        desiredFormat.mBytesPerFrame   = desiredFormat.mChannelsPerFrame * sizeof( float );
        desiredFormat.mBitsPerChannel  = sizeof( float ) * 8;
        desiredFormat.mBytesPerPacket  = desiredFormat.mFramesPerPacket * desiredFormat.mBytesPerFrame;

        error = ::ExtAudioFileSetProperty( *file_, kExtAudioFileProperty_ClientDataFormat, sizeof( desiredFormat ), &desiredFormat );
        if ( error != noErr )
        {
            LE_TRACE( "OSX error: %d", error );
            return "Unable to set desired format.";
        }
    #if 0
        //auto & abl_
        //(
        //    *static_cast<::AudioBufferList *>
        //    (
        //        alloca
        //        (
        //            offsetof( ::AudioBufferList, mBuffers )
        //                +
        //            ( sizeof( ::AudioBuffer ) * inputFormat.mChannelsPerFrame )
        //        )
        //    )
        //);
        //abl_.mNumberBuffers = inputFormat.mChannelsPerFrame;
    #endif
        abl_.mNumberBuffers = 1;
        abl_.mBuffers[ 0 ].mNumberChannels = desiredFormat.mChannelsPerFrame;

        return nullptr;
    }

    LE_COLD void close() { file_.close(); }

    LE_COLD std::uint32_t LE_FASTCALL read( float * LE_RESTRICT const pOutput, std::uint32_t const samples ) const
    {
        abl_.mBuffers[ 0 ].mDataByteSize = abl_.mBuffers[ 0 ].mNumberChannels * samples * sizeof( float );
        abl_.mBuffers[ 0 ].mData         = pOutput;
        UInt32 numberOfSampleFrames( samples );
        if ( BOOST_UNLIKELY( ::ExtAudioFileRead( *file_, &numberOfSampleFrames, &abl_ ) != noErr ) )
            LE_TRACE( "File::read() error." );
        BOOST_ASSERT( numberOfSampleFrames <= samples );
        return numberOfSampleFrames;
    }

    std::uint8_t  numberOfChannels     () const { return static_cast<std::uint8_t>( abl_.mBuffers[ 0 ].mNumberChannels ); }
    std::uint32_t sampleRate           () const { return Math::convert<std::uint32_t>( format().mSampleRate ); }
    std::uint32_t remainingSampleFrames() const { return lengthInSampleFrames() - position(); }

    LE_COLD std::uint32_t lengthInSampleFrames() const
    {
        SInt64 fileLengthInFrames;
        UInt32 propertySize( sizeof( fileLengthInFrames ) );
        BOOST_VERIFY( ::ExtAudioFileGetProperty( *file_, kExtAudioFileProperty_FileLengthFrames, &propertySize, &fileLengthInFrames ) == noErr );
        BOOST_ASSERT( propertySize == sizeof( fileLengthInFrames ) );
        return static_cast<std::uint32_t>( fileLengthInFrames );
    }

    LE_NOTHROW LE_COLD void LE_FASTCALL_ABI setTimePosition( std::uint32_t const positionInMilliseconds )
    {
        setSamplePosition( static_cast<std::uint32_t>( positionInMilliseconds * format().mSampleRate / 1000 ) );
    }

    LE_NOTHROW LE_COLD void LE_FASTCALL_ABI setSamplePosition( std::uint32_t const positionInSampleFrames )
    {
        // http://lists.apple.com/archives/coreaudio-api/2010/Sep/msg00063.html ExtAudioFileSeek bug workaround
        // https://developer.apple.com/library/mac/qa/qa1678/_index.html
        BOOST_VERIFY( ::ExtAudioFileSeek( *file_, positionInSampleFrames ) == noErr );
    }

    LE_NOTHROW LE_COLD std::uint32_t LE_FASTCALL_ABI getTimePosition  () const { return samples2milliseconds( getSamplePosition() ); }
    LE_NOTHROW LE_COLD std::uint32_t LE_FASTCALL_ABI getSamplePosition() const { return position(); }

    LE_COLD void restart() { setSamplePosition( 0 ); }

    explicit operator bool() const { return *file_; }

private:
    LE_COLD std::uint32_t position() const
    {
        // https://developer.apple.com/library/ios/qa/qa1609/_index.html
        // https://developer.apple.com/library/ios/documentation/MusicAudio/Reference/ExtendedAudioFileServicesReference/#//apple_ref/c/func/ExtAudioFileTell
        SInt64 positionInFrames; // in file's samplerate
        BOOST_VERIFY( ::ExtAudioFileTell( *file_, &positionInFrames ) == noErr );
        return static_cast<std::uint32_t>( positionInFrames );
    }

    LE_NOTHROWNOALIAS AudioStreamBasicDescription LE_FASTCALL format() const
    {
        AudioStreamBasicDescription inputFormat;
        UInt32 propertySize = sizeof( inputFormat );
        BOOST_VERIFY( ::ExtAudioFileGetProperty( *file_, kExtAudioFileProperty_FileDataFormat, &propertySize, &inputFormat ) == noErr );
        BOOST_ASSERT( propertySize == sizeof( inputFormat ) );
        return inputFormat;
    }

    //...mrmlj...copied from fileAndroid.cpp
    LE_COLD std::uint32_t LE_FASTCALL milliseconds2samples( std::uint32_t const milliseconds ) const { return static_cast<std::uint32_t>( std::uint64_t( milliseconds ) * sampleRate() / 1000 ); }
    LE_COLD std::uint32_t LE_FASTCALL samples2milliseconds( std::uint32_t const sampleFrames ) const { return static_cast<std::uint32_t>( std::uint64_t( sampleFrames ) * 1000 / sampleRate() ); }

private:
    mutable ::AudioBufferList abl_;
    ExtAudioFileGuard file_;
}; // class FileImpl


// http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/SupportedAudioFormatsMacOSX/SupportedAudioFormatsMacOSX.html
char const File::supportedFormats[] = "*.aac;*.ac3;*.adts;*.aif;*.aiff;*.au;*.caf;*.mpa;*.mp3;*.mp4;*.m4a;*.sd2;*.snd;*.wav";

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "pimplForwarders.inl"
