////////////////////////////////////////////////////////////////////////////////
///
/// android.cpp
/// -----------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "device.hpp"
#include "deviceImpl.hpp"

#include "le/audioio/openSL.hpp"

#include "le/utility/countof.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/trace.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <boost/assert.hpp>
#include <boost/smart_ptr/scoped_array.hpp>

#ifndef NDEBUG
#include <android/log.h>
#endif // NDEBUG
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <sys/system_properties.h>
#include <unistd.h>

/// \note Android implementation does not support the SLThreadSyncItf interface.
/// https://groups.google.com/forum/#!topic/android-ndk/7cB8fZ9zTKY
///                                           (05.03.2014.) (Domagoj Saric)
//#define LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
#ifdef LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
#else
#include "pthread.h"
#endif // LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX

#include <algorithm>
#include <cstdint>
#include <cstring> // std::strcmp
#include <mutex>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

// http://www.wiseandroid.com/post/2010/07/13/Intro-to-the-three-Android-Audio-APIs.aspx
// http://source.android.com/devices/audio_terminology.html
//
// Latency:
//  Low latency requires Jellybean http://developer.android.com/about/versions/jelly-bean.html (4.1/4.2)
//  Google I/O 2013 - High Performance Audio https://developers.google.com/events/io/sessions/325993827 / http://www.youtube.com/watch?v=d3kfEeMZ65c ~30:00
//  Google I/O 2014 https://www.youtube.com/watch?v=92fgcUNCHic
//  Samsung Professional Audio SDK
//      http://developer.samsung.com/galaxy#professional-audio
//      http://forum.audiob.us/discussion/6023/samsung-s-new-pro-audio-sdk-zero-low-latency-audio-midi-and-a-daw-called-soundcamp-for-android
//      http://superpowered.com/why-samsung-professional-audio-sdk-is-not-the-best-solution-for-low-latency-android-audio/#axzz3ZRVDpoLV
//  http://source.android.com/devices/latency_design.html
//  http://superpowered.com/androidaudiopathlatency
//  https://source.android.com/devices/audio/latency_measure.html
//  http://www.sonomawireworks.com/pr/android-low-latency-audio-solution.php
//  http://www.sonomawireworks.com/forums/viewtopic.php?id=11468
//  http://www.androidannoyances.com/post/38
//  http://heatvst.com/wp/2013/11/30/high-performance-low-latency-audio-on-android-why-it-still-doesnt-work
//  http://audioprograming.wordpress.com/2012/12/02/an-update-on-the-latency-issue
//  http://www.reddit.com/r/Android/comments/1j6erw/android_43_latency_measurements
//  http://developer.android.com/reference/android/content/pm/PackageManager.html#FEATURE_AUDIO_LOW_LATENCY
//  http://stackoverflow.com/questions/14842803/low-latency-audio-playback-on-android
//  On the performance of real-time DSP on Android devices http://smcnetwork.org/system/files/smc2012-167.pdf
//  http://www.kvraudio.com/forum/viewtopic.php?f=166&t=331703
//  http://createdigitalnoise.com/discussion/2057/low-latency-android-devices-is-there-a-list
//  http://createdigitalmusic.com/2012/07/android-high-performance-audio-in-4-1-and-what-it-means-plus-libpd-goodness-today
//  Bugs:
//    https://code.google.com/p/android/issues/detail?id=3434 Google Issue 3434: Add APIs for low-latency audio
//    https://groups.google.com/forum/#!topic/android-ndk/HP1n8fphSs0 OpenSL ES buffer queue callback being called a bit late
//    https://groups.google.com/forum/#!topic/android-ndk/F0QIivCSTtk
//    https://groups.google.com/forum/#!topic/android-ndk/aSemcXzr3Sc
//    http://comments.gmane.org/gmane.comp.handhelds.android.ndk/21030
//    https://code.google.com/p/csipsimple/issues/detail?id=2096 csipsimple Issue 2096: Android 4.2 low-latency audio
//  Thread priorities:
//  - https://groups.google.com/forum/m/#!topic/android-developers/b2SKprSxPvw
//  - http://developer.android.com/reference/android/os/Process.html#THREAD_PRIORITY_URGENT_AUDIO
//  - http://music.columbia.edu/pipermail/andraudio/2011-February/000164.html
//  - https://groups.google.com/forum/#!topic/android-developers/IgJIzSfigcY
//
// OpenSL ES:
//  http://www.khronos.org/registry/sles/specs/OpenSL_ES_Specification_1.0.1.pdf
//  http://mobilepearls.com/labs/native-android-api/ndk/docs/opensles/index.html
//  http://audioprograming.wordpress.com/2012/03/03/android-audio-streaming-with-opensl-es-and-the-ndk
//  http://audioprograming.wordpress.com/2012/10/29/lock-free-audio-io-with-opensl-es-on-android
//  http://www.slideshare.net/DSPIP/android-audio-opensl
//  http://mindtherobot.com/blog/555/android-audio-problems-hidden-limitations-and-opensl-es/#more-555
//  http://diaryofagamesprogrammer.blogspot.com/2012/02/android-opensl-audio-latency-woes.html
//  http://04795323645711959901.googlegroups.com/attach/edd18ef636f993a/buffer_queue_player.png
//  Csound for Android http://lac.linuxaudio.org/2012/papers/20.pdf
//  OpenSL ES - Is possible to set DataSource as MP3 buffer? https://groups.google.com/forum/#!searchin/android-ndk/opensl|sort:date/android-ndk/cMHlkyQkFU0/vMkyO2201yYJ
//  Android NDK mp3 support https://groups.google.com/forum/#!topic/android-ndk/E7S5TY5f4Ok
//  http://stackoverflow.com/questions/tagged/opensl
//
// OpenMAX:
//  http://jan.newmarch.name/LinuxSound/Sampled/OpenMAX
//  https://groups.google.com/forum/#!topic/android-ndk/XzJ_vOGiGPo
//
// ALSA:
//  http://code.paulk.fr/article3/tinyalsa-audio
//  http://stackoverflow.com/questions/15837374/add-alsa-to-android
//  http://seasonofcode.com/posts/using-alsa-audio-drivers-in-android.html
//  http://forum.xda-developers.com/showthread.php?t=1425421
//  https://play.google.com/store/apps/details?id=com.gilsken.mixget&hl=de
//  https://gitorious.org/android-on-freerunner/platform_hardware_alsa_sound/source/ec33b7928abc81aec19ed34953d30dee1010996f:AudioHardwareALSA.cpp
//
// code:
//  https://code.google.com/p/high-performance-audio
//  https://github.com/nettoyeurny/opensl_stream
//  https://code.google.com/p/music-synthesizer-for-android
//  http://sourceforge.net/projects/csound http://sourceforge.net/p/csound/csound6-git/ci/master/tree/android/CsoundAndroid/jni/rtopensl.c
//  https://github.com/libpd/pd-for-android
//  https://github.com/android/platform_hardware_libhardware/blob/master/include/hardware/audio_effect.h
//  https://code.google.com/p/high-performance-audio/source/browse/audio-buffer-size/src/com/levien/audiobuffersize/AudioBufferSize.java
//  https://bitbucket.org/victorlazzarini/android-audiotest
//  http://www.musicalandroid.com/1/category/programming/1.html
//  http://androidxref.com Android source code indexer/cross-reference
//  http://atastypixel.com/blog/a-simple-fast-circular-buffer-implementation-for-audio-processing
//
// rooting, custom builds, hacks:
// http://forum.xda-developers.com
// http://codefirex.com


