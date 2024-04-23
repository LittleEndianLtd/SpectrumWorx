////////////////////////////////////////////////////////////////////////////////
///
/// \file openSL.hpp
/// ----------------
///
/// OpenSL utility wrappers.
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef openSL_hpp__FDD75079_3A33_450E_B8C0_7CEAD29D8139
#define openSL_hpp__FDD75079_3A33_450E_B8C0_7CEAD29D8139
#pragma once
//------------------------------------------------------------------------------
/// \note Android implementation does not support the SLThreadSyncItf interface.
/// https://groups.google.com/forum/#!topic/android-ndk/7cB8fZ9zTKY
///                                           (05.03.2014.) (Domagoj Saric)
//#define LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
#ifndef LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
#include "le/utility/criticalSection.hpp"
#endif // LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
#include "le/utility/referenceCounter.hpp"
#include "le/utility/trace.hpp"

#include "le/math/vector.hpp"

#include <boost/assert.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <sys/system_properties.h>
#include <unistd.h>

#ifndef NDEBUG
#include <android/log.h>
#endif // NDEBUG

#include <algorithm>
#include <cstdint>
#include <memory>
#include <mutex>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------
namespace OpenSL
{
//------------------------------------------------------------------------------

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

using sample_t = signed short;

/// \note Multichannel audio is available since 4.1.
/// http://developer.android.com/about/versions/jelly-bean.html#media
///                                           (08.02.2016.) (Domagoj Saric)
std::uint8_t constexpr maximumNumberOfChannels = 8; //...mrmlj...


void LE_FASTCALL verify( ::SLresult, unsigned int lineNumber );

#ifdef NDEBUG
    #define LE_OSL_VERIFY( expression ) ( expression )
#else
    #define LE_OSL_VERIFY( expression ) OpenSL::verify( ( expression ), __LINE__ )
#endif // NDEBUG


class ObjectPtr
{
public:
    LE_COLD LE_NOTHROW  ObjectPtr() : ptr_( nullptr ) {}
    LE_COLD LE_NOTHROW  ObjectPtr( ::SLObjectItf const ptr ) : ptr_( ptr ) { BOOST_ASSERT( ptr ); }
    LE_COLD LE_NOTHROW  ObjectPtr( ObjectPtr && other ) : ptr_( other.ptr_ ) { other.ptr_ = nullptr; }
    LE_COLD LE_NOTHROW ~ObjectPtr() { destroy(); }

    LE_COLD LE_NOINLINE ::SLresult LE_FASTCALL realize() { return        obj().Realize( ptr_, false ); }
    LE_COLD LE_NOINLINE void       LE_FASTCALL destroy()
    {
        if ( ptr_ )
        {
            /// \note There are known deadlock problems/bugs in OpenSL,
            /// particularly WRT object destruction. Test the below workaround
            /// and enable it defensively (even if no such problems were yet
            /// reported with our code).
            ///                               (27.01.2016.) (Domagoj Saric)
        #if 0
            // https://groups.google.com/forum/#!topic/android-ndk/G7dLKAGGL28 Why does openSL destroy function block forever on some devices?
            // http://grokbase.com/t/gg/android-ndk/156tayqf1c/opensl-deadlock-issue
            // http://grokbase.com/t/gg/android-ndk/127kvtkj45/opensles-deadlock
            // https://code.google.com/p/android/issues/detail?id=80436
            // http://stackoverflow.com/questions/13752060/android-linux-thread-join-with-timeout
            // http://igourd.blogspot.hr/2009/05/work-around-on-pthreadcancel-for.html
            // https://groups.google.com/forum/#!topic/android-ndk/SHpZOMhVVRM
            // https://code.google.com/p/android/issues/detail?id=3197
            // https://bugzilla.mozilla.org/show_bug.cgi?id=1020227
            // https://bugs.chromium.org/p/webrtc/issues/detail?id=2201
            auto const destroyer
            (
                []( void * const pOSLObject ) -> void *
                {
                    struct sigaction actions;
                    std::memset( &actions, 0, sizeof( actions ) );
                    ::sigemptyset( &actions.sa_mask );
                    actions.sa_flags   = 0;
                    actions.sa_handler = []( int const signal )
                        {
                            BOOST_ASSERT( signal == SIGUSR1 );
                            LE_TRACE( "OpenSL deadlock: killing stuck thread." )
                            pthread_exit( 0 );
                        };
                    BOOST_VERIFY( ::sigaction( SIGUSR1, &actions, nulptr ) == 0 );

                    auto & ptr( *static_cast<SLObjectItf *>( ppOSLObject ) );
                    (*ptr).Destroy( ptr );
                    ptr = nullptr;
                    return nullptr;
                }
            );
            ::pthread_t destroyerThread;
            auto const succeeded
            (
                ::pthread_create
                (
                    &destroyerThread,
                    nullptr,
                    destroyer,
                    ptr_
                ) == 0
            );
            auto volatile & ptr( ptr_ );
            for ( std::uint8_t counter( 10 ); counter && ptr; --counter )
            {
                BOOST_VERIFY( sched_yield() == 0 );
            }
            if ( BOOST_LIKELY( !ptr ) )
                BOOST_VERIFY( pthread_join( destroyerThread, nullptr ) == 0 );
            else
            {
                BOOST_VERIFY( pthread_kill( destroyerThread, SIGUSR1 ) == 0 );
                ptr_ = nullptr;
            }
        #else
            obj().Destroy( ptr_ );
            ptr_ = nullptr;
        #endif
        }
    }

