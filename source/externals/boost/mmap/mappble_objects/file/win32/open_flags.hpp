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
#ifndef open_flags_hpp__77AE8A6F_0E93_433B_A1F2_531BBBB353FC
#define open_flags_hpp__77AE8A6F_0E93_433B_A1F2_531BBBB353FC
#pragma once
//------------------------------------------------------------------------------
#include "boost/assert.hpp"
#include "boost/noncopyable.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct file_open_flags;

typedef int flags_t;

template <>
struct file_open_flags<win32>
{
    struct handle_access_rights
    {
        enum values
        {
            read    = 0x80000000L,
            write   = 0x40000000L,
            execute = 0x20000000L,
            all     = 0x10000000L
        };
    };

    struct open_policy
    {
        enum value_type
        {
            create_new                      = 1,
            create_new_or_truncate_existing = 2,
            open_existing                   = 3,
            open_or_create                  = 4,
            open_and_truncate_existing      = 5
        };
    };

    struct system_hints
    {
        enum values
        {
            random_access     = 0x10000000,
            sequential_access = 0x08000000,
            avoid_caching     = 0x20000000 | 0x80000000,
            temporary         = 0x00000100 | 0x04000000
        };
    };

    struct on_construction_rights
    {
        enum values
        {
            read    = 0x00000001,
            write   = 0x00000080,
            execute = 0x00000001,
        };
    };

    static file_open_flags<win32> create
    (
        flags_t handle_access_flags   ,
        open_policy::value_type       ,
        flags_t system_hints          ,
        flags_t on_construction_rights
    );

    static file_open_flags<win32> create_for_opening_existing_files
    (
        flags_t handle_access_flags,
        bool    truncate           ,
        flags_t system_hints
    );

    unsigned long desired_access      ;
    unsigned long creation_disposition;
    unsigned long flags_and_attributes;
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
