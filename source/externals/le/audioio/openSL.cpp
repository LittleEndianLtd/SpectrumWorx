////////////////////////////////////////////////////////////////////////////////
///
/// openSL.cpp
/// ----------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "openSL.hpp"

#include <memory>
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

Engine Engine::singleton_;

LE_COLD LE_PURE_FUNCTION
Engine::Ptr LE_FASTCALL Engine::singleton()
{
    if ( BOOST_UNLIKELY( !singleton_ ) )
    {
        /// \note Android implementation ignores the engine options and is
        /// always thread safe.
        ///                               (05.03.2014.) (Domagoj Saric)
    #if 0
        static ::SLEngineOption const engineOption[] = { { SL_ENGINEOPTION_THREADSAFE, SL_BOOLEAN_TRUE } };
        unsigned int const numberOfEngineOptions( _countof( engineOption ) );
    #else
        ::SLEngineOption const * const engineOptions( nullptr );
        unsigned int const numberOfEngineOptions( 0 );
    #endif
        ::SLresult result( ::slCreateEngine( &static_cast<ObjectPtr &>( singleton_ ), numberOfEngineOptions, engineOptions, 0, nullptr, nullptr ) );
        if ( result != SL_RESULT_SUCCESS )
        {
            BOOST_ASSERT( result == SL_RESULT_MEMORY_FAILURE || result == SL_RESULT_RESOURCE_ERROR );
            return nullptr/*"Out of memory"*/;
        }
        result = singleton_.realize();
        if ( result != SL_RESULT_SUCCESS )
        {
            BOOST_ASSERT( result == SL_RESULT_MEMORY_FAILURE || result == SL_RESULT_RESOURCE_ERROR || result == SL_RESULT_IO_ERROR );
            return nullptr/*"Out of memory"*/;
        }

        singleton_.getInterface( SL_IID_ENGINE    , singleton_.pEngine_     );
    #if defined( LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX )
        singleton_.getInterface( SL_IID_THREADSYNC, singleton_.pThreadSync_ );
    #elif !defined( NDEBUG )
        ::SLThreadSyncItf pThreadSync;
        BOOST_ASSERT_MSG( !singleton_.tryGetInterface( SL_IID_THREADSYNC, pThreadSync ), "ThreadSync interface supported." );
    #endif // LE_AUDIOIO_ANDROID_USE_BUILTIN_MUTEX
    }

    /// \note Unsupported on Android 4.0.3 on HTC One V and Android 4.1.2 on
    /// Galaxy S3 mini.
    ///                                       (16.01.2014.) (Domagoj Saric)
    /// \todo Reinvestigate - probably all that is needed is to request the
    /// interfaces in the slCreateEngine() call.
    ///                                       (30.12.2015.) (Domagoj Saric)
#if 0
    {
        ::SLDeviceVolumeItf pDeviceVolume;
        singleton_.getInterface( SL_IID_DEVICEVOLUME, pDeviceVolume );
        SLint32 minimumValue;
        SLint32 maximumValue;
        SLboolean millibels;
        oslVerify( (*pDeviceVolume)->GetVolumeScale( pDeviceVolume, SL_DEFAULTDEVICEID_AUDIOINPUT, &minimumValue, &maximumValue, &millibels ) );
        SLint32 volume;
        oslVerify( (*pDeviceVolume)->GetVolume( pDeviceVolume, SL_DEFAULTDEVICEID_AUDIOINPUT, &volume ) );
        BOOST_ASSERT( volume == maximumValue );
        oslVerify( (*pDeviceVolume)->GetVolumeScale( pDeviceVolume, SL_DEFAULTDEVICEID_AUDIOOUTPUT, &minimumValue, &maximumValue, &millibels ) );
        oslVerify( (*pDeviceVolume)->GetVolume( pDeviceVolume, SL_DEFAULTDEVICEID_AUDIOOUTPUT, &volume ) );
        BOOST_ASSERT( volume == maximumValue );
    }
#elif !defined( NDEBUG )
    ::SLDeviceVolumeItf pDeviceVolume;
    BOOST_ASSERT_MSG( !singleton_.tryGetInterface( SL_IID_DEVICEVOLUME, pDeviceVolume ), "DeviceVolume interface supported." );
#endif // NDEBUG

    return Ptr( std::addressof( singleton_ ) );
}

