////////////////////////////////////////////////////////////////////////////////
///
/// \file reverserImpl.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef reverserImpl_hpp__C3A377D7_0B2D_4B86_B9F9_5609C8675D7E
#define reverserImpl_hpp__C3A377D7_0B2D_4B86_B9F9_5609C8675D7E
#pragma once
//------------------------------------------------------------------------------
#include "reverser.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/historyBuffer.hpp"

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

class ReverserImpl : public EffectImpl<Reverser>
{
public: // LE::Effect interface.
    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    typedef ReversedHistoryChannelState<Length::unscaledMaximum> ChannelState;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;    

private:
    std::uint16_t lengthInSteps_;
}; // class ReverserImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // reverserImpl_hpp
