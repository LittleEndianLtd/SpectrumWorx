////////////////////////////////////////////////////////////////////////////////
///
/// \file wobbler.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef wobbler_hpp__A1154887_29E9_43FB_BFA9_4D29C344D513
#define wobbler_hpp__A1154887_29E9_43FB_BFA9_4D29C344D513
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/symmetric/parameter.hpp"

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
/// \class Wobbler
///
/// \ingroup Effects
///
/// \brief Input spectrum amplitude sine modulation.
///
/// This module creates a sinusoidal-based “wobbling” sound by changing the 
/// amplitudes of the input frequencies. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct Wobbler
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Amplitude )( LinearFloat           )( Minimum< 0> )( Maximum< +48> )( Default<  6> )( Unit<' dB'> ) )
        ( ( Period    )( LinearUnsignedInteger )( Minimum<10> )( Maximum<1000> )( Default<100> )( Unit<' ms'> ) )
        ( ( PreGain   )( SymmetricFloat        )( MaximumOffset<48> )                           ( Unit<' dB'> ) )
    );

    /// \typedef Amplitude
    /// \brief Intensity of the wobbling (or how much the sound seems to go up
    /// and down).
    /// \typedef Period
    /// \brief Rate of wobbling.
    /// \typedef PreGain
    /// \brief Shifts the whole input before the wobbling stage.
    /// \details It might be advisable to start with a negative PreGain value
    /// that reflects the positive setting of the Amplitude setting.

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

#endif // wobbler_hpp
