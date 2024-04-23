////////////////////////////////////////////////////////////////////////////////
///
/// outputWaveFileImpl.cpp
/// ----------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "outputWaveFileImpl.hpp"
#include "outputWaveFile.hpp"

#include "structures.hpp"

#include "le/math/conversion.hpp"
#include "le/math/vector.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/filesystem.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tracePrivate.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include "boost/assert.hpp"

#include "fcntl.h"
#if defined( __APPLE__ )
    #define _GNU_SOURCE
    #include "pthread.h"
    #include "sys/stat.h"
#elif defined( _WIN32 )
    #include "le/utility/parentFromMember.hpp"

    #include "io.h" // _get_osfhandle
#endif // OS

#include <algorithm>
#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

LE_COLD
OutputWaveFileBase::OutputWaveFileBase()
{
    using namespace Detail;

    format_.size                            = useExtendedFormat ? sizeof( format_.fmt ) : sizeof( format_.fmt.Format );
    format_.fmt.Format.wFormatTag           = useExtendedFormat ? WAVE_FORMAT_EXTENSIBLE : ( write16bitData ? WAVE_FORMAT_PCM : WAVE_FORMAT_IEEE_FLOAT );
    format_.fmt.Format.wBitsPerSample       = 8 * ( write16bitData ? sizeof( short ) : sizeof( float ) );
    format_.fmt.Format.cbSize               = static_cast<WORD>( format_.size - sizeof( format_.fmt.Format ) );
    format_.fmt.Samples.wValidBitsPerSample = format_.fmt.Format.wBitsPerSample;
    format_.fmt.SubFormat                   = write16bitData ? KSDATAFORMAT_SUBTYPE_PCM : KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;

    riffHeader_.size = 0;
}


std::uint16_t OutputWaveFileBase::headerSize() const
{
    return static_cast<std::uint16_t>( sizeof( riffHeader_ ) + format_.totalSize() + dataHeader_.headerSize() );
}

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.
LE_COLD
void OutputWaveFileBase::initialiseHeader( std::uint8_t const numberOfChannels, std::uint32_t const sampleRate )
{
    using namespace Detail;

    static DWORD const channelMasks[] =
    {
        SPEAKER_FRONT_CENTER,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER,
        SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER | SPEAKER_LOW_FREQUENCY,
    };

    BOOST_ASSERT( !useExtendedFormat || ( format_.fmt.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE ) );
    BOOST_ASSERT( !useExtendedFormat || ( format_.fmt.Format.cbSize == sizeof( format_.fmt ) - sizeof( format_.fmt.Format ) ) );
    format_.fmt.Format.nChannels       = numberOfChannels;
    format_.fmt.Format.nSamplesPerSec  = sampleRate;
    format_.fmt.Format.nBlockAlign     = format_.fmt.Format.nChannels      * format_.fmt.Format.wBitsPerSample / 8;
    format_.fmt.Format.nAvgBytesPerSec = format_.fmt.Format.nSamplesPerSec * format_.fmt.Format.nBlockAlign;

    if ( useExtendedFormat )
        format_.fmt.dwChannelMask = channelMasks[ std::min<std::uint8_t>( numberOfChannels, _countof( channelMasks ) ) - 1 ];

    dataHeader_.size = 0;
}
#pragma warning( pop )

