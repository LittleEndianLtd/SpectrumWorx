////////////////////////////////////////////////////////////////////////////////
///
/// \file baseParameters.hpp
/// ------------------------
///
/// Base parameters included by all effects.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef baseParameters_hpp__8696E957_A8FB_469A_9725_CB6F63216163
#define baseParameters_hpp__8696E957_A8FB_469A_9725_CB6F63216163
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "parameters.hpp"
#include "le/parameters/factoryMacro.hpp"
#include "le/parameters/parameter.hpp"
#include "le/parameters/boolean/parameter.hpp"
#include "le/parameters/linear/parameter.hpp"
#include "le/parameters/symmetric/parameter.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

/// \addtogroup Effects
/// @{

namespace Effects
{
//------------------------------------------------------------------------------

/// \addtogroup Effects
/// @{

/// \brief Basic parameters included by all effects
namespace BaseParameters
{
/// \name Base parameters
/// \brief Basic parameters included by all effects
/// @{

LE_DEFINE_PARAMETERS
(
    ( ( Bypass         )( Boolean        )                                                              )
    ( ( Gain           )( SymmetricFloat )( MaximumOffset<20> )                         ( Unit<' dB'> ) )
    ( ( Wet            )( LinearFloat    )( Minimum<0> )( Maximum<100> )( Default<100> )( Unit<' %' > ) )
    ( ( StartFrequency )( LinearFloat    )( Minimum<0> )( Maximum<  1> )( Default<  0> )                )
    ( ( StopFrequency  )( LinearFloat    )( Minimum<0> )( Maximum<  1> )( Default<  1> )                )
);

/// \typedef Parameters
/// \brief Basic parameters shared/included by all effects.
///
/// - Bypass: Enables/disables the effect.
///
/// - Gain: Per effect attenuation or amplification, range [-20 dB, +20 dB]
///
/// - Wet: Dry/Wet blending between original and effected signal.
///
/// - StartFrequency: Normalised frequency value (range 0.0 to 1.0) representing
///                   starting frequency for the start-stop range where
///                   the effect will perform its action.
///
/// - StopFrequency: Normalised frequency value (range 0.0 to 1.0) representing
///                  ending frequency for the start-stop range where
///                  the effect will perform its action. Frequencies
///                  outside the range will remain unaffected.
///

/// @}
} // namespace BaseParameters
/// @}

//------------------------------------------------------------------------------
} // namespace Effects
/// @}
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // baseParameters_hpp
