////////////////////////////////////////////////////////////////////////////////
///
/// \file freqverbImpl.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef freqverbImpl_hpp__338562C9_17C6_4009_876E_4756164099DC
#define freqverbImpl_hpp__338562C9_17C6_4009_876E_4756164099DC
#pragma once
//------------------------------------------------------------------------------
#include "freqverb.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
#include "le/spectrumworx/engine/buffers.hpp"

#include <cstdint>
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

class FreqverbImpl : public EffectImpl<Freqverb>
{
public: // LE::Effect required interface.
    LE_NAMED_DYNAMIC_CHANNEL_STATE
    (
        ChannelState,
        ( ( Engine::HalfFFTBuffer<>                        )( feedbackSumReals ) )
        ( ( Engine::HalfFFTBuffer<>                        )( feedbackSumImags ) )
        ( ( PhaseVocoderShared::PitchShifter::ChannelState )( ps               ) )
    );


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_ReIm, Engine::Setup const & ) const;

private:
    float         roomLevel_;
    std::uint16_t noEchoBin_;

    PhaseVocoderShared::PitchShifter ps_;
}; // class FreqverbImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // freqverbImpl_hpp
