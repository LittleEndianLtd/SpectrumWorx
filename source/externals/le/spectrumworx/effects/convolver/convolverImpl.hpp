////////////////////////////////////////////////////////////////////////////////
///
/// \file convolverImpl.hpp
/// -----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef convolverImpl_hpp__A7F10DAF_A6F4_44E2_B6B4_460805ACC405
#define convolverImpl_hpp__A7F10DAF_A6F4_44E2_B6B4_460805ACC405
#pragma once
//------------------------------------------------------------------------------
#include "convolver.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/engine/buffers.hpp"
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

class ConvolverImpl : public EffectImpl<Convolver>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_DYNAMIC_CHANNEL_STATE
    (
        ( ( Engine::HalfFFTBuffer<> )( frozenAmps   ) )
        ( ( Engine::HalfFFTBuffer<> )( frozenPhases ) )
    );

    struct ChannelState : DynamicChannelState
    {
        bool frozenFlagConsumed;

        void reset() { DynamicChannelState::reset(); frozenFlagConsumed = false; };
    };


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    bool freeze_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // convolverImpl_hpp
