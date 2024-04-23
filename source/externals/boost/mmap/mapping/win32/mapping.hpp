////////////////////////////////////////////////////////////////////////////////
///
/// \file mapping.hpp
/// -----------------
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
#ifndef mapping_hpp__8B2CEDFB_C87C_4AA4_B9D0_8EF0A42825F2
#define mapping_hpp__8B2CEDFB_C87C_4AA4_B9D0_8EF0A42825F2
#pragma once
//------------------------------------------------------------------------------
#include "../../handles/win32/handle.hpp"
#include "../../mappble_objects/file/win32/mapping_flags.hpp"

#include "boost/config.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct mapping;

template <>
struct mapping<win32>
    :
    handle<win32>
{
    typedef handle<win32>::native_handle_t         native_handle_t;
    typedef mapping                        const & reference;

    BOOST_STATIC_CONSTANT( bool, owns_parent_handle = true );

    mapping( native_handle_t const native_handle, unsigned int const view_mapping_flags_param )
        : handle<win32>( native_handle ), view_mapping_flags( view_mapping_flags_param ) {}

    bool is_read_only() const { return ( view_mapping_flags & file_mapping_flags<win32>::handle_access_rights::write ) == 0; }

    unsigned int const view_mapping_flags;
};

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "mapping.inl"
#endif // BOOST_MMAP_HEADER_ONLY

#endif // mapping_hpp
