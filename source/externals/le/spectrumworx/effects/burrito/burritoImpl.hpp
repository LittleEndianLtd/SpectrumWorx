////////////////////////////////////////////////////////////////////////////////
///
/// \file burritoImpl.hpp
/// ---------------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef burritoImpl_hpp__C27A00BA_023C_4D1F_9742_F431F45C6018
#define burritoImpl_hpp__C27A00BA_023C_4D1F_9742_F431F45C6018
#pragma once
//------------------------------------------------------------------------------
#include "burrito.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
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

class BurritoImpl : public EffectImpl<Burrito>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_DYNAMIC_CHANNEL_STATE
    (
        ( ( Engine::HalfFFTBuffer<bool> )( positions ) )
    );

    using ChannelState = CompoundChannelState
    <
        ModuloCounterChannelState,
        DynamicChannelState
    >;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    IndexRange::value_type range_   ;
    IndexRange::value_type period_  ;
    float                  sideGain_;
}; // class BurritoImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // burritoImpl_hpp