/// \note The weird StackPimpl reserved size difference for 64 bit Android
/// builds is because of different definitions of pthread structures.
///                                           (22.01.2016.) (Domagoj Saric)

class DeviceImpl
{
public:
    DeviceImpl()
        :
    #ifndef NDEBUG
        sampleRate_( 0       ),
    #endif // NDEBUG
        pRecorder_ ( nullptr )
    {}

    LE_COLD
    ~DeviceImpl()
    {
        /// \note Ensure no concurrent playback/recording callbacks try to
        /// access the dead DeviceImpl (and/or OpenSL::Engine singleton)
        /// instances.
        ///                                   (16.07.2014.) (Domagoj Saric)
        stop();
        //...mrmlj...hunting for async destruction crashes/deadlocks...reinvestigate...
        {
            OpenSL::Engine::Lock const playerLock( playerMutex() );
            player_.destroy();
        }
        {
            OpenSL::Engine::Lock const recorderLock( recorderMutex() );
            recorderBQ_.destroy();
            pRecorderObject_.destroy();
            pRecorder_ = nullptr;
        }
    }

    static std::uint8_t const maximumNumberOfChannels = OpenSL::maximumNumberOfChannels;

    using sample_t = std::int16_t;

    using IOLayout = InputOutputLayout;

    LE_COLD
    char const * LE_FASTCALL setup
    (
        std::uint8_t  const numberOfChannels,
        std::uint32_t const sampleRate,
        std::uint16_t const desiredLatencyInSampleFrames
    )
    {
    #ifndef NDEBUG
        ::SLuint32 recordState; if ( pRecorder_ ) LE_OSL_VERIFY( (*pRecorder_)->GetRecordState( pRecorder_, &recordState ) ); else recordState = SL_RECORDSTATE_STOPPED;
        BOOST_ASSERT_MSG( !pRecorder_ || recordState     == SL_RECORDSTATE_STOPPED, "Device not in stopped state" );
        BOOST_ASSERT_MSG( !player_    || player_.state() == SL_PLAYSTATE_STOPPED  , "Device not in stopped state" );
    #endif // NDEBUG

        if ( BOOST_UNLIKELY( numberOfChannels > maximumNumberOfChannels ) )
            return "Unsupported number of channels";

        if ( BOOST_UNLIKELY( !( pOSLEngine_ || ( pOSLEngine_ = OpenSL::Engine::singleton() ) ) ) )
            return "Out of memory";

        // http://audiobuffersize.appspot.com
    #if defined( __aarch64__ ) /*|| defined( __ARM_NEON__ )*/ || defined( __i386__ )
        std::uint16_t const processingBlockSizeInFrames(  240 ); //...mrmlj...check hardware native...
        // https://groups.google.com/forum/#!topic/android-ndk/H8L8VqZ-VVY
        // https://android.googlesource.com/platform/frameworks/wilhelm/+/92e53bc98cd938e9917fb02d3e5a9be88423791d%5E!
        std::uint8_t const minimumBuffersForFastTrack( 1 );
    #elif defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7__ )
        std::uint16_t const processingBlockSizeInFrames( 480 );
        std::uint8_t  const minimumBuffersForFastTrack (   2 );
    #else
        std::uint16_t const processingBlockSizeInFrames( 1104 );
        std::uint8_t  const minimumBuffersForFastTrack (    2 );
    #endif
        std::uint16_t const processingBlockSizeInSamples( desiredLatencyInSampleFrames ? desiredLatencyInSampleFrames : processingBlockSizeInFrames );
        sampleRate_ = sampleRate ? sampleRate : 48000; //...mrmlj...check hardware native...
        if
        (
            !dataConverter_.setup
            (
                numberOfChannels ? numberOfChannels : 1, //...mrmlj...detect the connected device (use mono for internal microphone and speaker)...
                minimumBuffersForFastTrack,
                processingBlockSizeInSamples
            )
        )
            return "Out of memory";
    #ifndef LE_PUBLIC_BUILD
        LE_TRACE( "Device setup: numberOfChannels %d, sampleRate %d, numberOfBuffers %d", dataConverter_.numberOfChannels(), sampleRate_, dataConverter_.numberOfBuffers() );
    #endif // !LE_PUBLIC_BUILD
        return nullptr;
    }

