////////////////////////////////////////////////////////////////////////////////
///
/// \file pitchDetector.hpp
/// -----------------------
///
/// Copyright (c) 2010 - 2016. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef pitchDetector_hpp__1C810919_7127_459E_981C_46F7AC84CF16
#define pitchDetector_hpp__1C810919_7127_459E_981C_46F7AC84CF16
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/channelStateStatic.hpp"
#include "le/spectrumworx/engine/buffers.hpp"

#include <boost/range/iterator_range_core.hpp>

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE { namespace SW { LE_IMPL_NAMESPACE_BEGIN( Engine ) class Setup; LE_IMPL_NAMESPACE_END( Engine ) } }
LE_IMPL_NAMESPACE_BEGIN( LE )
//------------------------------------------------------------------------------
//namespace Analysis
//{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class HPS
///
/// \brief Harmonic Product Spectrum. Holds product and position.
///
////////////////////////////////////////////////////////////////////////////////

struct HPS
{
    float         harmonicProduct;
    std::uint16_t bin;
}; // struct HPS


////////////////////////////////////////////////////////////////////////////////
///
/// \class PitchDetector
///
/// \brief Finds pitch.
/// 
/// First PeakDetector detects peaks, then HPS spectrum is found. Then from 
/// these two information pitch is estimated. Example: HPS says the pitch
/// is concentrated in bin x; if bin x belongs to the peak y, then parabola
/// fit frequency is taken from the peak y and that is the pitch.
/// 
////////////////////////////////////////////////////////////////////////////////

struct Peak;
class PeakDetector;

class PitchDetector
{
public:
    struct ChannelState : LE::SW::Effects::StaticChannelState
    {
        float        lastPitch ;
    #ifdef LE_SW_PURE_ANALYSIS
        float        amplitude ;
        std::uint8_t confidence;
    #endif // LE_SW_PURE_ANALYSIS

        void reset();
    }; // struct ChannelState

public:
    static float LE_FASTCALL findPitch( SW::Engine::ReadOnlyDataRange const & amplitudes, ChannelState &, float lfb, float hfb, SW::Engine::Setup const & );

private:
    using HPSRange = boost::iterator_range<HPS * LE_RESTRICT>;

    static void          LE_FASTCALL findHarmonicProductSpectrumAndSort( SW::Engine::ReadOnlyDataRange amplitudes, HPSRange );
    static float         LE_FASTCALL estimatePitch( float lastPitch, float lfb, float hfb, HPSRange, PeakDetector const & );
    static Peak  const * LE_FASTCALL binPeak      ( std::uint16_t bin, PeakDetector const & );
}; // class PitchDetector

//------------------------------------------------------------------------------
//} // namespace Analysis
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( LE )
//------------------------------------------------------------------------------
#endif // pitchDetector_hpp
