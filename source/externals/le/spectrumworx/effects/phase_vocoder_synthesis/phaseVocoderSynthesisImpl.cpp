////////////////////////////////////////////////////////////////////////////////
///
/// phaseVocoderSynthesis.cpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "phaseVocoderSynthesisImpl.hpp"

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
// PhaseVocoderSynthesis static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PhaseVocoderSynthesis::title      [] = "PVD stop";
char const PhaseVocoderSynthesis::description[] = "Transform back from the \"Phase vocoder domain\".";


////////////////////////////////////////////////////////////////////////////////
//
// PhaseVocoderSynthesisImpl::process()
// ------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PhaseVocoderSynthesisImpl::process( ChannelState & state, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
#ifdef LE_PV_USE_TSS
    data.pSynthesisState = &state;
#endif // LE_PV_USE_TSS
    PhaseVocoderShared::synthesis( state, data.full().phases(), pvParameters_ );
}


void PhaseVocoderSynthesisImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    pvParameters_.setup( engineSetup );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
