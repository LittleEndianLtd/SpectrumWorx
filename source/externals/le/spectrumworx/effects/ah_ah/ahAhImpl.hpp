////////////////////////////////////////////////////////////////////////////////
///
/// \file ahahImpl.hpp
/// ------------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef ahahImpl_hpp__112349D97_2DB9_4249_87EF_851D727FAF71
#define ahahImpl_hpp__112349D97_2DB9_4249_87EF_851D727FAF71
#pragma once
//------------------------------------------------------------------------------
#include "ahAh.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
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

class AhAhImpl : public EffectImpl<AhAh>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
    float                  gain_               ;
    float                  omega_              ;
    IndexRange::value_type beginBin_           ;
    IndexRange::value_type endBin_             ;
    float                  offsetFromUserRange_;
}; // class AhAhImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // ahAhImpl_hpp
