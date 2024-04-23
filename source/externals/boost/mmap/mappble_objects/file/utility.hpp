////////////////////////////////////////////////////////////////////////////////
///
/// \file utility.hpp
/// -----------------
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
#ifndef utility_hpp__3713A8AF_A516_4A23_BE6A_2BB79EBF7B5F
#define utility_hpp__3713A8AF_A516_4A23_BE6A_2BB79EBF7B5F
#pragma once
//------------------------------------------------------------------------------
#include "../../mapped_view/mapped_view.hpp"

#include <cstddef>
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

basic_mapped_view_ref           map_file          ( char const * file_name, std::size_t desired_size );
basic_mapped_read_only_view_ref map_read_only_file( char const * file_name                           );

#ifdef _WIN32
basic_mapped_view_ref           map_file          ( wchar_t const * file_name, std::size_t desired_size );
basic_mapped_read_only_view_ref map_read_only_file( wchar_t const * file_name                           );
#endif // _WIN32

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "utility.inl"
#endif // BOOST_MMAP_HEADER_ONLY

#endif // utility_hpp
