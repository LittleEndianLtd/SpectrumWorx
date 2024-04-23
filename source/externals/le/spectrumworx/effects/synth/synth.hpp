////////////////////////////////////////////////////////////////////////////////
///
/// \file synth.hpp
/// ---------------
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef synth_hpp__84523FA6_3644_4D1D_9AA8_0C6C4E416D82
#define synth_hpp__84523FA6_3644_4D1D_9AA8_0C6C4E416D82
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/linear/parameter.hpp"
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

////////////////////////////////////////////////////////////////////////////////
///
/// \class Synth
///
/// \ingroup Effects Instruments
///
/// \brief <B>Advanced&Experimental:</B> a pure sine/tone generating effect
/// (i.e. an 'instrument').
/// \details The waveform is generated into the side-chain bus/signal.
///
////////////////////////////////////////////////////////////////////////////////

struct Synth
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Frequency       ) ( LinearFloat           )( Minimum<40> )( Maximum<8000> )( Default<110> )               )
        ( ( HarmonicSlope   ) ( LinearUnsignedInteger )( Minimum< 0> )( Maximum< 100> )( Default< 50> )( Unit< '%'> ) )
        ( ( FlangeIntensity ) ( LinearUnsignedInteger )( Minimum< 0> )( Maximum< 100> )( Default<  0> )( Unit< '%'> ) )
        ( ( FlangeOffset    ) ( SymmetricFloat        )( MaximumOffset<180> )          ( Default< 10> )( Unit< '°'> ) )
    );

    /// \typedef Frequency
    /// \brief .
    /// \typedef HarmonicSlope
    /// \brief .
    /// \typedef FlangeIntensity
    /// \brief .
    /// \typedef FlangeOffset
    /// \brief .

    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
}; // struct Synth

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // synth_hpp
