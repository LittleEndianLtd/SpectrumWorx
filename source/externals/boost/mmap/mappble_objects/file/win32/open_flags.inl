////////////////////////////////////////////////////////////////////////////////
///
/// \file open_flags.inl
/// --------------------
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
#ifndef open_flags_inl__77AE8A6F_0E93_433B_A1F2_531BBBB353FC
#define open_flags_inl__77AE8A6F_0E93_433B_A1F2_531BBBB353FC
#pragma once
//------------------------------------------------------------------------------
#include "open_flags.hpp"

#include "../../detail/impl_inline.hpp"

#include "boost/static_assert.hpp"

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include "windows.h"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_STATIC_ASSERT( file_open_flags<win32>::handle_access_rights::read    == GENERIC_READ    );
BOOST_STATIC_ASSERT( file_open_flags<win32>::handle_access_rights::write   == GENERIC_WRITE   );
BOOST_STATIC_ASSERT( file_open_flags<win32>::handle_access_rights::execute == GENERIC_EXECUTE );
BOOST_STATIC_ASSERT( file_open_flags<win32>::handle_access_rights::all     == GENERIC_ALL     );

BOOST_STATIC_ASSERT( file_open_flags<win32>::system_hints::random_access     ==   FILE_FLAG_RANDOM_ACCESS                                );
BOOST_STATIC_ASSERT( file_open_flags<win32>::system_hints::sequential_access ==   FILE_FLAG_SEQUENTIAL_SCAN                              );
BOOST_STATIC_ASSERT( file_open_flags<win32>::system_hints::avoid_caching     == ( FILE_FLAG_NO_BUFFERING   | FILE_FLAG_WRITE_THROUGH   ) );
BOOST_STATIC_ASSERT( file_open_flags<win32>::system_hints::temporary         == ( FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE ) );

BOOST_STATIC_ASSERT( file_open_flags<win32>::on_construction_rights::read    == FILE_ATTRIBUTE_READONLY );
BOOST_STATIC_ASSERT( file_open_flags<win32>::on_construction_rights::write   == FILE_ATTRIBUTE_NORMAL   );
BOOST_STATIC_ASSERT( file_open_flags<win32>::on_construction_rights::execute == FILE_ATTRIBUTE_READONLY );

BOOST_IMPL_INLINE
file_open_flags<win32> file_open_flags<win32>::create
(
    flags_t                 const handle_access_flags   ,
    open_policy::value_type const open_flags            ,
    flags_t                 const system_hints          ,
    flags_t                 const on_construction_rights
)
{
    file_open_flags<win32> const flags =
    {
        static_cast<unsigned long>( handle_access_flags ), // desired_access
        static_cast<unsigned long>( open_flags          ), // creation_disposition
        static_cast<unsigned long>
        (
            system_hints
                |
            (
                ( on_construction_rights & FILE_ATTRIBUTE_NORMAL )
                    ? FILE_ATTRIBUTE_NORMAL
                    : on_construction_rights
            ) // flags_and_attributes
        )
    };

    return flags;
}


BOOST_IMPL_INLINE
file_open_flags<win32> file_open_flags<win32>::create_for_opening_existing_files
(
    flags_t const handle_access_flags,
    bool    const truncate,
    flags_t const system_hints
)
{
    return create
    (
        handle_access_flags,
        truncate
            ? open_policy::open_and_truncate_existing
            : open_policy::open_existing,
        system_hints,
        0
    );
}


//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------

#endif // open_flags_inl