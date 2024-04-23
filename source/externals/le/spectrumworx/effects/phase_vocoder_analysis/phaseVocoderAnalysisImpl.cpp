////////////////////////////////////////////////////////////////////////////////
///
/// phaseVocoderAnalysisImpl.cpp
/// ----------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "phaseVocoderAnalysisImpl.hpp"

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
// PhaseVocoderAnalysis static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PhaseVocoderAnalysis::title      [] = "PVD start";
char const PhaseVocoderAnalysis::description[] = "Transform into the \"Phase vocoder domain\".";


////////////////////////////////////////////////////////////////////////////////
//
// PhaseVocoderAnalysisImpl::setup()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PhaseVocoderAnalysisImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    pvParameters_.setup( engineSetup );
}


////////////////////////////////////////////////////////////////////////////////
//
// PhaseVocoderAnalysisImpl::process()
// -----------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PhaseVocoderAnalysisImpl::process( ChannelState & state, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
#ifdef LE_PV_USE_TSS
    if ( data.pSynthesisState )
        data.pAnalysisState = &state;
#endif // LE_PV_USE_TSS
    PhaseVocoderShared::analysis( state, data.full(), pvParameters_ );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