    template <typename DataLayout>
    LE_COLD
    char const * LE_FASTCALL setCallback( std::function<void (DataLayout)> && callback )
    {
        BOOST_ASSERT( callback );

        using CallbackInfo = CallbackInfo<DataLayout>;

        stop();

        switch ( CallbackInfo::ioLayout )
        {
            case IOLayout::InputOnly           : player_.clearInterface(); player_.OpenSL::ObjectPtr::destroy();                           break;
            case IOLayout::OutputOnly          : pRecorder_ = nullptr    ; pRecorderObject_          .destroy(); player_.clearInterface(); break; //...mrmlj...the player interface has to be cleared always because of the check in getPlayerInterface()...
            case IOLayout::Inplace             :
            case IOLayout::SeparatedInputOutput:                                                                 player_.clearInterface(); break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }

        char const * error;
        ::SLDataLocator_AndroidSimpleBufferQueue const bufferQueue( makeBQ        () );
        ::SLDataFormat_PCM                       const dataFormat ( makeDataFormat() );
        if ( ( CallbackInfo::ioLayout != IOLayout::OutputOnly ) && ( error = setupInput ( dataFormat, bufferQueue, &recorderCallback<CallbackInfo::channelLayout, CallbackInfo::ioLayout> ) ) ) return error;
        if ( ( CallbackInfo::ioLayout != IOLayout::InputOnly  ) && ( error = setupOutput( dataFormat, bufferQueue, &playerCallback  <CallbackInfo::channelLayout, CallbackInfo::ioLayout> ) ) ) return error;

        storeCallback<DataLayout>( callback_, std::move( callback ) );

        if ( CallbackInfo::ioLayout == IOLayout::OutputOnly )
        {
            BOOST_ASSERT( !player_.interfaceFetched() );
            // Fill and enqueue all available buffers before playback starts...
            for ( std::uint8_t counter( 0 ); counter < dataConverter_.numberOfBuffers(); ++counter )
            {
                playerCallback<CallbackInfo::channelLayout, IOLayout::OutputOnly>( player_.bq().pBQ, this );
            }
            getPlayerInterface();
        }
        return nullptr;
    }