LE_COLD
void OutputWaveFileBase::finalise( std::uint32_t const size )
{
    riffHeader_.size = static_cast<Detail::DWORD>
    (
        sizeof( riffHeader_ ) - riffHeader_.headerSize()
            +
        format_    .totalSize()
            +
        dataHeader_.totalSize()
    );

    BOOST_ASSERT( !riffHeader_.requiresPaddingByte() );
    BOOST_ASSERT( !format_    .requiresPaddingByte() );
    BOOST_ASSERT( !dataHeader_.requiresPaddingByte() );

#ifndef NDEBUG //.........mrmlj...................
    if ( !size ) return;
    using namespace Detail;
    if
    (
        ( riffHeader_.tag    .invalid() ) ||
        ( riffHeader_.waveTag.invalid() ) ||
        ( riffHeader_.size > size - riffHeader_.headerSize() ) ||
        ( dataHeader_.size > size - sizeof( riffHeader_ ) - format_.totalSize() - dataHeader_.headerSize() ) ||
        (
            format_.size != sizeof( format_.fmt  )                                       &&
            format_.size != sizeof( WAVEFORMATEX )                                       &&
            format_.size != sizeof( WAVEFORMATEX ) - sizeof( format_.fmt.Format.cbSize )
        ) ||
        (
            write16bitData &&
            format_.fmt.Format.wBitsPerSample != 16 &&
            format_.fmt.Format.wBitsPerSample != 24
        ) ||
        (
            !write16bitData &&
            format_.fmt.Format.wBitsPerSample != 32
        )
    )
    {
        BOOST_ASSERT( !"Broken OutputWaveFile" );
    }
#else
    (void)size;
#endif // NDEBUG
}


LE_COLD
std::uint32_t OutputWaveFileBase::getTimePosition() const
{   //...mrmlj...duplicated ms<->sample logic also in fileAndroid.cpp and fileApple.cpp...extract...
    return static_cast<std::uint32_t>( std::uint64_t( getSamplePosition() ) * 1000 / format_.fmt.Format.nSamplesPerSec );
}
LE_COLD
std::uint32_t OutputWaveFileBase::getSamplePosition() const
{
    return dataHeader_.size / static_cast<std::uint8_t>( format_.fmt.Format.nBlockAlign );
}

LE_COLD LE_NOTHROW
OutputWaveFileImpl::~OutputWaveFileImpl() { close(); }

LE_COLD LE_NOTHROW
void OutputWaveFileImpl::close()
{
    if ( !stream_ )
        return;

    auto const size( stream_.size() );
#ifndef _WIN32
    /// \note The file descriptor does not seem to track the underlying HANDLE
    /// position.
    ///                                       (21.04.2015.) (Domagoj Saric)
    BOOST_ASSERT_MSG
    (
        stream_.position() == size || !size /*no data was written (empty wav)*/,
        "WAVE file in inconsistent state. Perhaps the same file was modified "
        "through multiple OutputWaveFile objects?"
    );
#endif // _WIN32

    OutputWaveFileBase::finalise( size );

    BOOST_VERIFY( stream_.seek( 0, SEEK_SET ) );

    BOOST_VERIFY( stream_.write( &riffHeader_, sizeof( riffHeader_ )    ) == sizeof( riffHeader_ )    );
    BOOST_VERIFY( stream_.write( &format_    , format_    .totalSize () ) == format_    .totalSize () );
    BOOST_VERIFY( stream_.write( &dataHeader_, dataHeader_.headerSize() ) == dataHeader_.headerSize() );

    BOOST_ASSERT( stream_.size    () == size || !size                                                          );
    BOOST_ASSERT( stream_.position() == sizeof( riffHeader_ ) + format_.totalSize() + dataHeader_.headerSize() );

    stream_.close();
    BOOST_ASSERT( !stream_ );
}


LE_COLD LE_NOTHROW
char const * OutputWaveFileImpl::create
(
    Utility::File::Stream && file,
    std::uint8_t  const numberOfChannels,
    std::uint32_t const sampleRate
)
{
    if ( BOOST_UNLIKELY( !file ) )
    {
        BOOST_ASSERT( errno );
        return "Error creating file";
    }

    if ( BOOST_UNLIKELY( !file.seek( headerSize(), SEEK_SET ) ) )
    {
        BOOST_ASSERT( errno );
        return "Error creating space in the output file for the WAVE header structures.";
    }

    close();
    stream_ = std::move( file );
    initialiseHeader( numberOfChannels, sampleRate );
    return nullptr;
}


