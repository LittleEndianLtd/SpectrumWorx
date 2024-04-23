////////////////////////////////////////////////////////////////////////////////
///
/// \file gainImpl.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef gainImpl_hpp__FD752E55_5DB0_41FF_80CF_BFB7CE1C42E9
#define gainImpl_hpp__FD752E55_5DB0_41FF_80CF_BFB7CE1C42E9
#pragma once
//------------------------------------------------------------------------------
#include "gain.hpp"

#include "le/spectrumworx/effects/effects.hpp"
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

class GainImpl : public NoParametersEffectImpl<Gain>
{
public: // LE::Effect interface.
    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    static void setup  ( IndexRange      const &, Engine::Setup const & ) {}
    static void process( Engine::ChannelData_AmPh const &, Engine::Setup const & ) {}
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // gainImpl_hpp