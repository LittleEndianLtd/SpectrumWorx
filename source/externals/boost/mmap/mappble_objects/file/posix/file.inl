////////////////////////////////////////////////////////////////////////////////
///
/// \file file.inl
/// --------------
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
#ifndef file_inl__1E2F9841_1C6C_40D9_9AA7_BAC0003CD909
#define file_inl__1E2F9841_1C6C_40D9_9AA7_BAC0003CD909
#pragma once
//------------------------------------------------------------------------------
#include "file.hpp"

#include "open_flags.hpp"
#include "../../../detail/impl_inline.hpp"
#include "../../../detail/posix.hpp"
#include "../../../mapping/posix/mapping.hpp"

#include "boost/assert.hpp"

#include "errno.h"
#include "fcntl.h"
#include "sys/stat.h"
#include "sys/types.h"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_IMPL_INLINE
file_handle<posix> create_file( char const * const file_name, file_open_flags<posix> const & flags )
{
    typedef file_handle<posix> posix_file_handle;

    BOOST_ASSERT( file_name );

    int const current_mask( ::umask( 0 ) );
    int const file_handle ( ::open( file_name, flags.oflag, flags.pmode ) );
    //...zzz...investigate posix_fadvise, posix_madvise, fcntl for the system hints...
    BOOST_VERIFY( ::umask( current_mask ) == 0 );

    return posix_file_handle( file_handle );
}

#ifdef BOOST_MSVC
BOOST_IMPL_INLINE
file_handle<posix> create_file( wchar_t const * const file_name, file_open_flags<posix> const & flags )
{
    BOOST_ASSERT( file_name );

    int const current_mask( ::umask( 0 ) );
    int const file_handle ( ::_wopen( file_name, flags.oflag, flags.pmode ) );
    BOOST_VERIFY( ::umask( current_mask ) == 0 );

    return file_handle<posix>( file_handle );
}
#endif // BOOST_MSVC


BOOST_IMPL_INLINE
bool delete_file( char const * const file_name, posix )
{
    return ::unlink( file_name ) == 0;
}

#ifdef BOOST_MSVC
BOOST_IMPL_INLINE
bool delete_file( wchar_t const * const file_name, posix )
{
    return ::_wunlink( file_name ) == 0;
}
#endif // BOOST_MSVC


#ifdef BOOST_HAS_UNISTD_H
BOOST_IMPL_INLINE
bool set_size( file_handle<posix>::reference const file_handle, std::size_t const desired_size )
{
    return ::ftruncate( file_handle, desired_size ) != -1;
}
#endif // BOOST_HAS_UNISTD_H


BOOST_IMPL_INLINE
std::size_t get_size( file_handle<posix>::reference const file_handle )
{
    struct stat file_info;
    BOOST_VERIFY( ( ::fstat( file_handle, &file_info ) == 0 ) || ( file_handle == -1 ) );
    return file_info.st_size;
}


#ifdef BOOST_HAS_UNISTD_H
// Apple guidelines http://developer.apple.com/library/mac/#documentation/Performance/Conceptual/FileSystem/Articles/MappingFiles.html
BOOST_IMPL_INLINE
mapping<posix> create_mapping( file_handle<posix>::reference const file, file_mapping_flags<posix> const & flags )
{
    return mapping<posix>( file, flags );
}
#endif // BOOST_HAS_UNISTD_H

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------

#endif // file_inl
