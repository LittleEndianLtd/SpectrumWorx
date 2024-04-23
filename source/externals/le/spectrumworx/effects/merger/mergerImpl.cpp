////////////////////////////////////////////////////////////////////////////////
///
/// mergerImpl.cpp
/// --------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "mergerImpl.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"
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
// Merger static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Merger::title      [] = "Merger";
char const Merger::description[] = "Conditional combinations.";


////////////////////////////////////////////////////////////////////////////////
//
// Merger UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Merger::Threshold, "Threshold" )
EFFECT_PARAMETER_NAME( Merger::Operation, "Condition" )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Merger, Operation,
    (( MainLargerThanSide, "Main>Side" ))
    (( SideLargerThanMain, "Side>Main" ))
    (( MainAboveThreshold, "Main>Thr"  ))
    (( SideAboveThreshold, "Side>Thr"  ))
    (( MainBelowThreshold, "Main<Thr"  ))
    (( SideBelowThreshold, "Side<Thr"  ))
)


////////////////////////////////////////////////////////////////////////////////
//
// MergerImpl::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void MergerImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
  //threshold_ = parameters().get<Threshold>();
    threshold_ = engineSetup.maximumAmplitude() * Math::dB2NormalisedLinear( parameters().get<Threshold>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// MergerImpl::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void MergerImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    // Possible threshold value sources
    ReadOnlyDataRange const &       mainAmps ( static_cast<Engine::ChannelData_AmPh const &>( data.main() ).amps() );
    ReadOnlyDataRange const &       sideAmps (                                                data.side()  .amps() );
    ReadOnlyDataRange         const threshold( &threshold_, &threshold_ + 1                                        );

    ReadOnlyDataRange const * pLargerValue ;
    ReadOnlyDataRange const * pSmallerValue;
    switch ( parameters().get<Operation>().getValue() )
    {
        case Operation::MainLargerThanSide: pLargerValue = &mainAmps ; pSmallerValue = &sideAmps ; break;
        case Operation::SideLargerThanMain: pLargerValue = &sideAmps ; pSmallerValue = &mainAmps ; break;
        case Operation::MainAboveThreshold: pLargerValue = &mainAmps ; pSmallerValue = &threshold; break;
        case Operation::MainBelowThreshold: pLargerValue = &threshold; pSmallerValue = &mainAmps ; break;
        case Operation::SideAboveThreshold: pLargerValue = &sideAmps ; pSmallerValue = &threshold; break;
        case Operation::SideBelowThreshold: pLargerValue = &threshold; pSmallerValue = &sideAmps ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }

    while ( data )
    {
        if ( *pLargerValue->begin() > *pSmallerValue->begin() )
        {
            data.main().amps  ().front() = data.side().amps  ().front();
            data.main().phases().front() = data.side().phases().front();
        }
        ++data;
    }
}


/// \todo Make a new effect out of the code left from Merger.
///                                    (05.02.2010.) (Danijel Domazet)
///

/*
case Operation::Collusion:

if ( data.side().amps()[ k ] < data.amps()[ k ] * threshold_ )
{
data.amps()[ k ] += data.side().amps()[ k ];
}
else
{
/// \todo See if this should be abs of the whole thing.
///                                    (08.01.2010.) (Ivan Dokmanic)
data.amps()[ k ] = std::abs( data.amps()[ k ] ) - std::abs( data.side().amps()[ k ] );
if ( data.amps()[ k ] < 1E-20f )
{
data.amps()[ k ] = 1E-20f;
}
if ( data.phases()[ k ] == 0.f )
{
data.phases()[ k ] = data.side().phases()[ k ];
}
}
break;

case Operation::InvertedCollusion:
if ( data.side().amps()[ k ] > data.amps()[ k ] * threshold_ )
{
data.amps()[ k ] += data.side().amps()[ k ];
}
else
{
/// \todo See if this should be abs of the whole thing.
///                                    (08.01.2010.) (Ivan Dokmanic)
data.amps()[ k ] = std::abs( data.amps()[ k ] ) - std::abs( data.side().amps()[ k ] );
if ( data.amps()[ k ] < 1E-20f )
{
data.amps()[ k ] = 1E-20f;
}
if ( data.phases()[ k ] == 0.f )
{
data.phases()[ k ] = data.side().phases()[ k ];
}
}
break;

case Operation::Forsk:

if ( ( data.amps()[ k ] < threshold_ ) && ( data.amps()[ k ] < data.side().amps()[ k ] ) )
{
data.amps()[ k ] = data.side().amps()[ k ];
data.phases    [ k ] = data.side().phases()    [ k ];
}
break;

case Operation::InvertedForsk:

if ( ( data.amps()[ k ] > threshold_ ) && ( data.amps()[ k ] > data.side().amps()[ k ] ) )
{
data.amps()[ k ] = data.side().amps()[ k ];
data.phases    [ k ] = data.side().phases()    [ k ];
}
break;

case Operation::Mode13:

if ( data.side().amps()[ k ] < data.amps()[ k ] * threshold_ )
{
data.amps()[ k ] = data.side().amps()[ k ] * 2;
data.phases    [ k ] = data.side().phases()    [ k ];
}
else
{
data.amps()[ k ] = 0.0f;
data.phases    [ k ] = 0.0f;
}
break;
*/
//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
