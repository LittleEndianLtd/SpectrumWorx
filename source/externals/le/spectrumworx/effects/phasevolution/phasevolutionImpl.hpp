////////////////////////////////////////////////////////////////////////////////
///
/// \file phasevolutionImpl.hpp
/// ---------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef phasevolutionImpl_hpp__F60D23C3_A21E_4D23_B742_3E9E59D1D3F4
#define phasevolutionImpl_hpp__F60D23C3_A21E_4D23_B742_3E9E59D1D3F4
#pragma once
//------------------------------------------------------------------------------
#include "phasevolution.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/channelStateStatic.hpp"
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

class PhasevolutionImpl : public EffectImpl<Phasevolution>
{
public: // LE::Effect required interface.

    struct ChannelState : StaticChannelState
    {
        float phaseShift_;
        float time_      ;

        void reset() 
        {            
            phaseShift_ = 0; 
            time_       = 0;
        }
    };

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
    float stepTime_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // phasevolutionImpl_hpp
