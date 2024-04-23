////////////////////////////////////////////////////////////////////////////////
///
/// \file wobblerImpl.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef wobblerImpl_hpp__A1154887_29E9_43FB_BFA9_4D29C344D513
#define wobblerImpl_hpp__A1154887_29E9_43FB_BFA9_4D29C344D513
#pragma once
//------------------------------------------------------------------------------
#include "wobbler.hpp"

#include "le/spectrumworx/effects/effects.hpp"
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

class WobblerImpl : public EffectImpl<Wobbler>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////

    typedef ModuloCounterChannelState ChannelState;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////
  
    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
    unsigned int period_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // wobblerImpl_hpp