////////////////////////////////////////////////////////////////////////////////
///
/// osx.cpp
/// -------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "device.hpp"
#include "deviceImpl.hpp"

#include "le/math/math.cpp" //...mrmlj...

#include "le/utility/buffers.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"

#include "boost/assert.hpp"

#import "AudioToolbox/AudioConverter.h"
#ifndef NDEBUG
#import "CoreServices/../Frameworks/CarbonCore.framework/Headers/Debugging.h"
#endif // NDEBUG
#import "CoreAudio/AudioHardware.h"
#import "CoreServices/../Frameworks/CarbonCore.framework/Headers/MacErrors.h"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

// https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/CoreAudioOverview/Introduction/Introduction.html
// Playing a sound file using the Default Output Audio Unit https://developer.apple.com/library/mac/technotes/tn2097/_index.html
// Moving Off Deprecated HAL APIs https://developer.apple.com/library/mac/technotes/tn2223/_index.html
// https://developer.apple.com/library/mac/technotes/tn2321/_index.html kAudioHardwarePropertyPowerHint
// https://developer.apple.com/library/mac/samplecode/HALExamples
// https://github.com/superpoweredSDK/Low-Latency-Android-Audio-iOS-Audio-Engine/blob/master/Superpowered/SuperpoweredOSXAudioIO.mm
// http://spaghetticode.org/G5/pa_mac_core-audacity-hog.c

namespace
{
#ifndef NDEBUG
    bool doTrace = true;
#endif // NDEBUG
    //...mrmlj...copied from ios.cpp
    ::OSStatus trace( ::OSStatus const status, unsigned int const lineNumber )
    {
        BOOST_LIKELY( status == noErr );
    #ifndef NDEBUG
        if ( ( status != noErr ) /*|| ( status != kAudioSessionNoError )*/ && doTrace )
        {
            // http://stackoverflow.com/questions/2196869/how-do-you-convert-an-iphone-osstatus-code-to-something-useful
            // http://stackoverflow.com/questions/8118891/are-there-ios-equivalents-of-getmacosstatuserrorstring-and-getmacosstatuscomment
            // http://stackoverflow.com/questions/14699604/replacements-for-getmacosstatuserrorstring-getmacosstatuscommentstring
            Utility::Tracer::error
            (
                "Apple error: %d (%s), (%s) @ %d.",
                status,
                ::GetMacOSStatusCommentString( status ),
                ::GetMacOSStatusErrorString  ( status ),
                lineNumber
            );
        }
    #else
        (void)lineNumber;
    #endif // NDEBUG
        return status;
    }

#ifdef NDEBUG
    #define LE_AIO_TRACE( expression ) expression
#else
    #define LE_AIO_TRACE( expression ) trace( (expression), __LINE__ )
#endif // NDEBUG

    struct AudioObject
    {
        std::uint16_t LE_FASTCALL latency( ::AudioObjectPropertyScope const scope ) const LE_NOTHROW LE_COLD
        {
            // http://lists.apple.com/archives/coreaudio-api/2010/Jan/msg00046.html how to calculate latency?
            // http://lists.apple.com/archives/coreaudio-api/2011/May/msg00057.html
            auto const sum =
                getProperty<UInt32>( kAudioDevicePropertyLatency     , scope ) +
                getProperty<UInt32>( kAudioStreamPropertyLatency     , scope ) +
                getProperty<UInt32>( kAudioDevicePropertySafetyOffset, scope ) +
                frameSize( scope );
            return static_cast<std::uint16_t>( sum );
        }

        std::uint16_t LE_FASTCALL frameSize( ::AudioObjectPropertyScope const scope ) const LE_NOTHROW LE_COLD
        {
            return static_cast<std::uint16_t>( getProperty<UInt32>( kAudioDevicePropertyBufferFrameSize, scope ) );
        }

        bool LE_FASTCALL isRunning() const LE_NOTHROW LE_COLD { return getProperty<UInt32>( kAudioDevicePropertyDeviceIsRunning ); }

        template <typename T> LE_NOTHROW LE_COLD
        ::OSStatus LE_FASTCALL setProperty( ::AudioObjectPropertySelector const id, ::AudioObjectPropertyScope const scope, ::AudioObjectPropertyElement const element, T const & data )
        {
            AudioObjectPropertyAddress const propertyAddress = { id, scope, element };
            return LE_AIO_TRACE( ::AudioObjectSetPropertyData( *this, &propertyAddress, 0, nullptr, sizeof( data ), &data ) );
        }

        template <typename T> LE_NOTHROW LE_COLD
        ::OSStatus LE_FASTCALL setProperty( ::AudioObjectPropertySelector const id, ::AudioObjectPropertyScope const scope, T const & data )
        {
            AudioObjectPropertyAddress const propertyAddress = { id, scope, kAudioObjectPropertyElementMaster };
            return LE_AIO_TRACE( ::AudioObjectSetPropertyData( *this, &propertyAddress, 0, nullptr, sizeof( data ), &data ) );
        }

