////////////////////////////////////////////////////////////////////////////////
///
/// \file mapping_flags.inl
/// -----------------------
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
#ifndef mapping_flags_inl__CD518463_D4CB_4E18_8E35_E0FBBA8CA1D1
#define mapping_flags_inl__CD518463_D4CB_4E18_8E35_E0FBBA8CA1D1
#pragma once
//------------------------------------------------------------------------------
#include "mapping_flags.hpp"

#include "../../../detail/windows.hpp"

#include "boost/static_assert.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_STATIC_ASSERT( file_mapping_flags<win32>::handle_access_rights::read    == FILE_MAP_READ    );
BOOST_STATIC_ASSERT( file_mapping_flags<win32>::handle_access_rights::write   == FILE_MAP_WRITE   );
BOOST_STATIC_ASSERT( file_mapping_flags<win32>::handle_access_rights::execute == FILE_MAP_EXECUTE );

BOOST_STATIC_ASSERT( file_mapping_flags<win32>::share_mode::shared == 0             );
BOOST_STATIC_ASSERT( file_mapping_flags<win32>::share_mode::hidden == FILE_MAP_COPY );


BOOST_IMPL_INLINE
file_mapping_flags<win32> file_mapping_flags<win32>::create
(
    flags_t                const combined_handle_access_flags,
    share_mode::value_type const share_mode
)
{
    file_mapping_flags flags;

    flags.create_mapping_flags = ( combined_handle_access_flags & handle_access_rights::execute ) ? PAGE_EXECUTE : PAGE_NOACCESS;
    if ( share_mode == share_mode::hidden ) // WRITECOPY
        flags.create_mapping_flags *= 8;
    else
    if ( combined_handle_access_flags & handle_access_rights::write )
        flags.create_mapping_flags *= 4;
    else
    {
        BOOST_ASSERT( combined_handle_access_flags & handle_access_rights::read );
        flags.create_mapping_flags *= 2;
    }

    flags.map_view_flags = combined_handle_access_flags;

    return flags;
}

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------

#endif // mapping_flags.inl