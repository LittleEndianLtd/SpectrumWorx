////////////////////////////////////////////////////////////////////////////////
///
/// fileAndroid.cpp
/// ---------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
LE_OPTIMIZE_FOR_SIZE_BEGIN()

#include "file.hpp"

#include "le/audioio/openSL.hpp"

#include "le/math/vector.hpp"

#include "le/utility/conditionVariable.hpp"
#include "le/utility/trace.hpp"

#include "boost/utility/string_ref.hpp"

#include <android/api-level.h>
#include "android/asset_manager.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#if __ANDROID_API__ >= 14
#include <SLES/OpenSLES_AndroidMetadata.h>
#else
#define ANDROID_KEY_PCMFORMAT_NUMCHANNELS   "AndroidPcmFormatNumChannels"
#define ANDROID_KEY_PCMFORMAT_SAMPLERATE    "AndroidPcmFormatSampleRate"
#define ANDROID_KEY_PCMFORMAT_BITSPERSAMPLE "AndroidPcmFormatBitsPerSample"
#endif
#include "sys/types.h"

#ifndef NDEBUG
#include <android/log.h>
#endif // NDEBUG

#include <atomic>
#include <iterator>
#include <memory>
//------------------------------------------------------------------------------
struct AAssetManager;

/// \note malloc_usable_size() is no longer available in the NDK so we fallback
/// to the undocumented/internal alternative.
/// http://markmail.org/message/23eidnkrqkcxbb6q#query:+page:1+mid:23eidnkrqkcxbb6q+state:results
///                                           (25.01.2016.) (Domagoj Saric)
LE_NOTHROW LE_PURE_FUNCTION LE_NOINLINE __attribute__( ( weak ) ) extern "C"
size_t dlmalloc_usable_size( void const * const p ); //...mrmlj...ends up always calling this one...{ LE_TRACE( "dlmalloc_usable_size() no longer available!!" ); return SIZE_MAX; }
LE_NOTHROW LE_PURE_FUNCTION LE_NOINLINE __attribute__( ( weak ) ) extern "C"
size_t   malloc_usable_size( void const * const p ) { return dlmalloc_usable_size( p ); }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility { LE_NOTHROW LE_CONST_FUNCTION ::AAssetManager & LE_FASTCALL_ABI resourceManager(); }
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

// http://list-archives.org/2013/06/12/andraudio-music-columbia-edu/aac-decoding-on-android-from-2-1-to-4-2-with-low-latency/f/2212082755
// https://groups.google.com/forum/#!msg/android-ndk/SHpZOMhVVRM/HtiuykkiBFUJ

// https://code.google.com/p/aacdecoder-android
// http://keyj.emphy.de/minimp3

/// \note The weird StackPimpl reserved size difference for 64 bit Android
/// builds is because of different definitions of pthread structures.
///                                           (22.01.2016.) (Domagoj Saric)

