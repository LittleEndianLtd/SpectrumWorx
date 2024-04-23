////////////////////////////////////////////////////////////////////////////////
///
/// \file quietBoost.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef quietBoost_hpp__5E3F3097_A492_4350_8DFC_1E0B575039A0
#define quietBoost_hpp__5E3F3097_A492_4350_8DFC_1E0B575039A0
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
/// \class QuietBoost
///
/// \ingroup Effects
///
/// \brief Amplifies low magnitudes.
///
/// Amplifies signal below the desired threshold, working as a sort of expander 
/// in the time domain. The ratio of expansion can be selected, and there is a 
/// noise threshold control. Effect will not boost anything below this threshold.
/// 
////////////////////////////////////////////////////////////////////////////////

struct QuietBoost
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Threshold          )( LinearFloat )( Minimum<-60> )( Maximum< 0> )( Default< -25> )( Unit<' dB'> ) )        
        ( ( Ratio              )( LinearFloat )( Minimum<  1> )( Maximum<15> )( Default<   3> )                )
        ( ( NoiseGateThreshold )( LinearFloat )( Minimum<-60> )( Maximum< 0> )( Default< -45> )( Unit<' dB'> ) )        
    );

    /// \typedef Threshold
    /// \brief Noise threshold.
    /// \typedef Ratio
    /// \brief Ratio of expansion to apply to the signal.
    /// \typedef NoiseGateThreshold
    /// \brief Threshold below which the effect will not be applied.

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

#endif // quietBoost_hpp
