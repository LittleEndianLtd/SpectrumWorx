////////////////////////////////////////////////////////////////////////////////
///
/// \file subOctaver.hpp
/// --------------------
///
/// Copyright (c) 2010. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef sub_octaver_hpp__9579DBDB_C9A8_486D_985B_DB6955708192
#define sub_octaver_hpp__9579DBDB_C9A8_486D_985B_DB6955708192
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
#include "common/buffers.hpp"
#include "../phase_vocoder/shared.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class SubOctaver
///
/// \ingroup Algorithms Alpha
///
/// \brief Adds two sub octaves.
///
////////////////////////////////////////////////////////////////////////////////

class SubOctaver
{
public: // LE::Algorithm interface.
    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////
    
    DEFINE_PARAMETERS
    (
        ( ( GainOrig        )( float        )( MinimumValue<-48> )( MaximumValue< +12> )( DefaultValue< -12> ) ( DisplayValueSuffix<' dB'> ) )
        ( ( GainOct1        )( float        )( MinimumValue<-48> )( MaximumValue< +12> )( DefaultValue< +6> ) ( DisplayValueSuffix<' dB'> ) )
        ( ( GainOct2        )( float        )( MinimumValue<-48> )( MaximumValue< +12> )( DefaultValue< +6> ) ( DisplayValueSuffix<' dB'> ) )
        ( ( CutoffFrequency )( unsigned int )( MinimumValue<  0> )( MaximumValue<6000> )( DefaultValue<350> ) ( DisplayValueSuffix<' Hz'> ) )
    );


    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    struct ChannelState
    {
        PhaseVocoderShared::PitchShifter::ChannelState cs1_;
        PhaseVocoderShared::PitchShifter::ChannelState cs2_;

        void clear();
    };


  void setup  ( EngineSetup const &, Parameters const & );
  void process( ChannelState &, ChannelData_AmPh2ReIm ) const;

public: // Algorithm traits.
  static bool const canUseTwoInputs = false;

public: 
  static char const title      [];
  static char const description[];

private:
    unsigned int num_bins_;

    float gain0_;
    float gain1_;
    float gain2_;

    unsigned int cutoff_;

    PhaseVocoderShared::PitchShifter ps1_;
    PhaseVocoderShared::PitchShifter ps2_;

    mutable Common::SSEAlignedHalfFFTBuffer shifted1Reals_;
    mutable Common::SSEAlignedHalfFFTBuffer shifted1Imags_;
    mutable Common::SSEAlignedHalfFFTBuffer shifted2Reals_;
    mutable Common::SSEAlignedHalfFFTBuffer shifted2Imags_;

};


//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // octaver_hpp
