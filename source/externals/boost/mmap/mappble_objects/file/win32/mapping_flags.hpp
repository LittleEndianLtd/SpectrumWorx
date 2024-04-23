////////////////////////////////////////////////////////////////////////////////
///
/// \file mapping_flags.hpp
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
#ifndef mapping_flags_hpp__CD518463_D4CB_4E18_8E35_E0FBBA8CA1D1
#define mapping_flags_hpp__CD518463_D4CB_4E18_8E35_E0FBBA8CA1D1
#pragma once
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct file_mapping_flags;

struct win32;

typedef int flags_t;

template <>
struct file_mapping_flags<win32>
{
    struct handle_access_rights
    {
        enum values
        {
            read    = 0x0004,
            write   = 0x0002,
            execute = 0x0020,
            all     = read | write | execute
        };
    };

    struct share_mode
    {
        enum value_type
        {
            shared = 0,
            hidden = 0x0001
        };
    };

    static file_mapping_flags<win32> create
    (
        flags_t                combined_handle_access_rights,
        share_mode::value_type share_mode
    );


    unsigned int create_mapping_flags;
    unsigned int map_view_flags      ;
};


//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "mapping_flags.inl"
#endif

#endif // mapping_flags.hpp
