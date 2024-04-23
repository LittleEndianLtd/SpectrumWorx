////////////////////////////////////////////////////////////////////////////////
///
/// \file mapping.hpp
/// -----------------
///
/// Copyright (c) Domagoj Saric 2011.-2013.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef mapping_hpp__99837E03_86B1_42F5_A57D_69A6E828DD08
#define mapping_hpp__99837E03_86B1_42F5_A57D_69A6E828DD08
#pragma once
//------------------------------------------------------------------------------
#include "../../handles/posix/handle.hpp"
#include "../../mappble_objects/file/posix/mapping_flags.hpp"

#include "boost/config/suffix.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> struct mapping;

template <>
struct mapping<posix>
    :
    handle<posix>::reference
{
    typedef handle<posix>::native_handle_t         native_handle_t;
    typedef mapping                        const & reference;

    BOOST_STATIC_CONSTANT( bool, owns_parent_handle = false );

    mapping( native_handle_t const native_handle, file_mapping_flags<posix> const & view_mapping_flags_param )
        : handle<posix>::reference( native_handle ), view_mapping_flags( view_mapping_flags_param ) {}

    bool is_read_only() const { return ( view_mapping_flags.protection & file_mapping_flags<posix>::handle_access_rights::write ) == 0; }

    file_mapping_flags<posix> const view_mapping_flags;
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