#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.
error_msg_t OutputWaveFileImpl::write( float const * LE_RESTRICT /*const*/ pInput, std::uint32_t const sampleFrames )
{
    std::uint8_t  const numberOfChannels( format_.fmt.Format.nChannels    );
    std::uint32_t const samples         ( sampleFrames * numberOfChannels );

    if ( write16bitData )
    {
        auto const blockSamples( std::min<std::uint32_t>( samples, 16384 / sizeof( internal_sample_t ) ) );
        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( integers, internal_sample_t, blockSamples );
        for ( std::uint32_t sample( 0 ); sample < samples; sample += blockSamples )
        {
            auto const currentSamples( std::min<std::uint32_t>( samples - sample, blockSamples ) );
            Math::convertSamples( pInput, integers.begin(), currentSamples );
            auto const bytesToWrite( static_cast<std::uint32_t>( currentSamples * sizeof( integers.front() ) ) );
            auto const bytesWritten( static_cast<std::uint32_t>( stream_.write( integers.begin(), bytesToWrite ) ) );
            BOOST_ASSERT_MSG( !bytesWritten || ( bytesToWrite == bytesWritten ), "Unexpected result." );
            dataHeader_.size += bytesWritten;
            pInput           += currentSamples;
            if ( bytesWritten != bytesToWrite )
                return "Not enough disk space.";
        }

        return nullptr;
    }
    else
    {
        auto const bytesToWrite( static_cast<std::uint32_t>( samples * sizeof( *pInput )         ) );
        auto const bytesWritten(                             stream_.write( pInput, bytesToWrite ) );

        BOOST_ASSERT_MSG( !bytesWritten || ( bytesToWrite == bytesWritten ), "Unexpected result." );

        dataHeader_.size += bytesWritten;

        return ( bytesWritten == bytesToWrite ) ? nullptr : "Not enough disk space.";
    }
}
#pragma warning( pop )

//...mrmlj...ugh...revisit this with lots of spare nerves...
OutputWaveFileAsyncImpl::OutputWaveFileAsyncImpl( OutputWaveFileAsyncImpl && other )
    :
    OutputWaveFileImpl
    ((
    #if defined( __ANDROID__ )
        /*mrmlj cause the thread to stop so that it can be restarted with the new object*/other.join(),
    #endif // __ANDROID__
        std::move( other )
    )),
#if defined( __APPLE__ )
    dispatcher_( other.dispatcher_                                ),
    counter_   ( other.counter_.load( std::memory_order_acquire ) ), //...mrmlj...
    error_     ( other.error_                                     )
#else
    #if defined( __ANDROID__ )
        bufferNotEmpty_( std::move( other.bufferNotEmpty_ ) ),
        bufferNotFull_ ( std::move( other.bufferNotFull_  ) ),
        bufferLock_    ( std::move( other.bufferLock_     ) ),
        thread_        (            other.thread_           ),
      //queue_         ( std::move( other.queue_          ) ), //...mrmlj...no move...http://lists.boost.org/boost-users/2014/11/83304.php
        stopWorker_    ( false                              ),
    #elif defined( _WIN32 )
        overlappedIOHandle_( other.overlappedIOHandle_ ),
    #endif
    error_(            other.error_   )
 //,cache_( std::move( other.cache_ ) ) //...mrmlj...no move...
#endif
{
#if defined( __APPLE__ )
#else
    #if defined( __ANDROID__ )
        other.queue_.reset();
        run();
    #endif
    other.cache_.reset();
#endif
    new ( &other ) OutputWaveFileAsyncImpl;
}