        template <typename T> LE_NOTHROW LE_COLD
        ::OSStatus LE_FASTCALL setProperty( ::AudioObjectPropertySelector const id, T const & data )
        {
            return setProperty<T>( id, kAudioObjectPropertyScopeGlobal, data );
        }

        template <typename T> LE_NOTHROW LE_COLD LE_PURE_FUNCTION
        T LE_FASTCALL getProperty( ::AudioObjectPropertySelector const id, ::AudioObjectPropertyScope const scope, ::AudioObjectPropertyElement const element ) const
        {
            T property;
            AudioObjectPropertyAddress const propertyAddress = { id, scope, element };
            UInt32 size( sizeof( property ) );
            BOOST_VERIFY( LE_AIO_TRACE( ::AudioObjectGetPropertyData( *this, &propertyAddress, 0, nullptr, &size, &property ) ) == noErr );
            BOOST_ASSERT( size == sizeof( property ) );
            return property;
        }
        template <typename T> T LE_FASTCALL getProperty( ::AudioObjectPropertySelector const id, ::AudioObjectPropertyScope const scope ) const LE_NOTHROW LE_COLD { return getProperty<T>( id, scope                          , kAudioObjectPropertyElementMaster ); }
        template <typename T> T LE_FASTCALL getProperty( ::AudioObjectPropertySelector const id                                         ) const LE_NOTHROW LE_COLD { return getProperty<T>( id, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster ); }

        // http://stackoverflow.com/questions/3199199/how-do-you-select-audio-input-device-in-core-audio
        static AudioObject LE_FASTCALL defaultInputDevice () LE_NOTHROW;
        static AudioObject LE_FASTCALL defaultOutputDevice() LE_NOTHROW;

        operator ::AudioObjectID() const { return id; }

        explicit operator bool() const { return id != kAudioDeviceUnknown; }

        ::AudioObjectID id = kAudioDeviceUnknown;
    }; // struct AudioObject

    AudioObject /*constexpr*/ systemAudio = { kAudioObjectSystemObject };

    template <typename T> LE_NOTHROW LE_COLD
    ::OSStatus setHALProperty( ::AudioObjectPropertySelector const id, ::AudioObjectPropertyScope const scope, T const & data )
    {
        return systemAudio.setProperty<T>( id, scope, data );
    }

    template <typename T> LE_NOTHROW LE_COLD
    ::OSStatus setHALProperty( ::AudioObjectPropertySelector const id, T const & data )
    {
        return setHALProperty<T>( id, kAudioObjectPropertyScopeGlobal, data );
    }

    template <typename T>
    T getHALProperty( ::AudioObjectPropertySelector const id, ::AudioObjectPropertyScope const scope ) { return systemAudio.getProperty<T>( id, scope ); }
    template <typename T>
    T getHALProperty( ::AudioObjectPropertySelector const id                                         ) { return systemAudio.getProperty<T>( id        ); }

    __attribute__(( constructor )) LE_NOTHROW LE_COLD
    void initialiseHAL()
    {
        // http://lists.apple.com/archives/coreaudio-api/2002/Dec/msg00060.html
        // http://lists.apple.com/archives/coreaudio-api/2009/Oct/msg00131.html
        // http://lists.apple.com/archives/coreaudio-api/2009/Oct/msg00212.html
        // http://osdir.lowified.com/coreaudio-api/2009-10/msg00142.html
        BOOST_VERIFY( setHALProperty<CFRunLoopRef>( kAudioHardwarePropertyRunLoop, nullptr ) == noErr );
    }

    LE_NOTHROW LE_COLD
    std::uint8_t LE_FASTCALL numberOfDevices()
    {
        UInt32 dataSize;
        AudioObjectPropertyAddress propertyAddress = { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        BOOST_VERIFY( LE_AIO_TRACE( ::AudioObjectGetPropertyDataSize( kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &dataSize ) ) == noErr );
        return static_cast<std::uint8_t>( dataSize / sizeof( AudioDeviceID ) );
    }

    // kAudioDevicePropertyRelatedDevices
    AudioObject AudioObject::defaultInputDevice () { return { getHALProperty<AudioDeviceID>( kAudioHardwarePropertyDefaultInputDevice  ) }; }
    AudioObject AudioObject::defaultOutputDevice() { return { getHALProperty<AudioDeviceID>( kAudioHardwarePropertyDefaultOutputDevice ) }; }

    //...mrmlj...copied from AU plugin.hpp
    class AudioBuffers
    {
    public:
         AudioBuffers();
        ~AudioBuffers();

        AudioBuffers   ( AudioBuffers const & ) = delete;
        void operator =( AudioBuffers const & ) = delete;

        bool resize( std::uint8_t numberOfChannels, std::uint16_t numberOfSamples, bool interleaved );

        static void alias( ::AudioBufferList const &, ::AudioBufferList & );
        static void copy ( ::AudioBufferList const &, ::AudioBufferList & );

        void aliasTo  ( ::AudioBufferList       & ) const;
        void aliasFrom( ::AudioBufferList const & )      ;
        void copyTo   ( ::AudioBufferList       & ) const;

        ::AudioBufferList       & bufferList()       LE_NOTHROW LE_PURE_FUNCTION { return *pIOBuffers_; }
        ::AudioBufferList const & bufferList() const LE_NOTHROW LE_PURE_FUNCTION { return *pIOBuffers_; }

                 ::AudioBufferList * __restrict  operator -> () const LE_NOTHROW LE_PURE_FUNCTION { return  pIOBuffers_; }
        operator ::AudioBufferList * __restrict              () const LE_NOTHROW LE_PURE_FUNCTION { return  pIOBuffers_; }
        operator ::AudioBufferList & __restrict              () const LE_NOTHROW LE_PURE_FUNCTION { return *pIOBuffers_; }

        explicit operator bool() const { return pIOBuffers_; }

    private:
        using AudioBufferListPtr /*alignas( 16 )*/ = ::AudioBufferList * __restrict;
        AudioBufferListPtr pIOBuffers_;
    }; // class AudioBuffers
} // anonymous namespace

class DeviceImpl
{
public:
     DeviceImpl()
         :
        input_ { AudioObject::defaultInputDevice () },
        output_{ AudioObject::defaultOutputDevice() }
    {}
    ~DeviceImpl()
    {
        stop();
        BOOST_ASSERT( !isRunning() );
    }


