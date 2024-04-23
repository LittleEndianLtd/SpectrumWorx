////////////////////////////////////////////////////////////////////////////////
///
/// \file flags.hpp
/// ---------------
///
/// Copyright (c) Domagoj Saric 2010 - 2014.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef flags_hpp__F9CD9C91_1F07_4107_A422_0D814F0FE487
#define flags_hpp__F9CD9C91_1F07_4107_A422_0D814F0FE487
#pragma once
//------------------------------------------------------------------------------
#include "../../file/posix/mapping_flags.hpp"
#include "../../../detail/posix.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct shared_memory_flags;

struct posix;

typedef int flags_t;

template <>
struct shared_memory_flags<posix> : file_mapping_flags<posix>
{
    struct system_hints
    {
        enum value_type
        {
            default_                   = 0,
            only_reserve_address_space = MAP_NORESERVE
        };
    };

    static shared_memory_flags<posix> create
    (
        flags_t                  combined_handle_access_rights,
        share_mode  ::value_type share_mode,
        system_hints::value_type system_hint
    );
};


//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "flags.inl"
#endif

#endif // flags.hpp
