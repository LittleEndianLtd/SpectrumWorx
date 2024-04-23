////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchMagnet.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchMagnet_hpp__745857B1_F6A4_4855_9FAE_07E1A776BC57
#define pitchMagnet_hpp__745857B1_F6A4_4855_9FAE_07E1A776BC57
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
    struct PitchMagnetBase
    {
        LE_DEFINE_PARAMETERS
        (
            ( ( Target ) ( LinearUnsignedInteger )( Minimum< 20> )( Maximum<2000> )( Default<200> )( Unit<' Hz'  > ) )
            ( ( Speed  ) ( LinearFloat           )( Minimum<  0> )( Maximum<  60> )( Default<  1> )( Unit<' \'/s'> ) )
        );

        /// \typedef Target
        /// \brief Target frequency to shift to.
        /// \typedef Speed
        /// \brief Speed of the pitch-shifting in semitones per second.

        static bool const usesSideChannel = false;

        static char const description[];
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchMagnet
///
/// \ingroup Effects
///
/// \brief Attracts target to given pitch.
///
/// Pitch Magnet forces the input pitch to the target pitch with selected speed. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct PitchMagnet : Detail::PitchMagnetBase
{
    static char const title[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchMagnetPVD
///
/// \ingroup Effects
///
/// \brief Attracts target to given pitch.
///
////////////////////////////////////////////////////////////////////////////////

struct PitchMagnetPVD : Detail::PitchMagnetBase
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

#endif // pitchMagnet_hpp