    char const * setup( std::uint8_t numberOfChannels, std::uint32_t sampleRate, std::uint16_t desiredLatencyInSamples );

    template <typename DataLayout>
    char const * setCallback( void (^callback)( DataLayout ) );

    void LE_COLD start() const
    {
        BOOST_ASSERT( !isRunning() );
        input_ .start();
        output_.start();
        BOOST_ASSERT( isRunning () );
    }

    void LE_COLD stop() const
    {
        output_.stop();
        /// \note Check the related implementation note in renderProcedure().
        ///                                   (16.09.2015.) (Domagoj Saric)
        if ( inputData_ ) inputData_->mBuffers[ 0 ].mDataByteSize = 0;
        input_ .stop();
        if ( inputData_ ) inputData_->mBuffers[ 0 ].mDataByteSize = 0;
        //...mrmlj... http://lists.apple.com/archives/coreaudio-api/2001/Aug/msg00097.html
        //BOOST_ASSERT( !isRunning() );
    }

    std::uint8_t  numberOfChannels() const { return numberOfChannels_; }
    std::uint32_t sampleRate      () const { return sampleRate_      ; }

    LE_COLD LE_NOTHROW
    Device::LatencyAndBufferSize LE_FASTCALL latency() const
    {
        auto const inputLatency ( input_ .renderProc ? input_ .object.latency( kAudioDevicePropertyScopeInput  ) : 0 );
        auto const outputLatency( output_.renderProc ? output_.object.latency( kAudioDevicePropertyScopeOutput ) : 0 );
        auto const totalLatency ( inputLatency + outputLatency                                                       );
        BOOST_ASSERT
        (
            !totalLatency || // allow queries prior to a setCallback() call
            totalLatency >= frameSize()
        );

        return Device::LatencyAndBufferSize
        (
            static_cast<Device::LatencyAndBufferSize::first_type >( totalLatency ),
            static_cast<Device::LatencyAndBufferSize::second_type>( frameSize()  )
        );
    }

private:
    std::uint16_t frameSize() const
    {
        BOOST_ASSERT( frameSize_ == input_ .object.frameSize( kAudioDevicePropertyScopeInput  ) );
        BOOST_ASSERT( frameSize_ == output_.object.frameSize( kAudioDevicePropertyScopeOutput ) );
        return frameSize_;
    }

    bool isRunning() const
    {
        return
            ( input_  && input_ .object.isRunning() ) ||
            ( output_ && output_.object.isRunning() );
    }

    bool singleIODevice() const { return input_.object == output_.object; }

    char const * LE_FASTCALL setCallback
    (
        AudioDeviceIOProc callback,
        bool              interleaved,
        bool              inputEnabled,
        bool              outputEnabled
    );

    // aggregate devices
    // http://daveaddey.com/?p=51
    // http://lists.apple.com/archives/coreaudio-api/2014/Mar/msg00035.html
    // AudioDeviceRead
    LE_HOT
    static OSStatus inputRenderProcedure
    (
        ::AudioDeviceID                        const deviceID   ,
        ::AudioTimeStamp  const * __restrict__ const pNowTime   ,
        ::AudioBufferList const * __restrict__ const pInputData ,
        ::AudioTimeStamp  const * __restrict__ const pInputTime ,
        ::AudioBufferList       * __restrict__ const pOutputData,
        ::AudioTimeStamp  const * __restrict__ const pOutputTime,
        void                    * __restrict__ const pContext
    )
    {
        LE_ASSUME( deviceID    );
        LE_ASSUME( pNowTime    );
        LE_ASSUME( pInputData  );
        LE_ASSUME( pInputTime  );
        LE_ASSUME( pOutputData );
        LE_ASSUME( pOutputTime );
        LE_ASSUME( pContext    );

        auto & __restrict__ device( *static_cast<DeviceImpl *>( pContext ) );
        device.inputData_.aliasFrom( *pInputData );
        return kAudioHardwareNoError;
    }

