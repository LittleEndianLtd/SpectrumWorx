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
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef file_hpp__FB482005_18D9_4E3B_9193_A13DBFE88F45
#define file_hpp__FB482005_18D9_4E3B_9193_A13DBFE88F45
#pragma once
//------------------------------------------------------------------------------
#include "../../../implementations.hpp"
#include "../../../mapping/mapping.hpp"
#include "../handle.hpp"

#include "boost/cstdint.hpp"

#include <cstddef>
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl  > struct file_open_flags;
template <typename Impl  > struct file_mapping_flags;
template <class    Handle> struct is_resizable;

template <> struct is_resizable< file_handle<win32> > : mpl::true_ {};


file_handle<win32> create_file( char    const * file_name, file_open_flags<win32> const & );
file_handle<win32> create_file( wchar_t const * file_name, file_open_flags<win32> const & );
bool               delete_file( char    const * file_name, win32                          );
bool               delete_file( wchar_t const * file_name, win32                          );


bool        set_size( file_handle<win32>::reference, std::size_t desired_size );
std::size_t get_size( file_handle<win32>::reference                           );


mapping<win32> create_mapping( file_handle<win32>::reference, file_mapping_flags<win32> const & );

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "file.inl"
#endif // BOOST_MMAP_HEADER_ONLY

#endif // file_hpp