    LE_COLD
    void LE_FASTCALL start()
    {
        BOOST_ASSERT_MSG( player_.bq().oldestBuffer == 0, "Internal inconsistency" );
        BOOST_ASSERT_MSG( recorderBQ_ .oldestBuffer == 0, "Internal inconsistency" );
        BOOST_ASSERT_MSG( !player_          || player_         .getState() == SL_OBJECT_STATE_REALIZED, "Player resources lost"   );
        BOOST_ASSERT_MSG( !pRecorderObject_ || pRecorderObject_.getState() == SL_OBJECT_STATE_REALIZED, "Recorder resources lost" );
        if ( pRecorder_ )
        {
            /// \note See the note for the SeparatedInputOutput case in the
            /// recorderCallback() member function.
            ///                               (03.03.2014.) (Domagoj Saric)
            BOOST_ASSERT( !player_.interfaceFetched() );
            // Enqueue all available buffers before recording starts...
            for ( std::uint8_t counter( 0 ); counter < dataConverter_.numberOfBuffers(); ++counter )
                dataConverter_.enqueue( recorderBQ_ );
            BOOST_ASSERT( recorderBQ_.oldestBuffer == 0 );
            LE_OSL_VERIFY( (*pRecorder_)->SetRecordState( pRecorder_, SL_RECORDSTATE_RECORDING ) );
        }
        else // OutputOnly callback
        {
            /// \note Unlike for recording, we cannot run the "fill and enqueue
            /// all available buffers before playback starts" loop here because
            /// we don't know the type (channel layout) of the player callback.
            /// This is only relevant if the user uses/calls stop() as a pause
            /// operation and wishes to call start() later as a resume
            /// operation.
            ///                               (04.03.2014.) (Domagoj Saric)
            player_.start();
        }
    }

    LE_COLD
    void LE_FASTCALL stop()
    {
        if ( BOOST_UNLIKELY( !pOSLEngine_ ) )
            return;

        /// \note It seems that SL_PLAY/RECORDSTATE_STOPPED is not synchronous
        /// (i.e. does not wait for the current callback to finish) so we have
        /// to perform our own synchronization in order to prevent race
        /// conditions (context objects being destroyed while a callback is
        /// still active).
        ///                                   (05.03.2014.) (Domagoj Saric)
        {
            OpenSL::Engine::Lock const playerLock  ( playerMutex  () );
            OpenSL::Engine::Lock const recorderLock( recorderMutex() );

            if ( player_ )
                player_.stop();
            if ( pRecorder_ )
            {
                recorderBQ_.reset();
                LE_OSL_VERIFY( (*pRecorder_)->SetRecordState( pRecorder_, SL_RECORDSTATE_STOPPED ) );
                player_.clearInterface(); //...mrmlj...
            }
        }

        if ( pRecorder_ )
        {
            /// \note Hack-attempt to workaround the Android OSL bug described
            /// in reset() w/o recreating and reinitialising the recorder
            /// object.
            ///                               (22.01.2016.) (Domagoj Saric)
            recorderBQ_.registerCallback( &recorderBQ_.nopCallback, nullptr );
            LE_OSL_VERIFY( (*pRecorder_)->SetRecordState( pRecorder_, SL_RECORDSTATE_RECORDING ) );
            SLAndroidSimpleBufferQueueState state;
            while ( ( state = recorderBQ_.state(), state.count != 0 ) )
                BOOST_VERIFY( sched_yield() == 0 );
            LE_OSL_VERIFY( (*pRecorder_)->SetRecordState( pRecorder_, SL_RECORDSTATE_STOPPED ) );
            BOOST_ASSERT( recorderBQ_.state().count == 0 );
          //BOOST_ASSERT( recorderBQ_.state().index == 0 );
            BOOST_ASSERT( recorderBQ_.oldestBuffer  == 0 );
            recorderBQ_.registerCallback( recorderCallback_, this );
            recorderBQ_.reset();
        }
    }

