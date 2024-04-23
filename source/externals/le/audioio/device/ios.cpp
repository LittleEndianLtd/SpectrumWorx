////////////////////////////////////////////////////////////////////////////////
///
/// iOS.cpp
/// -------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "device.hpp"
#include "deviceImpl.hpp"

#include "le/utility/countof.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"

#include "boost/assert.hpp"

#include "AvailabilityMacros.h"
#import  "AudioUnit/AudioUnit.h"
#import  "AudioToolbox/AudioToolbox.h"

#include <cctype>
#include <cstdint>
#include <cstdio>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

// OpenAL FAQ for iPhone OS https://developer.apple.com/library/IOS/technotes/tn2199/_index.html
// MoMu: A Mobile Music Toolkit http://momu.stanford.edu/toolkit
// RemoteIO AU https://developer.apple.com/library/ios/documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/UsingSpecificAudioUnits/UsingSpecificAudioUnits.html#//apple_ref/doc/uid/TP40009492-CH17-SW1
// http://www.slideshare.net/invalidname/core-audio-dont-be-afraid-to-play-it-loud-360idev-san-jose-2010
// http://stackoverflow.com/questions/4014614/ios-audio-units-vs-openal-vs-core-audio
// http://stackoverflow.com/questions/10141526/audiooutputunitstart-very-slow
// http://stackoverflow.com/questions/10113977/recording-to-aac-from-remoteio-data-is-getting-written-but-file-unplayable
// http://stackoverflow.com/questions/13263064/ios-remoteio-audiounitaddrendernotify-callback
// http://teragonaudio.com/article/How-to-do-realtime-recording-with-effect-processing-on-iOS.html
// http://www.subfurther.com/blog/2009/04/28/an-iphone-core-audio-brain-dump


