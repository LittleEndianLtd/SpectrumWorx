////////////////////////////////////////////////////////////////////////////////
///
/// \file impl_selection.hpp
/// ------------------------
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
#ifndef impl_selection_hpp__05AF14B5_B23B_4CB8_A253_FD2D07B37ECF
#define impl_selection_hpp__05AF14B5_B23B_4CB8_A253_FD2D07B37ECF
#pragma once
//------------------------------------------------------------------------------
#include "boost/config.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/stringize.hpp"

// Implementation note:
//   "Anti-pattern" forward includes to reduce the verbosity of files that
// include this header.
//                                            (26.08.2011.) (Domagoj Saric)
#include "boost/preprocessor/facilities/empty.hpp"
#include "boost/preprocessor/facilities/identity.hpp"
//------------------------------------------------------------------------------
#if defined( _WIN32 )
    #define BOOST_MMAP_IMPL() win32
#elif defined( _WIN32_WINNT )
    #define BOOST_MMAP_IMPL() nt
#elif defined( BOOST_HAS_UNISTD_H )
    #define BOOST_MMAP_IMPL() posix
#endif

#define BOOST_MMAP_IMPL_INCLUDE( prefix_path, include ) \
    BOOST_PP_STRINGIZE( prefix_path()BOOST_MMAP_IMPL()include() )

//------------------------------------------------------------------------------
#endif // impl_selection_hpp
