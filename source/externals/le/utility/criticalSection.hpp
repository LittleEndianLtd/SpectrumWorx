////////////////////////////////////////////////////////////////////////////////
///
/// \file criticalSection.hpp
/// -------------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef criticalSection_hpp__FA935666_72C2_4CED_934B_2B9CE788D195
#define criticalSection_hpp__FA935666_72C2_4CED_934B_2B9CE788D195
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

/// \note The boost::signals2::mutex class is not recursive under POSIX.
///                                           (27.09.2013.) (Domagoj Saric)
/// \todo  Cleanup the code base so that it does not require recursive mutexes
/// or at least minimise and document where they are required...
///                                           (22.03.2016.) (Domagoj Saric)
#ifdef _WIN32
#include <boost/signals2/mutex.hpp>
#else // POSIX
#include <pthread.h>
#endif // OS
#include <boost/noncopyable.hpp>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

#ifdef _WIN32

using CriticalSection = boost::signals2::mutex;

#else // POSIX

class CriticalSection
{
public:
    LE_NOTHROW LE_COLD CriticalSection()
    #ifdef PTHREAD_RECURSIVE_MUTEX_INITIALIZER
        : mutex_( PTHREAD_RECURSIVE_MUTEX_INITIALIZER ) {}
    #else // PTHREAD_RECURSIVE_MUTEX_INITIALIZER ...mrmlj...for osx10.6 only? recheck...
     {
        ::pthread_mutexattr_t attributes;
         BOOST_VERIFY( ::pthread_mutexattr_init   ( &attributes                          ) == 0 );
         BOOST_VERIFY( ::pthread_mutexattr_settype( &attributes, PTHREAD_RECURSIVE_MUTEX ) == 0 );
         BOOST_VERIFY( ::pthread_mutex_init       ( &mutex_, &attributes                 ) == 0 );
    }
    #endif // PTHREAD_RECURSIVE_MUTEX_INITIALIZER
    LE_NOTHROW LE_COLD ~CriticalSection() { BOOST_VERIFY( ::pthread_mutex_destroy( &mutex_ ) == 0 ); }

    LE_NOTHROW void LE_FASTCALL   lock() { BOOST_VERIFY( ::pthread_mutex_lock  ( &mutex_ ) == 0 ); }
    LE_NOTHROW void LE_FASTCALL unlock() { BOOST_VERIFY( ::pthread_mutex_unlock( &mutex_ ) == 0 ); }

    LE_NOTHROW bool LE_FASTCALL try_lock() { return ::pthread_mutex_trylock( &mutex_ ) == 0; }

    enum RecursiveType { NonRecursive }; //...mrmlj...
    LE_NOTHROW LE_COLD explicit CriticalSection( RecursiveType )
    #ifdef NDEBUG
        : mutex_( PTHREAD_MUTEX_INITIALIZER            ) {}
    #else
        : mutex_( PTHREAD_ERRORCHECK_MUTEX_INITIALIZER ) {}
    #endif // NDEBUG

    CriticalSection( CriticalSection && other ) : mutex_( other.mutex_ ) { other.mutex_ = PTHREAD_RECURSIVE_MUTEX_INITIALIZER; }

    CriticalSection( CriticalSection const & ) = delete;

private: friend class ConditionVariable; //...mrmlj...
    ::pthread_mutex_t mutex_;
}; // class CriticalSection

#endif // OS

class CriticalSectionLock : boost::noncopyable
{
#if defined( __GNUC__ ) || _MSC_VER >= 1900 //...mrmlj...no 'RVO@compiletime'...
public:
    explicit LE_NOTHROW  CriticalSectionLock( CriticalSection & cs ) : pCriticalSection_( &cs ) {                          pCriticalSection_->lock  (); }
             LE_NOTHROW ~CriticalSectionLock(                      )                            { if ( pCriticalSection_ ) pCriticalSection_->unlock(); }
             LE_NOTHROW  CriticalSectionLock( CriticalSectionLock && lock ) : pCriticalSection_( lock.pCriticalSection_ ) { lock.pCriticalSection_ = nullptr; }
private:
    CriticalSection * LE_RESTRICT pCriticalSection_;
#else
public:
    explicit LE_NOTHROW  CriticalSectionLock( CriticalSection & cs ) : criticalSection_( cs ) { criticalSection_.lock  (); }
             LE_NOTHROW ~CriticalSectionLock(                      )                          { criticalSection_.unlock(); }
private:
    CriticalSection & criticalSection_;
#endif // GCC or Clang
}; // class CriticalSectionLock

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // criticalSection_hpp