    LE_COLD ::SLuint32 getState() const
    {
        ::SLuint32 state;
        LE_OSL_VERIFY( obj().GetState( ptr_, &state ) );
        return state;
    }

    template <class Interface>
    void    getInterface( ::SLInterfaceID const & interfaceID, Interface * & pInterface ) const { return    getInterface( interfaceID, static_cast<void *>( &pInterface ) ); }
    template <class Interface>
    bool tryGetInterface( ::SLInterfaceID const & interfaceID, Interface * & pInterface ) const { return tryGetInterface( interfaceID, static_cast<void *>( &pInterface ) ); }

    ::SLObjectItf_ const * const & operator->() const { return *ptr_; }

    operator ::SLObjectItf () const { return ptr_; }

    ::SLObjectItf * operator & ()
    {
        BOOST_ASSERT_MSG( !ptr_, "ObjectPtr already assigned." );
        return &ptr_;
    }

    ::SLObjectItf release()
    {
        ::SLObjectItf ptr( ptr_ );
        ptr_ = nullptr;
        return ptr;
    }

private:
    LE_COLD void LE_FASTCALL getInterface( ::SLInterfaceID const & interfaceID, void * const ppInterface ) const
    {
        BOOST_VERIFY( tryGetInterface( interfaceID, ppInterface ) );
    }

    LE_COLD bool LE_FASTCALL tryGetInterface( ::SLInterfaceID const & interfaceID, void * const ppInterface ) const
    {
        ::SLresult const result( obj().GetInterface( ptr_, interfaceID, ppInterface ) );
        BOOST_ASSERT_MSG( result == SL_RESULT_SUCCESS || result == SL_RESULT_FEATURE_UNSUPPORTED, "Unexpected OpenSL result" );
        return result == SL_RESULT_SUCCESS;
    }

    ::SLObjectItf_ const & LE_RESTRICT obj() const { BOOST_ASSERT( ptr_ ); return **ptr_; }

    ObjectPtr( ObjectPtr const & );

private:
    ::SLObjectItf ptr_;
}; // class ObjectPtr


struct BufferQueue
{
    BufferQueue() = default;
    BufferQueue( BufferQueue && other ) : 
        pBQ( other.pBQ ), oldestBuffer( other.oldestBuffer ), mutex( std::move( other.mutex ) ) { other.pBQ = nullptr; other.oldestBuffer = 0; }
    ~BufferQueue() { destroy(); }

    LE_COLD LE_NOINLINE
    void LE_FASTCALL registerCallback( ::slAndroidSimpleBufferQueueCallback const pCallback, void * const pContext )
    {
        BOOST_ASSERT( pBQ );
        LE_OSL_VERIFY( (*pBQ)->RegisterCallback( pBQ, pCallback, pContext ) );
    }

