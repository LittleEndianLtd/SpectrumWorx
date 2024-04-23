////////////////////////////////////////////////////////////////////////////////
///
/// \file armonizerImpl.hpp
/// -----------------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef armonizerImpl_hpp__D2DC61B5_FEEC_4FD5_B4DA_E89976A55507
#define armonizerImpl_hpp__D2DC61B5_FEEC_4FD5_B4DA_E89976A55507
#pragma once
//------------------------------------------------------------------------------
#include "armonizer.hpp"

#include "le/spectrumworx/effects/effects.hpp"

#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
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

class ArmonizerImpl
    : 
    public EffectImpl<Armonizer>,
    public PhaseVocoderShared::PitchShifter
{
public: // LE::Effect interface.
    void setup( IndexRange const &, Engine::Setup const & );
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // armonizerImpl_hpp
