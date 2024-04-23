////////////////////////////////////////////////////////////////////////////////
///
/// \file handle.inl
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
#include "handle.hpp"

#include "../../detail/impl_inline.hpp"

#include "boost/assert.hpp"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include "windows.h"

#ifdef BOOST_MSVC
    #include "io.h"
#endif // BOOST_MSVC
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
__declspec( selectany )
#endif // BOOST_MMAP_HEADER_ONLY
handle<win32>::native_handle_t const handle<win32>::invalid_handle = INVALID_HANDLE_VALUE;


BOOST_IMPL_INLINE
handle<win32>::handle( native_handle_t const handle )
    :
    handle_( handle )
{}


BOOST_IMPL_INLINE
handle<win32>::~handle()
{
    BOOST_VERIFY
    (
        ( ::CloseHandle( handle_ ) != false         ) ||
        ( handle_ == 0 || handle_ == invalid_handle )
    );
}


#ifdef BOOST_MSVC
    BOOST_IMPL_INLINE
    handle<posix> make_posix_handle( handle<win32>::native_handle_t const native_handle, int const flags )
    {
        return handle<posix>( ::_open_osfhandle( reinterpret_cast<intptr_t>( native_handle ), flags ) );
    }
#endif // BOOST_MSVC

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------

#ifndef BOOST_MMAP_HEADER_ONLY
    #include "../posix/handle.inl"
#endif // BOOST_MMAP_HEADER_ONLY