    LE_COLD LE_NOINLINE
    void LE_FASTCALL reset()
    {
        BOOST_ASSERT( pBQ );
        auto const result( (*pBQ)->Clear( pBQ ) );
        if ( result != SL_RESULT_SUCCESS )
        {
            /// \note Inexplicable (and, it seems, harmless) failures. When
            /// called (indirectly) from playStatusCallback() even the
            /// assertions below fail.
            /// https://www.assembla.com/code/android-gb-for-sharp-is01/git/nodes/debug/system/media/opensles/libopensles/android_AudioPlayer.cpp
            /// https://groups.google.com/forum/?fromgroups=#!topic/android-ndk/hLSygQrmcPI
            /// https://code.google.com/p/android/issues/detail?id=16110
            /// https://src.chromium.org/svn/trunk/src/media/audio/android/opensles_input.cc
            ///                               (30.12.2015.) (Domagoj Saric)
            BOOST_ASSERT( result == SL_RESULT_INTERNAL_ERROR );
          //BOOST_ASSERT( state().count == state().index );
          //BOOST_ASSERT( state().count == 0 );
          //BOOST_ASSERT( state().index == 0 );
          //BOOST_ASSERT( oldestBuffer  == 0 );
        }
        oldestBuffer = 0;
    }

    LE_NOTHROWNOALIAS LE_COLD
    SLAndroidSimpleBufferQueueState LE_FASTCALL state() const { return state( pBQ ); }

    LE_COLD
    bool LE_FASTCALL enqueue( sample_t const * const pData, std::uint16_t const length )
    {
        auto const result( (*pBQ)->Enqueue( pBQ, pData, length * sizeof( *pData ) ) );
        LE_ASSUME( result == SL_RESULT_SUCCESS || result == SL_RESULT_BUFFER_INSUFFICIENT );
        return result == SL_RESULT_SUCCESS;
    }

//private: ...mrmlj...properly encapsulate this...
    LE_NOTHROWNOALIAS LE_COLD
    static SLAndroidSimpleBufferQueueState LE_FASTCALL state( SLAndroidSimpleBufferQueueItf const pBQ )
    {
        BOOST_ASSERT( pBQ );
        SLAndroidSimpleBufferQueueState bqState;
        LE_OSL_VERIFY( (*pBQ)->GetState( pBQ, &bqState ) );
        return bqState;
    }

    LE_COLD LE_NOINLINE
    void LE_FASTCALL destroy()
    {
        if ( pBQ ) registerCallback( &nopCallback, nullptr );
        pBQ          = nullptr;
        oldestBuffer = 0      ;
    }

    static void nopCallback( ::SLAndroidSimpleBufferQueueItf const pBQ, void * /*const pContext*/ )
    {
    #ifdef NDEBUG
        (void)pBQ;
    #else
        auto const counts( state( pBQ ) );
        ::__android_log_print( ANDROID_LOG_INFO, Utility::Tracer::pTagString, "Draining orphaned OSL buffer (%p %u/%u).", pBQ, static_cast<unsigned int>( counts.index ), static_cast<unsigned int>( counts.count ) );
    #endif // !NDEBUG
    }

    using index_t = std::uint8_t;
    SLAndroidSimpleBufferQueueItf pBQ          = nullptr;
    index_t                       oldestBuffer = 0      ;
    /// \note Has to be recursive so that stop() can be called from the
    /// processing/callback thread (as both the callback function and stop() try
    /// to take the mutex).
    ///                                       (22.01.2016.) (Domagoj Saric)
    Utility::CriticalSection mutex;
}; // struct BufferQueue


class Engine : public ObjectPtr
{
private:
    LE_COLD
    Engine()
        :
    #ifndef NDEBUG
        pEngine_    ( nullptr ),
    #ifdef LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
        pThreadSync_( nullptr ),
    #endif // LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
    #endif // NDEBUG
        referenceCount_( 0 )
    {}

    LE_COLD ~Engine() = default;

public:
    ::SLEngineItf interface() const { BOOST_ASSERT( *this ); return pEngine_; }

    static ::SLEngineItf singletonInterface() { BOOST_ASSERT( singleton_.pEngine_ ); return singleton_.pEngine_; }

    using Ptr = boost::intrusive_ptr<Engine const>;

