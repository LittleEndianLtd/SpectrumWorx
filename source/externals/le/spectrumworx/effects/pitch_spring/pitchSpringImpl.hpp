////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchSpringImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchSpringImpl_hpp__B5B2F53F_B59F_4E79_85A5_C741E0182BF5
#define pitchSpringImpl_hpp__B5B2F53F_B59F_4E79_85A5_C741E0182BF5
#pragma once
//------------------------------------------------------------------------------
#include "pitchSpring.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
#include "le/spectrumworx/effects/vibrato.hpp"
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
    /// \class PitchSpringBase
    /// \internal
    ///
    ////////////////////////////////////////////////////////////////////////////

    class PitchSpringBaseImpl
        :
        public EffectImpl<PitchSpringBase>,
        public VibratoEffect
    {
    protected:
        void setup( IndexRange const &, Engine::Setup const & );
        float calculateNewPitch( ChannelState & ) const;
    };
} // namespace Detail

class PitchSpringImpl
    :
    public PitchSpring,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::PitchSpringBaseImpl,
        PhaseVocoderShared::PitchShifter
    >
{
public: // LE::Effect interface.
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

class PitchSpringPVDImpl
    :
    public PitchSpringPVD,
    public PhaseVocoderShared::PitchShifterBasedEffect
    <
        Detail::PitchSpringBaseImpl,
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
#endif // pitchSpringImpl_hpp
