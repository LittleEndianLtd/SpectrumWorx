////////////////////////////////////////////////////////////////////////////////
///
/// etherealImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "etherealImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/math/conversion.hpp"
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
// Ethereal static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Ethereal::title      [] = "Ethereal";
char const Ethereal::description[] = "Compare and replace.";


////////////////////////////////////////////////////////////////////////////////
//
// Ethereal UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Ethereal::Threshold, "Threshold"      )
EFFECT_PARAMETER_NAME( Ethereal::Condition, "Swap condition" )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Ethereal, Condition,
    (( DiffHigher, "Main - Side > Thr." ))
    (( DiffLower , "Main - Side < Thr." ))
)


////////////////////////////////////////////////////////////////////////////////
//
// EtherealImpl::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void EtherealImpl::setup( IndexRange const &, Engine::Setup const & )
{
    threshold_ = Math::dB2NormalisedLinear( parameters().get<Threshold>() );

    mode_.unpack( parameters().get<Mode>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// EtherealImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void EtherealImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    bool const replaceWhenWeaker( parameters().get<Condition>() == Condition::DiffLower );

    float const threshold( threshold_ );

    Engine::MainSideChannelData_AmPh const & constData( data );
    ReadOnlyDataRange const & ampSource  ( mode_.magnitudes() ? constData.side().amps  () : constData.main().amps  () );
    ReadOnlyDataRange const & phaseSource( mode_.phases    () ? constData.side().phases() : constData.main().phases() );

    while ( data )
    {
        bool const sideIsWeaker ( data.side().amps().front() < ( data.main().amps().front() * threshold ) );
        bool const shouldReplace( sideIsWeaker == replaceWhenWeaker                                       );

        if ( shouldReplace )
        {
            data.main().amps  ().front() = ampSource  .front();
            data.main().phases().front() = phaseSource.front();
        }

        ++data;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
