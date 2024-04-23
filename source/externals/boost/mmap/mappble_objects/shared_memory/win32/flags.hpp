////////////////////////////////////////////////////////////////////////////////
///
/// \file mapping_flags.hpp
/// -----------------------
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
#ifndef flags_hpp__504C3F9E_97C2_4E8C_82C6_881340C5FBA6
#define flags_hpp__504C3F9E_97C2_4E8C_82C6_881340C5FBA6
#pragma once
//------------------------------------------------------------------------------
#include "../../file/win32/mapping_flags.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct shared_memory_flags;

struct win32;

typedef int flags_t;

template <>
struct shared_memory_flags<win32> : file_mapping_flags<win32>
{
    struct system_hints
    {
        enum value_type
        {
            default_                   = 0x8000000,
            only_reserve_address_space = 0x4000000
        };
    };

    static shared_memory_flags<win32> create
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