    std::uint8_t  numberOfChannels() const { return dataConverter_.numberOfChannels(); }
    std::uint16_t sampleRate      () const { return sampleRate_; }

    LE_COLD
    Device::LatencyAndBufferSize LE_FASTCALL latency() const
    {
        return Device::LatencyAndBufferSize
        (
            dataConverter_.numberOfSamplesPerBuffer() * dataConverter_.numberOfBuffers(),
            dataConverter_.numberOfSamplesPerBuffer()
        );
    }

private:
    static bool runningUnderEmulator()
    {
        /// \note Recording seems broken under emulator so we have to
        /// special-handle it (or at least warn the user).
        /// http://stackoverflow.com/questions/8422931/recording-with-opensl-es-in-emulator
        /// https://groups.google.com/forum/#!topic/android-ndk/YdgnpYEPGOY
        /// http://stackoverflow.com/questions/2799097/how-can-i-detect-when-an-android-application-is-running-in-the-emulator
        /// https://review.webrtc.org/55419004
        /// https://android.googlesource.com/platform/hardware/ril.git/+/android-4.4_r0.7/reference-ril/reference-ril.c
        ///                                   (03.03.2014.) (Domagoj Saric)

        /// \note __system_property_get has been removed from 64bit builds in
        /// Android L.
        /// https://github.com/mikereidis/spirit2_free/issues/8
        /// http://stackoverflow.com/questions/28413530/api-to-get-android-system-properties-is-removed-in-arm64-platforms
        /// http://stackoverflow.com/questions/26722040/replacement-for-system-property-get-in-android-l-ndk
        /// https://codereview.chromium.org/393923002
        ///                                   (10.12.2015.) (Domagoj Saric)
    #if __LP64__
        return /*...mrmlj...quick fix...*/ false;
    #else
        /// \note hickup.co.uk developers reported that the following Samsung
        /// devices:
        /// - Samsung Tab 2
        /// - Samsung Mini
        /// - Samsung Galaxy S2
        /// get misdetected as the emulator if the existence of the
        /// "ro.kernel.qemu" property is used to detect the emulator.
        ///                                   (03.09.2014.) (Domagoj Saric)
        char propValue[ PROP_VALUE_MAX ];
        //return __system_property_get( "ro.kernel.qemu", propValue ) != 0;
        BOOST_VERIFY( __system_property_get( "ro.hardware", propValue ) );
        return std::strcmp( propValue, "goldfish" ) == 0;
    #endif // __LP64__
    }

