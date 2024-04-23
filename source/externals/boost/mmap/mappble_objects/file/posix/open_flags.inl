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
#ifndef open_flags_inl__0F422517_D9AA_4E3F_B3E4_B139021D068E
#define open_flags_inl__0F422517_D9AA_4E3F_B3E4_B139021D068E
#pragma once
//------------------------------------------------------------------------------
#include "open_flags.hpp"

#include "../../../detail/impl_inline.hpp"

#include "boost/assert.hpp"

#include "errno.h"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_IMPL_INLINE
file_open_flags<posix> file_open_flags<posix>::create
(
    flags_t                 const handle_access_flags   ,
    open_policy::value_type const open_flags            ,
    flags_t                 const system_hints          ,
    flags_t                 const on_construction_rights
)
{
    //...zzz...use fadvise...
    // http://stackoverflow.com/questions/2299402/how-does-one-do-raw-io-on-mac-os-x-ie-equivalent-to-linuxs-o-direct-flag

    unsigned int const oflag
    (
        ( ( handle_access_flags == ( O_RDONLY | O_WRONLY ) ) ? O_RDWR : handle_access_flags )
                |
            open_flags
                |
            system_hints
    );

    unsigned int const pmode( on_construction_rights );

    file_open_flags<posix> const flags =
    {
        static_cast<int>( oflag ),
        static_cast<int>( pmode )
    };
    return flags;
}


BOOST_IMPL_INLINE
file_open_flags<posix> file_open_flags<posix>::create_for_opening_existing_files
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