namespace
{
    ::OSStatus trace( ::OSStatus const status, std::uint16_t const lineNumber )
    {
        BOOST_LIKELY( status == noErr );
    #ifndef NDEBUG
        if ( ( status != noErr ) || ( status != kAudioSessionNoError ) )
        {
        #if 0
            // http://stackoverflow.com/questions/2196869/how-do-you-convert-an-iphone-osstatus-code-to-something-useful
            // http://stackoverflow.com/questions/8118891/are-there-ios-equivalents-of-getmacosstatuserrorstring-and-getmacosstatuscomment
            // http://stackoverflow.com/questions/14699604/replacements-for-getmacosstatuserrorstring-getmacosstatuscommentstring
            Utility::Tracer::error
            (
                "Apple error: %d (%s), (%s) @ %hu.",
                status
                ::GetMacOSStatusCommentString( status ),
                ::GetMacOSStatusErrorString  ( status ),
                lineNumber
            );
        #else
            // see if it appears to be a 4-char-code
            std::uint32_t const fourCCInt( CFSwapInt32HostToBig( status ) );
            using FourCC = char const[ 4 ];
            auto const & fourCC( reinterpret_cast<FourCC const &>( fourCCInt ) );
            if ( std::isprint( fourCC[ 0 ] ) && std::isprint( fourCC[ 1 ] ) && std::isprint( fourCC[ 2 ] ) && std::isprint( fourCC[ 3 ] ) )
                Utility::Tracer::error( "Apple error: %d / %X - %.4s @ %d.", status, status, fourCC, lineNumber );
            else
                Utility::Tracer::error( "Apple error: %d / %X @ %d."       , status, status        , lineNumber );
        #endif
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

    template <typename T>
    ::OSStatus LE_COLD setProperty( ::AudioUnit const au, ::AudioUnitPropertyID const id, ::AudioUnitScope const scope, ::AudioUnitElement const element, T const & data )
    {
        return LE_AIO_TRACE( ::AudioUnitSetProperty( au, id, scope, element, &data, sizeof( data ) ) );
    }

    template <typename T>
    T LE_COLD getProperty( ::AudioUnit const au, ::AudioUnitPropertyID const id, ::AudioUnitScope const scope, ::AudioUnitElement const element )
    {
        T property;
        UInt32 size( sizeof( property ) );
        BOOST_VERIFY( LE_AIO_TRACE( ::AudioUnitGetProperty( au, id, scope, element, &property, &size ) ) == noErr );
        BOOST_ASSERT( size == sizeof( property ) );
        return property;
    }


    template <typename T>
    ::OSStatus LE_COLD setSessionProperty( ::AudioSessionPropertyID const id, T const & data )
    {
        return LE_AIO_TRACE( ::AudioSessionSetProperty( id, sizeof( data ), &data ) );
    }

    template <typename T>
    T LE_COLD getSessionProperty( ::AudioSessionPropertyID const id )
    {
        T property;
        UInt32 size( sizeof( property ) );
        BOOST_VERIFY( LE_AIO_TRACE( ::AudioSessionGetProperty( id, &size, &property ) ) == kAudioSessionNoError );
        BOOST_ASSERT( size == sizeof( property ) );
        return property;
    }
} // anonymous namespace

class DeviceImpl
{
public:
    LE_NOTHROWNOALIAS  DeviceImpl() : remoteIOAU_( nullptr ) {}
    LE_NOTHROWNOALIAS ~DeviceImpl()
    {
        stop          ();
        uninitialiseAU();
        BOOST_VERIFY( ::AudioComponentInstanceDispose( remoteIOAU_ ) == noErr || !remoteIOAU_ );
        /// \note Deactivating the audio session should be initiated by the app,
        /// however, until a suitable API is produced (clients request this
        /// functionality) we perform the deactivation here unconditionally.
        ///                                   (05.02.2016.) (Domagoj Saric)
        auto const sessionDeactivationResult( ::AudioSessionSetActive( false ) );
        BOOST_VERIFY( sessionDeactivationResult == kAudioSessionNoError || sessionDeactivationResult == kAudioSessionNotActiveError || sessionDeactivationResult == kAudioSessionNotInitialized );
    }

    char const * setup( std::uint8_t numberOfChannels, std::uint32_t sampleRate, std::uint16_t desiredLatencyInSamples );

    template <typename DataLayout>
    char const * setCallback( void (^callback)(DataLayout) );

    void LE_COLD start() const
    {
        /// \note The AudioSession API has been deprecated since iOS 7.0 and
        /// calling AudioSessionSetActive( true/false ) from the start()/stop()
        /// member functions has strange/random side effects (e.g. some sample
        /// files refusing to play/being silent).
        /// http://stackoverflow.com/questions/21464530/ios-deprecation-of-audiosessioninitialize-and-audiosessionsetproperty
        /// http://stackoverflow.com/questions/7922410/audiosessionsetactive-fails-after-interruption
        /// http://forum.unity3d.com/threads/setting-mediaplayback-audio-session-category.206958
        /// http://stackoverflow.com/questions/18807157/how-do-i-route-audio-to-speaker-without-using-audiosessionsetproperty
        /// https://github.com/muhku/FreeStreamer/pull/10/files
        ///                                   (18.11.2013.) (Domagoj Saric)
        BOOST_ASSERT( !isRunning() );
        BOOST_VERIFY( LE_AIO_TRACE( ::AudioSessionSetActive( true        ) ) == kAudioSessionNoError );
        BOOST_VERIFY( LE_AIO_TRACE( ::AudioOutputUnitStart ( remoteIOAU_ ) ) == noErr                );
        BOOST_ASSERT( isRunning() );
    }

    void LE_COLD stop() const
    {
        BOOST_VERIFY( ::AudioOutputUnitStop( remoteIOAU_ ) == noErr || !remoteIOAU_ );
        /// \note Verify ::AudioOutputUnitStop() blocking behaviour.
        /// http://music.columbia.edu/pipermail/portaudio/2007-June/007152.html
        /// http://lists.apple.com/archives/coreaudio-api/2005/Dec/msg00048.html
        /// http://stackoverflow.com/questions/19620337/ios-audiooutputunitstop-results-in-app-freeze-and-warning
        ///                                   (18.11.2013.) (Domagoj Saric)
        BOOST_ASSERT( !remoteIOAU_ || !isRunning() );
    }

    std::uint8_t  numberOfChannels() const { return getProperty<::AudioStreamBasicDescription>( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, kOutputBus ).mChannelsPerFrame; }
    std::uint32_t sampleRate      () const { return getProperty<::AudioStreamBasicDescription>( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, kOutputBus ).mSampleRate      ; }

    Device::LatencyAndBufferSize LE_COLD latency() const
    {
        // http://www.mailinglistarchive.com/html/coreaudio-api@lists.apple.com/2011-05/msg00057.html

        auto const sessionCategory( getSessionProperty<UInt32>( kAudioSessionProperty_AudioCategory )                                              );
        bool const inputOnly      ( sessionCategory == kAudioSessionCategory_RecordAudio                                                           );
        bool const outputOnly     ( sessionCategory != kAudioSessionCategory_RecordAudio && sessionCategory != kAudioSessionCategory_PlayAndRecord );

        auto const sampleRate    (                  getSessionProperty<Float64>( kAudioSessionProperty_CurrentHardwareSampleRate       ) );
        auto const bufferDuration(                  getSessionProperty<Float32>( kAudioSessionProperty_CurrentHardwareIOBufferDuration ) );
        auto const inputLatency  ( outputOnly ? 0 : getSessionProperty<Float32>( kAudioSessionProperty_CurrentHardwareInputLatency     ) );
        auto const outputLatency ( inputOnly  ? 0 : getSessionProperty<Float32>( kAudioSessionProperty_CurrentHardwareOutputLatency    ) );
        return Device::LatencyAndBufferSize
        (
            static_cast<Device::LatencyAndBufferSize::first_type >( ( inputLatency + outputLatency ) * sampleRate ),
            static_cast<Device::LatencyAndBufferSize::second_type>( ( bufferDuration               ) * sampleRate )
        );
    }

    bool isRunning() const
    {
        return
            getProperty<UInt32>( remoteIOAU_, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, kOutputBus ) ||
            getProperty<UInt32>( remoteIOAU_, kAudioOutputUnitProperty_IsRunning, kAudioUnitScope_Global, kInputBus  );
    }

private:
    // https://developer.apple.com/library/ios/documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/UsingSpecificAudioUnits/UsingSpecificAudioUnits.html#//apple_ref/doc/uid/TP40009492-CH17-SW1
    static UInt32 const kInputBus  = 1;
    static UInt32 const kOutputBus = 0;

    char const * setCallback
    (
        AURenderCallback    callback,
        AudioUnitPropertyID callbackProperty,
        UInt32              audioSessionCategory,
        bool                interleaved,
        bool                inputEnabled,
        bool                outputEnabled
    );

    static void remoteIOInterruptionListener( void * pContext, UInt32 interruptionReason );

    bool initialiseAudioSession() const
    {
        static bool initialised;
        if ( BOOST_UNLIKELY( !initialised ) )
        {
            auto const result
            (
                LE_AIO_TRACE( ::AudioSessionInitialize( nullptr, nullptr, &remoteIOInterruptionListener, remoteIOAU_ ) )
            );
            initialised = ( result == kAudioSessionNoError );
        }
        return initialised;
    }

    template <ChannelLayout channelLayout, InputOutputLayout ioLayout>
    LE_HOT
    static OSStatus renderProcedure
    (
        void                               *             const pContext,
        ::AudioUnitRenderActionFlags       * LE_RESTRICT const pActionFlags,
        ::AudioTimeStamp             const * LE_RESTRICT const pTimeStamp,
        ::UInt32                                         const busNumber,
        ::UInt32                                         const numberFrames,
        ::AudioBufferList                  * LE_RESTRICT       pIOData
    )
    {
        BOOST_ASSERT( pTimeStamp   );
        BOOST_ASSERT( pActionFlags );
        BOOST_ASSERT( *pActionFlags == 0 );

        LE_ASSUME( pContext != nullptr );

        auto const & LE_RESTRICT device( *static_cast<DeviceImpl const *>( pContext ) );

        /// \note Clang seems not to align dynamic alloca allocations (currently
        /// using a fixed size struct because there is no truly efficient way to
        /// fetch the number of channels here).
        /// https://code.google.com/p/nativeclient/issues/detail?id=3795
        ///                                   (05.06.2014.) (Domagoj Saric)
        std::uint8_t constexpr maximumNumberOfChannels( 8 ); //...mrmlj...
        std::uint8_t constexpr numberOfBuffers( ( channelLayout == Device::SeparatedChannels ) ? maximumNumberOfChannels : 1 );
        struct LocalBufferList : AudioBufferList { ::AudioBuffer _[ numberOfBuffers - 1 ]; };
        LocalBufferList localBufferList;

        if ( ioLayout != InputOutputLayout::OutputOnly )
        {
            if ( ioLayout == InputOutputLayout::InputOnly )
            {
                LE_ASSUME( busNumber == kInputBus );
                LE_ASSUME( pIOData   == nullptr   );
                *pActionFlags = kAudioUnitRenderAction_DoNotCheckRenderArgs;
                localBufferList.mNumberBuffers = numberOfBuffers;
                for ( std::uint8_t buffer( 0 ); buffer < numberOfBuffers; ++buffer )
                    localBufferList.mBuffers[ buffer ].mData = nullptr;
                pIOData = &localBufferList;
            }
            else
            {
                LE_ASSUME( pIOData != nullptr );
            }
            BOOST_VERIFY( LE_AIO_TRACE( ::AudioUnitRender( device.remoteIOAU_, pActionFlags, pTimeStamp, kInputBus, numberFrames, pIOData ) ) == noErr );
            BOOST_ASSERT( pIOData->mNumberBuffers <= numberOfBuffers );
        }
    #ifndef NDEBUG
        BOOST_ASSERT( pIOData != nullptr );
        if ( channelLayout == Device::SeparatedChannels )
        {
            BOOST_ASSERT( pIOData->mNumberBuffers > 0 && pIOData->mNumberBuffers <= 8 );
            for ( std::uint8_t channel( 0 ); channel < pIOData->mNumberBuffers; ++channel )
            {
                BOOST_ASSERT_MSG( pIOData->mBuffers[ channel ].mNumberChannels == 1, "Unexpected interleaved data"  );
                BOOST_ASSERT_MSG( pIOData->mBuffers[ channel ].mData           != 0, "Unexpected null input buffer" );
            }
        }
        else
        {
            BOOST_ASSERT_MSG( pIOData->mNumberBuffers      == 1, "Unexpected separated data"    );
            BOOST_ASSERT_MSG( pIOData->mBuffers[ 0 ].mData != 0, "Unexpected null input buffer" );
        }
    #endif // NDEBUG

        auto const &       callback    ( device.callback_ );
        auto         const sampleFrames( static_cast<std::uint16_t>( numberFrames ) );
        if ( channelLayout == ChannelLayout::SeparatedChannels )
        {
            std::uint8_t const numberOfChannels( pIOData->mNumberBuffers );
            using sample_t = typename std::conditional<( ioLayout == InputOutputLayout::InputOnly ), float const, float>::type;
            sample_t * separatedChannels[ maximumNumberOfChannels ];
            LE_ASSUME( numberOfChannels <= _countof( separatedChannels ) );
            for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
            {
                BOOST_ASSERT_MSG( pIOData->mBuffers[ channel ].mNumberChannels == 1, "Unexpected interleaved data" );
                separatedChannels[ channel ] = static_cast<sample_t *>( pIOData->mBuffers[ channel ].mData );
            }

            using Buffer = Device::Audio<ChannelLayout::SeparatedChannels, ioLayout>;

            if ( ioLayout != InputOutputLayout::OutputOnly ) { LE_AUDIOIO_CRIPPLE( const_cast<float * const *>( separatedChannels ), sampleFrames ); }
            invoke( callback, Buffer{ separatedChannels, sampleFrames } );
            if ( ioLayout != InputOutputLayout::InputOnly  ) { LE_AUDIOIO_CRIPPLE( const_cast<float * const *>( separatedChannels ), sampleFrames ); }
        }
        else
        {
            LE_ASSUME( pIOData->mNumberBuffers == 1 );
            auto const pData( static_cast<float *>( pIOData->mBuffers[ 0 ].mData ) );

            using Buffer = Device::Audio<ChannelLayout::InterleavedChannels, ioLayout>;
            if ( ioLayout != InputOutputLayout::OutputOnly ) { LE_AUDIOIO_CRIPPLE( pData, sampleFrames ); }
            invoke( callback, Buffer{ pData, sampleFrames } );
            if ( ioLayout != InputOutputLayout::InputOnly  ) { LE_AUDIOIO_CRIPPLE( pData, sampleFrames ); }
        }

        return noErr;
    }

    LE_COLD void uninitialiseAU() { BOOST_VERIFY( ::AudioUnitUninitialize( remoteIOAU_ ) == noErr || !remoteIOAU_ ); }

private:
    ::AudioUnit    remoteIOAU_;
    PublicCallback callback_;
}; // class DeviceImpl


void DeviceImpl::remoteIOInterruptionListener( void * const pContext, UInt32 const interruptionReason )
{
    ::AudioUnit const remoteIOAU( static_cast<::AudioUnit>( pContext ) );

    switch ( interruptionReason )
    {
        case kAudioSessionEndInterruption:
        {
            LE_TRACE( "AudioSession EndInterruption" );
            // make sure we are again the active session
            BOOST_VERIFY( ::AudioSessionSetActive( true       ) == noErr );
            BOOST_VERIFY( ::AudioOutputUnitStart ( remoteIOAU ) == noErr );
            break;
        }

        case kAudioSessionBeginInterruption:
            LE_TRACE( "AudioSession BeginInterruption" );
            BOOST_VERIFY( ::AudioOutputUnitStop( remoteIOAU ) == noErr );
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

LE_COLD
char const * DeviceImpl::setup( std::uint8_t const numberOfChannels, std::uint32_t const sampleRate, std::uint16_t const frameSize )
{
    // http://momu.stanford.edu/toolkit
    // http://atastypixel.com/blog/using-remoteio-audio-unit
    // http://www.appcoda.com/ios-avfoundation-framework-tutorial
    // http://arivibes.com/realtime-audio-on-ios-tutorial-making-a-mandolin
    // http://stackoverflow.com/questions/7184341/ios-sample-code-for-simultaneous-record-and-playback
    // http://developer.apple.com/library/ios/#samplecode/aurioTouch2/Introduction/Intro.html
    // http://code.google.com/p/lowlatencyaudio
    // http://kowalski.googlecode.com/svn-history/r5/trunk/src/engine/hosts/iphone/kwl_soundengine_iphone.m
    // https://groups.google.com/a/chromium.org/forum/#!msg/chromium-reviews/S9Et4PxXw08/dXXFLRdlkC8J

    // http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/CoreAudioOverview/Introduction/Introduction.html
    // http://developer.apple.com/library/ios/#documentation/AudioToolbox/Reference/AudioSessionServicesReference/Reference/reference.html
    // http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/Introduction/Introduction.html
    // http://developer.apple.com/library/mac/#technotes/tn2091/_index.html

    ////////////////////////////////////////////////////////////////////////////
    // AU
    ////////////////////////////////////////////////////////////////////////////

    auto const ioAU( kAudioUnitSubType_RemoteIO /*kAudioUnitSubType_VoiceProcessingIO*/ );
    if ( !remoteIOAU_ )
    {
        ::AudioComponentDescription remoteIODescription;
        remoteIODescription.componentType         = kAudioUnitType_Output;
        remoteIODescription.componentSubType      = ioAU;
        remoteIODescription.componentManufacturer = kAudioUnitManufacturer_Apple;
        remoteIODescription.componentFlags        = 0;
        remoteIODescription.componentFlagsMask    = 0;

        ::AudioComponent const remoteIOComponent( ::AudioComponentFindNext( nullptr, &remoteIODescription ) );
        BOOST_ASSERT_MSG( remoteIOComponent, "RemoteIO AU not found!?" );

        if ( LE_AIO_TRACE( ::AudioComponentInstanceNew( remoteIOComponent, &remoteIOAU_ ) ) != noErr ) return "Couldn't open the remote I/O unit";
    }
    else
    {
        uninitialiseAU();
    }

    ::AudioStreamBasicDescription format;
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
    if ( setProperty( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input , kOutputBus, format ) != noErr ) return "Error setting desired stream format";
    if ( setProperty( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, kInputBus , format ) != noErr ) return "Error setting desired stream format";

    if ( ioAU == kAudioUnitSubType_VoiceProcessingIO )
    {
        BOOST_VERIFY( setProperty<UInt32>( remoteIOAU_, kAUVoiceIOProperty_BypassVoiceProcessing   , kAudioUnitScope_Global, kInputBus, true  ) == noErr );
        BOOST_VERIFY( setProperty<UInt32>( remoteIOAU_, kAUVoiceIOProperty_VoiceProcessingEnableAGC, kAudioUnitScope_Global, kInputBus, false ) == noErr );
    }


    ////////////////////////////////////////////////////////////////////////////
    // Audio session
    ////////////////////////////////////////////////////////////////////////////

    //...mrmlj...remoteIOInterruptionListener handler disabled until AudioSessionSetActive problems are resolved...
    if ( !initialiseAudioSession() ) return "Out of memory";

    // http://www.techotopia.com/index.php/Detecting_when_an_iPhone_Headphone_or_Docking_Connector_is_Unplugged_(iOS_4)
    //if ( !check( ::AudioSessionAddPropertyListener( kAudioSessionProperty_AudioRouteChange, propListener, nullptr ), "Failed to set 'audio route change' listener." ) return false;

    BOOST_VERIFY( setSessionProperty<Float64>( kAudioSessionProperty_PreferredHardwareSampleRate, sampleRate ) == kAudioSessionNoError );
    LE_TRACE_IF ( getSessionProperty<Float64>( kAudioSessionProperty_CurrentHardwareSampleRate ) != sampleRate, "Requested sample rate not supported by hardware, resampling will occur." );

    auto const bufferLengthInSeconds( static_cast<Float32>( frameSize ) / sampleRate );
    if
    (
        ( ioAU == kAudioUnitSubType_RemoteIO ) &&
        ( setSessionProperty( kAudioSessionProperty_PreferredHardwareIOBufferDuration, bufferLengthInSeconds ) != kAudioSessionNoError )
    ) return "Failed to set the desired frame length.";

    // http://stackoverflow.com/questions/6682892/turning-off-agc-on-the-ipad
    //UInt32 const sessionMode( kAudioSessionMode_Measurement );
    //if ( !check( ::AudioSessionSetProperty( kAudioSessionProperty_Mode           , sizeof( sessionMode ), &sessionMode ), "Failed to set desired session mode." ) ) return false;
    //Float32 const gain( 1.0f );
    //if ( !check( ::AudioSessionSetProperty( kAudioSessionProperty_InputGainScalar, sizeof( gain        ), &gain        ), "Failed to set desired session gain." ) ) return false;
    //UInt32 inputGainSupported;
    //UInt32 size( sizeof( inputGainSupported ) );
    //BOOST_VERIFY( ::AudioSessionGetProperty( kAudioSessionProperty_InputGainAvailable, &size, &inputGainSupported ) == noErr );
    //if ( !inputGainSupported )
    //    std::fputs( "Input gain not supported.\n", stderr );

#if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
    {
        // http://developer.apple.com/library/ios/documentation/AudioToolbox/Reference/AudioSessionServicesReference/Reference/reference.html
        auto const sessionCategory( getSessionProperty<UInt32>( kAudioSessionProperty_AudioCategory )                                              );
        bool const inputOnly      ( sessionCategory == kAudioSessionCategory_RecordAudio                                                           );
        bool const outputOnly     ( sessionCategory != kAudioSessionCategory_RecordAudio && sessionCategory != kAudioSessionCategory_PlayAndRecord );

        BOOST_ASSERT( outputOnly || getSessionProperty<UInt32>( kAudioSessionProperty_AudioInputAvailable ) );

        // http://lists.apple.com/archives/coreaudio-api/2010/May/msg00281.html
        if ( !inputOnly )
        {
             auto const actualOutputLatency( getSessionProperty<Float32>( kAudioSessionProperty_CurrentHardwareOutputLatency ) );
             LE_TRACE_IF
             (
                 actualOutputLatency > bufferLengthInSeconds,
                 "Failed to set the desired latency"
              );
        }
        if ( !outputOnly )
        {
            auto const actualInputLatency( getSessionProperty<Float32>( kAudioSessionProperty_CurrentHardwareInputLatency    ) );
            LE_TRACE_IF
            (
                actualInputLatency > bufferLengthInSeconds,
                "Failed to set the desired latency"
            );
        }
    }
#endif // NDEBUG

    return nullptr;
}

LE_COLD
char const * DeviceImpl::setCallback
(
    AURenderCallback    const callback,
    AudioUnitPropertyID const callbackProperty,
    UInt32              const audioSessionCategory,
    bool                const interleaved,
    bool                const inputEnabled,
    bool                const outputEnabled
)
{
    BOOST_ASSERT( callback );

    uninitialiseAU();

    ::AudioStreamBasicDescription format( getProperty<::AudioStreamBasicDescription>( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, kOutputBus ) );
    BOOST_ASSERT( format.mFormatID         = kAudioFormatLinearPCM );
    BOOST_ASSERT( format.mBitsPerChannel   = sizeof( float ) * 8   );
    BOOST_ASSERT( format.mFramesPerPacket  = 1                     );
    if ( interleaved ) format.mFormatFlags &= ~kAudioFormatFlagIsNonInterleaved;
    else               format.mFormatFlags |=  kAudioFormatFlagIsNonInterleaved;
    format.mBytesPerFrame  = sizeof( float ) * ( interleaved ? format.mChannelsPerFrame : 1 );
    format.mBytesPerPacket = format.mBytesPerFrame;

    //...mrmlj...output enabled by default...verify...
    // http://developer.apple.com/library/ios/#documentation/AudioUnit/Reference/AudioUnitPropertiesReference/Reference/reference.html
    if ( setProperty<UInt32>( remoteIOAU_, kAudioOutputUnitProperty_EnableIO      , kAudioUnitScope_Input , kInputBus , inputEnabled   ) != noErr ) return "Error setting desired stream format";
    if ( setProperty<UInt32>( remoteIOAU_, kAudioOutputUnitProperty_EnableIO      , kAudioUnitScope_Output, kOutputBus, outputEnabled  ) != noErr ) return "Error setting desired stream format";

    if ( setProperty<UInt32>( remoteIOAU_, kAudioUnitProperty_ShouldAllocateBuffer, kAudioUnitScope_Input , kOutputBus,  outputEnabled ) != noErr ) return "Out of memory";
    if ( setProperty<UInt32>( remoteIOAU_, kAudioUnitProperty_ShouldAllocateBuffer, kAudioUnitScope_Output, kInputBus , !outputEnabled ) != noErr ) return "Out of memory";
  //if ( setProperty<UInt32>( remoteIOAU_, kAudioUnitProperty_InPlaceProcessing   , kAudioUnitScope_Global, kOutputBus, true           ) != noErr ) return "Couldn't enable in-place operation"; // ... kAudioUnitErr_InvalidProperty

    if ( inputEnabled  && ( setProperty( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input , kOutputBus, format ) != noErr ) ) return "Error setting desired stream format";
    if ( outputEnabled && ( setProperty( remoteIOAU_, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, kInputBus , format ) != noErr ) ) return "Error setting desired stream format";

    // http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/ConstructingAudioUnitApps/ConstructingAudioUnitApps.html
    ::AURenderCallbackStruct renderProc;
    renderProc.inputProc       = callback;
    renderProc.inputProcRefCon = this;
    BOOST_VERIFY( setProperty( remoteIOAU_, callbackProperty, kAudioUnitScope_Input, kOutputBus, renderProc ) == noErr );

    /// \note Changing the audio session category silently fails on active
    /// sessions.
    ///                                       (01.08.2014.) (Domagoj Saric)
    // https://developer.apple.com/library/ios/qa/qa1754/_index.html
    // http://stackoverflow.com/questions/2660836/avaudioplayer-via-speakers
    // http://stackoverflow.com/questions/18807157/how-do-i-route-audio-to-speaker-without-using-audiosessionsetproperty
    BOOST_ASSERT( !isRunning() );
    {
        auto const result( ::AudioSessionSetActive( false ) );
        if ( result == '!act' ) LE_TRACE( "AudioSession deactivated outside LE.AudioIO" );
        else                    BOOST_ASSERT( result == kAudioSessionNoError );
    }
    BOOST_VERIFY( setSessionProperty( kAudioSessionProperty_AudioCategory, audioSessionCategory ) == noErr );
    BOOST_VERIFY( LE_AIO_TRACE( ::AudioSessionSetActive( true ) ) == kAudioSessionNoError );

    if ( LE_AIO_TRACE( ::AudioUnitInitialize( remoteIOAU_ ) ) != noErr ) return "Out of memory";

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

    // http://stackoverflow.com/questions/13608789/attaching-a-callback-to-the-output-element-of-the-remote-io
    ::AudioUnitPropertyID const callbackProperty
    (
        ( CallbackInfo::ioLayout == InputOutputLayout::InputOnly )
            ? kAudioOutputUnitProperty_SetInputCallback
            : kAudioUnitProperty_SetRenderCallback
    );

    /// \note At some point changing the Audio Session category on the fly
    /// caused dubious AudioUnitRender failures in the render procedure and was
    /// disabled. However this (always using the
    /// kAudioSessionCategory_PlayAndRecord) category caused simple
    /// playback-only operation to use the "receiver speaker" on the iPhone.
    /// http://iphonedevsdk.com/forum/iphone-sdk-development/108508-error-switching-kaudiosessionproperty-audiocategory-after-ios-6-update.html
    ///                                       (31.07.2014.) (Domagoj Saric)
    UInt32 audioSessionCategory;
    switch ( CallbackInfo::ioLayout )
    {
        case InputOutputLayout::InputOnly : audioSessionCategory = kAudioSessionCategory_RecordAudio  ; break;
        case InputOutputLayout::OutputOnly: audioSessionCategory = kAudioSessionCategory_MediaPlayback; break;
        default                           : audioSessionCategory = kAudioSessionCategory_PlayAndRecord; break;
    }

    return setCallback
    (
        &renderProcedure<CallbackInfo::channelLayout, CallbackInfo::ioLayout>,
        callbackProperty,
        audioSessionCategory,
        interleaved,
        inputEnabled,
        outputEnabled
    );
}

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "pimplForwarders.inl"
