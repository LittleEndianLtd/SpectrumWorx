////////////////////////////////////////////////////////////////////////////////
///
/// \file open_flags.hpp
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
#ifndef open_flags_hpp__0F422517_D9AA_4E3F_B3E4_B139021D068E
#define open_flags_hpp__0F422517_D9AA_4E3F_B3E4_B139021D068E
#pragma once
//------------------------------------------------------------------------------
#include "../../../detail/posix.hpp"

#include "fcntl.h"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct file_open_flags;

typedef int flags_t;

// Implementation note:
//   Using structs with public members and factory functions to enable (almost)
// zero-overhead 'link-time' conversion to native flag formats and to allow the
// user to modify the created flags or create fully custom ones so that specific
// platform-dependent use-cases, not otherwise covered through the generic
// interface, can also be covered.
//                                            (10.10.2010.) (Domagoj Saric)

template <>
struct file_open_flags<posix>
{
    struct handle_access_rights
    {
        enum values
        {
            read    = O_RDONLY,
            write   = O_WRONLY,
            execute = O_RDONLY,
            all     = read | write | execute
        };
    };

    struct open_policy
    {
        enum value_type
        {
            create_new                      = O_CREAT | O_EXCL ,
            create_new_or_truncate_existing = O_CREAT | O_TRUNC,
            open_existing                   = 0                ,
            open_or_create                  = O_CREAT          ,
            open_and_truncate_existing      = O_TRUNC
        };
    };

    struct system_hints
    {
        enum values
        {
            random_access     = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( 0,        0, 0, O_RANDOM                     ),
            sequential_access = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( 0,        0, 0, O_SEQUENTIAL                 ),
            avoid_caching     = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( 0, O_DIRECT, 0, 0                            ),
            temporary         = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( 0,        0, 0, O_TEMPORARY | _O_SHORT_LIVED ),
        };
    };

    struct on_construction_rights
    {
        enum values
        {
            read    = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( S_IRUSR, S_IRUSR, S_IRUSR, _S_IREAD  ),
            write   = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( S_IWUSR, S_IWUSR, S_IWUSR, _S_IWRITE ),
            execute = BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( S_IXUSR, S_IXUSR, S_IXUSR, _S_IEXEC  )
        };
    };

    static file_open_flags<posix> create
    (
        flags_t handle_access_flags   ,
        open_policy::value_type       ,
        flags_t system_hints          ,
        flags_t on_construction_rights
    );

    static file_open_flags<posix> create_for_opening_existing_files
    (
        flags_t handle_access_flags,
        bool    truncate           ,
        flags_t system_hints
    );

    int oflag;
    int pmode;
};

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "open_flags.inl"
#endif // BOOST_MMAP_HEADER_ONLY

#endif // open_flags_hpp
