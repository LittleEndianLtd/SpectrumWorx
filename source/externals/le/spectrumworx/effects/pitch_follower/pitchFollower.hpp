////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchFollower.hpp
/// -----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchFollower_hpp__2BDC51A5_6E70_4841_B2B6_E2C764777A79
#define pitchFollower_hpp__2BDC51A5_6E70_4841_B2B6_E2C764777A79
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

namespace Detail
{
    struct PitchFollowerBase
    {
        LE_DEFINE_PARAMETERS
        (
            ( ( Speed )( LinearFloat )( Minimum< 0> )( Maximum<60> )( Default< 1> )( Unit<' \'/s'> ) )
        );

        /// \typedef Speed
        /// \brief Determines how fast the pitch is followed (in semitones per
        /// second).

        static bool const usesSideChannel = true;

        static char const description[];
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchFollower
///
/// \ingroup Effects
///
/// \brief Relay pitch from side channel to main channel.
///
/// The main channel is pitch-shifted to the dominant frequency detected in 
/// the side-channel. The Speed parameter determines how quickly the 
/// pitch-shifting to the target frequency should be done in order to provide 
/// a smoother result.
/// 
////////////////////////////////////////////////////////////////////////////////

struct PitchFollower : Detail::PitchFollowerBase
{
    static char const title[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchFollowerPVD
///
/// \ingroup Effects
///
/// \brief Relay pitch from side channel to main channel.
///
////////////////////////////////////////////////////////////////////////////////

struct PitchFollowerPVD : Detail::PitchFollowerBase
{
    static char const title[];
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // pitchFollower_hpp
