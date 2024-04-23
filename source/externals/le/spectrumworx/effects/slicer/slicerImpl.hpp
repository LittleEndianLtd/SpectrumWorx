////////////////////////////////////////////////////////////////////////////////
///
/// \file slicerImpl.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef slicerImpl_hpp__5484C80F_BF0B_46A9_BCB8_8F68F22B46DA
#define slicerImpl_hpp__5484C80F_BF0B_46A9_BCB8_8F68F22B46DA
#pragma once
//------------------------------------------------------------------------------
#include "slicer.hpp"

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

class SlicerImpl : public EffectImpl<Slicer>
{
public: // LE::Effect interface.

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_DYNAMIC_CHANNEL_STATE
    (
        ( ( Engine::HalfFFTBuffer<float> )( mags ) )
        ( ( Engine::HalfFFTBuffer<float> )( phas ) )
    );

    struct ChannelState : DynamicChannelState
    {
        ModuloCounter counter;
        bool          silence;

        void reset();
    };
    

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    unsigned int timeOn_ ;
    unsigned int timeOff_;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // slicerImpl_hpp
