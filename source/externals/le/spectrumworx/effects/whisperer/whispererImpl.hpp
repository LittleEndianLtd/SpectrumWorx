////////////////////////////////////////////////////////////////////////////////
///
/// \file whispererImpl.hpp
/// -----------------------
/// 
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef whispererImpl_hpp__7E88348A_B8DC_4D2C_B805_7E845F3B5ACF
#define whispererImpl_hpp__7E88348A_B8DC_4D2C_B805_7E845F3B5ACF
#pragma once
//------------------------------------------------------------------------------
#include "whisperer.hpp"

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

class WhispererImpl : public NoParametersEffectImpl<Whisperer>
{
public: // LE::Effect interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    static void setup  ( IndexRange const &, Engine::Setup const & ) {}
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // whispererImpl_hpp