    template <ChannelLayout channelLayout, InputOutputLayout ioLayout>
    LE_HOT
    static OSStatus renderProcedure
    (
        ::AudioDeviceID                        const deviceID   ,
        ::AudioTimeStamp  const * __restrict__ const pNowTime   ,
        ::AudioBufferList const * __restrict__       pInputData ,
        ::AudioTimeStamp  const * __restrict__ const pInputTime ,
        ::AudioBufferList       * __restrict__ const pOutputData,
        ::AudioTimeStamp  const * __restrict__ const pOutputTime,
        void                    * __restrict__ const pContext
    )
    {
        LE_ASSUME( deviceID    );
        LE_ASSUME( pNowTime    );
        LE_ASSUME( pInputData  );
        LE_ASSUME( pInputTime  );
        LE_ASSUME( pOutputData );
        LE_ASSUME( pOutputTime );
        LE_ASSUME( pContext    );

        auto constexpr inputOnly ( ioLayout == InputOutputLayout::InputOnly  );
        auto constexpr outputOnly( ioLayout == InputOutputLayout::OutputOnly );

        auto const & __restrict__ device    ( *static_cast<DeviceImpl const *>( pContext ) );
        auto const & __restrict__ callback  ( device.callback_ );
        auto       & __restrict__ bufferList( inputOnly ? device.input_.workBuffer( const_cast<AudioBufferList &>( *pInputData ) ) : device.output_.workBuffer( *pOutputData ) );

        auto const sampleFrames( device.frameSize() );
        LE_ASSUME( sampleFrames == static_cast<std::uint16_t>( bufferList.mBuffers[ 0 ].mDataByteSize / bufferList.mBuffers[ 0 ].mNumberChannels / sizeof( float ) ) );

        if ( !inputOnly && !outputOnly )
        {
            bool const synchronousInput( pInputData->mNumberBuffers != 0 );
            BOOST_ASSERT( synchronousInput == device.singleIODevice() );

            if ( !synchronousInput )
            {
                /// \note If we are dealing with separate input and output
                /// devices, CoreAudio will start a separate thread for input
                /// and output (even if they are actually part of the same
                /// physical device). As a precaution, we check here if the
                /// output thread has by some chance been started earlier (i.e.
                /// there is no input data yet).
                ///                               (16.09.2015.) (Domagoj Saric)
                while
                ( BOOST_UNLIKELY(
                    device.inputData_->mBuffers[ 0 ].mDataByteSize == 0 &&
                    device.input_.object.isRunning()
                ))
                    //return kAudioHardwareUnspecifiedError;
                    BOOST_VERIFY( sched_yield() == 0 );

                pInputData = device.inputData_;
            }

            //...mrmlj...redundant copy...convert directly to target...
            AudioBuffers::copy
            (
                device.input_.convertFrom( *pInputData, sampleFrames ),
                bufferList
            );
        }

        if ( channelLayout == ChannelLayout::SeparatedChannels )
        {
            using sample_t = typename std::conditional<inputOnly, float const, float>::type;

            std::uint8_t const numberOfChannels( bufferList.mNumberBuffers );
            sample_t * separatedChannels[ numberOfChannels ];
            for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
            {
                BOOST_ASSERT_MSG( bufferList.mBuffers[ channel ].mNumberChannels == 1, "Unexpected interleaved data" );
                separatedChannels[ channel ] = static_cast<sample_t *>( bufferList.mBuffers[ channel ].mData );
            }

            using Buffer = Device::Audio<ChannelLayout::SeparatedChannels, ioLayout>;

            if ( !outputOnly ) { LE_AUDIOIO_CRIPPLE( const_cast<float * const *>( separatedChannels ), sampleFrames ); }
            invoke( callback, Buffer{ separatedChannels, sampleFrames } );
            if ( !inputOnly  ) { LE_AUDIOIO_CRIPPLE( const_cast<float * const *>( separatedChannels ), sampleFrames ); }
        }
        else
        {
            LE_ASSUME( bufferList.mNumberBuffers == 1 );
            auto const pData( static_cast<float *>( bufferList.mBuffers[ 0 ].mData ) );

            using Buffer = Device::Audio<ChannelLayout::InterleavedChannels, ioLayout>;
            if ( !outputOnly ) { LE_AUDIOIO_CRIPPLE( pData, sampleFrames ); }
            invoke( callback, Buffer{ pData, sampleFrames } );
            if ( !inputOnly  ) { LE_AUDIOIO_CRIPPLE( pData, sampleFrames ); }
        }
        if ( !inputOnly )
            device.output_.convertTo( bufferList, *pOutputData, sampleFrames );

        return kAudioHardwareNoError;
    }

    LE_COLD void uninitialise()
    {
        input_ .uninitialise();
        output_.uninitialise();
    }

private:
    // AUHAL vs HAL
    // http://comments.gmane.org/gmane.comp.audio.portaudio.devel/1281
    // https://github.com/eddieringle/portaudio/blob/master/src/hostapi/coreaudio/notes.txt
    struct AudioDevice
    {
        ~AudioDevice() { uninitialise(); object = AudioObject(); }