LE_COLD LE_NOTHROW
void OutputWaveFileAsyncImpl::join()
{
#if defined( __APPLE__ )
    if ( dispatcher_ )
    {
        //dispatch_semaphore_t sem = dispatch_semaphore_create(0);
        //dispatch_semaphore_wait
        BOOST_ASSERT( counter_ >= 0 );
        while ( BOOST_UNLIKELY( counter_ ) )
            BOOST_VERIFY( ::sched_yield() == 0 );
    #ifndef NDEBUG
        auto const fd( dispatch_io_get_descriptor( dispatcher_ ) );
        BOOST_ASSERT( stream_.position() == ::lseek( fd, 0, SEEK_CUR ) );
        struct stat file_status;
        BOOST_VERIFY( ::fstat( fd, &file_status ) == 0 );
        BOOST_ASSERT( stream_.size() == file_status.st_size );
    #endif // NDEBUG
        dispatch_io_close( dispatcher_, 0/*DISPATCH_IO_STOP*/ );
        dispatch_release ( dispatcher_                        );
        dispatcher_ = nullptr;
        BOOST_ASSERT( counter_ == 0 );
    }
#elif defined( __ANDROID__ )
    LE_TRACE_IF( error_, "Async write error" );
    if ( thread_ != -1 )
    {
        stopWorker_ = true;
        bufferNotEmpty_.signal();
        BOOST_VERIFY( ::pthread_join( thread_, nullptr ) == 0 );
        thread_ = -1;
    }
    BOOST_ASSERT( queue_.empty() );
    cache_.consume_all( []( BufferNode * const pNode ){ delete pNode; } );
#elif defined( _WIN32 )
    // Force any completion callbacks to run.
    ::SleepEx( 0, true );
    BOOST_ASSERT( ::SleepEx( 0, true ) == 0                                                           );
    BOOST_VERIFY( ::CloseHandle( overlappedIOHandle_ ) || overlappedIOHandle_ == INVALID_HANDLE_VALUE );
    overlappedIOHandle_ = INVALID_HANDLE_VALUE;
    cache_.consume_all( []( BufferNode * const pNode ){ delete pNode; } );
#endif // OS
}


LE_COLD LE_NOTHROW
bool OutputWaveFileAsyncImpl::close()
{
    join();
    OutputWaveFileImpl::close();
    return error_;
}

#if defined( __APPLE__ )
namespace
{
    LE_NOTHROWRESTRICTNOALIAS LE_CONST_FUNCTION
    dispatch_queue_t LE_FASTCALL gdcQueueSingleton()
    {
        /// \note QOS_CLASS_BACKGROUND is available only since OSX 10.10/iOS 8.
        ///                                   (24.04.2015.) (Domagoj Saric)
        auto const queue( dispatch_get_global_queue( DISPATCH_QUEUE_PRIORITY_BACKGROUND, 0 ) );
        BOOST_ASSERT( queue );
        return queue;
    }
} // anonymous namespace
#endif // OS

