////////////////////////////////////////////////////////////////////////////////
///
/// convolverImpl.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "convolverImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
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
// Convolver static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Convolver::title      [] = "Convolver";
char const Convolver::description[] = "Convolution between main and side channels.";


////////////////////////////////////////////////////////////////////////////////
//
// Convolver UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Convolver::ConvolutionType, "Type"    )
EFFECT_PARAMETER_NAME( Convolver::GrabIR         , "Grab IR" )
EFFECT_PARAMETER_NAME( Convolver::Phase          , "Phase"   )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Convolver, ConvolutionType,
    (( Triggered , "Triggered"  ))
    (( Continuous, "Continuous" ))
)

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Convolver, Phase,
    (( Sum , "Sum"  ))
    (( Side, "Side" ))
    (( Main, "Main" ))
)

////////////////////////////////////////////////////////////////////////////////
//
// ConvolverImpl::setup()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void ConvolverImpl::setup( IndexRange const &, Engine::Setup const & )
{ 
    freeze_ = parameters().get<GrabIR>().consumeValue();
}


////////////////////////////////////////////////////////////////////////////////
//
// Convolver::process()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void ConvolverImpl::process( ChannelState & cs, Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    using namespace Math;

    //------------------------------------------------------------------------//

    bool const freeze( freeze_ & !cs.frozenFlagConsumed ); //...mrmlj...quick-workaround for non-deterministic relationship
    cs.frozenFlagConsumed = freeze_;                       //...mrmlj...between setup() and process() calls...

    if ( freeze )
    {
        // Take a snapshot of the Side channel:
        copy( data.full().side().amps  (), cs.frozenAmps   );
        copy( data.full().side().phases(), cs.frozenPhases );
    }

    //------------------------------------------------------------------------//

    float const * pSourceAmps  ;
    float const * pSourcePhases;
    switch ( parameters().get<ConvolutionType>().getValue() )
    {
        case ConvolutionType::Continuous:
            pSourceAmps   = data.side().amps  ().begin();
            pSourcePhases = data.side().phases().begin();
            break;

        case ConvolutionType::Triggered:
            pSourceAmps   = &cs.frozenAmps  [ data.beginBin() ];
            pSourcePhases = &cs.frozenPhases[ data.beginBin() ];
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    multiply( pSourceAmps, data.main().amps().begin(), data.main().amps().end() );

    switch ( parameters().get<Phase>().getValue() )
    {
        case Phase::Sum:
            add( pSourcePhases, data.main().phases().begin(), data.main().phases().end() );
            break;
        case Phase::Main:
            break;
        case Phase::Side:
            copy( data.side().phases(), data.main().phases() );
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
