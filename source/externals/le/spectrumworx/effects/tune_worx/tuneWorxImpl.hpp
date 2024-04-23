////////////////////////////////////////////////////////////////////////////////
///
/// \file tuneWorxImpl.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef tuneWorxImpl_hpp__08D752F7_70CC_436C_8C8A_BE59A2A4900D
#define tuneWorxImpl_hpp__08D752F7_70CC_436C_8C8A_BE59A2A4900D
#pragma once
//------------------------------------------------------------------------------
#include "tuneWorx.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
#ifdef LE_SW_SDK_BUILD
#include "le/spectrumworx/effects/vibrato.hpp"
#endif // LE_SW_SDK_BUILD
#include "le/analysis/musical_scales/musicalScales.hpp"
#include "le/analysis/pitch_detector/pitchDetector.hpp"
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

//#define LE_SW_TW_RETUNE_TEST

namespace Detail
{
    class TuneWorxBaseImpl
        :
        public EffectImpl<TuneWorxBase>
    #ifdef LE_SW_SDK_BUILD
        ,public VibratoEffect
    #endif // LE_SW_SDK_BUILD
    {
    public:
    #if defined( LE_SW_SDK_BUILD )
        struct ChannelState
            :
            PitchDetector::ChannelState,
            VibratoEffect::ChannelState
        {
            unsigned int tuneStep      ;
			float        lastTargetTone;

			void reset()
			{
				PitchDetector::ChannelState::reset();
				VibratoEffect::ChannelState::reset();
				tuneStep       = 0;
				lastTargetTone = 0;
			}
        };
    #elif defined( LE_SW_TW_RETUNE_TEST )
        struct ChannelState : PitchDetector::ChannelState
        {
            unsigned int tuneStep      ;
            float        lastTargetTone;

			void reset()
			{
				PitchDetector::ChannelState::reset();
				tuneStep       = 0;
				lastTargetTone = 0;
			}
        };
    #else
        typedef PitchDetector::ChannelState ChannelState;
    #endif // LE_SW_SDK_BUILD

    protected:
        float findNewPitchScale( Engine::ChannelData_AmPh const &, Engine::Setup const &, ChannelState & ) const;
		float findVibratoPitch ( ChannelState & ) const;

        void setup( IndexRange const &, Engine::Setup const & );

    private:
    #if defined( LE_SW_SDK_BUILD )
		unsigned int retuneTime_  ;
		unsigned int vibratoDelay_;
		float		 pitchShift_  ;
    #elif defined( LE_SW_TW_RETUNE_TEST )
        unsigned int retuneTime_  ;
    #endif // LE_SW_SDK_BUILD

        Music::Scale userScale_;
    }; // class TuneWorxBaseImpl
} // namespace Detail


class TuneWorxImpl
    :
    public TuneWorx,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::TuneWorxBaseImpl,
        PhaseVocoderShared::PitchShifter
    >
{
public: // LE::Effect interface.
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

class TuneWorxPVDImpl
    :
    public TuneWorxPVD,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::TuneWorxBaseImpl,
        PhaseVocoderShared::PVPitchShifter
    >
{
public: // LE::Effect interface.
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // tuneWorxImpl_hpp