class FileImpl
{
public:
    FileImpl() noexcept = default;
   ~FileImpl() noexcept { close(); }
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    LE_COLD
    FileImpl( FileImpl && other ) noexcept
        :
        /// \note Force a complete stop so that all the callbacks and queues can
        /// be reregistered.
        ///                                   (27.01.2016.) (Domagoj Saric)
        sampleRate_            ( ( other.stop(), other.sampleRate_ ) ),
        numberOfChannels_      ( other.numberOfChannels_             ),
        finished_              ( false                               ),
        pOSLEngine_            ( std::move( other.pOSLEngine_    )   ),
        player_                ( std::move( other.player_        )   ),
        bufferFilled_          ( std::move( other.bufferFilled_  )   ),
        bufferWaiting_         ( std::move( other.bufferWaiting_ )   ),
        pBlockBuffer_          ( std::move( other.pBlockBuffer_  )   ),
        blockSize_             ( other.blockSize_                    ),
        unreadBlockBufferBegin_( other.unreadBlockBufferBegin_       ),
        unreadBlockBufferEnd_  ( other.unreadBlockBufferEnd_         ),
        extractCallbackActive  ( false                               )
    #if __ANDROID_API__ < 18
       ,brokenMonoPlayback_    ( other.brokenMonoPlayback_           )
    #endif // __ANDROID_API__ < 18
    #if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
       ,position_              ( other.position_                     )
    #endif // !NDEBUG && !LE_PUBLIC_BUILD
    {
        LE_ASSUME( other.finished_             == false );
        LE_ASSUME( other.extractCallbackActive == false );
        BOOST_ASSERT( !other.player_ );
        if ( player_ )
        {
            BOOST_ASSERT( player_.state() == SL_PLAYSTATE_STOPPED );
            auto & bq    ( player_.bq       () );
            auto & player( player_.interface() );
            bq.registerCallback( &extractDataCallback, this );
            LE_OSL_VERIFY( player->RegisterCallback( &player, &playStatusCallback, this ) );
            //player_.start(); //...mrmlj...try to make it work w/o restarting
            restart();
        }
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

    template <Utility::SpecialLocations parentDirectory>
    char const * LE_COLD open( char const * const fileName )
    {
        // (requires the INTERNET permission depending on the URI parameter)
        // http://grokbase.com/t/gg/android-ndk/121naq22vj/opensl-in-ics-does-not-support-playbackrate-for-uriplayers

        /// \note Getting WVMExtractor errors ("Failed to open libwvm.so") in
        /// the LogCat for WAVE files here seems expected:
        /// http://e2e.ti.com/support/other_analog/touch/f/750/t/209724.aspx
        /// http://e2e.ti.com/support/embedded/android/f/509/t/209880.aspx
        /// https://groups.google.com/forum/#!topic/android-building/3H4ftVG9N9c/discussion
        ///                                   (07.05.2014.) (Domagoj Saric)
        SLDataLocator_URI const uri
        {
            SL_DATALOCATOR_URI,
            reinterpret_cast<SLchar *>( const_cast<char *>
            (
                Utility::fullPath<parentDirectory>( fileName )
            ))
        };
        return open( &uri, fileName );
    }

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    LE_NOTHROW LE_COLD
    char const * LE_FASTCALL open( void const * const pLocator, char const * const fileName )
    {
        if ( BOOST_UNLIKELY( !( pOSLEngine_ || ( pOSLEngine_ = OpenSL::Engine::singleton() ) ) ) )
            return "Out of memory";

        close();

        // http://stackoverflow.com/questions/19062499/playing-pcm-wave-sound-from-memory-with-opensl-on-android?rq=1
        SLDataFormat_MIME constexpr formatMIME{ SL_DATAFORMAT_MIME, nullptr, SL_CONTAINERTYPE_UNSPECIFIED };
        SLDataSource      const     audioSrc  { const_cast<void *>( pLocator ), const_cast<SLDataFormat_MIME *>( &formatMIME ) };

        // configure audio sink
        // https://groups.google.com/forum/#!topic/Android-ndk/JPKFcJ1p9vw PCM format when decoding to buffer with OpenSL ES
        // http://stackoverflow.com/questions/19827673/opensl-es-decode-24bit-flac
        // http://grokbase.com/t/gg/android-ndk/123vyf8r96/sample-code-to-decode-audio-with-opensl-es-to-androidsimplebufferqueue
        // ...mrmlj...supported only since Android 4.0...
        // http://stackoverflow.com/questions/10196361/how-to-check-the-device-running-api-level-using-c-code-via-ndk
        static SLDataLocator_AndroidSimpleBufferQueue constexpr bqSink   { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, bqBuffers };
        static SLDataFormat_PCM                       constexpr formatPCM
        {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
        };
        static SLDataSink constexpr audioSnk{ const_cast<SLDataLocator_AndroidSimpleBufferQueue *>( &bqSink ), const_cast<SLDataFormat_PCM *>( &formatPCM ) };

        SLInterfaceID const ids[] = { SL_IID_ANDROIDSIMPLEBUFFERQUEUE, SL_IID_PLAY, SL_IID_METADATAEXTRACTION, SL_IID_SEEK, SL_IID_PREFETCHSTATUS };
        auto const pError( player_.setup( audioSrc, audioSnk, ids, _countof( ids ) - !usePrefetch ) );
        if ( BOOST_UNLIKELY( pError != nullptr ) )
            return pError;

        auto & bq( player_.bq() );

        if ( usePrefetch )
        {
            // https://android.googlesource.com/platform/frameworks/wilhelm/+/idea133/tests/examples/slesTestDecodeToBuffQueue.cpp
            // https://android.googlesource.com/platform/frameworks/wilhelm/+/idea133/tests/examples/slesTestDecodeAac.cpp
            SLPrefetchStatusItf pPrefetch;
            player_.OpenSL::ObjectPtr::getInterface( SL_IID_PREFETCHSTATUS, pPrefetch );
            bool volatile prefetchError( false );
            LE_OSL_VERIFY
            (
                (*pPrefetch)->RegisterCallback
                (
                    pPrefetch,
                    []( SLPrefetchStatusItf const caller, void * const pContext, SLuint32 const event )
                    {
                        auto & error( *static_cast<bool *>( pContext ) );
                        error = false;
                        ::SLpermille level ; LE_OSL_VERIFY( (*caller)->GetFillLevel     ( caller, &level  ) );
                        ::SLuint32   status; LE_OSL_VERIFY( (*caller)->GetPrefetchStatus( caller, &status ) );
                    #ifndef LE_PUBLIC_BUILD
                        LE_TRACE( "prefetch callback level %d, status %d", level, status );
                    #endif // LE_PUBLIC_BUILD
                        if ( ( level == 0 ) && ( status == SL_PREFETCHSTATUS_UNDERFLOW ) )
                             error = true;
                    },
                    const_cast<bool *>( &prefetchError )
                )
            );
            LE_OSL_VERIFY( (*pPrefetch)->SetCallbackEventsMask( pPrefetch, SL_PREFETCHEVENT_STATUSCHANGE | SL_PREFETCHEVENT_FILLLEVELCHANGE ) );
            LE_OSL_VERIFY( (*pPrefetch)->SetFillUpdatePeriod  ( pPrefetch, 1                                                                ) );

            // Prefetch the data so we can get information about the format
            // before starting to decode:
            // - cause the player to prefetch the data
            player_.getInterface(); //...mrmlj...
            player_.pause();
            // - block until data has been prefetched
            SLuint32 prefetchStatus;
            while
            (
                ( LE_OSL_VERIFY( (*pPrefetch)->GetPrefetchStatus( pPrefetch, &prefetchStatus ) ), prefetchStatus )
                    ==
                SL_PREFETCHSTATUS_UNDERFLOW
            )
            {
                if ( prefetchError )
                {
                    LE_TRACE( "Prefetch status %d", prefetchStatus );
                    return "Error decoding file header";
                }
                BOOST_VERIFY( sched_yield() == 0 );
            }

            extractMetaData();
            BOOST_ASSERT( sampleRate_       );
            BOOST_ASSERT( numberOfChannels_ );
            player_.stop();
            LE_OSL_VERIFY( (*pPrefetch)->SetCallbackEventsMask( pPrefetch, 0                ) );
            LE_OSL_VERIFY( (*pPrefetch)->RegisterCallback     ( pPrefetch, nullptr, nullptr ) );
            // experiment: recreate the player w/o the prefetch interface to
            // avoid the bugs associated with it:
            if ( false ) // causes timeouts in the read() function...
            {
                // Assertion from OpenSL code
                // https://groups.google.com/forum/#!topic/android-ndk/F9IKyyVOuy4
                // https://code.google.com/p/android/issues/detail?id=27956
                player_.destroy();
                auto const pError( player_.setup( audioSrc, audioSnk, ids, _countof( ids ) - 1 ) );
                if ( BOOST_UNLIKELY( pError != nullptr ) )
                    return pError;
                player_.getInterface(); //...mrmlj...
            }
        }
        else
        {
            bq.registerCallback
            (
                []( ::SLAndroidSimpleBufferQueueItf /*const pBQ*/, void * const pContext ) { static_cast<FileImpl *>( pContext )->notifyCallbackWaiter(); },
                this
            );
            bufferFilled_.wait_enter( mutex() );
            OpenSL::sample_t dummy;
            BOOST_VERIFY( bq.enqueue( &dummy, 1 ) );
            player_.getInterface(); //...mrmlj...
            player_.start();
            bufferFilled_.wait     ( mutex() );
            bufferFilled_.wait_exit( mutex() );
            extractMetaData();
            BOOST_ASSERT( sampleRate_       );
            BOOST_ASSERT( numberOfChannels_ );
            restart();
        }
        bq.registerCallback( &extractDataCallback, this );

        {
            auto & player( player_.interface() );
            LE_OSL_VERIFY( player->RegisterCallback     ( &player, &playStatusCallback, this ) );
            LE_OSL_VERIFY( player->SetCallbackEventsMask( &player, SL_PLAYEVENT_HEADATEND    ) );
        }

        LE_ASSUME( !extractCallbackActive );

        finished_ = false;

        // http://android.2317887.n4.nabble.com/OpenSL-File-decoding-to-arbitratry-sized-buffers-td309104.html
        boost::string_ref const extension( std::strrchr( fileName, '.' ) + 1 );
        std::uint16_t blockSize;
             if ( extension == "mp3"                       ) blockSize = mp3BlockSize    ;
        else if ( extension == "aac" || extension == "m4a" ) blockSize = aacBlockSize    ;
        else                                                 blockSize = genericBlockSize;
    #if __ANDROID_API__ < 18
        if
        (
            ( numberOfChannels() >  1                ) ||
            ( blockSize          == genericBlockSize ) ||
            ( apiLevel()         >= 18               )
        )
            brokenMonoPlayback_ = MonoPlayback::NotBroken;
        else
            brokenMonoPlayback_ = MonoPlayback::NotChecked;
    #endif // __ANDROID_API__ < 18
        blockSize *= numberOfChannels();
        blockSize *= 2; //...mrmlj...use larger buffers to enable/cause more prefetching (to reduce hickups in low-latency realtime playback)

        /// \note We kept getting weird errors/lockups/crashes here with the
        /// following logcat entry:
        /// I/mtk_dlmalloc_debug: [DEBUG_INFO]FUNCTION try_realloc_chunk Line 6268 address 4f5166f0 function 43 action 4100 structure type 4 error_member 20 mstate 0 DEBUG bed602c0
        /// in the realloc() call upon reusing a File object (i.e. calling
        /// open() more than once even with the same file) even though no
        /// heap-corrupting write-access could be found and adding an explicit
        /// free() before realloc() seemed to 'fix' the problem. As a workaround
        /// we now perform a manual size check and call realloc only if
        /// necessary.
        ///                                   (25.01.2016.) (Domagoj Saric)
        std::uint32_t const requiredStorage( bqBuffers * blockSize * sizeof( pBlockBuffer_[ 0 ] ) );
        if ( BOOST_UNLIKELY( blockSize != blockSize_ ) )
        {
            auto const newStorage( static_cast<OpenSL::sample_t *>( std::realloc( pBlockBuffer_.get(), requiredStorage ) ) );
            if ( !newStorage )
                return "Out of memory";
            pBlockBuffer_.reset( newStorage );
            blockSize_ = blockSize;
        }
        BOOST_ASSERT( ::malloc_usable_size( pBlockBuffer_.get() ) >= requiredStorage );

        restart();
        BOOST_ASSERT( player_.state() == SL_PLAYSTATE_PLAYING );

        return nullptr;
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

    LE_COLD //...mrmlj...clean this up...defensive/paranoid hacks to cause OSL to actually 'cease and desist' so that destructors and move-constructors work 'as expected'...
    void stop()
    {
        if ( player_ )
        {
            //...mrmlj...we have to make a 'full stop' (with bq reset) otherwise queued buffers will be called with the old player instance after moving
            player_.stop();
            finished_ = true;
            bufferWaiting_.notify();

            while ( extractCallbackActive ) // recheck if this is necessary (with the below draining)...
            {
                bufferWaiting_.notify();
                BOOST_VERIFY( sched_yield() == 0 );
            }
            {
                auto & bq( player_.bq() );
                bq.registerCallback( &OpenSL::BufferQueue::nopCallback, nullptr );
                player_.start();
                while ( bq.state().count != 0 )
                    BOOST_VERIFY( sched_yield() == 0 );
                player_.stop();
                BOOST_ASSERT( bq.state().count == 0 );
                BOOST_ASSERT( bq.oldestBuffer  == 0 );
                bq.registerCallback( &extractDataCallback, this );
            }
            finished_ = false; //...mrmlj...
        }
        BOOST_ASSERT_MSG( !extractCallbackActive, "OpenSL stopping incomplete." );
    }

    LE_COLD void close()
    {
        stop();
        player_.destroy();
        BOOST_ASSERT_MSG( !extractCallbackActive, "OpenSL stopping incomplete." );
    }

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    static LE_COLD void SLAPIENTRY playStatusCallback( SLPlayItf const pPlayer, void * const pContext, SLuint32 const event )
    {
        // Threading issues...
        // https://code.google.com/p/android/issues/detail?id=80436
        // https://groups.google.com/forum/#!topic/android-ndk/zANdS2n2cQI
        // (In)accurate detection of position/actually read samples...
        // http://grokbase.com/t/gg/android-ndk/122awrq4zd/decoding-to-buffer-with-opensl-es
        // https://groups.google.com/forum/#!topic/android-ndk/7YaqlKlDMO4
        // https://groups.google.com/forum/#!topic/android-ndk/tgrNuGdYJpg SL_PLAYEVENT_HEADATEND delivered too soon
        // http://www.gamedev.net/topic/655115-opensl-getposition
        // https://groups.google.com/forum/#!msg/android-ndk/Ez5_nvAmhE8/Xge_p3FY0t0J
        // https://groups.google.com/forum/#!topic/android-ndk/9bEfFjg4WXs
        LE_ASSUME( event == SL_PLAYEVENT_HEADATEND );
        //SL_PLAYEVENT_HEADATNEWPOS
        //SL_PLAYEVENT_HEADATMARKER
        auto & LE_RESTRICT file( *static_cast<FileImpl *>( pContext ) );
        LE_ASSUME( &file.player_.interface() == pPlayer );
        file.finished_ = true;
    #if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
        auto const state( file.player_.bq().state() );
        LE_TRACE( "OSL SL_PLAYEVENT_HEADATEND, count %d, index %d, oldest buffer %d.", state.count, state.index, static_cast<std::uint8_t>( file.player_.bq().oldestBuffer ) );
    #endif // !NDEBUG && !LE_PUBLIC_BUILD
        file.player_.stop();
        file.notifyCallbackWaiter();
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

    struct MetaDataExtractor
    {
        MetaDataExtractor( OpenSL::Player & player ) { player.OpenSL::ObjectPtr::getInterface( SL_IID_METADATAEXTRACTION, pMetadata_ ); }

    	LE_COLD std::uint8_t LE_FASTCALL itemCount() const
        {
            SLuint32 mdCount;
            LE_OSL_VERIFY( (*pMetadata_)->GetItemCount( pMetadata_, &mdCount ) );
            return static_cast<std::uint8_t>( mdCount );
        }

        template <typename T>
        LE_COLD T LE_FASTCALL value( SLuint32 const index ) const
        {
            // http://stackoverflow.com/questions/21182307/how-does-gcc-implement-variable-length-arrays
            unsigned int constexpr storageSize( sizeof( SLMetadataInfo ) /*...mrmlj...4.4.2 emulator...- alignof( SLMetadataInfo ) or sizeof( SLMetadataInfo::data )*/ + sizeof( T ) );
            alignas( alignof( SLMetadataInfo ) ) char storage[ storageSize ];
            SLMetadataInfo & info( *new ( storage ) SLMetadataInfo );
            value( index, storageSize, info );
            BOOST_ASSERT( info.size     == sizeof( T )                 );
            BOOST_ASSERT( info.encoding == SL_CHARACTERENCODING_BINARY );
            T result;
            std::memcpy( &result, info.data, sizeof( result ) );
            return result;
        }

        LE_COLD void LE_FASTCALL value( SLuint32 const index, SLuint32 const infoSize, SLMetadataInfo & info ) const
        {
        #ifndef NDEBUG
            SLuint32 valueSize;
            LE_OSL_VERIFY( (*pMetadata_)->GetValueSize( pMetadata_, index, &valueSize ) );
            BOOST_ASSERT( valueSize <= infoSize );
        #endif // NDEBUG
            LE_OSL_VERIFY( (*pMetadata_)->GetValue( pMetadata_, index, infoSize, &info ) );
        }

        SLMetadataExtractionItf pMetadata_;
    }; // struct MetaDataExtractor

    LE_COLD void LE_FASTCALL extractMetaData()
    {
        // https://android.googlesource.com/platform/frameworks/wilhelm/+/master/tests/examples/slesTestDecodeToBuffQueue.cpp
        //BOOST_ASSERT_MSG( !pMetadata_, "Metadata already extracted" );

        BOOST_ASSERT( !extractCallbackActive ); extractCallbackActive = true;

        MetaDataExtractor metaData( player_ );

        boost::string_ref const bitsPerSampleKey   ( ANDROID_KEY_PCMFORMAT_BITSPERSAMPLE );
        boost::string_ref const numberOfChannelsKey( ANDROID_KEY_PCMFORMAT_NUMCHANNELS   );
        boost::string_ref const sampleRateKey      ( ANDROID_KEY_PCMFORMAT_SAMPLERATE    );

        std::uint8_t  numberOfChannels( 0 );
        std::uint32_t sampleRate      ( 0 );

        auto const mdCount( metaData.itemCount() );
        for ( std::uint8_t item( 0 ); item < mdCount; ++item )
        {
            SLuint32 keySize;
            LE_OSL_VERIFY( (*metaData.pMetadata_)->GetKeySize( metaData.pMetadata_, item, &keySize ) );
            alignas( alignof( SLuint32 ) ) char keyStorage[ keySize ];
            auto & __restrict key( *reinterpret_cast<SLMetadataInfo *>( keyStorage ) );
            LE_OSL_VERIFY( (*metaData.pMetadata_)->GetKey( metaData.pMetadata_, item, keySize, &key ) );

            BOOST_ASSERT
            (
                ( key.encoding == SL_CHARACTERENCODING_ASCII ) ||
                ( key.encoding == SL_CHARACTERENCODING_UTF8  )
            );
            auto const keyName( reinterpret_cast<char const *>( key.data ) );
                 if ( keyName == numberOfChannelsKey ) { numberOfChannels = static_cast<std::uint8_t >( metaData.value<SLuint32>( item ) ); }
            else if ( keyName == sampleRateKey       ) { sampleRate       = static_cast<std::uint32_t>( metaData.value<SLuint32>( item ) ); }
            else if ( keyName == bitsPerSampleKey    ) { BOOST_ASSERT_MSG( metaData.value<SLuint32>( item ) == 16, "Unexpected bit depth" ); }

            if ( numberOfChannels && sampleRate )
                break;
        }
        BOOST_ASSERT( numberOfChannels && sampleRate );

        sampleRate_       = sampleRate      ;
        numberOfChannels_ = numberOfChannels;

        BOOST_ASSERT( extractCallbackActive ); extractCallbackActive = false;
    }

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    static void SLAPIENTRY extractDataCallback( ::SLAndroidSimpleBufferQueueItf const pBQ, void * const pContext )
    {
        auto & LE_RESTRICT file( *static_cast<FileImpl *>( pContext ) );
        BOOST_ASSERT( file.player_ );
        LE_ASSUME( file.player_.bq().pBQ == pBQ );
        file.extractData();
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

    LE_FORCEINLINE
    void extractData()
    {
        BOOST_ASSERT( !extractCallbackActive ); extractCallbackActive = true;

        // http://stackoverflow.com/questions/19827673/opensl-es-decode-24bit-flac

    #if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
        auto const startPosition( position_ );
        /// \note Because of OpenSL's block-based decoding and API deficiencies,
        /// the position delta is not a reliable source for calculating the
        /// number of actually read samples (it is sometimes even zero).
        /// Check the links in playStatusCallback() for more information.
        /// http://www.gamedev.net/topic/655115-opensl-getposition
        ///                                   (22.07.2014.) (Domagoj Saric)
        auto const endPosition              ( position()                                          );
        auto       reportedReadSamplesFrames( milliseconds2samples( endPosition - startPosition ) );
        auto const expectedReadSampleFrames ( blockSize_ / numberOfChannels()                     );
        if ( BOOST_UNLIKELY( reportedReadSamplesFrames > expectedReadSampleFrames ) )
        {
            LE_TRACE( "OpenSL audio file decoder reported decoding more samples than requested (%d vs %d)", reportedReadSamplesFrames, expectedReadSampleFrames );
            reportedReadSamplesFrames = blockSize_;
        }
        else LE_TRACE( "OpenSL audio file decoder reported decoding %d vs %d", reportedReadSamplesFrames, expectedReadSampleFrames );
        position_ = endPosition;
    #endif // !NDEBUG && !LE_PUBLIC_BUILD

        BOOST_ASSERT( unreadBlockBufferEnd_ % blockSize_ == 0 );
        unreadBlockBufferEnd_ = ( unreadBlockBufferEnd_ + blockSize_ ) % ( bqBuffers * blockSize_ );
        BOOST_ASSERT( unreadBlockBufferEnd_ % blockSize_ == 0 );

        bufferFilled_.notify();

        /// \note The non-conforming Android OpenSL implementation can actually
        /// decode input files in the background while no buffer has been queued
        /// and silently discard the decoded data. For this reason we have to
        /// block the callback, prevent it from returning, until a new buffer
        /// has been queued.
        ///                                   (14.05.2015.) (Domagoj Saric)

        auto & bq   ( player_.bq()  );
        auto & mutex( this->mutex() );
        while ( BOOST_LIKELY( !finished_ ) )
        {
            std::uint8_t const blockToEnqueue( bq.oldestBuffer                      );
            std::uint8_t const blockInUse    ( unreadBlockBufferBegin_ / blockSize_ );

            if ( blockToEnqueue != blockInUse )
            {
                BOOST_ASSERT
                (
                    unreadBlockBufferBegin_ <    bq.oldestBuffer       * blockSize_ ||
                    unreadBlockBufferBegin_ >= ( bq.oldestBuffer + 1 ) * blockSize_
                );
                BOOST_ASSERT( blockToEnqueue < bqBuffers );
                BOOST_VERIFY( bq.enqueue( &pBlockBuffer_[ blockToEnqueue * blockSize_ ], blockSize_ ) );
                bq.oldestBuffer = ( blockToEnqueue + 1 ) % bqBuffers;
                //...mrmlj...player_.start();
                break;
            }
            //...mrmlj...player_.pause();
            bufferWaiting_.wait_enter( mutex );
            bufferWaiting_.wait      ( mutex );
            bufferWaiting_.wait_exit ( mutex );
        }

    #if 0
        while
        (
            BOOST_LIKELY( !finished_ ) &&
            !bq.enqueue( &pBlockBuffer_[ bq.oldestBuffer * blockSize_ ], blockSize_ )
        ) {}
    #endif

        BOOST_ASSERT( extractCallbackActive ); extractCallbackActive = false;
    }

    std::uint32_t LE_FASTCALL read( float * __restrict pOutput, std::uint32_t sampleFrames ) const
    {
        BOOST_ASSERT_MSG( player_, "No file opened" );

        auto & mutex( this->mutex() );
        std::uint32_t framesRead( 0 );
        std::uint16_t volatile const & __restrict unreadBlockBufferEnd( unreadBlockBufferEnd_ );
        BOOST_ASSERT( unreadBlockBufferEnd % blockSize_ == 0 );
        while ( BOOST_LIKELY( !finished_ ) )
        {
            while
            (
                BOOST_UNLIKELY( unreadBlockBufferBegin_ == unreadBlockBufferEnd ) &&
                BOOST_LIKELY  ( !finished_                                      )
            )
            {
                bufferFilled_.wait_enter( mutex );
            #if __ANDROID_API__ >= 17 // assume this bug has been ironed out
                bufferFilled_.wait( mutex );
            #else
                /// \note Some AAC files fail to load on Sony Xperia Mini
                /// (Android 4.0.4) phones w/o an actual OpenSL error being
                /// reported (an error only appears in the LogCat). The result
                /// of that being that the BQ callback is never called (and in
                /// turn bufferFilled_ is never signaled) so we use a timed wait
                /// here to avoid a deadlock.
                ///                           (14.05.2015.) (Domagoj Saric)
                if ( !bufferFilled_.wait( mutex, 2000 ) )
                {
                    LE_TRACE( "AudioIO::File error: timed out waiting for decoder." );
                    bufferFilled_.wait_exit( mutex );
                    return framesRead;
                }
            #endif
                bufferFilled_.wait_exit( mutex );
            }
            BOOST_ASSERT( unreadBlockBufferBegin_ != unreadBlockBufferEnd  || finished_ );
            BOOST_ASSERT( unreadBlockBufferBegin_ != bqBuffers * blockSize_             );
            auto const continuousBlockUnread
            (
                std::min<std::uint16_t>
                (
                    unreadBlockBufferEnd   - unreadBlockBufferBegin_,
                    bqBuffers * blockSize_ - unreadBlockBufferBegin_
                )
            );
        #if __ANDROID_API__ >= 18 // assume this bug has been ironed out
            bool constexpr brokenMonoPlayback( false );
        #else // __ANDROID_API__ < 18
            bool const brokenMonoPlayback( this->brokenMonoPlayback( &pBlockBuffer_[ unreadBlockBufferBegin_ ] ) );
        #endif
            if ( brokenMonoPlayback )
            {
                LE_ASSUME( continuousBlockUnread % 2 == 0 );
                const_cast<std::uint16_t &>( continuousBlockUnread ) /= 2;
            }
            auto const startingBuffer( unreadBlockBufferBegin_ / blockSize_ );
            auto const sizeToCopy    ( static_cast<std::uint16_t>( std::min<std::uint32_t>( continuousBlockUnread, sampleFrames * numberOfChannels() ) ) );
            BOOST_ASSERT( unreadBlockBufferBegin_ + sizeToCopy <= bqBuffers * blockSize_ );
            if ( !brokenMonoPlayback )
            {
                Math::convertSamples( &pBlockBuffer_[ unreadBlockBufferBegin_ ], pOutput, sizeToCopy );
                unreadBlockBufferBegin_ += sizeToCopy;
            }
            else
            {
            #if 0 // Clang 3.6 cannot vectorize this one
                auto samples( sizeToCopy );
                auto const * __restrict pLocalInput ( &pBlockBuffer_[ unreadBlockBufferBegin_ ] );
                auto       * __restrict pLocalOutput( pOutput                                   );
                #pragma clang loop vectorize( enable ) interleave( enable )
                while ( samples-- )
                {
                    Math::convertSample( *pLocalInput++, *pLocalOutput++ );
                    pLocalInput++; // skip duplicate sample
                }
            #else
                // The lower 16bits will be discarded anyway when we convert
                // back to shorts so this hack is safe.
                Math::convertSamples( reinterpret_cast<std::int32_t const *>( &pBlockBuffer_[ unreadBlockBufferBegin_ ] ), pOutput, sizeToCopy );
            #endif
                // unreadBlockBufferBegin_ has to be updated with what we
                // actually consumed (even if half was discarded).
                unreadBlockBufferBegin_ += ( sizeToCopy * 2 );
            }
            LE_AUDIOIO_CRIPPLE( pOutput, sizeToCopy );
            pOutput      += sizeToCopy;
            framesRead   += sizeToCopy / numberOfChannels();
            sampleFrames -= sizeToCopy / numberOfChannels();
            LE_ASSUME( unreadBlockBufferBegin_ <= bqBuffers * blockSize_ );
            if ( unreadBlockBufferBegin_ == bqBuffers * blockSize_ )
                unreadBlockBufferBegin_ = 0;
            auto const endingBuffer( unreadBlockBufferBegin_ / blockSize_ );
            BOOST_ASSERT( startingBuffer < bqBuffers );
            BOOST_ASSERT( endingBuffer   < bqBuffers );
            if ( endingBuffer != startingBuffer )
            {
                bufferWaiting_.notify();
            }
            if ( !sampleFrames )
            {
                break;
            }
            BOOST_ASSERT( unreadBlockBufferEnd % blockSize_ == 0 );
        }
        return framesRead;
    }
    LE_COLD LE_NOTHROW
    void LE_FASTCALL notifyCallbackWaiter()
    {
        auto & mutex( this->mutex() );
        //...mrmlj...ugh...clean this up...
        bufferFilled_ .notify_enter( mutex );
        bufferFilled_ .notify      (       );
        bufferFilled_ .notify_exit ( mutex );
        bufferWaiting_.notify_enter( mutex );
        bufferWaiting_.notify      (       );
        bufferWaiting_.notify_exit ( mutex );
    }

    std::uint8_t  numberOfChannels     () const { BOOST_ASSERT( numberOfChannels_ ); return numberOfChannels_; }
    std::uint16_t sampleRate           () const { BOOST_ASSERT( sampleRate_       ); return sampleRate_      ; }
    std::uint32_t remainingSampleFrames() const { return milliseconds2samples( duration() - position() ); }
    std::uint32_t lengthInSampleFrames () const { return milliseconds2samples( duration()              ); }

    LE_COLD LE_NOTHROW
    void LE_FASTCALL setTimePosition( std::uint32_t const positionInMilliseconds )
    {
        // http://stackoverflow.com/questions/18390857/opensl-poor-seeking-with-audio-player-object?rq=1
        SLSeekItf pSeek;
        player_.OpenSL::ObjectPtr::getInterface( SL_IID_SEEK, pSeek );
        LE_OSL_VERIFY( (*pSeek)->SetPosition( pSeek, positionInMilliseconds, SL_SEEKMODE_ACCURATE ) );
        finished_ = false;
    }
    LE_COLD LE_NOTHROW
    void LE_FASTCALL setSamplePosition( std::uint32_t const positionInSampleFrames ) { setTimePosition( samples2milliseconds( positionInSampleFrames ) ); }

    LE_COLD LE_NOTHROW std::uint32_t LE_FASTCALL getTimePosition  () const { return position(); }
    LE_COLD LE_NOTHROW std::uint32_t LE_FASTCALL getSamplePosition() const { return milliseconds2samples( getTimePosition() ); }

    LE_COLD void LE_FASTCALL restart()
    {
        // allow any blocked bq callback to exit so that the bq can be reset
        // (doesn't seem to work)
        finished_ = true;
        bufferWaiting_.notify();
        player_.stop();
        setTimePosition( 0 );
        BOOST_ASSERT( player_.state() == SL_PLAYSTATE_STOPPED );
      //BOOST_ASSERT( position()      == 0                    );!?
        finished_               = false;
    #if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
        position_               = 0;
    #endif // !NDEBUG && !LE_PUBLIC_BUILD
        unreadBlockBufferBegin_ = 0;
        unreadBlockBufferEnd_   = 0;
        auto & bq( player_.bq() );
        bq.oldestBuffer = 0;
        for ( std::uint8_t bqBuffer( 0 ); bqBuffer < bqBuffers; ++bqBuffer )
        {
            // https://groups.google.com/forum/?fromgroups=#!topic/android-ndk/hLSygQrmcPI (point 3)
            if ( /*BOOST_VERIFY*/( bq.enqueue( &pBlockBuffer_[ bqBuffer * blockSize_ ], blockSize_ ) ) )
                bq.oldestBuffer = ( bq.oldestBuffer + 1 ) % bqBuffers;
        }
        player_.start();
    }

    explicit operator bool() const { return player_; }

private:
    LE_COLD SLmillisecond LE_FASTCALL duration() const
    {
        SLmillisecond result( 0 );
        LE_OSL_VERIFY( player_.interface()->GetDuration( &player_.interface(), &result ) );
        BOOST_ASSERT( result != SL_TIME_UNKNOWN );
        BOOST_ASSERT( result != 0               );
        return result;
    }

    LE_COLD SLmillisecond LE_FASTCALL position() const
    {
        SLmillisecond result;
        LE_OSL_VERIFY( player_.interface()->GetPosition( &player_.interface(), &result ) );
        BOOST_ASSERT( result != SL_TIME_UNKNOWN );
        BOOST_ASSERT( result >= 0               );
        BOOST_ASSERT( result <= duration() /*...mrmlj...aac*/ || finished_ /*|| !position_*/ || blockSize_ == aacBlockSize * numberOfChannels() );
        return result;
    }

    LE_COLD std::uint32_t LE_FASTCALL milliseconds2samples( SLmillisecond const milliseconds ) const
    {
        return static_cast<std::uint32_t>( std::uint64_t( milliseconds ) * sampleRate() / 1000 );
    }
    LE_COLD SLmillisecond LE_FASTCALL samples2milliseconds( std::uint32_t const sampleFrames ) const
    {
        return static_cast<std::uint32_t>( std::uint64_t( sampleFrames ) * 1000 / sampleRate() );
    }

    Utility::CriticalSection & mutex() const { return player_.bq().mutex; }

#if __ANDROID_API__ < 18
    /// \note Some older phones/OS builds silently convert compressed mono files
    /// to stereo (causing them to be played at half speed).
    /// https://code.google.com/p/chromium/issues/detail?id=266006
    /// https://code.google.com/p/chromium/issues/detail?id=256851
    ///                                       (09.02.2016.) (Domagoj Saric)
    bool brokenMonoPlayback( OpenSL::sample_t const * __restrict const pData ) const
    {
        if ( BOOST_UNLIKELY( brokenMonoPlayback_ == MonoPlayback::NotChecked ) )
        {
            if
            (
                ( pData[ 0 ] == pData[ 1 ] ) &&
                ( pData[ 2 ] == pData[ 3 ] ) &&
                ( pData[ 4 ] == pData[ 5 ] ) &&
                ( pData[ 6 ] == pData[ 7 ] )
            )
            {
                LE_TRACE( "Detected broken mono decoding (workaround enabled)" );
                if ( std::max_element( pData, pData + 8 ) == 0 )
                    return true;
                brokenMonoPlayback_ = MonoPlayback::Broken;
            }
            else
            {
                LE_TRACE( "Detected correct mono decoding!?" );
                brokenMonoPlayback_ = MonoPlayback::NotBroken;
            }
        }
        LE_ASSUME( brokenMonoPlayback_ != MonoPlayback::NotChecked );
        return static_cast<bool>( brokenMonoPlayback_ );
    }

    static std::uint8_t apiLevel()
    {
        static std::uint8_t value( 0 );
        if ( BOOST_UNLIKELY( value == 0 ) )
        {
            char propValue[ PROP_VALUE_MAX ];
            BOOST_VERIFY( __system_property_get( "ro.build.version.sdk", propValue ) );
            BOOST_ASSERT( std::strlen( propValue ) == 2 );
            value = ( propValue[ 0 ] - '0' ) * 10 + ( propValue[ 1 ] - '0' );
        }
        LE_ASSUME( value >= __ANDROID_API__ );
        return value;
    }
#endif // __ANDROID_API__ < 18

private:
    static bool          constexpr usePrefetch = true;
    static std::uint8_t  constexpr bqBuffers   = 2;

    static std::uint16_t constexpr mp3BlockSize     = 1152;
    static std::uint16_t constexpr aacBlockSize     = 1024;
    static std::uint16_t constexpr genericBlockSize = 2048;

    /// \note std::uint_fast16_t maps to std::uint64_t on 64bit
    /// Android/GCC/Clang platforms which is too much.
    /// http://stackoverflow.com/questions/4116297/x86-64-why-is-uint-least16-t-faster-then-uint-fast16-t-for-multiplication
    ///                                       (26.03.2015.) (Domagoj Saric)
    std::uint16_t sampleRate_       = 0    ; // L should allow above 48kHz http://geeknizer.com/audio-improvements-in-android-5-0-l-audiophile
    std::uint8_t  numberOfChannels_ = 0    ;
    mutable volatile bool finished_ = false;
    OpenSL::Engine::Ptr pOSLEngine_        ;
    mutable OpenSL::Player player_         ;

    mutable Utility::WaitableWithSharedLock bufferFilled_ ;
    mutable Utility::WaitableWithSharedLock bufferWaiting_;

    struct CDeleter { void operator()( OpenSL::sample_t * const pBuffer ) const noexcept { std::free( pBuffer ); } };
    std::unique_ptr<OpenSL::sample_t [], CDeleter> pBlockBuffer_    ;
    std::uint16_t                                  blockSize_    = 0;
    mutable std::uint16_t unreadBlockBufferBegin_ = 0;
    mutable std::uint16_t unreadBlockBufferEnd_   = 0;

    std::atomic_bool extractCallbackActive{ false };

#if __ANDROID_API__ < 18
    enum struct MonoPlayback : std::uint8_t { NotBroken = false, Broken = true, NotChecked };
    mutable MonoPlayback brokenMonoPlayback_;
#endif // __ANDROID_API__ < 18

#if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
    mutable SLmillisecond position_;
#endif // !NDEBUG && !LE_PUBLIC_BUILD
}; // class FileImpl


template <>
char const * LE_COLD FileImpl::open<Utility::Resources>( char const * const fileName )
{
    AAsset * const pInputFileAsset( ::AAssetManager_open( &Utility::resourceManager(), fileName, AASSET_MODE_STREAMING ) );
    if ( !pInputFileAsset )
        return "Failed to open resource";

    SLDataLocator_AndroidFD loc_fd = { SL_DATALOCATOR_ANDROIDFD };
    //loc_fd.fd = AAsset_openFileDescriptor64( pInputFileAsset, &loc_fd.offset, &loc_fd.length );
    ::off_t offset, length;
    loc_fd.fd     = AAsset_openFileDescriptor( pInputFileAsset, &offset, &length );
    loc_fd.offset = offset;
    loc_fd.length = length;
    if ( loc_fd.fd < 0 )
        return "Failed to open resource";

    AAsset_close( pInputFileAsset );

    char const * const pError( open( &loc_fd, fileName ) );
    BOOST_VERIFY( ::close( loc_fd.fd ) == 0 );
    return pError;
}


#if defined( __arm__ ) && !( defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7__ ) || defined( __aarch64__ ) )
    /// If we don't have ARMv7 we certainly don't have Android 4+.
    /// http://android.stackexchange.com/questions/34958/what-are-the-minimum-hardware-specifications-for-android
    ///                                       (07.05.2014.) (Domagoj Saric)
    char const File::supportedFormats[] = "";
#else
    // http://developer.android.com/guide/appendix/media-formats.html
    char const File::supportedFormats[] = "*.3gp;*.mp4;*.m4a;*.aac;*.ts;*.flac;*.mp3;*.mid;*.xmf;*.mxmf;*.rtttl;*.rtx;*.ota;*.imy;*.ogg;*.mkv;*.wav";
#endif // Gingerbread/pre ARMv7

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "pimplForwarders.inl"

LE_OPTIMIZE_FOR_SIZE_END()