    static LE_NOTHROW LE_PURE_FUNCTION Ptr LE_FASTCALL singleton();

#ifdef LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
    using Mutex = Engine;
    using Lock  = std::lock_guard<Engine const>;
    void LE_COLD LE_FASTCALL   lock() const { LE_OSL_VERIFY( (*pThreadSync_)->EnterCriticalSection( pThreadSync_ ) ); }
    void LE_COLD LE_FASTCALL unlock() const { LE_OSL_VERIFY( (*pThreadSync_)-> ExitCriticalSection( pThreadSync_ ) ); }
#else
    /// \note A recursive mutex has to be used in order to allow stop() to
    /// be called from the callback thread (e.g. for BlockingDevice usage).
    ///                                       (05.03.2014.) (Domagoj Saric)
    using Mutex = Utility::CriticalSection;
    using Lock  = std::lock_guard<Mutex>;
    void LE_COLD LE_FASTCALL   lock() const { lock_.  lock(); }
    void LE_COLD LE_FASTCALL unlock() const { lock_.unlock(); }
    Mutex & mutex() const { return lock_; }
#endif // LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX

    static Mutex & singletonMutex() { BOOST_ASSERT( singleton_.pEngine_ ); return singleton_.mutex(); }

private:
    ::SLEngineItf pEngine_;
#ifdef LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
    ::SLThreadSyncItf pThreadSync_;
#else
    mutable Mutex lock_{ Utility::CriticalSection::NonRecursive };
#endif // LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX

private: // boost::intrusive_ptr required section
    friend void LE_NOTHROWNOALIAS LE_FASTCALL_ABI intrusive_ptr_add_ref( Engine const * LE_RESTRICT );
    friend void LE_NOTHROW        LE_FASTCALL_ABI intrusive_ptr_release( Engine const * LE_RESTRICT );

    mutable Utility::ReferenceCount referenceCount_;
    static Engine singleton_;
}; // class Engine


////////////////////////////////////////////////////////////////////////////////
///
/// \class DataConverter
///
/// \brief
///
////////////////////////////////////////////////////////////////////////////////

class DataConverter
{
public:
    DataConverter();

    bool LE_FASTCALL setup
    (
        std::uint8_t  numberOfChannels        ,
        std::uint8_t  numberOfBuffers         ,
        std::uint16_t numberOfSamplesPerBuffer
    );

    std::uint8_t  numberOfChannels        () const { return numberOfChannels_        ; }
    std::uint8_t  numberOfBuffers         () const { return numberOfBuffers_         ; }
    std::uint16_t numberOfSamplesPerBuffer() const { return numberOfSamplesPerBuffer_; }

    ::SLDataLocator_AndroidSimpleBufferQueue LE_COLD LE_FASTCALL makeBQ() const { return { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, numberOfBuffers_ }; }

    ::SLDataFormat_PCM LE_COLD LE_FASTCALL makeDataFormat( std::uint16_t const sampleRate ) const
    {
        return
        {
            SL_DATAFORMAT_PCM,
            numberOfChannels_,
            static_cast<unsigned int>( sampleRate * 1000 ), // in mHz
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            ( numberOfChannels_ == 1 ) ? SL_SPEAKER_FRONT_CENTER : ( SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT ),
            SL_BYTEORDER_LITTLEENDIAN
        };
    }

public: // Conversion helpers
    template <ChannelLayout channelLayout>
    LE_HOT void LE_FASTCALL convertOutputData( BufferQueue const & __restrict bq )
    {
        sample_t * LE_RESTRICT pInterleavedIntegerData( bufferPosition( bq ) );
        auto const numberOfChannels( numberOfChannels_ );
        if ( ( channelLayout == ChannelLayout::InterleavedChannels ) || ( numberOfChannels == 1 ) )
        {
            std::uint16_t const bufferSize( numberOfSamplesPerBuffer_ * numberOfChannels );
            Math::convertSamples( pFloatBuffer_.get(), pInterleavedIntegerData, bufferSize );
        }
        else
        {
            #pragma clang loop vectorize( enable ) interleave( enable )
            for ( std::uint16_t sample( 0 ); sample < numberOfSamplesPerBuffer_; ++sample )
                #pragma clang loop vectorize( enable ) interleave( enable )
                for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
                    Math::convertSample( separatedBuffers_[ channel ][ sample ], *pInterleavedIntegerData++ );
        }
    }

