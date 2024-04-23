////////////////////////////////////////////////////////////////////////////////
///
/// \file configuration.hpp
/// -----------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef configuration_hpp__58D224D7_933A_421B_98B8_CCB693BD44E7
#define configuration_hpp__58D224D7_933A_421B_98B8_CCB693BD44E7
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "stdint.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{
//------------------------------------------------------------------------------

#ifdef _WIN32
#   if !defined( _MSC_VER ) || ( _MSC_VER < 1600 )
#       error SW SDK requires Visual Studio 2010 SP1 or later
#   endif
#else // OS
#   if ( !defined( __clang__ ) && !defined( __GNUC__ ) ) || ( defined( __GNUC__ ) && ( __GNUC__ < 4 ) )
#       error SW SDK requires Clang 3.4 or GCC 4.7 or higher
#   endif
#endif // OS

////////////////////////////////////////////////////////////////////////////////
///
/// \brief SW Engine specific constants
///
////////////////////////////////////////////////////////////////////////////////

namespace Constants
{
    unsigned short const minimumFFTSize = 128 ;
    unsigned short const maximumFFTSize = 8192;
    unsigned short const defaultFFTSize = 2048;

    unsigned short const minimumOverlapFactor = 1;
    unsigned short const maximumOverlapFactor = 8;
    unsigned short const defaultOverlapFactor = 4;

    unsigned short const defaultSampleRate = 44100;

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4480 ) // Nonstandard extension used: specifying underlying type for enum.
#endif // _MSC_VER
    /// \brief DFT WOLA windowing function
    enum Window : /*std*/::uint8_t
    {
        Hann,
        Hamming,
        Blackman,
        BlackmanHarris,
        Gaussian,
        FlatTop,
        Welch,
        Triangle,
        Rectangle,

        NumberOfWindows
    };
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

    Window const defaultWindow = Hann;
} // namespace Constants

//------------------------------------------------------------------------------
} // namespace Engine
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // configuration_hpp
