////////////////////////////////////////////////////////////////////////////////
///
/// \file phasevolution.hpp
/// -----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef phasevolution_hpp__F60D23C3_A21E_4D23_B742_3E9E59D1D3F4
#define phasevolution_hpp__F60D23C3_A21E_4D23_B742_3E9E59D1D3F4
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
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
/// \class Phasevolution
///
/// \ingroup Effects
///
/// \brief Accelerated phase change.
///
/// Creates an accelerated change in phase over time, which results in a 
/// pulsating pitch effect. Demands a short frame size for proper operation.  
/// Works better on signals with lots of clear harmonics. 
///
////////////////////////////////////////////////////////////////////////////////

struct Phasevolution
{
    LE_DEFINE_PARAMETERS
    (
        ( ( PhasePeriod ) ( LinearFloat )( Minimum<1> )( Maximum<5000> )( Default<500> )( ValuesDenominator<1000> )( Unit<' s'> ) )
    );

    /// \typedef PhasePeriod
    /// \brief Controls the rate at which the phase comes full circle.

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

#endif // phasevolution_hpp
