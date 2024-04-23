////////////////////////////////////////////////////////////////////////////////
///
/// \file colorifer.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef colorifer_hpp__B6E8308A_87E9_4d87_8E9E_C537504C54DA
#define colorifer_hpp__B6E8308A_87E9_4d87_8E9E_C537504C54DA
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/enumerated/parameter.hpp"
#include "le/parameters/linear/parameter.hpp"

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
/// \class Colorifer
///
/// \ingroup Effects
///
/// \brief Spectral colouring.
///
/// Transfers the frequency shape from the side-channel to the signal arriving 
/// through the main input. It calculates main/side over the selected bandwidth, 
/// and then applies this "colour" to main. There is an option to also replace 
/// phases of the main channel with those from side.
/// 
////////////////////////////////////////////////////////////////////////////////

struct Colorifer
{
    LE_ENUMERATED_PARAMETER( SpectrumPreprocess, ( NotUsed )( SquareRoot )( Square )( Exponential ) );
    LE_ENUMERATED_PARAMETER( ReplacePhase      , ( No )( Yes )                                      );

    LE_DEFINE_PARAMETERS
    (
        ( ( SpectrumPreprocess ) )
        ( ( BandWidth          ) ( LinearUnsignedInteger )( Minimum<0> )( Maximum<6000> )( Default<1000> )( Unit<' Hz'> ) )
        ( ( ReplacePhase       ) )
    );

    /// \typedef SpectrumPreprocess
    /// \brief Specifies if preprocessing is done on the input signal.
    /// \details
    ///   - NotUsed: no preprocessing done.
    ///   - SquareRoot: square root applied to signal before further calculation.
    ///   - Square: square applied to signal before further calculation.
    ///   - Exponential: exponent applied to signal before further calculation.
    /// \typedef BandWidth
    /// \brief Bandwidth of the signal to take colour from.
    /// \typedef ReplacePhase
    /// \brief Specifies if input should take over side channel's phases.

    static bool const usesSideChannel = true;

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

#endif // colorifer_hpp
