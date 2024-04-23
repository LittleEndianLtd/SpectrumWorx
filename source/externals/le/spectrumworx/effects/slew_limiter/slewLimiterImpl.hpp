////////////////////////////////////////////////////////////////////////////////
///
/// \file slewLimiterImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef slewLimiterImpl_hpp__2763B150_F757_4976_939E_D3385E0962CE
#define slewLimiterImpl_hpp__2763B150_F757_4976_939E_D3385E0962CE
#pragma once
//------------------------------------------------------------------------------
#include "slewLimiter.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
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

class SlewLimiterImpl : public EffectImpl<SlewLimiter>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_DYNAMIC_CHANNEL_STATE
    (
        ( ( Engine::HalfFFTBuffer<> )( magsPrev ) )
    );

    struct ChannelState : DynamicChannelState
    {
        bool isInitialised;

        void reset();
    };


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
    float gainLowerBound_;
    float gainUpperBound_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // slewLimiterImpl_hpp