        LE_COLD bool LE_FASTCALL initialise( DeviceImpl & context, AudioDeviceIOProc const callback )  LE_NOTHROW
        {
            BOOST_ASSERT( callback );
            BOOST_ASSERT( renderProc == nullptr );

            //AudioDeviceCreateIOProcIDWithBlock AudioDeviceIOBlock
            return BOOST_LIKELY( LE_AIO_TRACE( ::AudioDeviceCreateIOProcID( object, callback, &context, &renderProc ) ) == noErr );
        }

        LE_COLD void LE_FASTCALL uninitialise() LE_NOTHROW
        {
            stop();
            BOOST_VERIFY( ::AudioDeviceDestroyIOProcID( object, renderProc ) == noErr || !renderProc );
            renderProc = nullptr;
        }


        // AudioDeviceStop() and AudioOutputUnitStop() behaviour
        // http://lists.apple.com/archives/coreaudio-api/2005/Dec/msg00055.html
        LE_COLD void start() const { if ( renderProc ) BOOST_VERIFY( LE_AIO_TRACE( ::AudioDeviceStart( object, renderProc ) ) == noErr ); }
        LE_COLD void stop () const {                   BOOST_VERIFY( LE_AIO_TRACE( ::AudioDeviceStop ( object, renderProc ) ) == noErr ); }


        char const * LE_FASTCALL setFormat
        (
            ::AudioStreamBasicDescription const & __restrict__       format,
            ::AudioObjectPropertyScope                         const scope ,
            std::uint16_t                                      const frameSize
        ) LE_NOTHROW LE_COLD
        {
            // http://lists.apple.com/archives/coreaudio-api/2010/Aug/msg00060.html Device sample rates and stream virtual formats
            // kAudioDevicePropertyActualSampleRate
            // kAudioDevicePropertyHogMode
            LE_AIO_TRACE( object.setProperty<Float64>( kAudioDevicePropertyNominalSampleRate, format.mSampleRate ) );

            ::AudioStreamBasicDescription availableFormat( format );

        #ifndef NDEBUG
            doTrace = false;
        #endif // NDEBUG
            LE_AIO_TRACE( object.setProperty<Float64>( kAudioDevicePropertyActualSampleRate,        format.mSampleRate ) );
            LE_AIO_TRACE( object.setProperty         ( kAudioStreamPropertyPhysicalFormat  , scope, availableFormat    ) );

            if ( object.setProperty( kAudioStreamPropertyVirtualFormat, scope, availableFormat ) != noErr )
            {
                {
                    AudioObjectPropertyAddress const propertyAddress = { kAudioDevicePropertyStreamFormatMatch, scope, kAudioObjectPropertyElementMaster };
                    UInt32 size( sizeof( availableFormat ) );
                    BOOST_VERIFY( LE_AIO_TRACE( ::AudioObjectGetPropertyData( object, &propertyAddress, 0, nullptr, &size, &availableFormat ) ) == noErr );
                }
                //...mrmlj...as mentioned in the kAudioDevicePropertyStreamFormatMatch property documentation, it does not work that well (it returns 96kHz and stereo for 44kHz mono)
            #if 0
                bool const interleaved( !( availableFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ) );
                availableFormat.mChannelsPerFrame = format.mChannelsPerFrame;
                availableFormat.mBytesPerFrame    = ( availableFormat.mBitsPerChannel / 8 ) * ( interleaved ? availableFormat.mChannelsPerFrame : 1 );
                availableFormat.mBytesPerPacket   = availableFormat.mBytesPerFrame;
                availableFormat.mFramesPerPacket  = 1;
            #endif
                availableFormat.mSampleRate       = format.mSampleRate;

                if ( object.setProperty( kAudioStreamPropertyVirtualFormat, scope, availableFormat ) != noErr )
                    availableFormat = object.getProperty<::AudioStreamBasicDescription>( kAudioDevicePropertyStreamFormat );

                static_assert( kAudioDevicePropertyStreamFormat == kAudioStreamPropertyVirtualFormat );
            }

            if ( object.setProperty( kAudioStreamPropertyPhysicalFormat, scope, availableFormat ) != noErr ) { LE_TRACE_IF( object && /*...mrmlj...*/ false, "Audio hardware emulates requested format." ); }

        #ifndef NDEBUG
            doTrace = true;
        #endif // NDEBUG
            if ( object.setProperty<UInt32>( kAudioDevicePropertyBufferFrameSize, frameSize ) != noErr ) return "Out of memory.";

            if ( std::memcmp( &format, &availableFormat, sizeof( format ) ) != 0 )
            {
                //LE_TRACE( "Audio format conversion required." );
                LE_TRACE_IF( format.mSampleRate != availableFormat.mSampleRate, "Audio sampling rate conversion required." );
                auto const * pSourceFormat( &availableFormat );
                auto const * pTargetFormat( &format          );
                if ( scope == kAudioDevicePropertyScopeOutput )
                    std::swap( pSourceFormat, pTargetFormat );
                AudioConverterRef converter;
                auto const result( LE_AIO_TRACE( ::AudioConverterNew( pSourceFormat, pTargetFormat, &converter ) ) );
                if ( result != noErr )
                {
                    BOOST_ASSERT( result == memFullErr );
                    return "Out of memory.";
                }
                bool const interleaved( !( format.mFormatFlags & kAudioFormatFlagIsNonInterleaved ) );
                if ( !converterData_.resize( static_cast<std::uint8_t>( format.mChannelsPerFrame ), frameSize, interleaved ) )
                    return "Out of memory.";
                pConverter.reset( converter );
            }
            return nullptr;
        }

