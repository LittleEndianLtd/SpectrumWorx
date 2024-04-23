////////////////////////////////////////////////////////////////////////////////
///
/// \file slowMotion.hpp
/// --------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef slowMotion_hpp__2763B150_F757_4976_939E_D3385E0962CE
#define slowMotion_hpp__2763B150_F757_4976_939E_D3385E0962CE
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class SlowMotion
///
/// \ingroup Algorithms Alpha
///
/// \brief Limits maximum per-bin magnitude change speed.
///
////////////////////////////////////////////////////////////////////////////////

class SlowMotion
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////


    DEFINE_PARAMETERS
    (
        ( ( LimitRise ) ( float )( MinimumValue<   0> )( MaximumValue<2000> )( DefaultValue< 50> )( DisplayValueSuffix<'dB/s'> ) )
        ( ( Threshold ) ( float )( MinimumValue<-120> )( MaximumValue<   0> )( DefaultValue<-60> )( DisplayValueSuffix< ' dB'> ) )  
        ( ( Step      ) ( float )( MinimumValue<   1> )( MaximumValue<10000> )( DefaultValue<100> ) )  
    );


    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    struct ChannelState
    {
        Common::SSEAlignedHalfFFTBuffer magsPrev  ;
        Common::SSEAlignedHalfFFTBuffer magsTarget;
        Common::SSEAlignedHalfFFTBuffer magsTargetNew;
        bool reached[ LE_CONFIGURATION_MAX_FFT_SIZE / 2 ];
        
        bool                            isInitialized;

        void clear();
    };


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( EngineSetup const &, Parameters const & );
    void process( ChannelState &, ChannelData_AmPh & ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];

private:
    unsigned int        num_bins_ ;
    float               limitRise_;
    float               threshold_;
    float               step_;
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // slowMotion_hpp