LE_COLD LE_NOTHROW
char const * LE_FASTCALL Player::setup
(
    SLDataSource          const source,
    SLDataSink            const sink,
    SLInterfaceID const * const pIDs,
    std::uint8_t          const numberOfIDs
)
{
    auto const pEngine( Engine::singletonInterface() );

    Engine::Lock const engineLock( Engine::singletonMutex() );

    //...mrmlj...predestroyed...
    //object().destroy();
    BOOST_ASSERT( !(*this) );

    static SLboolean constexpr req[] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
    BOOST_ASSERT( numberOfIDs <= _countof( req ) );

    /// \note Note from the official native-audio example:
    /// "note that an invalid URI is not detected here, but during prepare/
    /// prefetch on Android, // or possibly during Realize on other
    /// platforms".
    ///                                       (25.04.2014.) (Domagoj Saric)
    SLresult result;
    result = (*pEngine)->CreateAudioPlayer( pEngine, &object(), const_cast<SLDataSource *>( &source ), const_cast<SLDataSink *>( &sink ), numberOfIDs, pIDs, req );
    if ( BOOST_UNLIKELY( result != SL_RESULT_SUCCESS ) )
    {
        BOOST_ASSERT( result == SL_RESULT_MEMORY_FAILURE || result == SL_RESULT_IO_ERROR || SL_RESULT_CONTENT_UNSUPPORTED );
        return ( result == SL_RESULT_CONTENT_UNSUPPORTED ) ? "Out of memory" : "Unsupported audio format";
    }

    /// \note Note from the official native-audio example:
    /// "this will always succeed on Android, but we check result for
    /// portability to other platforms".
    ///                                       (25.04.2014.) (Domagoj Saric)
    result = object().realize();
    if ( BOOST_UNLIKELY( result != SL_RESULT_SUCCESS ) )
    {
        BOOST_ASSERT( result == SL_RESULT_MEMORY_FAILURE || result == SL_RESULT_RESOURCE_ERROR || result == SL_RESULT_IO_ERROR );
        return "Out of memory";
    }

    object().getInterface( SL_IID_ANDROIDSIMPLEBUFFERQUEUE, bq_.pBQ );

#if 0
    // get the mute/solo interface
    result = object().getInterface( SL_IID_MUTESOLO, &uriPlayerMuteSolo );
#endif

    /// \note Unsupported on Android 4.0.3 on HTC One V and Android 4.1.2 on
    /// Galaxy S3 mini.
    ///                                       (16.01.2014.) (Domagoj Saric)
#if 0
    {
        ::SLVolumeItf pVolume;
        object().getInterface( SL_IID_VOLUME, pVolume );
        ::SLmillibel maximumVolume;
        oslVerify( (*pVolume)->GetMaxVolumeLevel( pVolume, &maximumVolume ) );
        ::SLmillibel volume;
        oslVerify( (*pVolume)->GetVolumeLevel   ( pVolume, &volume        ) );
        BOOST_ASSERT( volume == maximumVolume );
        pOutputMixObject_.getInterface( SL_IID_VOLUME, pVolume );
        oslVerify( (*pVolume)->GetMaxVolumeLevel( pVolume, &maximumVolume ) );
        oslVerify( (*pVolume)->GetVolumeLevel   ( pVolume, &volume        ) );
        BOOST_ASSERT( volume == maximumVolume );
    }
#elif !defined( NDEBUG )
    ::SLVolumeItf pVolume;
    BOOST_ASSERT_MSG( !tryGetInterface( SL_IID_VOLUME, pVolume ), "DeviceVolume interface supported." );
#endif // NDEBUG

    return nullptr;
}

LE_COLD
void Player::destroy()
{
    if ( *this )
        stop();
    bq().destroy();
    pInterface_ = nullptr;
    ObjectPtr::destroy();
}

LE_COLD
void LE_FASTCALL verify( ::SLresult const result, unsigned int const lineNumber )
{
#ifndef NDEBUG
    if ( BOOST_UNLIKELY( result != SL_RESULT_SUCCESS ) )
        ::__android_log_assert( "result == SL_RESULT_SUCCESS", "LE.AudioIO", "OpenSL failure: 0x%08lx @ %u", static_cast<unsigned long>( result ), lineNumber );
    BOOST_ASSERT_MSG( result == SL_RESULT_SUCCESS, "OpenSL failure." );
#else
    (void)result;
    (void)lineNumber;
#endif // NDEBUG
}

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI intrusive_ptr_add_ref( Engine const * LE_RESTRICT const pEngine )
{
    LE_ASSUME( pEngine == std::addressof( Engine::singleton_ ) );
    ++pEngine->referenceCount_;
}

LE_NOTHROW void LE_FASTCALL_ABI intrusive_ptr_release( Engine const * LE_RESTRICT const pEngine )
{
    LE_ASSUME( pEngine == std::addressof( Engine::singleton_ ) );
    if ( BOOST_UNLIKELY( !--pEngine->referenceCount_ ) )
        const_cast<Engine *>( pEngine )->destroy();
}
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

DataConverter::DataConverter()
#ifndef NDEBUG
    :
    numberOfChannels_        ( 0 ),
    numberOfBuffers_         ( 0 ),
    numberOfSamplesPerBuffer_( 0 ),
    separatedBuffers_        { nullptr }
#endif // NDEBUG
{}

LE_COLD LE_NOTHROW
bool LE_FASTCALL DataConverter::setup
(
    std::uint8_t  const numberOfChannels,
    std::uint8_t  const numberOfBuffers,
    std::uint16_t const numberOfSamplesPerBuffer
)
{
    numberOfBuffers_          = numberOfBuffers         ;
    numberOfChannels_         = numberOfChannels        ;
    numberOfSamplesPerBuffer_ = numberOfSamplesPerBuffer;

    std::uint16_t const alignedProcessingBlockSizeInSamples( /*Math::alignIndex( numberOfSamplesPerBuffer_ )*/( numberOfSamplesPerBuffer_ + 4 - 1 ) & ~( 4 - 1 ) );
    pBuffer_     .reset( new ( std::nothrow ) sample_t[ numberOfChannels_ * numberOfSamplesPerBuffer_           * numberOfBuffers_ ] );
    pFloatBuffer_.reset( new ( std::nothrow ) float   [ numberOfChannels_ * alignedProcessingBlockSizeInSamples * numberOfBuffers_ ] );
    if ( BOOST_UNLIKELY( !pBuffer_ || !pFloatBuffer_ ) )
        return false;
    for ( std::uint8_t channel( 0 ); channel < numberOfChannels_; ++channel )
    {
        separatedBuffers_[ channel ] = &pFloatBuffer_[ channel * alignedProcessingBlockSizeInSamples ];
    }

    return true;
}

//------------------------------------------------------------------------------
} // namespace OpenSL
//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
