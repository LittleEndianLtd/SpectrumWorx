////////////////////////////////////////////////////////////////////////////////
///
/// rtAudio.cpp
/// -----------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma warning( disable : 4502 ) // Decorated name length exceeded, name was truncated.

#include "device.hpp"
#include "deviceImpl.hpp"

#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"

// http://www.music.mcgill.ca/~gary/rtaudio
#include "RtAudio.h"

#pragma comment( lib, "dsound.lib" )

#include <algorithm>
#include <cstdint>
#include <utility>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

class DeviceImpl
{
public:
#if 0 // ...mrmlj...for testing...
    DeviceImpl() : soundCard_( RtAudio::WINDOWS_WASAPI ) {}
#endif

    char const * LE_COLD setup( std::uint8_t const numberOfChannels, std::uint32_t const sampleRate, std::uint16_t const desiredLatencyInSamples )
    {
        numberOfChannels_        = numberOfChannels       ;
        desiredLatencyInSamples_ = desiredLatencyInSamples;
        sampleRate_              = sampleRate             ;

        //...mrmlj...ugh...force 'something' to be opened so that desiredLatencyInSamples_ would get updated/approximated...
        return setup( (RtAudioCallback)( soundCard_.rtApi() ? soundCard_.rtApi()->stream().callbackInfo.callback : nullptr ), InputOutputLayout::Inplace, 0 );
    }

    template <typename DataLayout>
    char const * setCallback( boost::function<void(DataLayout)> const & callback )
    {
        BOOST_ASSERT( callback );

        using CallbackInfo = CallbackInfo<DataLayout>;

        bool const interleaved( CallbackInfo::channelLayout == ChannelLayout::InterleavedChannels );

        storeCallback<DataLayout>( callback_, callback );

        return setup( &audioCallback<CallbackInfo::channelLayout, CallbackInfo::ioLayout>, CallbackInfo::ioLayout, interleaved ? 0 : RTAUDIO_NONINTERLEAVED );
    }

    void LE_COLD start() const {                                     const_cast<RtAudio &>( soundCard_ ).startStream(); }
    void LE_COLD stop () const { if ( soundCard_.isStreamRunning() ) const_cast<RtAudio &>( soundCard_ ).stopStream (); }

    std::uint8_t  numberOfChannels() const { return numberOfChannels_; }
    std::uint32_t sampleRate      () const { return sampleRate_      ; }

    Device::LatencyAndBufferSize LE_COLD latency() const
    {
        return Device::LatencyAndBufferSize
        (
            static_cast<std::uint16_t>( const_cast<RtAudio &>( soundCard_ ).getStreamLatency() ),
            static_cast<std::uint16_t>( desiredLatencyInSamples_                               )
        );
    }

private:
    char const * LE_COLD setup( RtAudioCallback const audioCallback, InputOutputLayout const ioLayout, RtAudioStreamFlags const extraStreamFlag )
    {
        BOOST_ASSERT( !soundCard_.isStreamRunning() );

        unsigned int const numberOfDevices( soundCard_.getDeviceCount() );
        if ( numberOfDevices < 1 )
            return "No audio devices found.";

        RtAudio::StreamParameters inputParameters;
        inputParameters.nChannels    = numberOfChannels_;
        inputParameters.firstChannel = 0;
        RtAudio::StreamParameters outputParameters( inputParameters );
        unsigned int bufferFrames( desiredLatencyInSamples_ );

        RtAudio::StreamOptions options;
        options.flags           = RTAUDIO_MINIMIZE_LATENCY | RTAUDIO_SCHEDULE_REALTIME | extraStreamFlag;
        options.numberOfBuffers = 0;
        options.priority        = 0;

        if ( soundCard_.isStreamOpen() ) { soundCard_.closeStream(); }
        static std::unique_ptr<char> pErrorMessage;
        bool const inputEnabled ( ioLayout != InputOutputLayout::OutputOnly );
        bool const outputEnabled( ioLayout != InputOutputLayout::InputOnly  );
    #ifndef NDEBUG
        LE_TRACE_IF( audioCallback, "AudioIO: searching for an appropriate device for the requested IO configuration..." );
    #endif // NDEBUG
        //...mrmlj...ugh iterating through all input-output device combinations required for CoreAudio...
        for ( inputParameters.deviceId = 0; inputParameters.deviceId < numberOfDevices; ++inputParameters.deviceId )
        {
            for ( outputParameters.deviceId = 0; outputParameters.deviceId < numberOfDevices; ++outputParameters.deviceId )
            {
                try
                {
                #ifndef NDEBUG
                    auto const  inputName( soundCard_.getDeviceInfo(  inputParameters.deviceId ).name );
                    auto const outputName( soundCard_.getDeviceInfo( outputParameters.deviceId ).name );
                    if ( inputEnabled && outputEnabled )
                    {
                        LE_TRACE_IF( audioCallback, "AudioIO: trying \"%s\" -> \"%s\"...", inputName.c_str(), outputName.c_str() );
                    }
                    else
                    {
                        LE_TRACE_IF( audioCallback, "AudioIO: trying \"%s\"...", inputEnabled ? inputName.c_str() : outputName.c_str() );
                    }
                #endif // NDEBUG
                    soundCard_.openStream
                    (
                        outputEnabled ? &outputParameters : nullptr,
                        inputEnabled  ? &inputParameters  : nullptr,
                        RTAUDIO_FLOAT32,
                        sampleRate_,
                        &bufferFrames,
                        audioCallback,
                        this,
                        &options
                    #ifndef NDEBUG
                        ,&errorCallback
                    #endif // _DEBUG
                    );
                    BOOST_ASSERT( soundCard_.isStreamOpen() );
                    BOOST_ASSERT( sampleRate_ == soundCard_.getStreamSampleRate() );
                    desiredLatencyInSamples_ = static_cast<std::uint16_t>( bufferFrames );

                #ifndef NDEBUG
                    LE_TRACE_IF( audioCallback, "AudioIO: matching device (combination) found." );
                #endif // NDEBUG

                    return nullptr;
                }
                catch ( /*RtError*/ std::exception const & e )
                {
                    //LE_TRACE_IF( audioCallback, "...failed (%s).", e.what() );
                    try           { pErrorMessage.reset( ::_strdup( e.what() ) ); }
                    catch ( ... ) { return "Out of memory";  }
                    if ( soundCard_.isStreamOpen() ) { soundCard_.closeStream(); }
                }
            }
        }
    #ifndef NDEBUG
        LE_TRACE( "...no suitable device (combination) found." );
    #endif // NDEBUG
        return pErrorMessage ? pErrorMessage.get() : "No suitable audio device for the requested format.";
    }

