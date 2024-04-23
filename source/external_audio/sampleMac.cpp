////////////////////////////////////////////////////////////////////////////////
///
/// sampleMac.cpp
/// -------------
///
/// Copyright (c) 2010.-2013. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "sample.hpp"

#include "AudioToolbox/AudioToolbox.h"
#include "CoreServices/CoreServices.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace SW { namespace GUI { FSRef makeFSRefFromPath( juce::String const & path ); } } //...mrmlj...

juce::String Sample::supportedFormats()
{
    // http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/SupportedAudioFormatsMacOSX/SupportedAudioFormatsMacOSX.html
    return "*.aac;*.ac3;*.adts;*.aif;*.aiff;*.au;*.caf;*.mpa;*.mp3;*.mp4;*.m4a;*.sd2;*.snd;*.wav";
}

namespace
{
    class ExtAudioFileGuard
    {
    public:
        ExtAudioFileGuard() : file_( 0 ) {}
        ~ExtAudioFileGuard() { BOOST_VERIFY( ::ExtAudioFileDispose( file_ ) == noErr ); }

        ExtAudioFileRef   operator*() const { BOOST_ASSERT(  file_ ); return  file_; }
        ExtAudioFileRef * operator&()       { BOOST_ASSERT( !file_ ); return &file_; }

    private:
        ExtAudioFileRef file_;
    };
} // anonymous namespace

LE_NOTHROWNOALIAS
char const * Sample::doLoad( juce::String const & sampleFileName, unsigned int const desiredSampleRate, Sample::DataHolder & data )
{
    FSRef const samplePath( SW::GUI::makeFSRefFromPath( sampleFileName ) );

    ExtAudioFileGuard sampleFile;
    OSStatus error( ::ExtAudioFileOpen( &samplePath, &sampleFile ) );
    if ( error != noErr )
        return "Unable to open file.";

    UInt32 propertySize;

    AudioStreamBasicDescription inputFormat;
    propertySize = sizeof( inputFormat );
    BOOST_VERIFY( ::ExtAudioFileGetProperty( *sampleFile, kExtAudioFileProperty_FileDataFormat, &propertySize, &inputFormat ) == noErr );
    BOOST_ASSERT( propertySize == sizeof( inputFormat ) );

    AudioStreamBasicDescription desiredFormat;

    desiredFormat.mSampleRate       = desiredSampleRate;
    desiredFormat.mFormatID         = kAudioFormatLinearPCM;
    desiredFormat.mFormatFlags      = kLinearPCMFormatFlagIsFloat | kAudioFormatFlagIsNonInterleaved;
    desiredFormat.mFramesPerPacket  = 1;
    desiredFormat.mBytesPerFrame    = sizeof( float );
    desiredFormat.mChannelsPerFrame = 2;
    desiredFormat.mBitsPerChannel   = 32;
    desiredFormat.mReserved         = 0;

    desiredFormat.mBytesPerPacket   = desiredFormat.mFramesPerPacket * desiredFormat.mBytesPerFrame;

    error = ::ExtAudioFileSetProperty( *sampleFile, kExtAudioFileProperty_ClientDataFormat, sizeof( desiredFormat ), &desiredFormat );
    if ( error != noErr )
        return "Unable to set desired format.";

    // Implementation note:
    //   ExtAudio does not automatically expand mono files to stereo (even if we
    // set the output format to stereo) so we have to explicitly tell it to do
    // so.
    // http://web.archiveorange.com/archive/v/q7bub8POgFuTNzMP0MsJ
    // http://www.modejong.com/iPhone (example 3).
    //                                        (08.12.2010.) (Domagoj Saric)
    if ( inputFormat.mChannelsPerFrame == 1 )
    {
        AudioConverterRef converter;
        propertySize = sizeof( converter );
        BOOST_VERIFY( ::ExtAudioFileGetProperty( *sampleFile, kExtAudioFileProperty_AudioConverter, &propertySize, &converter ) == noErr );
        BOOST_ASSERT( propertySize == sizeof( converter ) );

        static SInt32 const channelMap[] = { 0, 0 };
        BOOST_VERIFY( ::AudioConverterSetProperty( converter, kAudioConverterChannelMap, sizeof( channelMap ), channelMap ) == noErr );
    }

    SInt64 fileLengthInFrames;
    propertySize = sizeof( fileLengthInFrames );
    error = ::ExtAudioFileGetProperty( *sampleFile, kExtAudioFileProperty_FileLengthFrames, &propertySize, &fileLengthInFrames );
    BOOST_ASSERT( error        == noErr                        );
    BOOST_ASSERT( propertySize == sizeof( fileLengthInFrames ) );

    UInt32 numberOfSamples( static_cast<std::size_t>( fileLengthInFrames ) );

    if ( !data.recreate( numberOfSamples ) )
        return "Out of memory";

    struct AudioBufferList2
    {
        UInt32      mNumberBuffers;
        AudioBuffer mBuffers[ 2 ];
    };

    AudioBufferList2 fillBufList;
    fillBufList.mNumberBuffers = 2;
    fillBufList.mBuffers[ 0 ].mNumberChannels = 1;
    fillBufList.mBuffers[ 0 ].mDataByteSize   = numberOfSamples * sizeof( float );
    fillBufList.mBuffers[ 0 ].mData           = data.pBuffer.get();
    fillBufList.mBuffers[ 1 ].mNumberChannels = 1;
    fillBufList.mBuffers[ 1 ].mDataByteSize   = numberOfSamples * sizeof( float );
    fillBufList.mBuffers[ 1 ].mData           = data.pChannel2Beginning;

    error = ::ExtAudioFileRead( *sampleFile, &numberOfSamples, reinterpret_cast<AudioBufferList *>( &fillBufList ) );
    if ( error != noErr )
        return "Failed reading data";

    data.pChannel1End += numberOfSamples;
    data.pChannel2End += numberOfSamples;

    return nullptr;
}

//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
