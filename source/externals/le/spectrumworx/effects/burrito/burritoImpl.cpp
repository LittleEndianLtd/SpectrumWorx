////////////////////////////////////////////////////////////////////////////////
///
/// burritoImpl.cpp
/// ---------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "burritoImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
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
// Burrito static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Burrito::title      [] = "Burrito";
char const Burrito::description[] = "Combination at random locations.";


////////////////////////////////////////////////////////////////////////////////
//
// Burrito UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Burrito::Mode    , "Target creation" )
EFFECT_PARAMETER_NAME( Burrito::Range   , "Target range"    )
EFFECT_PARAMETER_NAME( Burrito::Period  , "Range period"    )
EFFECT_PARAMETER_NAME( Burrito::SideGain, "Side gain"       )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Burrito, Mode,
    (( Replace, "Replace" ))
    (( Sum    , "Sum"     ))
)


////////////////////////////////////////////////////////////////////////////////
//
// BurritoImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void BurritoImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{  
    range_    = parameters().get<Range>() * workingRange.size() / 100;
    period_   = engineSetup.numberOfChannels() * engineSetup.milliSecondsToSteps( parameters().get<Period>() );
 // sideGain_ = parameters().get<Factor>();
    sideGain_ = Math::dB2NormalisedLinear( parameters().get<SideGain>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// BurritoImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void BurritoImpl::process( ChannelState & channelState, Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    {
        // Count frames
        bool const wrappedAround( channelState.frameCounter.nextValueFor( period_ ).second );
        if ( wrappedAround )
        {
            // Clear old positions:
            channelState.positions.clear();

            // Random number of new replacements (limited to range_):
            IndexRange::value_type const range  ( Math::rangedRand( range_ ) );
            IndexRange::value_type const numBins( data.numberOfBins()        );
            for ( IndexRange::value_type k( 0 ); k < range; ++k )
            {
                // Random replacement positions:
                IndexRange::value_type const x( Math::rangedRand( numBins ) );
                channelState.positions[ x ] = true;
            }
        }
    }

    Mode::value_type const mode    ( parameters().get<Mode>() );
    float            const sideGain( sideGain_                );
    bool const * LE_RESTRICT pPosition( channelState.positions.begin() );
    while ( data )
    {
        // Replace:
        if ( *pPosition++ )
        {
            float       & mainAmp( data.main().amps().front() );
            float const   sideAmp( data.side().amps().front() );

            float newMainAmp( sideAmp * sideGain );

            switch ( mode )
            {
                case Mode::Replace: data.main().phases().front() = data.side().phases().front(); break;
                case Mode::Sum    : newMainAmp += mainAmp;                                       break;
                LE_DEFAULT_CASE_UNREACHABLE();
            }

            mainAmp = newMainAmp;
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