LE_COLD
error_msg_t OutputWaveFileAsyncImpl::write( float const * LE_RESTRICT const pInput, std::uint16_t const sampleFrames )
{
    std::uint8_t  const numberOfChannels( format_.fmt.Format.nChannels    );
    std::uint32_t const samples         ( sampleFrames * numberOfChannels );

#ifdef __APPLE__
    dispatch_data_t data( nullptr );
    if ( write16bitData )
    {
        LE_ALIGN( 16 ) internal_sample_t integers[ samples ];
        Math::convertSamples( pInput, integers, samples );
        auto const numberOfBytes( samples * sizeof( integers[ 0 ] ) );
        data = dispatch_data_create( integers, numberOfBytes, nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
        dataHeader_.size += numberOfBytes;
    }
    else
    {
        auto const numberOfBytes( samples * sizeof( *pInput ) );
        data = dispatch_data_create( pInput  , numberOfBytes, nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT );
        dataHeader_.size += numberOfBytes;
    }
    if ( BOOST_UNLIKELY( !data ) )
        return "Out of memory";
    BOOST_ASSERT( counter_ >= 0 );
    ++counter_;
    dispatch_io_write
    (
        dispatcher_,
        0,
        data,
        gdcQueueSingleton(),
        ^( bool const done, dispatch_data_t const data, int const error )
        {
            LE_TRACE_IF( error, "Dispatch write handler %d", error );
            error_ |= ( error != 0 );
            if ( done )
            {
                BOOST_ASSERT( data == nullptr );
                BOOST_ASSERT( counter_ > 0    );
                --counter_;
            }
        }
    );
    return nullptr;
#else
#ifdef _WIN32
    ::SleepEx( 0, true );
#endif // _WIN32
    BufferNode * LE_RESTRICT pBuffer( nullptr );
    auto cachedBuffers( cache_.read_available() );
    if ( cachedBuffers )
    {
        while ( cachedBuffers-- )
        {
            BOOST_VERIFY( cache_.pop( pBuffer ) );
            if ( pBuffer->capacity >= sampleFrames )
            {
                pBuffer->size = sampleFrames;
                break;
            }
            LE_TRACE( "Discarding an insufficient cache buffer." );
            delete pBuffer;
        }
        pBuffer = nullptr;
    }
    if ( !pBuffer )
    {
        auto const storage( new ( std::nothrow ) char[ 1 * sizeof( BufferNode ) + samples * sizeof( pBuffer->data[ 0 ] ) ] );
        if ( !storage ) return "Out of memory";
        pBuffer = new ( storage ) BufferNode( sampleFrames, *this );
    }

    Math::convertSamples( pInput, pBuffer->data, samples );
#ifdef __ANDROID__
    while ( BOOST_UNLIKELY( !queue_.push( static_cast<BufferNode *>( pBuffer ) ) ) )
    {
        LE_TRACE( "OutputWaveFileAsync::write() queue full - blocking!" );
        using Lock = std::lock_guard<Utility::ConditionVariable::Lock>;
        Lock const lock( bufferLock_ );
        bufferNotFull_.wait( bufferLock_ );
    }
    bufferNotEmpty_.signal();
#else
    error_ |=
        ::WriteFileEx
        (
            overlappedIOHandle_, pBuffer->data, pBuffer->size * sizeof( pBuffer->data[ 0 ] ), pBuffer,
            []( DWORD const errorCode, DWORD const numberOfBytesTransfered, OVERLAPPED * LE_RESTRICT const pOverlapped )
            {
                auto & node( *static_cast     <BufferNode *>( pOverlapped )              );
                auto & impl( *reinterpret_cast<OutputWaveFileAsyncImpl *>( node.hEvent ) );
                impl.error_ |= ( errorCode != ERROR_SUCCESS );
                impl.dataHeader_.size += numberOfBytesTransfered;
                // Return the buffer to the cache or delete it if the cache is
                // full
                if ( BOOST_UNLIKELY( !impl.cache_.push( &node ) ) )
                {
                    LE_TRACE( "OutputWaveFileAsync::write() cache full!" );
                    delete &node;
                }
            }
        ) == false;
#endif // __ANDROID__
    return BOOST_UNLIKELY( error_ ) ? "Asynchronous write error" : nullptr;
#endif // OS
}

#ifdef __ANDROID__
LE_COLD
void OutputWaveFileAsyncImpl::worker()
{
    for ( ; ; )
    {
        using Lock = std::lock_guard<Utility::ConditionVariable::Lock>;

        while ( queue_.empty() && !stopWorker_ )
        {
            Lock const lock( bufferLock_ ); //...mrmlj...lock required?
            bufferNotEmpty_.wait( bufferLock_ );
        }

        if ( stopWorker_ && queue_.empty() )
            break;

        // Consume the first available item
        BufferNode * pWorkRaw;
        BOOST_VERIFY( queue_.pop( pWorkRaw ) );
        std::unique_ptr<BufferNode> pWork( pWorkRaw );

        bufferNotFull_.signal();

        if ( !pWork->size )
            LE_TRACE_LOGONLY( "Zero-sized work queued" );
        error_ = OutputWaveFileImpl::write( pWork->data, pWork->size ) != nullptr;
        LE_TRACE_IF( error_, "OutputWaveFileAsync: asynchronous write error." );

        //...mrmlj...?...BOOST_VERIFY( cache_.push( pWork.release() ) );
        // Return the buffer to the cache or delete it if the cache is
        // full
        if ( BOOST_LIKELY( cache_.push( pWork.get() ) ) )
            pWork.release();
        else
            LE_TRACE( "OutputWaveFileAsync::write() cache full" );
    }
}
#endif // __ANDROID__

LE_COLD
bool OutputWaveFileAsyncImpl::run()
{
    error_ = false;
#if defined( __APPLE__ )
    BOOST_ASSERT( !dispatcher_ );
    dispatcher_ = dispatch_io_create
    (
        DISPATCH_IO_STREAM,
        stream_.nativeHandle(),
        gdcQueueSingleton(),
        ^( int const error )
        {
            LE_TRACE_IF( error, "Dispatch handler %d", error );
        }
    );
    #ifndef NDEBUG
        auto const fd( dispatch_io_get_descriptor( dispatcher_ ) );
        BOOST_ASSERT( stream_.position() == ::lseek( fd, 0, SEEK_CUR ) );
        struct stat file_status;
        BOOST_VERIFY( ::fstat( fd, &file_status ) == 0 );
        BOOST_ASSERT( stream_.size() == file_status.st_size );
    #endif // NDEBUG
    return dispatcher_ != nullptr;
#elif defined( __ANDROID__ )
        BOOST_ASSERT( thread_     == -1    );
        BOOST_ASSERT( stopWorker_ == false );
    #ifdef SCHED_IDLE
        auto const schedulingPolicy( SCHED_IDLE  );
    #else
        auto const schedulingPolicy( SCHED_OTHER );
    #endif // SCHED_OTHER
        sched_param const schedulingParameters = { ::sched_get_priority_min( schedulingPolicy ) };
        pthread_attr_t threadAttributes;
        BOOST_VERIFY( pthread_attr_init           ( &threadAttributes                         ) == 0 );
        BOOST_VERIFY( pthread_attr_setschedpolicy ( &threadAttributes, schedulingPolicy       ) == 0 );
        BOOST_VERIFY( pthread_attr_setschedparam  ( &threadAttributes, &schedulingParameters  ) == 0 );
      //BOOST_VERIFY( pthread_attr_setinheritsched( &threadAttributes, PTHREAD_EXPLICIT_SCHED ) == 0 ); // unavailable on Android
        auto const succeeded( ::pthread_create( &thread_, &threadAttributes, []( void * const pFile ) -> void * { static_cast<OutputWaveFileAsyncImpl *>( pFile )->worker(); return nullptr; }, this ) == 0 );
        BOOST_VERIFY( pthread_attr_destroy( &threadAttributes ) == 0 );
        if ( BOOST_LIKELY( succeeded ) )
        {
            BOOST_VERIFY( ::pthread_setschedparam( thread_, schedulingPolicy, &schedulingParameters ) == 0 );
            return true;
        }
        thread_ = -1;
        return false;
#elif defined( _WIN32 )
    BOOST_ASSERT( overlappedIOHandle_ == INVALID_HANDLE_VALUE );
    auto const fileHandle( reinterpret_cast<HANDLE>( _get_osfhandle( stream_.nativeHandle() ) ) );
    overlappedIOHandle_ = ::ReOpenFile( fileHandle, FILE_GENERIC_WRITE, FILE_SHARE_WRITE, FILE_FLAG_OVERLAPPED );
    return overlappedIOHandle_ != INVALID_HANDLE_VALUE;
#endif // OS
}

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
// PImpl forwarders
////////////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

template <> struct Implementation<AudioIO::OutputWaveFile     > { using type = AudioIO::OutputWaveFileImpl     ; };
template <> struct Implementation<AudioIO::OutputWaveFileAsync> { using type = AudioIO::OutputWaveFileAsyncImpl; };

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

LE_NOTHROW OutputWaveFile:: OutputWaveFile() {}
LE_NOTHROW OutputWaveFile:: OutputWaveFile( OutputWaveFile && other ) : ConcreteStackPimpl( std::move( other ) ) {}
LE_NOTHROW OutputWaveFile::~OutputWaveFile() {}

template <Utility::SpecialLocations rootLocation>
error_msg_t OutputWaveFile::create( char const * const fileName, std::uint8_t const numberOfChannels, std::uint32_t const sampleRate )
{
    return impl( *this ).create
    (
        Utility::File::open<rootLocation>( fileName, O_CREAT | O_TRUNC | O_WRONLY ),
        numberOfChannels,
        sampleRate
    );
}

error_msg_t OutputWaveFile::write( float const * const pInput, std::uint32_t const sampleFrames ) { return impl( *this ).write( pInput, sampleFrames ); }
void        OutputWaveFile::close(                                                              ) { return impl( *this ).close(                      ); }

LE_PURE_FUNCTION std::uint32_t OutputWaveFile::getTimePosition  () const { return impl( *this ).getTimePosition  (); }
LE_PURE_FUNCTION std::uint32_t OutputWaveFile::getSamplePosition() const { return impl( *this ).getSamplePosition(); }


template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::AbsolutePath   >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::AppData        >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::Documents      >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::Library        >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::ExternalStorage>( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::Temporaries    >( char const *, std::uint8_t, std::uint32_t );
#ifdef __ANDROID__
template error_msg_t LE_FASTCALL_ABI OutputWaveFile::create<Utility::ToolOutput     >( char const *, std::uint8_t, std::uint32_t );
#endif // __ANDROID__

LE_NOTHROW OutputWaveFileAsync:: OutputWaveFileAsync() {}
LE_NOTHROW OutputWaveFileAsync:: OutputWaveFileAsync( OutputWaveFileAsync && other ) : ConcreteStackPimpl( std::move( other ) ) {}
LE_NOTHROW OutputWaveFileAsync::~OutputWaveFileAsync() {}

template <Utility::SpecialLocations rootLocation>
error_msg_t OutputWaveFileAsync::create( char const * const fileName, std::uint8_t const numberOfChannels, std::uint32_t const sampleRate )
{
    auto & impl( Utility::impl( *this ) );
    impl.join();
    auto const msg( /*...mrmlj...*/reinterpret_cast<OutputWaveFile &>( impl ).create<rootLocation>( fileName, numberOfChannels, sampleRate ) );
    if ( msg )
        return msg;
    if ( impl.run() )
        return nullptr;
    return "Out of memory";
}

error_msg_t OutputWaveFileAsync::write( float const * const pInput, std::uint16_t const sampleFrames ) { return impl( *this ).write( pInput, sampleFrames ); }
bool        OutputWaveFileAsync::close(                                                              ) { return impl( *this ).close(); }

LE_PURE_FUNCTION std::uint32_t OutputWaveFileAsync::getTimePosition  () const { return impl( *this ).getTimePosition  (); }
LE_PURE_FUNCTION std::uint32_t OutputWaveFileAsync::getSamplePosition() const { return impl( *this ).getSamplePosition(); }


template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::AbsolutePath   >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::AppData        >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::Documents      >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::Library        >( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::ExternalStorage>( char const *, std::uint8_t, std::uint32_t );
template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::Temporaries    >( char const *, std::uint8_t, std::uint32_t );
#ifdef __ANDROID__
template error_msg_t LE_FASTCALL_ABI OutputWaveFileAsync::create<Utility::ToolOutput     >( char const *, std::uint8_t, std::uint32_t );
#endif // __ANDROID__

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