    LE_COLD
    char const * LE_FASTCALL setupInput
    (
        ::SLDataFormat_PCM                       const &       dataFormat,
        ::SLDataLocator_AndroidSimpleBufferQueue const &       bufferQueue,
        ::slAndroidSimpleBufferQueueCallback             const callback
    )
    {
        pRecorderObject_.destroy();
    #ifndef NDEBUG
        if ( ( dataFormat.samplesPerSec != 8000 ) && runningUnderEmulator() )
            Utility::Tracer::error( "Android emulator usually only supports an 8kHz recording samplerate." );
    #endif // NDEBUG

        auto const pEngine( OpenSL::Engine::singletonInterface() );

        OpenSL::Engine::Lock const engineLock( OpenSL::Engine::singletonMutex() );

        ::SLDataLocator_IODevice constexpr deviceDataLocator =
        {
            SL_DATALOCATOR_IODEVICE,
            SL_IODEVICE_AUDIOINPUT,
            SL_DEFAULTDEVICEID_AUDIOINPUT,
            nullptr
        };
        ::SLDataSource  const     audioSource = { const_cast<SLDataLocator_IODevice                 *>( &deviceDataLocator ), nullptr                                       };
        ::SLDataSink    const     audioSink   = { const_cast<SLDataLocator_AndroidSimpleBufferQueue *>( &bufferQueue       ), const_cast<SLDataFormat_PCM *>( &dataFormat ) };
        ::SLInterfaceID const     ids[]       = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE };
        ::SLboolean     constexpr req[]       = { SL_BOOLEAN_TRUE };
        ::SLresult result = (*pEngine)->CreateAudioRecorder( pEngine, &pRecorderObject_, const_cast<SLDataSource *>( &audioSource ), const_cast<SLDataSink *>( &audioSink ), _countof( ids ), ids, req );
        if ( result != SL_RESULT_SUCCESS )
        {
            BOOST_ASSERT_MSG
            (
                result == SL_RESULT_MEMORY_FAILURE      ||
                result == SL_RESULT_RESOURCE_ERROR      ||
                result == SL_RESULT_IO_ERROR            ||
                result == SL_RESULT_CONTENT_UNSUPPORTED,
                "Unexpected OpenSL ES result code."
            );
            BOOST_ASSERT( !pRecorderObject_ );
            return ( result == SL_RESULT_CONTENT_UNSUPPORTED ) ? "Unsupported audio input format" : "Out of memory";
        }
        result = pRecorderObject_.realize();
        if ( result != SL_RESULT_SUCCESS )
        {
            BOOST_ASSERT_MSG
            (
                result == SL_RESULT_MEMORY_FAILURE      ||
                result == SL_RESULT_RESOURCE_ERROR      ||
                result == SL_RESULT_IO_ERROR            ||
                result == SL_RESULT_CONTENT_UNSUPPORTED,
                "Unexpected OpenSL ES result code."
            );
            return ( result == SL_RESULT_CONTENT_UNSUPPORTED ) ? "Unsupported audio input format" : "Out of memory";
        }

        pRecorderObject_.getInterface( SL_IID_RECORD                  , pRecorder_      );
        pRecorderObject_.getInterface( SL_IID_ANDROIDSIMPLEBUFFERQUEUE, recorderBQ_.pBQ );
        recorderBQ_.reset();

        recorderBQ_.registerCallback( callback, this );
        recorderCallback_ = callback;

    #ifndef NDEBUG
        LE_OSL_VERIFY
        (
            (*pRecorder_)->RegisterCallback
            (
                pRecorder_,
                []( SLRecordItf /*caller*/, void * /*const pContext*/, SLuint32 const event )
                {
                    BOOST_ASSERT( event == SL_RECORDEVENT_BUFFER_FULL );
                    LE_TRACE( "Device: input buffer underrun" );
                },
                nullptr
            )
        );
        LE_OSL_VERIFY( (*pRecorder_)->SetCallbackEventsMask( pRecorder_, SL_RECORDEVENT_BUFFER_FULL ) );
    #endif // NDEBUG

