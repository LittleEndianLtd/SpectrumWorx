////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchSpring.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchSpring_hpp__B5B2F53F_B59F_4E79_85A5_C741E0182BF5
#define pitchSpring_hpp__B5B2F53F_B59F_4E79_85A5_C741E0182BF5
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/commonParameters.hpp"
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

namespace Detail
{
    struct PitchSpringBase ///<
    {
        /// \name Parameters
        /// @{

        typedef CommonParameters::SpringType SpringType;

        LE_DEFINE_PARAMETERS
        (
            ( ( SpringType )                                                                                            )
            ( ( Depth      ) ( LinearSignedInteger   )( Minimum< 0> )( Maximum< 2400> )( Default< 200> )( Unit<'\"' > ) )
            ( ( Period     ) ( LinearUnsignedInteger )( Minimum<10> )( Maximum<10000> )( Default<1000> )( Unit<' ms'> ) )
        );

        /// @}

        /// \typedef SpringType
        /// \brief Determines the type of oscillation.
        /// \details
        ///   - Symmetric: pitch oscillates back and forth from a negative pitch 
        /// through the original pitch of the incoming signal and towards a 
        /// positive pitch.
        ///   - Up: pitch oscillates between the incoming pitch and a positive 
        /// (higher) pitch.
        ///   - Down: pitch oscillates between the incoming pitch and a negative 
        /// (lower) pitch.
        /// \typedef Depth
        /// \brief Determines the amount of pitch oscillation in semitones.
        /// \typedef Period
        /// \brief Period of the pitch oscillation.

        static bool const usesSideChannel = false;

        static char const description[];
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchSpring
///
/// \ingroup Effects
///
/// \brief Pulsating change of pitch.
///
/// This is a pitch oscillator where depth, period and direction of the 
/// oscillations are selectable. 
///
////////////////////////////////////////////////////////////////////////////////

struct PitchSpring : Detail::PitchSpringBase
{
    static char const title[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchSpringPVD
///
/// \ingroup Effects
///
/// \brief Pulsating change of pitch.
///
////////////////////////////////////////////////////////////////////////////////

struct PitchSpringPVD : Detail::PitchSpringBase
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

#endif // pitchSpring_hpp