        char const * LE_FASTCALL setInterleaved( bool const enabled, ::AudioObjectPropertyScope const scope ) LE_NOTHROW LE_COLD
        {
            auto format( object.getProperty<::AudioStreamBasicDescription>( kAudioDevicePropertyStreamFormat, scope ) );
            BOOST_ASSERT(  format.mFormatID         = kAudioFormatLinearPCM );
            BOOST_ASSERT(  format.mBitsPerChannel   = sizeof( float ) * 8   );
            BOOST_ASSERT(  format.mFramesPerPacket  = 1                     );
            if ( enabled ) format.mFormatFlags     &= ~kAudioFormatFlagIsNonInterleaved;
            else           format.mFormatFlags     |=  kAudioFormatFlagIsNonInterleaved;
            format.mBytesPerFrame  = sizeof( float ) * ( enabled ? format.mChannelsPerFrame : 1 );
            format.mBytesPerPacket = format.mBytesPerFrame;
            return setFormat( format, scope, object.frameSize( scope ) );
        }

        explicit operator bool() const { return object; }

        AudioObject           object;
        ::AudioDeviceIOProcID renderProc = nullptr;

        // Format conversion...cleanup/rethink this through...
        ::AudioBufferList /*const*/ & workBuffer( ::AudioBufferList /*const*/ & data ) const
        {
            if ( !pConverter )
                return data;
            return converterData_;
        }

        ::AudioBufferList const & convertFrom( ::AudioBufferList const & data, std::uint16_t const sampleFrames ) const
        {
            if ( !pConverter )
                return data;

            BOOST_VERIFY( LE_AIO_TRACE
            (
                ::AudioConverterConvertComplexBuffer( pConverter.get(), sampleFrames, &data, converterData_ )
            ) == noErr );
            return converterData_;
        }

        void convertTo( ::AudioBufferList const & source, ::AudioBufferList & target, std::uint16_t const sampleFrames ) const
        {
            if ( &source == &target )
                return;
            BOOST_ASSERT( &source == &converterData_.bufferList() );

            BOOST_VERIFY( LE_AIO_TRACE
            (
                ::AudioConverterConvertComplexBuffer( pConverter.get(), sampleFrames, &source, &target )
            ) == noErr );
        }

        struct AudioConverterDeleter { void operator()( AudioConverterRef const pAudioConverter ) const noexcept { BOOST_VERIFY( ::AudioConverterDispose( pAudioConverter ) == noErr ); } };
        using ConverterPtr = std::unique_ptr<::OpaqueAudioConverter, AudioConverterDeleter>;

        ConverterPtr pConverter;
        AudioBuffers converterData_;
    }; // AudioDevice

    AudioDevice    input_    ;
    AudioDevice    output_   ;
    PublicCallback callback_ ;
    AudioBuffers   inputData_;

