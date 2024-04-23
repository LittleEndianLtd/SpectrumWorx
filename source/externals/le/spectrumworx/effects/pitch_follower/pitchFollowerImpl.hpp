////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchFollowerImpl.hpp
/// ---------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchFollowerImpl_hpp__2BDC51A5_6E70_4841_B2B6_E2C764777A79
#define pitchFollowerImpl_hpp__2BDC51A5_6E70_4841_B2B6_E2C764777A79
#pragma once
//------------------------------------------------------------------------------
#include "pitchFollower.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/channelStateStatic.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
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

namespace Detail
{
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class PitchMagnetBase
    /// \internal
    ///
    ////////////////////////////////////////////////////////////////////////////

    class PitchFollowerBaseImpl : public EffectImpl<PitchFollowerBase>
    {
    public: // LE::Effect interface.
        ////////////////////////////////////////////////////////////////////////
        // ChannelState
        ////////////////////////////////////////////////////////////////////////

        struct ChannelState : PitchDetector::ChannelState
        {
            float prevPitchScaleSemitones;
            void reset();
        };

    protected:
        void  setup          ( IndexRange const &, Engine::Setup const & );
        float findTargetPitch( ChannelState &, Engine::MainSideChannelData_AmPh const &, Engine::Setup const & ) const;

    private:
        float pitchChangeLimitSemitones_;
    };
} // namespace Detail


class PitchFollowerImpl
    :
    public PitchFollower,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::PitchFollowerBaseImpl,
        PhaseVocoderShared::PitchShifter
    >
{
public: // LE::Effect interface.
    void process( ChannelState &, Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;
};


class PitchFollowerPVDImpl
    :
    public PitchFollowerPVD,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::PitchFollowerBaseImpl,
        PhaseVocoderShared::PVPitchShifter
    >
{
public: // LE::Effect interface.
    void process( ChannelState &, Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // pitchFollowerImpl_hpp