        return nullptr;
    }

    LE_COLD
    char const * LE_FASTCALL setupOutput
    (
        ::SLDataFormat_PCM                       const &       dataFormat,
        ::SLDataLocator_AndroidSimpleBufferQueue const &       bufferQueue,
        ::slAndroidSimpleBufferQueueCallback             const callback
    )
    {
        /// \note The Player object has to be destroyed before the OutputMix
        /// object (reverse order from construction), otherwise a corresponding
        /// libOpenSLES error gets reported in the Android log.
        ///                                   (13.01.2014.) (Domagoj Saric)
        player_          .destroy();
        pOutputMixObject_.destroy();

        {
            auto const pEngine( OpenSL::Engine::singletonInterface() );
            OpenSL::Engine::Lock const engineLock( OpenSL::Engine::singletonMutex() );
            ::SLresult result = (*pEngine)->CreateOutputMix( pEngine, &pOutputMixObject_, 0, nullptr, nullptr );
            if ( result != SL_RESULT_SUCCESS )
            {
                BOOST_ASSERT( result == SL_RESULT_MEMORY_FAILURE );
                return "Out of memory";
            }
            result = pOutputMixObject_.realize();
            if ( result != SL_RESULT_SUCCESS )
            {
                BOOST_ASSERT( result == SL_RESULT_MEMORY_FAILURE || result == SL_RESULT_RESOURCE_ERROR || result == SL_RESULT_IO_ERROR );
                return "Out of memory";
            }
        }

        ::SLDataSource const audioSource = { const_cast<SLDataLocator_AndroidSimpleBufferQueue *>( &bufferQueue ), const_cast<SLDataFormat_PCM *>( &dataFormat ) };

        ::SLDataLocator_OutputMix const outputMix = { SL_DATALOCATOR_OUTPUTMIX, pOutputMixObject_                  };
        ::SLDataSink              const audioSink = { const_cast<SLDataLocator_OutputMix *>( &outputMix ), nullptr };

        ::SLInterfaceID const ids[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_PLAY }; // SL_IID_MUTESOLO, SL_IID_VOLUME
        auto const pError( player_.setup( audioSource, audioSink, ids, _countof( ids ) ) );
        if ( BOOST_UNLIKELY( pError != nullptr ) )
            return pError;
        player_.bq().registerCallback( callback, this );
        return nullptr;
    }

    LE_COLD
    ::SLDataLocator_AndroidSimpleBufferQueue LE_FASTCALL makeBQ() const
    {
        return { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, dataConverter_.numberOfBuffers() };
    }

    LE_COLD
    ::SLDataFormat_PCM LE_FASTCALL makeDataFormat() const
    {
        return
        {
            SL_DATAFORMAT_PCM,
            dataConverter_.numberOfChannels(),
            static_cast<SLuint32>( sampleRate_ * 1000 ), // in mHz
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            ( dataConverter_.numberOfChannels() == 1 ) ? SL_SPEAKER_FRONT_CENTER : ( SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT ),
            SL_BYTEORDER_LITTLEENDIAN
        };
    }

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    template <ChannelLayout channelLayout, InputOutputLayout ioLayout>
    static void playerCallback( ::SLAndroidSimpleBufferQueueItf LE_RESTRICT const pBQ, void * const pContext )
    {
        LE_ASSUME( pBQ && pContext );
        DeviceImpl & LE_RESTRICT device( *static_cast<DeviceImpl *>( pContext ) );
        LE_ASSUME( pBQ == device.player_.bq().pBQ );

        BOOST_ASSERT( device.player_.interfaceFetched() || ioLayout == IOLayout::OutputOnly );

        /// \note See the related note in the stop() member function.
        ///                                   (05.03.2014.) (Domagoj Saric)
        OpenSL::Engine::Lock const playerLock( device.playerMutex() );
        if ( BOOST_UNLIKELY( device.player_.interfaceFetched() && device.getPlaybackState() == SL_PLAYSTATE_STOPPED ) )
            return;

        BOOST_ASSERT_MSG( !device.player_.interfaceFetched() || device.getPlaybackState() == SL_PLAYSTATE_PLAYING, "Streaming state changed in the middle of the callback" );

        auto & data( device.dataConverter_ );

        switch ( ioLayout )
        {
            case IOLayout::OutputOnly:
            {
                invoke( device.callback_, Device::Audio<channelLayout, IOLayout::OutputOnly>{ data.internalBuffer<channelLayout>(), data.numberOfSamplesPerBuffer() } );
                data.cripple          <channelLayout>();
                data.convertOutputData<channelLayout>( device.player_.bq() );
                data.enqueue( device.player_.bq() ); //...mrmlj...this might be called after the callback function calls stop()...
                break;
            }

            case IOLayout::Inplace:
            case IOLayout::SeparatedInputOutput:
                data.enqueue( device.recorderBQ_ );
                break;

            LE_DEFAULT_CASE_UNREACHABLE();
        }

        //...mrmlj...!?...BOOST_ASSERT_MSG( !device.player_.interfaceFetched() || device.getPlaybackState() == SL_PLAYSTATE_PLAYING, "Streaming state changed in the middle of the callback" );
    }

    template <ChannelLayout channelLayout, InputOutputLayout ioLayout>
    static void recorderCallback( ::SLAndroidSimpleBufferQueueItf LE_RESTRICT const pBQ, void * const pContext )
    {
        LE_ASSUME( pBQ && pContext );
        DeviceImpl & LE_RESTRICT device( *static_cast<DeviceImpl *>( pContext ) );
        LE_ASSUME( pBQ == device.recorderBQ_.pBQ );

        /// \note See the related note in the stop() member function.
        ///                                   (05.03.2014.) (Domagoj Saric)
        OpenSL::Engine::Lock const recorderLock( device.recorderMutex() );
        if ( BOOST_UNLIKELY( device.getRecordState() == SL_RECORDSTATE_STOPPED ) )
            return;

        BOOST_ASSERT_MSG( device.getRecordState() == SL_RECORDSTATE_RECORDING, "Streaming state changed in the middle of the callback" );
      //BOOST_ASSERT_MSG( device.callback_ != PublicCallback()               , "Callback not correctly registered"                     );

        auto       & data    ( device.dataConverter_ );
        auto const & callback( device.callback_      );

        switch ( ioLayout )
        {
            case IOLayout::InputOnly:
                data.convertInputData<channelLayout>( device.recorderBQ_ );
                data.enqueue( device.recorderBQ_ );
                data.cripple<channelLayout>();
                invoke( callback, Device::Audio<channelLayout, IOLayout::InputOnly>{ data.internalBuffer<channelLayout>(), data.numberOfSamplesPerBuffer() } );
                break;

            case IOLayout::Inplace:
            case IOLayout::SeparatedInputOutput:
            {
                data.convertInputData<channelLayout>( device.player_.bq() );
                invoke( callback, Device::Audio<channelLayout, ioLayout>{ data.internalBuffer<channelLayout>(), data.numberOfSamplesPerBuffer() } );
                data.cripple          <channelLayout>(); //...mrmlj...this can be misused to get uncrippled input with input+output mode...
                data.convertOutputData<channelLayout>( device.player_.bq() );
                data.enqueue                         ( device.player_.bq() );
                /// \note Just like in the case of OutputOnly operation here we
                /// try to first queue all available player buffers and only
                /// then start the playback. The existence of the Player
                /// interface is checked as an indicator as to whether the
                /// playback has yet started (see the related note for the
                /// getPlayerInterface() member function).
                ///                           (17.01.2014.) (Domagoj Saric)
                if
                (
                    ( device.player_.interfaceFetched() == false ) && // initial playback queuing
                    ( device.player_.bq().oldestBuffer  == 0     )
                )
                {
                    BOOST_ASSERT( device.player_.bq().oldestBuffer == 0 );
                    BOOST_ASSERT( device.recorderBQ_ .oldestBuffer == 0 );
                    //BOOST_ASSERT_MSG( enqueueResult == SL_RESULT_BUFFER_INSUFFICIENT, "Unexpected enqueue result" );
                    device.getPlayerInterface();
                    device.player_.start();
                }
                break;
            }

            LE_DEFAULT_CASE_UNREACHABLE();
        }

        BOOST_ASSERT_MSG( device.getRecordState() == SL_RECORDSTATE_RECORDING, "Streaming state changed in the middle of the callback" );
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__
private:
    using Mutex = Utility::CriticalSection;
    Mutex & playerMutex  () { return player_.bq().mutex  ; }
    Mutex & recorderMutex() { return recorderBQ_ .mutex  ; }

    /// \note Because the Player interface is fetched late, it is requested from
    /// multiple locations and so the appropriate code is wrapped in a function.
    /// See the note for the SeparatedInputOutput case in the
    /// recorderCallback() member function.
    ///                                       (16.01.2014.) (Domagoj Saric)
    void       LE_COLD LE_FASTCALL getPlayerInterface()       { player_.getInterface(); }
    ::SLuint32 LE_COLD LE_FASTCALL getPlaybackState  () const { return player_.state(); }

    ::SLuint32 LE_COLD LE_FASTCALL getRecordState() const
    {
        ::SLuint32 state;
        LE_OSL_VERIFY( (*pRecorder_)->GetRecordState( pRecorder_, &state ) );
        return state;
    }

private:
    std::uint16_t sampleRate_;

    OpenSL::Engine::Ptr pOSLEngine_;

    OpenSL::DataConverter dataConverter_;

    // output
    OpenSL::ObjectPtr pOutputMixObject_;
    OpenSL::Player    player_          ;

    // input
    OpenSL::ObjectPtr   pRecorderObject_;
    ::SLRecordItf       pRecorder_      ;
    OpenSL::BufferQueue recorderBQ_     ;
    slAndroidSimpleBufferQueueCallback recorderCallback_ = nullptr; //...mrmlj...for draining

    PublicCallback callback_;
}; // class DeviceImpl

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#ifdef __ANDROID__
/// \note Dummy implementation to plug linker errors when the user
/// uses libstdc++.
///                                           (10.12.2015.) (Domagoj Saric)
LE_WEAK_FUNCTION std::runtime_error::runtime_error( char const * ) { LE_UNREACHABLE_CODE(); }
#endif // __ANDROID__

#include "pimplForwarders.inl"
