////////////////////////////////////////////////////////////////////////////////
///
/// wobblerImpl.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "wobblerImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
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
// Wobbler static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Wobbler::title      [] = "Wobbler";
char const Wobbler::description[] = "Amplitude modulation.";


////////////////////////////////////////////////////////////////////////////////
//
// Wobbler UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Wobbler::Amplitude, "Amplitude" )
EFFECT_PARAMETER_NAME( Wobbler::Period   , "Period"    )
EFFECT_PARAMETER_NAME( Wobbler::PreGain  , "Offset"    )


////////////////////////////////////////////////////////////////////////////////
//
// WobblerImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void WobblerImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    period_ = engineSetup.milliSecondsToSteps( parameters().get<Period>() );
}   


////////////////////////////////////////////////////////////////////////////////
//
// WobblerImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void WobblerImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    using namespace Math;

    float const amplitude( parameters().get<Amplitude>() );
    float const pregain  ( parameters().get<PreGain  >() );

    float const gain
    (
        dB2NormalisedLinear
        (
            pregain + amplitude * std::sin( Math::Constants::twoPi * convert<float>( cs.frameCounter.value() ) / convert<float>( period_ ) )
        )
    );

    cs.frameCounter.nextValueFor( period_ );

    multiply( data.amps(), gain );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
