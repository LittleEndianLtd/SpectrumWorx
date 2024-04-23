////////////////////////////////////////////////////////////////////////////////
///
/// \file sharper.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef sharper_hpp__F315fE63_42EA_4CF4_AE4D_180C4A01F4DE
#define sharper_hpp__F315fE63_42EA_4CF4_AE4D_180C4A01F4DE 
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
/// \class Sharper
///
/// \ingroup Effects
///
/// \brief Apply sharper effect using high pass filter. 
///
/// The opposite of the Smoother effect, this effect sharpens the spectrum. 
/// The smoothed signal is subtracted from original to arrive at the sharpened 
/// signal. 
///
////////////////////////////////////////////////////////////////////////////////

struct Sharper
{
    LE_DEFINE_PARAMETERS
    (        
        ( ( AveragingWidth )( LinearUnsignedInteger )( Minimum<  0> )( Maximum<5000> )( Default<1000> )( Unit<' Hz'> ) )
        ( ( Intensity      )( LinearFloat           )( Minimum<  0> )( Maximum< +72> )( Default<  20> )( Unit<' dB'> ) )
        ( ( Limiter        )( LinearFloat           )( Minimum<-80> )( Maximum<   0> )( Default< -20> )( Unit<' dB'> ) )
    );

    /// \typedef AveragingWidth
    /// \brief Width of the region to be sharpened.
    /// \typedef Intensity
    /// \brief Increases the sharpening effect.
    /// \typedef Limiter
    /// \brief Hard limiter maintains stability.

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

#endif // sharper_hpp
