////////////////////////////////////////////////////////////////////////////////
///
/// \file blenderImpl.hpp
/// ---------------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef blenderImpl_hpp__F2596834_8722_40A2_B5AB_B2E91D94182A
#define blenderImpl_hpp__F2596834_8722_40A2_B5AB_B2E91D94182A
#pragma once
//------------------------------------------------------------------------------
#include "blender.hpp"

#include "le/spectrumworx/effects/effects.hpp"
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

class BlenderImpl : public EffectImpl<Blender>
{
public: // LE::Effect interface.

  //////////////////////////////////////////////////////////////////////////////
  // setup() and process()
  //////////////////////////////////////////////////////////////////////////////

  void setup  ( IndexRange const &, Engine::Setup const & );
  void process( Engine::MainSideChannelData_ReIm, Engine::Setup const & ) const;

private:
    float amount_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // blenderImpl_hpp