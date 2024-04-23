////////////////////////////////////////////////////////////////////////////////
///
/// freqnamicsImpl.cpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "freqnamicsImpl.hpp"

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
// Freqnamics static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Freqnamics::title      [] = "Freqnamics";
char const Freqnamics::description[] = "Limit and noise-gate.";


////////////////////////////////////////////////////////////////////////////////
//
// Freqnamics UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Freqnamics::LimiterThreshold  , "Limiter"    )
EFFECT_PARAMETER_NAME( Freqnamics::NoisegateThreshold, "Noise gate" )


////////////////////////////////////////////////////////////////////////////////
//
// FreqnamicsImpl::setup()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void FreqnamicsImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    float const zeroDecibel( engineSetup.maximumAmplitude() );

    thrLimiter_   = zeroDecibel * Math::dB2NormalisedLinear( parameters().get<LimiterThreshold  >() );
    thrNoisegate_ = zeroDecibel * Math::dB2NormalisedLinear( parameters().get<NoisegateThreshold>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// FreqnamicsImpl::process()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

void FreqnamicsImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    float const limit( thrLimiter_   );
    float const gate ( thrNoisegate_ );
    for ( auto & amp : data.amps() )
    {
        amp = ( amp < gate )
            ? 0
            : std::min( amp, limit );
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
