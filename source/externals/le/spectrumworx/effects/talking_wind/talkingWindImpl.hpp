////////////////////////////////////////////////////////////////////////////////
///
/// \file talkingWindImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef talkingWindImpl_hpp__4426F456_173B_4D8A_9A45_20F9E315AFC7
#define talkingWindImpl_hpp__4426F456_173B_4D8A_9A45_20F9E315AFC7
#pragma once
//------------------------------------------------------------------------------
#include "talkingWind.hpp"

#include "le/spectrumworx/effects/effects.hpp"

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

class TalkingWindImpl : public EffectImpl<TalkingWind>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &              , Engine::Setup const & );
    void process( Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    void lowPassSpectrum_cepstrum( DataRange const & spectrum, DataRange const & workBuffer, Engine::Setup const & ) const;

private:
    float         envgain_;
    std::uint16_t cutoff_;
}; // class TalkingWindImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // talkingWindImpl_hpp