    template <ChannelLayout channelLayout>
    LE_HOT void LE_FASTCALL convertInputData( BufferQueue const & __restrict bq )
    {
        sample_t const * LE_RESTRICT pInterleavedIntegerData( bufferPosition( bq ) );
        auto const numberOfChannels( numberOfChannels_ );
        LE_ASSUME( numberOfChannels <= 2 );
        if ( ( channelLayout == ChannelLayout::InterleavedChannels ) || ( numberOfChannels == 1 ) )
        {
            std::uint16_t const bufferSize( numberOfSamplesPerBuffer_ * numberOfChannels );
            Math::convertSamples( pInterleavedIntegerData, pFloatBuffer_.get(), bufferSize );
        }
        else
        {
            #pragma clang loop vectorize( enable ) interleave( enable )
            for ( std::uint16_t sample( 0 ); sample < numberOfSamplesPerBuffer_; ++sample )
                #pragma clang loop vectorize( enable ) interleave( enable )
                for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
                    Math::convertSample( *pInterleavedIntegerData++, separatedBuffers_[ channel ][ sample ] );
        }
    }

    template <ChannelLayout> struct InternalBufferImpl;
    template <ChannelLayout channelLayout>
    typename InternalBufferImpl<channelLayout>::type internalBuffer() { return InternalBufferImpl<channelLayout>::get( &pFloatBuffer_[ 0 ], separatedBuffers_ ); }

    template <ChannelLayout channelLayout>
    void LE_FORCEINLINE cripple()
    {
    #ifdef LE_SDK_DEMO_BUILD
        ( channelLayout == ChannelLayout::InterleavedChannels )
            ? ( LE_AUDIOIO_CRIPPLE( pFloatBuffer_.get(), numberOfSamplesPerBuffer_ ) )
            : ( LE_AUDIOIO_CRIPPLE( separatedBuffers_  , numberOfSamplesPerBuffer_ ) );
    #endif // LE_SDK_DEMO_BUILD
    }

public: // BufferQueue helpers
    LE_COLD LE_NOINLINE void LE_FASTCALL enqueue( ::SLAndroidSimpleBufferQueueItf LE_RESTRICT const pBQ, BufferQueue::index_t & LE_RESTRICT bufferIndex )
    {
        BOOST_ASSERT_MSG( pBQ, "Accessing deleted BQ" );

        auto const bufferSize       ( numberOfSamplesPerBuffer_ * numberOfChannels_ );
        auto const bufferSizeInBytes( bufferSize * sizeof( pBuffer_[ 0 ] )          );
        auto const pBufferPosition  ( &pBuffer_[ bufferSize * bufferIndex ]         );

        //bufferIndex += 1;
        //bufferIndex %= numberOfBuffers_;
        bufferIndex = ( bufferIndex + 1 ) % numberOfBuffers_;

        LE_OSL_VERIFY( (*pBQ)->Enqueue( pBQ, pBufferPosition, bufferSizeInBytes ) );
    }

    LE_COLD LE_NOINLINE void LE_FASTCALL enqueue( BufferQueue & bq )
    {
        enqueue( bq.pBQ, bq.oldestBuffer );
    }

    sample_t * LE_FASTCALL bufferPosition( BufferQueue const & bq )
    {
        auto const bufferSize     ( numberOfSamplesPerBuffer_ * numberOfChannels_ );
        auto const pBufferPosition( &pBuffer_[ bufferSize * bq.oldestBuffer ]     );
        return pBufferPosition;
    }

    sample_t const * LE_FASTCALL bufferPosition( BufferQueue const & bq ) const { return const_cast<DataConverter &>( *this ).bufferPosition( bq ); }

private:
    std::uint8_t  numberOfChannels_        ;
    std::uint8_t  numberOfBuffers_         ;
    std::uint16_t numberOfSamplesPerBuffer_;