    std::uint8_t   numberOfChannels_ = 0;
    std::uint16_t  frameSize_        = 0;
    std::uint32_t  sampleRate_       = 0;
}; // class DeviceImpl


LE_COLD
char const * DeviceImpl::setup( std::uint8_t const numberOfChannels, std::uint32_t const sampleRate, std::uint16_t /*const*/ frameSize )
{
    /// \note The fully 'low-level' approach would be to analyse all the
    /// available 'AudioStreams' and select the 'best/closest one' and then
    /// perform any further conversion necessary [e.g. (de)interleaving]. This
    /// is a rather complex task and unless it proves beneficial we go the
    /// easier route - try to set the desired audio formats and use the builtin
    /// CoreAudio AudioConverters if the device refuses our desired format.
    /// AUHAL vs HAL
    /// http://music.columbia.edu/pipermail/portaudio/2005-March/004488.html
    ///                                       (16.09.2015.) (Domagoj Saric)
#if 0
    std::uint8_t constexpr numberOfBuffers( 32 );
    struct LocalBufferList : ::AudioBufferList { ::AudioBuffer _[ numberOfBuffers - 1 ]; };

    // http://stackoverflow.com/questions/4575408/audioobjectgetpropertydata-to-get-a-list-of-input-devices
    auto const inputIOConfiguration ( inputDevice .getProperty<LocalBufferList>( kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeInput  ) );
    auto const outputIOConfiguration( outputDevice.getProperty<LocalBufferList>( kAudioDevicePropertyStreamConfiguration, kAudioDevicePropertyScopeOutput ) );

    auto const numberOfStreams( static_cast<std::uint8_t>( inputIOConfiguration.mNumberBuffers ) );
    for ( std::uint8_t stream( 0 ); stream < numberOfStreams; ++stream )
    {
        AudioBuffer const & buffer( inputIOConfiguration.mBuffers[ stream ] );
        .mNumberChannels;
    }
#endif
    //uninitialise();

    // https://developer.apple.com/library/mac/documentation/MusicAudio/Reference/CoreAudioDataTypesRef/#//apple_ref/c/tdef/AudioStreamBasicDescription
    ::AudioStreamBasicDescription format = { 0 };
    bool const interleaved( true );
    format.mFormatID         = kAudioFormatLinearPCM;
    format.mFormatFlags      = kAudioFormatFlagIsFloat | ( interleaved ? 0 : kAudioFormatFlagIsNonInterleaved ) | kAudioFormatFlagIsPacked;
    format.mBytesPerFrame    = sizeof( float ) * ( interleaved ? numberOfChannels : 1 );
    format.mBitsPerChannel   = sizeof( float ) * 8;
    format.mSampleRate       = sampleRate; // http://stackoverflow.com/questions/4162556/ios-audio-units-setting-arbitrary-sample-rate
    format.mChannelsPerFrame = numberOfChannels;
    format.mBytesPerPacket   = format.mBytesPerFrame;
    format.mFramesPerPacket  = 1;
    format.mReserved         = 0;

    auto const inputBufferRange ( input_ .object.getProperty<AudioValueRange>( kAudioDevicePropertyBufferFrameSizeRange ) );
    auto const outputBufferRange( output_.object.getProperty<AudioValueRange>( kAudioDevicePropertyBufferFrameSizeRange ) );
    frameSize = Math::clamp
    (
        frameSize,
        std::max( static_cast<std::uint16_t>( outputBufferRange.mMinimum ), static_cast<std::uint16_t>( inputBufferRange.mMinimum ) ),
        std::min( static_cast<std::uint16_t>( outputBufferRange.mMaximum ), static_cast<std::uint16_t>( inputBufferRange.mMaximum ) )
    );
    /// \note CoreAudio allows for very short frame sizes, which cause audio to
    /// break so we choose some primitive-heuristic minimums.
    ///                                           (16.09.2015.) (Domagoj Saric)
#if defined( _DEBUG )
    frameSize = std::max<std::uint16_t>( 512, frameSize );
#elif !defined( NDEBUG )
    frameSize = std::max<std::uint16_t>( 256, frameSize );
#else
    frameSize = std::max<std::uint16_t>(  64, frameSize );
#endif // _DEBUG

    if ( auto err = input_ .setFormat( format, kAudioDevicePropertyScopeInput , frameSize ) ) { return err; }
    if ( auto err = output_.setFormat( format, kAudioDevicePropertyScopeOutput, frameSize ) ) { return err; }
    
    numberOfChannels_ = numberOfChannels;
    frameSize_        = frameSize       ;
    sampleRate_       = sampleRate      ;

    //::AudioObjectShow

    return nullptr;
}

LE_COLD
char const * DeviceImpl::setCallback
(
    AudioDeviceIOProc const callback,
    bool              const interleaved,
    bool              const inputEnabled,
    bool              const outputEnabled
)
{
    uninitialise();

    if ( inputEnabled  ) if ( auto err = input_ .setInterleaved( interleaved, kAudioDevicePropertyScopeInput  ) ) return err;
    if ( outputEnabled ) if ( auto err = output_.setInterleaved( interleaved, kAudioDevicePropertyScopeOutput ) ) return err;

    if ( inputEnabled  && !( singleIODevice() && outputEnabled ) && !input_ .initialise( *this, outputEnabled ? &inputRenderProcedure : callback ) ) return "Out of memory";
    if ( outputEnabled                                           && !output_.initialise( *this,                                         callback ) ) return "Out of memory";

    if ( inputEnabled && outputEnabled && !singleIODevice() )
    {
        auto const inputFormat( input_.object.getProperty<::AudioStreamBasicDescription>( kAudioDevicePropertyStreamFormat, kAudioDevicePropertyScopeInput ) );
        bool const separated  ( inputFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved );
        if ( !inputData_.resize( inputFormat.mChannelsPerFrame, 0, !separated ) )
            return "Out of memory.";
    }

    return nullptr;
}

template <typename DataLayout>
LE_COLD
char const * DeviceImpl::setCallback( void (^callback)( DataLayout ) )
{
    using CallbackInfo = CallbackInfo<DataLayout>;

    bool const interleaved( CallbackInfo::channelLayout == ChannelLayout::InterleavedChannels );

    bool const inputEnabled ( CallbackInfo::ioLayout != InputOutputLayout::OutputOnly );
    bool const outputEnabled( CallbackInfo::ioLayout != InputOutputLayout::InputOnly  );

    storeCallback<DataLayout>( callback_, callback );

    return setCallback
    (
        &renderProcedure<CallbackInfo::channelLayout, CallbackInfo::ioLayout>,
        interleaved,
        inputEnabled,
        outputEnabled
    );
}

namespace
{
    AudioBuffers:: AudioBuffers() : pIOBuffers_( nullptr ) {}
    AudioBuffers::~AudioBuffers() { std::free( pIOBuffers_ ); }

