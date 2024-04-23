////////////////////////////////////////////////////////////////////////////////
///
/// \file bandpassImpl.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef bandpassImpl_hpp__3E2335FA_F97E_47A9_9BED_977505A90353
#define bandpassImpl_hpp__3E2335FA_F97E_47A9_9BED_977505A90353
#pragma once
//------------------------------------------------------------------------------
#include "bandpass.hpp"

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

#if defined( _DEBUG ) && !( defined( LE_SW_SDK_BUILD ) && !defined( __MSVC_RUNTIME_CHECKS ) )
    /// \note Testing phase...
    ///                                       (17.02.2012.) (Domagoj Saric)
    #define LE_BAND_FILTER_USE_ENGINE_WINDOW
#endif // _DEBUG

namespace Detail ///< \internal
{
    class BandGainImpl : public EffectImpl<BandGain>
    {
    public: // LE::Effect interface.

        ////////////////////////////////////////////////////////////////////////
        // setup()
        ////////////////////////////////////////////////////////////////////////

        void setup( IndexRange const &, Engine::Setup const & );

    protected:
        float attenuation_;
        #ifdef LE_BAND_FILTER_USE_ENGINE_WINDOW
            DataRange  downSlope_;
            float    * pUpSlope_ ;
            Engine::StaticHalfFFTBuffer window_;
        #endif // LE_BAND_FILTER_USE_ENGINE_WINDOW
    };
} // namespace Detail


class BandpassImpl
    :
    public Detail::BandGainImpl,
    public Bandpass
{
public: // LE::Effect interface.
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};


class BandstopImpl
    :
    public Detail::BandGainImpl,
    public Bandstop
{
public: // LE::Effect interface.
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // bandpassImpl_hpp
