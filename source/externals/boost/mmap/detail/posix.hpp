////////////////////////////////////////////////////////////////////////////////
///
/// \file posix.hpp
/// ---------------
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
#ifndef posix_hpp__8FC3F669_80D4_4455_829D_F72E8ABDE9D0
#define posix_hpp__8FC3F669_80D4_4455_829D_F72E8ABDE9D0
#pragma once
//------------------------------------------------------------------------------
#include "boost/config.hpp"

#if defined( BOOST_HAS_UNISTD_H )
    #include "boost/config/posix_features.hpp"
#elif defined( BOOST_MSVC )
    #pragma warning ( disable : 4996 ) // "The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name."
    #include "io.h"
    #include "wchar.h"
#else
    #error no suitable POSIX implementation found
#endif // BOOST_MSVC

#if defined( BOOST_MSVC )
    #define BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( standard, linux, osx, msvc ) msvc
#elif defined( __APPLE__ )
    #define BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( standard, linux, osx, msvc ) osx
#elif defined( _GNU_SOURCE )
    #define BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( standard, linux, osx, msvc ) linux
#else
    #define BOOST_MMAP_POSIX_STANDARD_LINUX_OSX_MSVC( standard, linux, osx, msvc ) standard
#endif // POSIX impl
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------


//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------
#endif // posix_hpp
