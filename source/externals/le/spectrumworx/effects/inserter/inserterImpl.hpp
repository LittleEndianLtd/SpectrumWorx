////////////////////////////////////////////////////////////////////////////////
///
/// \file inserterImpl.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef inserterImpl_hpp__FBE18C67_5D60_4991_A6F1_5E5AAB7BA9B9
#define inserterImpl_hpp__FBE18C67_5D60_4991_A6F1_5E5AAB7BA9B9
#pragma once
//------------------------------------------------------------------------------
#include "inserter.hpp"

#include "le/spectrumworx/effects/commonParameters.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/utility/buffers.hpp"
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

class InserterImpl : public EffectImpl<Inserter>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    IndexRange::value_type source_    ;
    IndexRange::value_type target_    ;
    IndexRange::value_type insertSize_;
    UnpackedMagPhaseMode   mode_      ;
}; // class InserterImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // inserterImpl_hpp