    bool AudioBuffers::resize( std::uint8_t const numberOfChannels, std::uint16_t const numberOfSamples, bool const interleaved )
    {
        LE_ASSUME( numberOfChannels );

        std::uint8_t const numberOfBuffers          ( interleaved ? 1 : numberOfChannels );
        std::uint8_t const numberOfChannelsPerBuffer( interleaved ? numberOfChannels : 1 );

        auto const structureRequiredStorage ( sizeof( *pIOBuffers_ ) + ( ( numberOfBuffers - 1 ) * sizeof( *pIOBuffers_->mBuffers ) ) );
        auto const structureAlignmentPadding( 16 - ( structureRequiredStorage % 16 )                                                  );
        auto const perBufferRequiredStorage ( numberOfSamples * sizeof( float ) * numberOfChannelsPerBuffer                           );
        auto const perBufferAlignedStorage  ( Utility::align( perBufferRequiredStorage )                                              );
        auto const requiredStorage
        (
            structureRequiredStorage
                +
            ( structureAlignmentPadding + numberOfChannels * perBufferAlignedStorage )
        );

        BOOST_ASSERT( requiredStorage != 0                         );
        BOOST_ASSERT( numberOfSamples || !perBufferAlignedStorage );

        auto const pBufferList( static_cast<AudioBufferListPtr>( std::realloc( pIOBuffers_, requiredStorage ) ) );
        if ( !pBufferList )
            return false;

        pIOBuffers_                 = pBufferList    ;
        pIOBuffers_->mNumberBuffers = numberOfBuffers;

        auto                const buffers       ( boost::make_iterator_range_n( &pIOBuffers_->mBuffers[ 0 ], numberOfBuffers ) );
        char * __restrict__       pBufferStorage( numberOfSamples ? reinterpret_cast<char *>( buffers.end() ) + structureAlignmentPadding : nullptr );

        for ( auto & __restrict__ buffer : buffers )
        {
            buffer.mNumberChannels = numberOfChannelsPerBuffer;
            buffer.mDataByteSize   = perBufferRequiredStorage ;
            buffer.mData           = pBufferStorage           ;
            LE_ASSUME( reinterpret_cast<std::size_t>( buffer.mData ) % 16 == 0 );
            pBufferStorage += perBufferAlignedStorage;
        }

        BOOST_ASSERT_MSG( ( pBufferStorage - reinterpret_cast<char const *>( pIOBuffers_ ) == requiredStorage ) || !numberOfSamples, "Buffer overrun." );

        return true;
    }


    void AudioBuffers::alias( ::AudioBufferList const & LE_RESTRICT source, ::AudioBufferList & LE_RESTRICT target )
    {
        BOOST_ASSERT_MSG( source.mNumberBuffers == target.mNumberBuffers, "Mismatched buffers." );
        std::memcpy
        (
            &target,
            &source,
            reinterpret_cast<char const *>( &source.mBuffers[ source.mNumberBuffers ] ) - reinterpret_cast<char const *>( &source )
        );
    }

    void AudioBuffers::aliasFrom( ::AudioBufferList const & source )
    {
        BOOST_ASSERT_MSG( pIOBuffers_, "Uninitialised buffer." );
        alias( source, *pIOBuffers_ );
    }

    void AudioBuffers::aliasTo( ::AudioBufferList & target ) const
    {
        BOOST_ASSERT_MSG( pIOBuffers_, "Uninitialised buffer." );
        alias( *pIOBuffers_, target );
    }

    void AudioBuffers::copyTo( ::AudioBufferList & target ) const
    {
        BOOST_ASSERT_MSG( pIOBuffers_, "Uninitialised buffer." );
        copy( *pIOBuffers_, target );
    }

    void AudioBuffers::copy( ::AudioBufferList const & LE_RESTRICT source, ::AudioBufferList & LE_RESTRICT target )
    {
        BOOST_ASSERT_MSG( source.mNumberBuffers == target.mNumberBuffers, "Mismatched buffers." );

        ::AudioBuffer const * LE_RESTRICT pSource     ( source.mBuffers );
        ::AudioBuffer       * LE_RESTRICT pDestination( target.mBuffers );

        unsigned int const channelSize     ( pSource->mDataByteSize );
        unsigned int       numberOfChannels( source.mNumberBuffers  );
        while ( numberOfChannels-- )
        {
            BOOST_ASSERT( pSource        != pDestination        );
            BOOST_ASSERT( pSource->mData != pDestination->mData );
            BOOST_ASSERT( pSource     ->mDataByteSize == channelSize );
            BOOST_ASSERT( pDestination->mDataByteSize == channelSize );
            BOOST_ASSERT( reinterpret_cast<std::size_t>( pDestination->mData ) % 16 == 0 );
            BOOST_ASSERT( reinterpret_cast<std::size_t>( pSource     ->mData ) % 16 == 0 );
            std::memcpy
            (
                static_cast<LE_ALIGN( 16 ) void       *>( pDestination++->mData ),
                static_cast<LE_ALIGN( 16 ) void const *>( pSource     ++->mData ),
                channelSize
            );
        }
    }
}

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "pimplForwarders.inl"
