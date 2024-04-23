////////////////////////////////////////////////////////////////////////////////
///
/// \file talkBoxImpl.hpp
/// ---------------------
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef talkBoxImpl_hpp__4426F456_173B_4D8A_9A45_20F9E315AFC7
#define talkBoxImpl_hpp__4426F456_173B_4D8A_9A45_20F9E315AFC7
#pragma once
//------------------------------------------------------------------------------
#include "talkBox.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/vocoder/vocoderImpl.hpp"
#include "le/spectrumworx/effects/synth/synthImpl.hpp"
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

class TalkBoxImpl
    :
    public EffectImpl<TalkBox>
{
public: // LE::Effect required interface.
    //using ChannelState = CompoundChannelState<Vocoder::ChannelState, SynthImpl::ChannelState>;
    using ChannelState = SynthImpl::ChannelState;

    using TalkBox::title;
    using TalkBox::description;

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &                           , Engine::Setup const & )      ;
    void process( ChannelState &, Engine::ChannelData_ReIm2AmPh, Engine::Setup const & ) const;

    Vocoder::Parameters & vocoderParameters() { return vocoder_.parameters(); }

private:
    SynthImpl   synth_  ;
    VocoderImpl vocoder_;
}; // class TalkBoxImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // talkBoxImpl_hpp
