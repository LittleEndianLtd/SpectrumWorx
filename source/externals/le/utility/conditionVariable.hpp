////////////////////////////////////////////////////////////////////////////////
///
/// \file conditionVariable.hpp
/// ---------------------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef conditionVariable_hpp__78634241_A93B_4190_A9DC_3A607B8BC8D7
#define conditionVariable_hpp__78634241_A93B_4190_A9DC_3A607B8BC8D7
#pragma once
//------------------------------------------------------------------------------
#include "platformSpecifics.hpp"
#include "trace.hpp"
#include "windowsLite.hpp"

#include <boost/assert.hpp>

#ifndef _WIN32
    #include "criticalSection.hpp"

    #include <cerrno>
    #include <cstdint>
    #include <ctime>
    #include <pthread.h>
    #include <sys/time.h>
    #ifdef __APPLE__
        #include <chrono>
    #endif // __APPLE__
#endif // API
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

#ifdef _WIN32
    // Strategies for Implementing POSIX Condition Variables on Win32 http://www.cs.wustl.edu/~schmidt/win32-cv-1.html
    // http://developers.slashdot.org/story/07/02/26/1211220/pthreads-vs-win32-threads
    // http://nasutechtips.blogspot.com/2010/11/slim-read-write-srw-locks.html
    // https://mhesham.wordpress.com/tag/slim-read-write-lock
    // http://msdn.microsoft.com/en-us/magazine/cc163405.aspx

    class RWLock
    {
    public:
        LE_NOTHROW RWLock() /*: lock_ = { SRWLOCK_INIT }*/ { lock_ = { SRWLOCK_INIT }; }
        LE_NOTHROW RWLock( RWLock && other ) : lock_( other.lock_ ) { other.lock_ = { SRWLOCK_INIT }; }
       ~RWLock(                ) = default;
        RWLock( RWLock const & ) = delete ;

        LE_NOTHROW void   lock() { ::AcquireSRWLockExclusive( &lock_ ); }
        LE_NOTHROW void unlock() { ::ReleaseSRWLockExclusive( &lock_ ); }

        LE_NOTHROW bool tryLock() { return ::TryAcquireSRWLockExclusive( &lock_ ) != FALSE; }

    private: friend class ConditionVariable;
        ::SRWLOCK lock_;
    }; // class RWLock

    class ConditionVariable
    {
    public:
        using Lock = RWLock;

        ConditionVariable() /*: cv_{ CONDITION_VARIABLE_INIT }*/ { cv_ = { CONDITION_VARIABLE_INIT }; }
        LE_NOTHROW
        ConditionVariable( ConditionVariable && other ) : cv_( other.cv_ ) { other.cv_ = { CONDITION_VARIABLE_INIT }; }

       ~ConditionVariable(                           ) = default;
        ConditionVariable( ConditionVariable const & ) = delete ;

                    LE_NOTHROW void LE_FASTCALL signal(             ) { ::WakeAllConditionVariable( &cv_ ); }
        LE_NOINLINE LE_NOTHROW void LE_FASTCALL wait  ( Lock & lock ) { BOOST_VERIFY( wait( lock, INFINITE ) ); }

        LE_NOTHROW bool LE_FASTCALL wait( Lock & lock, std::uint32_t const milliseconds )
        {
            auto const result( ::SleepConditionVariableSRW( &cv_, &lock.lock_, milliseconds, 0/*CONDITION_VARIABLE_LOCKMODE_SHARED*/ ) );
            BOOST_ASSERT( result || ::GetLastError() == ERROR_TIMEOUT );
            return result != FALSE;
        }

    private:
        ::CONDITION_VARIABLE cv_;
    }; // class ConditionVariable
