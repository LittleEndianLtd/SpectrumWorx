////////////////////////////////////////////////////////////////////////////////
///
/// \file handle.hpp
/// ----------------
///
/// Copyright (c) Domagoj Saric 2010 - 2015.
///
///  Use, modification and distribution is subject to the
///  Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef handle_hpp__1CEA6D65_D5C0_474E_833D_2CE927A1C74D
#define handle_hpp__1CEA6D65_D5C0_474E_833D_2CE927A1C74D
#pragma once
//------------------------------------------------------------------------------
#ifdef BOOST_MSVC
    #include "../posix/handle.hpp"
#endif

#include "../handle_ref.hpp"
#include "../../implementations.hpp"

#include "boost/noncopyable.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> class handle;

template <>
class handle<win32> : noncopyable
{
public:
    typedef void *                      native_handle_t;
    typedef handle_ref< handle<win32> > reference;

    static native_handle_t const invalid_handle;

    explicit handle<win32>( native_handle_t );
    handle<win32>( handle<win32> && source ) : handle_( source.handle_ ) { const_cast<native_handle_t &>( source.handle_ ) = invalid_handle; }
    ~handle<win32>();

    native_handle_t const & get() const { return handle_; }

    bool operator! () const { return !handle_; }
    operator reference () const { return reference( handle_ ); }

private:
    native_handle_t const handle_;
}; // class handle<win32>

#ifdef BOOST_MSVC
    handle<posix> make_posix_handle( handle<win32>::reference, int flags );
#endif // BOOST_MSVC

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "handle.inl"
#endif // BOOST_MMAP_HEADER_ONLY

#endif // handle_hpp
