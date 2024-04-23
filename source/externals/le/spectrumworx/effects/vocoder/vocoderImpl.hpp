////////////////////////////////////////////////////////////////////////////////
///
/// \file talkingWindImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef vocoderImpl_hpp__A222DFB9_9541_4031_833C_7D332A621211
#define vocoderImpl_hpp__A222DFB9_9541_4031_833C_7D332A621211
#pragma once
//------------------------------------------------------------------------------
#include "vocoder.hpp"

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

class VocoderImpl : public EffectImpl<Vocoder>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &              , Engine::Setup const & )      ;
    void process( Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    void lowPassSpectrum_cepstrum     ( DataRange const & spectrum, DataRange const & workBuffer, Engine::Setup const & ) const;
    void lowPassSpectrum_movingAverage( DataRange const & spectrum, DataRange const & workBuffer, Engine::Setup const & ) const;

    FilterMethod::value_type filterMethod() const { return parameters().get<FilterMethod>(); }

private:
    std::uint16_t cutoff_;
    std::uint16_t filterLength_; // moving average specific
}; // class VocoderImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // vocoderImpl_hpp
