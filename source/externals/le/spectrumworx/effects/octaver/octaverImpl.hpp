////////////////////////////////////////////////////////////////////////////////
///
/// \file octaverImpl.hpp
/// ---------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef octaverImpl_hpp__A023A2C0_4D7C_47A4_BDA7_2E0E92A67D32
#define octaverImpl_hpp__A023A2C0_4D7C_47A4_BDA7_2E0E92A67D32
#pragma once
//------------------------------------------------------------------------------
#include "octaver.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"

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

class OctaverImpl : public EffectImpl<Octaver>
{
public: // LE::Effect interface.
    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_NAMED_DYNAMIC_CHANNEL_STATE
    (
        ChannelState,
        ( ( PhaseVocoderShared::PitchShifter::ChannelState )( pv1 ) )
        ( ( PhaseVocoderShared::PitchShifter::ChannelState )( pv2 ) )
    );

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh2ReIm, Engine::Setup const & ) const;

private:
    void LE_FASTCALL shiftAndMix
    (
        Engine::ChannelData_AmPh2ReIm                        & data,
        Engine::ChannelData_AmPh                             & shiftedData,
        Engine::Setup                                  const & engineSetup,
        PhaseVocoderShared::PitchShifter::ChannelState       & pvState,
        std::uint8_t                                           octave
    ) const;

private:
    struct OctaveSetup
    {
        float gain      ;
        float pitchScale;
    };

private:
    OctaveSetup octaveParameters_[ 2 ];

    std::uint16_t cutoff_;

    PhaseVocoderShared::PitchShifter ps_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // octaverImpl_hpp
