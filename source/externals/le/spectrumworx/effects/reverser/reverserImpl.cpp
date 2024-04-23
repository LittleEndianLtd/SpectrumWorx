////////////////////////////////////////////////////////////////////////////////
///
/// reverserImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "reverserImpl.hpp"

#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/buffers.hpp"
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
// Reverser static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Reverser::title      [] = "Reverser";
char const Reverser::description[] = "Play chunks backwards.";


////////////////////////////////////////////////////////////////////////////////
//
// Reverser UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Reverser::Length, "Chunk length" )


///////////////////////////////////////////////////////////////////////////////
//
// ReverserImpl::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void ReverserImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    lengthInSteps_ = engineSetup.milliSecondsToSteps( parameters().get<Length>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// ReverserImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void ReverserImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    using namespace Math;

    auto const fullNumberOfBins( data.full().numberOfBins() );

    ReversedHistoryBufferState::HistoryData const historyData
    (
        cs.getCurrentStepData( lengthInSteps_, fullNumberOfBins )
    );

    //   First we save the new input data and output the current history data.
    // After a chunk/step/frame of history is output we have no more need for it
    // so we can overwrite it with current/new input data. To accomplish saving
    // the current history chunk to output and the current input data to
    // the same location (the current history chunk) without an auxiliary buffer
    // the Math::swap() function is used. Since the individual history chunks
    // were already saved time-reversed we in effect play/output the
    // time-reversed version of the signal history.
    //                                        (21.05.2010.) (Domagoj Saric)
    auto const startBin( data.beginBin() );
    swap( data.amps  ().begin(), data.amps  ().end(), historyData.targetHistory.pAmplitudesOrReals + startBin );
    swap( data.phases().begin(), data.phases().end(), historyData.targetHistory.pPhasesOrImags     + startBin );

    data.copySkippedRanges( Engine::DataPair::Amps  , historyData.targetHistory.pAmplitudesOrReals );
    data.copySkippedRanges( Engine::DataPair::Phases, historyData.targetHistory.pPhasesOrImags     );

    // Time reverse the new, saved, history frame.
    // Implementation note:
    //   Here we use the time-reversing property of the DFT
    // ( https://ccrma.stanford.edu/~jos/ReviewFourier/Time_Reversal.html ) to
    // produce a time-reversed signal while staying in the frequency domain. As
    // we are dealing with real signals that have symmetrical/conjugated DFTs we
    // do not actually need to reverse all of the DFT data but only to negate
    // the phases (because reversing has no effect on amplitudes as they are
    // symmetrical, while on phases it has the effect of negation because they
    // are anti-symmetrical).
    //                                        (14.05.2010.) (Domagoj Saric)
    negate( historyData.targetHistory.pPhasesOrImags, fullNumberOfBins );

    // If the requested history data was emulated, the output (from
    // historyData.targetHistory) contains garbage so we must copy the emulated
    // history to the output).
    if ( historyData.isEmulated() )
    {
        auto const numberOfBins( data.numberOfBins() );
        copy( historyData.sourceHistory.pAmplitudesOrReals + startBin, data.amps  ().begin(), numberOfBins );
        copy( historyData.sourceHistory.pPhasesOrImags     + startBin, data.phases().begin(), numberOfBins );
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
