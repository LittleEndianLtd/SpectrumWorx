////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchShifter.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchShifter_hpp__4124BAA3_F146_4A5D_9C9B_BB09EFCE82DB
#define pitchShifter_hpp__4124BAA3_F146_4A5D_9C9B_BB09EFCE82DB
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

namespace Detail
{
    struct PitchShifterBase
    {
    #ifdef LE_PV_USE_TSS
        #define LE_PV_TSS_SENSITIVITY() ( ( TSSSensitivity )( LinearFloat )( Minimum<0> )( Default<65> )( Maximum<100> )( Unit<'%'> ) )
    #else
        #define LE_PV_TSS_SENSITIVITY()
    #endif // LE_PV_USE_TSS

        LE_DEFINE_PARAMETERS
        (
            ( ( SemiTones )( SymmetricFloat   )( MaximumOffset< 24> )( Unit<'\'' >  ) )
            ( ( Cents     )( SymmetricInteger )( MaximumOffset<100> )( Unit<'\'\''> ) )
            LE_PV_TSS_SENSITIVITY()
        );

        #undef LE_PV_TSS_SENSITIVITY

        /// \typedef SemiTones
        /// \brief Specifies the number of semitones to pitch shift.
        /// \typedef Cents
        /// \brief Specifies the number of cents to pitch shift (adds to
        /// semitones).

        static bool const usesSideChannel = false;

        static char const description[];
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchShifter
///
/// \ingroup Effects
///
/// \brief Pitch shift only into the selected band.
///
/// This module is as straight forward as it gets. It shifts the pitch of the 
/// incoming signal in semitones and cents. You can go anywhere between two 
/// octaves higher and two octaves lower than the original signal.
///
////////////////////////////////////////////////////////////////////////////////

struct PitchShifter : Detail::PitchShifterBase
{
    static char const title[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class PVPitchShifter
///
/// \ingroup Effects
///
/// \brief Pitch shift only into the selected band.
///
////////////////////////////////////////////////////////////////////////////////

struct PVPitchShifter : Detail::PitchShifterBase
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

#endif // pitchShifter_hpp