    std::unique_ptr<sample_t[]> pBuffer_     ;
    std::unique_ptr<float   []> pFloatBuffer_;
    float * LE_RESTRICT separatedBuffers_[ maximumNumberOfChannels ];
}; // class DataConverter

template <> struct DataConverter::InternalBufferImpl<ChannelLayout::SeparatedChannels>
{
    typedef Device::OutputSignal type;
    static type get( Device::InterleavedOutputSignal, Device::OutputSignal const pSeparated ) { return pSeparated; }
};
template <> struct DataConverter::InternalBufferImpl<ChannelLayout::InterleavedChannels>
{
    typedef Device::InterleavedOutputSignal type;
    static type get( Device::InterleavedOutputSignal const pInterleaved, Device::OutputSignal ) { return pInterleaved; }
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class Player
///
////////////////////////////////////////////////////////////////////////////////

class Player : public ObjectPtr
{
public:
    Player() : pInterface_( nullptr ) {}
    Player( Player && other )
        : ObjectPtr( std::move( other ) ), pInterface_( other.pInterface_ ), bq_( std::move( other.bq_ ) ) { other.pInterface_ = nullptr; }
    ~Player() { destroy(); }

    char const * LE_FASTCALL setup
    (
        SLDataSource,
        SLDataSink,
        SLInterfaceID const * pIDs,
        std::uint8_t numberOfIDs
    );

    void LE_FASTCALL destroy();

    LE_COLD void LE_FASTCALL start() { setState( SL_PLAYSTATE_PLAYING ); }
    LE_COLD void LE_FASTCALL stop()
    {
        // http://stackoverflow.com/questions/9871516/opensl-es-crashes-randomly-on-samsung-galaxy-sii-gt-i9100
        // (*bqPlayerSeek)->SetLoop(bqPlayerSeek, false, 0, SL_TIME_UNKNOWN);
        bq().reset(); // https://groups.google.com/forum/?fromgroups=#!topic/android-ndk/hLSygQrmcPI (point 3)
        setState( SL_PLAYSTATE_STOPPED );
        BOOST_ASSERT( state() == SL_PLAYSTATE_STOPPED );
        /// \note See the "Buffer queue behavior" section in the Android NDK
        /// OpenSL ES (HTML) documentation for an explanation as to why the BQ
        /// has to be reset here.
        ///                                   (24.04.2015.) (Domagoj Saric)
        bq().reset();
        // For the player BQ clear is supposed to work but it does not quite...
        //BOOST_ASSERT( bq().state().count == 0 );
        //BOOST_ASSERT( bq().state().index == 0 );
    }
    LE_COLD void LE_FASTCALL pause() { setState( SL_PLAYSTATE_PAUSED ); }

    /// \note Because the Player interface is fetched late, it is requested from
    /// multiple locations and so the appropriate code is wrapped in a function.
    /// See the note for the SeparatedInputOutput case in the recorderCallback()
    /// member function.
    ///                                       (16.01.2014.) (Domagoj Saric)
    LE_COLD LE_NOINLINE void LE_FASTCALL getInterface()
    {
        BOOST_ASSERT_MSG( pInterface_ == nullptr, "Player interface already acquired" );
        object().getInterface( SL_IID_PLAY, pInterface_ );
    }

    void clearInterface() //...mrmlj...
    {
        BOOST_ASSERT( pInterface_ == nullptr || state() == SL_PLAYSTATE_STOPPED );
        pInterface_ = nullptr;
    }

    bool interfaceFetched() const { return pInterface_ != nullptr; }

    SLPlayItf_ const * const & interface() const { BOOST_ASSERT_MSG( pInterface_, "Player not initialised." ); return *pInterface_; }

    LE_COLD ::SLuint32 LE_FASTCALL state() const
    {
        ::SLuint32 state;
        LE_OSL_VERIFY( interface()->GetPlayState( &interface(), &state ) );
        return state;
    }

    BufferQueue & bq() { return bq_; }

    explicit operator bool() const { return pInterface_ != nullptr; }

    LE_COLD void LE_FASTCALL setState( SLuint32 const state ) const
    {
        LE_OSL_VERIFY( interface()->SetPlayState( &interface(), state ) );
        /// \note At EOF, OSL immediately transitions back to the stopped state.
        ///                                   (13.04.2015.) (Domagoj Saric)
        BOOST_VERIFY
        (
            ( this->state() == state ) ||
            ( state == SL_PLAYSTATE_PLAYING && this->state() == SL_PLAYSTATE_STOPPED )
        );
    }

private:
    ObjectPtr & object() { return *this; }

private:
    SLPlayItf   pInterface_;
    BufferQueue bq_        ;
}; // class Player

//------------------------------------------------------------------------------
} // namespace OpenSL
//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // openSL_hpp