    #pragma warning( push )
    #pragma warning( disable : 4127 ) // Conditional expression is constant.
    template <ChannelLayout channelLayout, InputOutputLayout ioLayout>
    LE_NOTHROW LE_HOT
    static int audioCallback
    (
        void                * const outputBuffer,
        void                * const inputBuffer ,
        unsigned int          const nBufferFrames,
        double                const /*streamTime*/,
        RtAudioStreamStatus   const status,
        void                * const userData
    )
    {
    #ifndef NDEBUG
        switch ( status )
        {
            case RTAUDIO_INPUT_OVERFLOW  : LE_TRACE( "Input overflow."   ); break;
            case RTAUDIO_OUTPUT_UNDERFLOW: LE_TRACE( "Output underflow." ); break;
        }
    #else
        (void)status;
    #endif // __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__

        float const * LE_RESTRICT const pInput ( static_cast<float const *>( inputBuffer  ) );
        float       * LE_RESTRICT const pOutput( static_cast<float       *>( outputBuffer ) );

        DeviceImpl const * LE_RESTRICT const pDevice( static_cast<DeviceImpl const *>( userData ) );

        auto const & callback( pDevice->callback_ );

    #ifdef NDEBUG
        switch ( ioLayout )
        {
            case InputOutputLayout::InputOnly           : BOOST_ASSERT(  pInput && !pOutput ); break;
            case InputOutputLayout::OutputOnly          : BOOST_ASSERT( !pInput &&  pOutput ); break;
          //case InputOutputLayout::SeparatedInputOutput: BOOST_ASSERT(  pInput &&  pOutput ); break;
            case InputOutputLayout::Inplace             : BOOST_ASSERT(  pInput &&  pOutput ); break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }
    #endif // NDEBUG

        auto const sampleFrames( static_cast<std::uint16_t>( nBufferFrames ) );
        auto const samples     ( sampleFrames * pDevice->numberOfChannels_   );

        if ( ioLayout == InputOutputLayout::Inplace )
            std::copy_n( pInput, samples, pOutput );

        if ( channelLayout == ChannelLayout::SeparatedChannels )
        {
            using sample_t = typename std::conditional<( ioLayout == InputOutputLayout::InputOnly ), float const, float>::type;
            std::uint8_t const maxChannels( 8 );
            sample_t * separatedChannels[ maxChannels ];
            LE_ASSUME( pDevice->numberOfChannels_ <= maxChannels );
            auto /*const*/ pBuffer( ( ioLayout == InputOutputLayout::InputOnly ) ? const_cast<sample_t *>( pInput ) : pOutput );
            for ( std::uint8_t channel( 0 ); channel < pDevice->numberOfChannels_; ++channel )
            {
                separatedChannels[ channel ] = static_cast<sample_t *>( &pBuffer[ channel * sampleFrames ] );
            }

            using Buffer = Device::Audio<ChannelLayout::SeparatedChannels, ioLayout>;

            if ( ioLayout != InputOutputLayout::OutputOnly ) { LE_AUDIOIO_CRIPPLE( const_cast<float * const *>( separatedChannels ), sampleFrames ); }
            invoke( callback, Buffer{ separatedChannels, sampleFrames } );
            if ( ioLayout != InputOutputLayout::InputOnly  ) { LE_AUDIOIO_CRIPPLE( const_cast<float * const *>( separatedChannels ), sampleFrames ); }
        }
        else
        {
            using Buffer = Device::Audio<ChannelLayout::InterleavedChannels, ioLayout>;
            if ( ioLayout != InputOutputLayout::OutputOnly ) { LE_AUDIOIO_CRIPPLE( pOutput, sampleFrames ); }
            invoke( callback, Buffer{ pOutput, sampleFrames } );
            if ( ioLayout != InputOutputLayout::InputOnly  ) { LE_AUDIOIO_CRIPPLE( pOutput, sampleFrames ); }
        }

        return 0;
    }
    #pragma warning( pop )

#ifndef NDEBUG
    static void errorCallback( RtAudioError::Type const type, std::string const & errorText )
    {
        Utility::Tracer::error( errorText.c_str() );
        throw( RtAudioError( errorText, type ) );
    }
#endif // NDEBUG

private:
    /// \note The PublicCallback instance has to be destroyed after the RtAudio
    /// instance to avoid a race condition if the audio is still active on
    /// destruction.
    ///                                       (10.09.2015.) (Domagoj Saric)
    PublicCallback callback_;
    RtAudio        soundCard_;
    std::uint8_t   numberOfChannels_       ;
    std::uint16_t  desiredLatencyInSamples_;
    std::uint32_t  sampleRate_             ;
}; // class DeviceImpl

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "pimplForwarders.inl"
