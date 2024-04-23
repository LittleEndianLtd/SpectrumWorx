////////////////////////////////////////////////////////////////////////////////
///
/// \file outputWaveFileImpl.hpp
/// ----------------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef outputWaveFileImpl_hpp__C66141DB_3639_4C62_A8AF_A4D90342A69B
#define outputWaveFileImpl_hpp__C66141DB_3639_4C62_A8AF_A4D90342A69B
#pragma once
//------------------------------------------------------------------------------
#include "structures.hpp"

#include "le/utility/conditionVariable.hpp"
#include "le/utility/filesystem.hpp"

#ifdef __APPLE__
    #include "dispatch/dispatch.h"

    #include <atomic>
#elif defined( _WIN32 )
    #include "boost/lockfree/spsc_queue.hpp"
#else
    #include "boost/lockfree/spsc_queue.hpp"

    #include "pthread.h"

    #include <mutex>
#endif // __APPLE__
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

class OutputWaveFileBase
{
public:
    LE_NOTHROW std::uint32_t LE_FASTCALL getTimePosition  () const;
    LE_NOTHROW std::uint32_t LE_FASTCALL getSamplePosition() const;

protected:
    static bool const write16bitData    = true ;
    static bool const useExtendedFormat = false;

protected:
    LE_NOTHROW OutputWaveFileBase();
    LE_NOTHROW OutputWaveFileBase( OutputWaveFileBase && other )
        : riffHeader_( other.riffHeader_ ), format_( other.format_ ), dataHeader_( other.dataHeader_ )
    {
        new ( &other ) OutputWaveFileBase();
    }

    using internal_sample_t = boost::mpl::if_c<write16bitData, std::int16_t, float>::type;

    std::uint16_t headerSize      () const;
    void          initialiseHeader( std::uint8_t numberOfChannels, std::uint32_t sampleRate );

    void finalise( std::uint32_t size );

protected:
    Detail::RIFFHeader riffHeader_;
    Detail::Format     format_    ;
    Detail::DataHeader dataHeader_;
}; // class OutputWaveFileBase


class OutputWaveFileImpl : public OutputWaveFileBase
{
public:
    OutputWaveFileImpl(                       ) = default;
#if defined( _MSC_VER ) && ( _MSC_VER < 1900 )
    OutputWaveFileImpl( OutputWaveFileImpl && other ) : OutputWaveFileBase( std::move( other ) ), stream_( std::move( other.stream_ ) ) {}
#else
    OutputWaveFileImpl( OutputWaveFileImpl && ) = default;
#endif // _MSC_VER < 1900
    LE_NOTHROW ~OutputWaveFileImpl();

    LE_NOTHROW char const * LE_FASTCALL create( Utility::File::Stream &&, std::uint8_t numberOfChannels, std::uint32_t sampleRate );
    LE_NOTHROW void         LE_FASTCALL close();
    LE_NOTHROW char const * LE_FASTCALL write( float const * pInput, std::uint32_t sampleFrames );

protected:
    Utility::File::Stream stream_;
}; // OutputWaveFileImpl

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4127 ) // Conditional expression is constant (in boost::lockfree::spsc_queue).
#endif // _MSC_VER
class OutputWaveFileAsyncImpl : public OutputWaveFileImpl
{
public:
    OutputWaveFileAsyncImpl()
        :
    #if defined( __APPLE__ )
        dispatcher_( nullptr ),
        counter_   ( 0       ),
        error_     ( false   )
    #else
        #if defined( __ANDROID__ )
            thread_    ( -1    ),
            stopWorker_( false ),
        #elif defined( _WIN32 )
            overlappedIOHandle_( INVALID_HANDLE_VALUE ),
        #endif
        error_( false )
    #endif
    {}
    OutputWaveFileAsyncImpl( OutputWaveFileAsyncImpl && );
    ~OutputWaveFileAsyncImpl() { join(); }
    OutputWaveFileAsyncImpl( OutputWaveFileAsyncImpl const & ) = delete;

#if 0 //...mrmlj...
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI create( char const * const pathToFile, std::uint8_t const numberOfChannels, std::uint32_t const sampleRate )
    {
        return OutputWaveFileImpl::create<rootLocation>( fileName, numberOfChannels, sampleRate );
    }
#endif