#else // POSIX
    // http://stackoverflow.com/questions/1277627/overhead-of-pthread-mutexes
    // http://www.cognitus.net/html/howto/pthreadSemiFAQ.html
    // http://stackoverflow.com/questions/14924469/does-pthread-cond-waitcond-t-mutex-unlock-and-then-lock-the-mutex
    // http://stackoverflow.com/questions/2763714/why-do-pthreads-condition-variable-functions-require-a-mutex

    // http://linux.die.net/man/3/pthread_rwlock_init
    // http://stackoverflow.com/questions/2699993/using-pthread-condition-variable-with-rwlock
    // https://blogs.oracle.com/roch/entry/beware_of_the_performance_of

    class ConditionVariable
    {
    public:
        using Lock = CriticalSection;

        LE_NOTHROW LE_COLD  ConditionVariable() : cv_( PTHREAD_COND_INITIALIZER ) {}
        LE_NOTHROW LE_COLD ~ConditionVariable() { BOOST_VERIFY( ::pthread_cond_destroy( &cv_ ) == 0 ); }

        ConditionVariable( ConditionVariable && other ) noexcept : cv_( other.cv_ ) { other.cv_ = PTHREAD_COND_INITIALIZER; }

        ConditionVariable( ConditionVariable const & ) = delete;

        LE_NOTHROW LE_COLD void LE_FASTCALL signal(             ) { BOOST_VERIFY( ::pthread_cond_broadcast( &cv_               ) == 0 ); }
        LE_NOTHROW LE_COLD void LE_FASTCALL wait  ( Lock & lock ) { BOOST_VERIFY( ::pthread_cond_wait     ( &cv_, &lock.mutex_ ) == 0 ); }

        LE_NOTHROW LE_COLD bool LE_FASTCALL wait  ( Lock & lock, std::uint16_t /*const*/ milliseconds )
        {
        #if defined( HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE ) // deprecated and unavailable in 64bit Android
            //::timespec const timeout = { .tv_sec = 0, .tv_nsec = static_cast<long>( milliseconds * 1000 * 1000 ) };
            std::uint8_t const seconds( milliseconds / 1000 );
            milliseconds %= 1000;
            ::timespec const timeout = { .tv_sec = seconds, .tv_nsec = static_cast<long>( milliseconds * 1000 * 1000 ) };
            auto const result( ::pthread_cond_timedwait_relative_np( &cv_, &lock.mutex_, &timeout ) );
        #elif !defined( __APPLE__ ) // no clock_gettime()
            ::timespec timeout;
            BOOST_VERIFY( ::clock_gettime( CLOCK_REALTIME, &timeout ) == 0 );
            timeout.tv_nsec += static_cast<long>( milliseconds * 1000 * 1000 );
            BOOST_ASSERT( timeout.tv_nsec < 1000 * 1000 * 1000 );
            auto const result( ::pthread_cond_timedwait( &cv_, &lock.mutex_, &timeout ) );
        #else
            #if 0
                // http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
                // http://stackoverflow.com/questions/11893992/implementing-condition-variable-timed-wait-correctly
                // http://pubs.opengroup.org/onlinepubs/009695399/functions/pthread_cond_timedwait.html
                // http://marc.info/?l=cfe-commits&m=134635459632338&w=2 (libc++ commit that added overflow checking)
                // https://llvm.org/bugs/show_bug.cgi?id=13721

                using namespace std::chrono;
                nanoseconds       nsec( system_clock::now().time_since_epoch() );
                seconds     const sec ( duration_cast<seconds>( nsec )         );
                nsec -= sec;

                ::timespec timeout;
                timeout.tv_sec  = static_cast<decltype( timeout.tv_sec  )>(  sec.count() );
                timeout.tv_nsec = static_cast<decltype( timeout.tv_nsec )>( nsec.count() );
                BOOST_ASSERT( timeout.tv_nsec < 1000 * 1000 * 1000 );
            #else
                ::timeval tv;
                BOOST_VERIFY( ::gettimeofday( &tv, nullptr ) == 0 );
                ::timespec const timeout = { .tv_sec = tv.tv_sec, .tv_nsec = static_cast<long>( ( tv.tv_usec + milliseconds * 1000 ) * 1000 ) };
            #endif
            auto const result( ::pthread_cond_timedwait( &cv_, &lock.mutex_, &timeout ) );
        #endif
            BOOST_ASSERT( result == 0 || result == ETIMEDOUT );
            return BOOST_LIKELY( result == 0 );
        }

    private:
        ::pthread_cond_t cv_;
    }; // class ConditionVariable
#endif // POSIX

class WaitableWithSharedLock
{
public: //...mrmlj...
    WaitableWithSharedLock() : signaled_( false ) {}

    WaitableWithSharedLock( WaitableWithSharedLock && other ) : cv_( std::move( other.cv_ ) ), signaled_( false ) { BOOST_ASSERT( !signaled_ ); }

    // notify
    LE_COLD void LE_FASTCALL notify_enter( ConditionVariable::Lock & lock ) {                   lock.lock  (); }
    LE_COLD void LE_FASTCALL notify      (                                ) { signaled_ = true; cv_ .signal(); }
    LE_COLD void LE_FASTCALL notify_exit ( ConditionVariable::Lock & lock ) {                   lock.unlock(); }

    // wait
    LE_COLD void LE_FASTCALL wait_enter( ConditionVariable::Lock & lock ) { lock.lock  ();                                                }
    LE_COLD void LE_FASTCALL wait      ( ConditionVariable::Lock & lock ) { while ( !signaled_ ) { cv_.wait( lock ); } signaled_ = false; }
    LE_COLD void LE_FASTCALL wait_exit ( ConditionVariable::Lock & lock ) { lock.unlock();                                                }

    LE_COLD bool LE_FASTCALL wait      ( ConditionVariable::Lock & lock, std::uint16_t const milliseconds )
    {
        while ( !signaled_ && cv_.wait( lock, milliseconds ) ) {}
        bool const result( signaled_ );
        signaled_ = false;
        return BOOST_LIKELY( result );
    }

private:
    ConditionVariable cv_;

    bool volatile signaled_;
}; // class WaitableWithSharedLock

class Waitable
{
public: //...mrmlj...
    // notify
    LE_COLD void LE_FASTCALL notify_enter() { impl_.notify_enter( lock_ ); }
    LE_COLD void LE_FASTCALL notify      () { impl_.notify      (       ); }
    LE_COLD void LE_FASTCALL notify_exit () { impl_.notify_exit ( lock_ ); }

    // wait
    LE_COLD void LE_FASTCALL wait_enter() { impl_.wait_enter( lock_ ); }
    LE_NOINLINE
    LE_COLD void LE_FASTCALL wait      () { impl_.wait      ( lock_ ); }
    LE_COLD void LE_FASTCALL wait_exit () { impl_.wait_exit ( lock_ ); }

private:
    ConditionVariable::Lock lock_;
    WaitableWithSharedLock  impl_;
}; // class Waitable

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // conditionVariable_hpp
