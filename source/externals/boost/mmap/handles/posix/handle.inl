////////////////////////////////////////////////////////////////////////////////
///
/// \file handle.inl
/// ----------------
///
/// Copyright (c) Domagoj Saric 2010.-2013.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
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

#ifdef BOOST_MSVC
    #pragma warning ( disable : 4996 ) // "The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name."
    #include "io.h"
#else
    #include "sys/mman.h"      // mmap, munmap.
    #include "sys/stat.h"
    #include "sys/types.h"     // struct stat.
    #include "unistd.h"        // sysconf.
#endif // BOOST_MSVC
#include "errno.h"
#include "fcntl.h"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_IMPL_INLINE
handle<posix>::handle( native_handle_t const handle )
    :
    handle_( handle )
{}

BOOST_IMPL_INLINE
handle<posix>::~handle()
{
    BOOST_VERIFY
    (
        ( ::close( handle_ ) == 0 ) ||
        (
            ( handle_ == -1    ) &&
            ( errno   == EBADF )
        )
    );                
}

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------
