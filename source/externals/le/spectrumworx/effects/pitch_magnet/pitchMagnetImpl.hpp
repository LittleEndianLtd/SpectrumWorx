////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchMagnetImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Remove duplication with the PitchFollower effect.
///                                           (12.01.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchMagnetImpl_hpp__745857B1_F6A4_4855_9FAE_07E1A776BC57
#define pitchMagnetImpl_hpp__745857B1_F6A4_4855_9FAE_07E1A776BC57
#pragma once
//------------------------------------------------------------------------------
#include "pitchMagnet.hpp"

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
    /// \internal
    /// \class PitchMagnetBase
    ////////////////////////////////////////////////////////////////////////////

    class PitchMagnetBaseImpl : public EffectImpl<PitchMagnetBase>
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
        float findTargetPitch( ChannelState &, Engine::ChannelData_AmPh const &, Engine::Setup const & ) const;

    private:
        float pitchChangeLimitSemitones_;
        float targetFrequency_;
    };
} // namespace Detail


class PitchMagnetImpl 
    :
    public PitchMagnet,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::PitchMagnetBaseImpl,
        PhaseVocoderShared::PitchShifter
    >
{
public: // LE::Effect interface.
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

class PitchMagnetPVDImpl
    :
    public PitchMagnetPVD,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::PitchMagnetBaseImpl,
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
#endif // pitchMagnetImpl_hpp
