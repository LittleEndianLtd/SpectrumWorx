////////////////////////////////////////////////////////////////////////////////
///
/// \file tonalImpl.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef tonalImpl_hpp__71A9A670_AA87_4755_A67A_A61833B57203
#define tonalImpl_hpp__71A9A670_AA87_4755_A67A_A61833B57203
#pragma once
//------------------------------------------------------------------------------
#include "tonal.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/analysis/peak_detector/peakDetector.hpp"
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

namespace Detail ///< \internal
{
    class TonalBaseImpl : public TonalBase
    {
    public: // LE::Effect required interface.

        ////////////////////////////////////////////////////////////////////////
        // setup() and process()
        ////////////////////////////////////////////////////////////////////////

    protected:
        template <class Implementation, class Parameters>
        void setup( Parameters const & parameters, Engine::Setup const & engineSetup )
        {
            setup( engineSetup );

            pd_.setStrengthThreshold( parameters.template get<typename Implementation::Strength       >() );
            pd_.setGlobalThreshold  ( parameters.template get<typename Implementation::GlobalThreshold>() );
            pd_.setLocalThreshold   ( parameters.template get<typename Implementation::LocalThreshold >() );
        }

    private:
        void setup( Engine::Setup const & );

    protected:
        mutable PeakDetector pd_;
    };
} // namespace Detail


class TonalImpl 
    : 
    public EffectImpl<Tonal>,
    public Detail::TonalBaseImpl
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};


class AtonalImpl 
    : 
    public EffectImpl<Atonal>,
    public Detail::TonalBaseImpl
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( IndexRange const &, Engine::Setup const & );
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // tonalImpl_hpp