    LE_NOTHROW bool         LE_FASTCALL close();
    LE_NOTHROW char const * LE_FASTCALL write( float const * pInput, std::uint16_t sampleFrames );

    bool run ();
    void join();

private:
#ifdef __ANDROID__
    void worker();
#endif // __ANDROID__

private:
#ifdef __APPLE__
    // https://developer.apple.com/library/ios/documentation/Performance/Reference/GCD_libdispatch_Ref/index.html#//apple_ref/c/func/dispatch_io_write
    // http://amattn.com/p/grand_central_dispatch_gcd_summary_syntax_best_practices.html
    // http://www.objc.io/issue-2/low-level-concurrency-apis.html
    // http://nathanmock.com/files/com.apple.adc.documentation.AppleiOS6.0.iOSLibrary.docset/Contents/Resources/Documents/#documentation/FileManagement/Conceptual/FileSystemProgrammingGUide/TechniquesforReadingandWritingCustomFiles/TechniquesforReadingandWritingCustomFiles.html
    // portable libdispatch http://stackoverflow.com/questions/7542840/compiler-doesn-t-locate-gcd-header-file-on-windows
    dispatch_io_t             dispatcher_;
    std::atomic_uint_least8_t counter_   ;
    bool                      error_     ;
#else
#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4200 ) // Nonstandard extension used: zero-sized array in struct/union.
#endif // _MSC_VER
    struct BufferNode
    #ifdef _WIN32
        : OVERLAPPED
    #endif // _WIN32
    {
        BufferNode( std::uint16_t const sz, OutputWaveFileAsyncImpl & impl )
            : capacity( sz ), size( sz )
        {
        #ifdef _WIN32
            OVERLAPPED & overlappedIO( *this );
            overlappedIO = { 0 };
            overlappedIO.Offset     = static_cast<DWORD>( -1 );
            overlappedIO.OffsetHigh = static_cast<DWORD>( -1 );
            overlappedIO.hEvent     = reinterpret_cast<HANDLE>( &impl );
        #endif // _WIN32
        }

        std::uint16_t const capacity/*in frames*/;
        std::uint16_t       size    /*in frames*/;
    #ifdef _WIN32
        internal_sample_t   data[];
    #else
        float               data[];
    #endif // OS
    }; // class BufferNode
#ifdef _MSC_VER
    #pragma warning( push )
#endif // _MSC_VER

    // http://www.boost.org/doc/libs/release/doc/html/boost/lockfree/spsc_queue.html
    // https://github.com/arielelkin/BionicLooper/blob/master/TheAmazingAudioEngine/Modules/TPCircularBuffer/TPCircularBuffer.h
    // http://www.1024cores.net/home/lock-free-algorithms/queues/unbounded-spsc-queue
    static std::uint8_t const queueCapacity
    #if defined( _WIN32 ) || ( defined( __ANDROID__ ) && ( __LP64__ || defined( __i386__ ) ) )
        = 4;
    #elif defined( __ANDROID__ )
        = 10;
    #endif // _WIN32
    using Queue = boost::lockfree::spsc_queue<BufferNode *, boost::lockfree::capacity<queueCapacity>>;

#if defined( __ANDROID__ )
    Utility::ConditionVariable       bufferNotEmpty_;
    Utility::ConditionVariable       bufferNotFull_ ;
    Utility::ConditionVariable::Lock bufferLock_    ;

    /// \note Avoid std::thread STL dependency (libc++ vs libstdc++) and bloat
    /// (due to required static linking).
    ///                                       (09.12.2015.) (Domagoj Saric)
    ::pthread_t thread_;

    Queue queue_;

    volatile bool stopWorker_;
#elif defined( _WIN32 )
    HANDLE overlappedIOHandle_;
#endif // __ANDROID__
    bool error_;

    Queue cache_;
#endif // OS
}; // class OutputWaveFileAsyncImpl
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // outputWaveFileImpl_hpp
