////////////////////////////////////////////////////////////////////////////////
///
/// \file centroidExtractorImpl.hpp
/// -------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef centroidExtractorImpl_hpp__91D2BE08_ACCE_4C50_B172_CCB947A246DC
#define centroidExtractorImpl_hpp__91D2BE08_ACCE_4C50_B172_CCB947A246DC
#pragma once
//------------------------------------------------------------------------------
#include "centroidExtractor.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/analysis/pitch_detector/pitchDetector.hpp"
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

class CentroidExtractorImpl : public EffectImpl<CentroidExtractor>
{
public: // LE::Effect required interface.
    typedef PitchDetector::ChannelState ChannelState;

    void setup  ( IndexRange const &                      , Engine::Setup const & )      ;
    void process( ChannelState &, Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private: 
    IndexRange::value_type centroid( ReadOnlyDataRange         amplitudes                                        ) const;
    IndexRange::value_type dominant( ReadOnlyDataRange const & amplitudes, Engine::Setup const &, ChannelState & ) const;
    IndexRange::value_type maxPeak ( ReadOnlyDataRange const & amplitudes, Engine::Setup const &                 ) const;

private:
    IndexRange::value_type bandwidth_    ;
    float                  amplification_;
}; // class CentroidExtractorImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // centroidExtractorImpl_hpp
