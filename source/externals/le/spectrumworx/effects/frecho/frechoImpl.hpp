////////////////////////////////////////////////////////////////////////////////
///
/// \file frechoImpl.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef frechoImpl_hpp__9D813892_843C_41F8_888D_1CFD79EA0E22
#define frechoImpl_hpp__9D813892_843C_41F8_888D_1CFD79EA0E22
#pragma once
//------------------------------------------------------------------------------
#include "frecho.hpp"

#include "le/spectrumworx/effects/channelStateDynamic.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/historyBuffer.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
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

class FrechoImpl : public EffectImpl<Frecho>
{
protected:
    static unsigned int const maxDistance              = Distance::unscaledMaximum;
    static unsigned int const speedOfSound             = 343;
    static unsigned int const echoLengthInMilliseconds = maxDistance * 2 * 1000 / speedOfSound;

    using History = HistoryBuffer<float, echoLengthInMilliseconds>;

public: // LE::Effect interface.

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    LE_DYNAMIC_CHANNEL_STATE
    (
        ( ( PhaseVocoderShared::PitchShifter::ChannelState )( pvState       ) )
        ( ( History                                        )( historyBuffer ) )
    );

    using ChannelState = CompoundChannelState
    <
        DynamicChannelState,
        ModuloCounterChannelState
    >;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( ChannelState &, Engine::ChannelData_ReIm, Engine::Setup const & ) const;

protected:
    void doProcess( ChannelState &, Engine::ChannelData_ReIm &, Engine::Setup const & ) const;

    float        gain           () const { return gain_           ; }
    std::uint8_t echoSizeInSteps() const { return echoSizeInSteps_; }

private:
    float        gain_           ;
    std::uint8_t echoSizeInSteps_;

    PhaseVocoderShared::PitchShifter ps_;
}; // class FrechoImpl


class FrevchoImpl
    :
    private FrechoImpl,
    public  Frevcho
{
private:
    using ReversedHistoryState = ReversedHistoryChannelState<echoLengthInMilliseconds>;

public: // LE::Effect interface.

    using Parameters = Frevcho::Parameters;

    using FrechoImpl::parameters;

    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    using ChannelState = CompoundChannelState
    <
        FrechoImpl::ChannelState,
        ReversedHistoryState
    >;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void process( ChannelState &, Engine::ChannelData_ReIm, Engine::Setup const & ) const;
    using FrechoImpl::setup;

    using Frevcho::description;
    using Frevcho::title;
#if defined( __GNUC__ ) && !defined( __clang__ )
    static bool const usesSideChannel = Frevcho::usesSideChannel;
#else
    using Frevcho::usesSideChannel;
#endif // __GNUC__
}; // class FrevchoImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // frecho_hpp
