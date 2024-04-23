////////////////////////////////////////////////////////////////////////////////
///
/// \file freezeImpl.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef freezeImpl_hpp__1C855F51_D0CA_48B9_8F40_4B259941C1F9
#define freezeImpl_hpp__1C855F51_D0CA_48B9_8F40_4B259941C1F9
#pragma once
//------------------------------------------------------------------------------
#include "freeze.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
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

class FreezeImpl : public EffectImpl<Freeze>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_DYNAMIC_CHANNEL_STATE
    (
        ( ( PhaseVocoderShared::PitchShifter::ChannelState )( pvState ) )
        ( ( Engine::HalfFFTBuffer<float> )( frozenMagNew  ) )
        ( ( Engine::HalfFFTBuffer<float> )( frozenFreqNew ) )
        ( ( Engine::HalfFFTBuffer<float> )( frozenMagOld  ) )
        ( ( Engine::HalfFFTBuffer<float> )( frozenFreqOld ) )
    );

    struct ChannelState : DynamicChannelState
    {
        float frameCounter;

        bool freezeDone;
        bool meltDone  ;

        bool frozen;
        bool normal;

        //...mrmlj...quick-workaround for a non-deterministic relationship
        //...mrmlj...between setup() and process() calls...
        bool previousFreezeFlag;
        bool previousMeltFlag  ;

        void reset();
    }; // struct ChannelState
    
    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
    float inverseTransitionTime_;
    bool  freeze_;
    bool  melt_  ;

    PhaseVocoderShared::BaseParameters pvParameters_;
}; // class FreezeImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // freezeImpl_hpp
