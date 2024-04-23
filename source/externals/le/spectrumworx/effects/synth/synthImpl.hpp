////////////////////////////////////////////////////////////////////////////////
///
/// \file synthImpl.hpp
/// -------------------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef synthImpl_hpp__AC07601A_1A48_4E30_B810_3E13409B3910
#define synthImpl_hpp__AC07601A_1A48_4E30_B810_3E13409B3910
#pragma once
//------------------------------------------------------------------------------
#include "synth.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/phase_vocoder/shared.hpp"
#include "le/spectrumworx/engine/buffers.hpp"
#include "le/spectrumworx/engine/configuration.hpp"

#include <array>
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

class SynthImpl : public EffectImpl<Synth>
{
public:
    static std::uint8_t const numberOfOvertones       = 1;
    static std::uint8_t const numberOfTones           = 1 + numberOfOvertones;
  //static std::uint8_t const numberOfOscillators     = numberOfTones;

public: // LE::Effect required interface.
    SynthImpl() : lastFreq_( 0 ), lastWindow_( static_cast<Engine::Constants::Window>( -1 ) ), lastFFTSize_( 0 ) {}

    struct ChannelState
    {
        using OscillatorPhases = Engine::HalfFFTBuffer<>;
        using Phases           = std::array<OscillatorPhases, numberOfTones>;

        Phases phases;

               void          LE_FASTCALL reset          ();
               void          LE_FASTCALL resize         ( Engine::StorageFactors const &, Engine::Storage & );
        static std::uint32_t LE_FASTCALL requiredStorage( Engine::StorageFactors const & );
    }; // struct ChannelState

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void LE_FASTCALL     setup  ( IndexRange const &, Engine::Setup const & );
    void LE_FASTCALL_ABI process( ChannelState &, Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const; //...mrmlj..._ABI due to msvc12 bogus compilation errors...

private:
    PhaseVocoderShared::BaseParameters synthesisParameters_;

    float flangeDelayPhase_;
    float flangeGain_      ;

    float        harmonicSlope_;
    std::uint8_t harmonics_    ;
    std::uint8_t startHarmonic_;

    std::array<float, 3>       coefficients_;

    float                      lastFreq_    ;
    Engine::Constants::Window  lastWindow_  ;
    std::uint16_t              lastFFTSize_ ;
    bool                       resetState_  ;
}; // class SynthImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // synthImpl_hpp
