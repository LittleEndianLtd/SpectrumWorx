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
#ifndef mapping_flags_hpp__79CF82B8_F71B_4C75_BE77_98F4FB8A7FFA
#define mapping_flags_hpp__79CF82B8_F71B_4C75_BE77_98F4FB8A7FFA
#pragma once
//------------------------------------------------------------------------------
#include "../../../detail/posix.hpp"

#include "sys/mman.h"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct file_mapping_flags;

struct posix;

typedef int flags_t;

template <>
struct file_mapping_flags<posix>
{
    struct handle_access_rights
    {
        enum values
        {
            read    = PROT_READ ,
            write   = PROT_WRITE,
            execute = PROT_EXEC ,
            all     = read | write | execute
        };
    };

    struct share_mode
    {
        enum value_type
        {
            shared = MAP_SHARED,
            hidden = MAP_PRIVATE
        };
    };

    static file_mapping_flags<posix> create
    (
        flags_t                combined_handle_access_rights,
        share_mode::value_type share_mode
    );


    int protection;
    int flags     ;
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
