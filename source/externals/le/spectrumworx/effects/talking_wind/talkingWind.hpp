////////////////////////////////////////////////////////////////////////////////
///
/// \file talkingWind.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef talkingWind_hpp__4426F456_173B_4D8A_9A45_20F9E315AFC7
#define talkingWind_hpp__4426F456_173B_4D8A_9A45_20F9E315AFC7
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
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

////////////////////////////////////////////////////////////////////////////////
///
/// \class TalkingWind
///
/// \ingroup Effects
///
/// \brief Classic vocoder.
///
/// A classic vocoding effect where the frequency spectrum of the modulator is
/// used to modulate the carrier. This is achieved via cross-synthesis.
/// Cross-synthesis is a technique of impressing the spectral envelope of one
/// sound on the (flattened) spectrum of another. The modulator may be a voice,
/// and the carrier may be any spectrally rich sound.
///
////////////////////////////////////////////////////////////////////////////////

struct TalkingWind
{
    LE_DEFINE_PARAMETERS
    (
        ( ( EnvelopeBorder )( LinearUnsignedInteger )( Minimum<  0> )( Maximum<12000> )( Default<1000> )( Unit<' Hz'> ) )
        ( ( EnvelopeGain   )( LinearFloat           )( Minimum<-10> )( Maximum<   10> )( Default<   0> )( Unit<' dB'> ) )
    );


    /// \typedef EnvelopeBorder
    /// \brief Controls the perceived "smoothness" of the modulator.
    /// \typedef EnvelopeGain
    /// \brief Adds gain to envelope to amplify the effect.


    static bool const usesSideChannel = true;

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

#endif // talkingWind_hpp
