////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchShifterImpl.hpp
/// --------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchShifterImpl_hpp__70354A08_96D9_4A83_8038_E18D73C2F0FB
#define pitchShifterImpl_hpp__70354A08_96D9_4A83_8038_E18D73C2F0FB 
#pragma once
//------------------------------------------------------------------------------
#include "pitchShifter.hpp"

#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
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

class PitchShifterImpl
    :
    public EffectImpl<PitchShifter>,
    public PhaseVocoderShared::PitchShifter
{
public: // LE::Effect required interface.
    void setup( IndexRange const &, Engine::Setup const & );
};


class PVPitchShifterImpl
    :
    public EffectImpl<PVPitchShifter>,
    public PhaseVocoderShared::PVPitchShifter
{
public: // LE::Effect required interface.
    void setup( IndexRange const &, Engine::Setup const & );
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // pitchShifterImpl_hpp
