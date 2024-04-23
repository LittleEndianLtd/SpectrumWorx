////////////////////////////////////////////////////////////////////////////////
///
/// tonalImpl.cpp
/// -------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "tonalImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
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
// Tonal static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Tonal::title      [] = "Tonal";
char const Tonal::description[] = "Suppress non-tonal regions.";


////////////////////////////////////////////////////////////////////////////////
//
// Atonal static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Atonal::title      [] = "Atonal";
char const Atonal::description[] = "Suppress tonal regions.";


////////////////////////////////////////////////////////////////////////////////
//
// Tonal UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Detail::TonalBase::Strength       , "Peak strength"    )
EFFECT_PARAMETER_NAME( Detail::TonalBase::GlobalThreshold, "Global threshold" )
EFFECT_PARAMETER_NAME( Detail::TonalBase::LocalThreshold , "Local threshold"  )
EFFECT_PARAMETER_NAME( Detail::TonalBase::Attenuation    , "Attenuation"      )


////////////////////////////////////////////////////////////////////////////////
//
// Atonal UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Atonal::Strength       , "Peak strength"    )
EFFECT_PARAMETER_NAME( Atonal::GlobalThreshold, "Global threshold" )
EFFECT_PARAMETER_NAME( Atonal::Attenuation    , "Attenuation"      )


namespace Detail
{
    void TonalBaseImpl::setup( Engine::Setup const & engineSetup )
    {   
        pd_.setZeroDecibelValue( engineSetup.maximumAmplitude() );
    }
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
//
// TonalImpl::setup()
// ------------------
//
////////////////////////////////////////////////////////////////////////////////

void TonalImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    TonalBaseImpl::setup<Tonal>( parameters(), engineSetup );
}


////////////////////////////////////////////////////////////////////////////////
//
// TonalImpl::setup()
// ------------------
//
////////////////////////////////////////////////////////////////////////////////

void AtonalImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    TonalBaseImpl::setup<Atonal>( parameters(), engineSetup );
}


////////////////////////////////////////////////////////////////////////////////
//
// TonalImpl::process()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void TonalImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    unsigned int const numberOfBins( data.numberOfBins() );
    pd_.findPeaks        ( data.amps().begin(), numberOfBins );
    pd_.attenuateNonPeaks( data.amps().begin(), 0, numberOfBins - 1, parameters().get<Tonal::Attenuation>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// AtonalImpl::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void AtonalImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    unsigned int const numberOfBins( data.numberOfBins() );
    pd_.findPeaks     ( data.amps().begin(), numberOfBins );
    pd_.attenuatePeaks( data.amps().begin(), 0, numberOfBins - 1, parameters().get<Atonal::Attenuation>() );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
