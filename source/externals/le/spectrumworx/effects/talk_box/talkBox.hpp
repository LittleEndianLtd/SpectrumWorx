////////////////////////////////////////////////////////////////////////////////
///
/// \file talkBox.hpp
/// -----------------
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef talkBox_hpp__5FCE18B2_5D73_4BE1_A361_A694B2655204
#define talkBox_hpp__5FCE18B2_5D73_4BE1_A361_A694B2655204
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/spectrumworx/effects/synth/synth.hpp"
#include "le/parameters/boolean/parameter.hpp"
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
/// \class TalkBox
///
/// \ingroup Effects
///
/// \brief Vocoder with a builtin 'robotic' carrier.
///
////////////////////////////////////////////////////////////////////////////////

struct TalkBox
{
    /// \name Parameters
    /// @{
    typedef Synth::HarmonicSlope   HarmonicSlope  ;
    typedef Synth::FlangeIntensity FlangeIntensity;
    typedef Synth::FlangeOffset    FlangeOffset   ;
    /// @}

    LE_DEFINE_PARAMETERS
    (
        ( ( ExternalCarrier ) ( Boolean ) )
        ( ( BaseFrequency   ) ( LinearFloat )( Minimum<40> )( Maximum<400> )( Default<100> ) )
        ( ( CutOff          ) ( LinearUnsignedInteger )( Minimum<0> )( Maximum<12000> )( Default<9000> )( Unit< 'Hz'> ) )
        ( ( HarmonicSlope   ) )
        ( ( FlangeIntensity ) )
        ( ( FlangeOffset    ) )
    );

    /// \typedef BaseFrequency
    /// \brief .
    /// \typedef Flanging
    /// \brief .
    /// \typedef Whispering
    /// \brief .

    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
}; // struct TalkBox

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // talkBox_hpp
