////////////////////////////////////////////////////////////////////////////////
///
/// \file robotizer.hpp
/// -------------------
/// 
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef robotizer_hpp__29E16506_821A_4004_B789_C7903123F1B7
#define robotizer_hpp__29E16506_821A_4004_B789_C7903123F1B7
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
/// \class Robotizer
///
/// \ingroup Effects
///
/// \brief Robotic sound effect.
///
/// Produces a classic robotic effect by flattening the phase of the signal. 
/// Effects run the gamut from pseudo-vocoder effects to unusual 
/// phase-cancellation filtering. 
///
////////////////////////////////////////////////////////////////////////////////

struct Robotizer
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

#endif // robotizer_hpp
