////////////////////////////////////////////////////////////////////////////////
///
/// \file phaseVocoderAnalysis.hpp
/// ------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef phaseVocoderAnalysisImpl_hpp__4045FD35_53B7_4FA5_A623_EA09E59959A8
#define phaseVocoderAnalysisImpl_hpp__4045FD35_53B7_4FA5_A623_EA09E59959A8
#pragma once
//------------------------------------------------------------------------------
#include "phaseVocoderAnalysis.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
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

class PhaseVocoderAnalysisImpl : public NoParametersEffectImpl<PhaseVocoderAnalysis>
{
public: // LE::Effect interface.
    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    typedef PhaseVocoderShared::AnalysisChannelState ChannelState;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
    PhaseVocoderShared::BaseParameters pvParameters_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // phaseVocoderAnalysisImpl_hpp
