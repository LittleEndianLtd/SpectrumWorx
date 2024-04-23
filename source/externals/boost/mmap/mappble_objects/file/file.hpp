////////////////////////////////////////////////////////////////////////////////
///
/// \file file.hpp
/// --------------
///
/// Copyright (c) Domagoj Saric 2010.-2013.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
/// (obsolete version, latest @ https://github.com/psiha/mmap
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef file_hpp__D3705ED0_EC0D_4747_A789_1EE17252B6E2
#define file_hpp__D3705ED0_EC0D_4747_A789_1EE17252B6E2
#pragma once
//------------------------------------------------------------------------------
#include "../../detail/impl_selection.hpp"

#include BOOST_MMAP_IMPL_INCLUDE( BOOST_PP_EMPTY, BOOST_PP_IDENTITY( /file.hpp  ) )
#include "flags.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

typedef file_open_flags<BOOST_MMAP_IMPL()> native_file_open_flags;

inline bool delete_file( char    const * const file_name ) { return delete_file( file_name, BOOST_MMAP_IMPL() () ); }
inline bool delete_file( wchar_t const * const file_name ) { return delete_file( file_name, BOOST_MMAP_IMPL() () ); }

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "file.inl"
#endif // BOOST_MMAP_HEADER_ONLY

#endif // file_hpp
