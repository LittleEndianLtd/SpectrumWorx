////////////////////////////////////////////////////////////////////////////////
///
/// quietBoostImpl.cpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "quietBoostImpl.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
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
//
// QuietBoost static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const QuietBoost::title      [] = "Quiet Boost";
char const QuietBoost::description[] = "Amplify low magnitudes.";


////////////////////////////////////////////////////////////////////////////////
//
// QuietBoost UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( QuietBoost::Threshold         , "Threshold"       )
EFFECT_PARAMETER_NAME( QuietBoost::Ratio             , "Ratio"           )
EFFECT_PARAMETER_NAME( QuietBoost::NoiseGateThreshold, "Noise threshold" )


////////////////////////////////////////////////////////////////////////////////
//
// QuietBoostImpl::process()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

void QuietBoostImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & setup ) const
{
    float const maxGlobal( setup.maximumAmplitude() );

    float const threshold         ( parameters().get<Threshold         >() );
    float const ratio             ( parameters().get<Ratio             >() );
    float const noiseGateThreshold( parameters().get<NoiseGateThreshold>() );

    /// \todo Convert all the parameters to linear values to avoid the linear<->
    /// dB conversions inside the loop.
    ///                                       (13.10.2011.) (Domagoj Saric)

    for ( auto & amp : data.amps() )
    {
        // Transfer magnitudes to decibel scale, take global Max as a reference:
        float mag( Math::normalisedLinear2dB( amp / maxGlobal + 0.000000001f ) );

        // Expander:

        // If magnitude lower than threshold, "expand" it:
        if ( mag < threshold && mag > noiseGateThreshold )
            mag = threshold - ( threshold - mag ) / ratio;

        // Back from dB:
        amp = maxGlobal * Math::dB2NormalisedLinear( mag );
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
