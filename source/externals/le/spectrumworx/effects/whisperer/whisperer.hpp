////////////////////////////////////////////////////////////////////////////////
///
/// \file whisperer.hpp
/// -------------------
/// 
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef whisperer_hpp__7E88348A_B8DC_4D2C_B805_7E845F3B5ACF
#define whisperer_hpp__7E88348A_B8DC_4D2C_B805_7E845F3B5ACF
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "boost/config/abi_prefix.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Whisperer
///
/// \ingroup Effects
///
/// \brief Creates a whispering sound. Use with small frame sizes.
///
/// Provides a whispering effect by randomizing the phases. 
/// 
/// \note Needs a short frame size for proper operation. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct Whisperer
{
    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // whisperer_hpp
