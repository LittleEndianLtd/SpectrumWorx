////////////////////////////////////////////////////////////////////////////////
///
/// \file sumoPitch.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef sumoPitch_hpp__FD93AFF5_C447_439F_96EC_CED05B70909C
#define sumoPitch_hpp__FD93AFF5_C447_439F_96EC_CED05B70909C
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
/// \class SumoPitch
///
/// \ingroup Effects
///
/// \brief Blends pitch-shifted signals.
///
/// What happens when your channels are pitted against one another like Sumo 
/// wrestlers? The input signal and side-channel are analyzed with the internal 
/// pitch detector. Both are then pitch-shifted towards each other and towards 
/// a center pitch. After both signals have been shifted, they are blended 
/// together. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct SumoPitch
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Blend )( LinearUnsignedInteger )( Minimum<0> )( Maximum<100> )( Default<50> )( Unit<' %'>    ) )
        ( ( Speed )( LinearFloat           )( Minimum<0> )( Maximum< 60> )( Default< 1> )( Unit<' \'/s'> ) )
    );

    /// \typedef Blend
    /// \brief Controls the amount of blending between main and side channel
    /// after they have been pitch-shifted.
    /// \details When set to 0, only the main channel is output. A setting of
    /// 100% sends only the side-channel to the output.
    /// \typedef Speed
    /// \brief Controls the speed of the shift in semitones per second.

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

#endif // sumoPitch_hpp
