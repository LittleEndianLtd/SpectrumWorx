////////////////////////////////////////////////////////////////////////////////
///
/// \file tai.hpp
/// -------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef tai_hpp__375BC492_EC5A_4FF8_8499_88B8192BDDDE
#define tai_hpp__375BC492_EC5A_4FF8_8499_88B8192BDDDE
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
#include "common/buffers.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Tai
///
/// \ingroup Algorithms RC
///
/// \brief Product of frequency domain compressed signals.
///
////////////////////////////////////////////////////////////////////////////////

class Tai
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////
    
    DEFINE_PARAMETERS
    (       
        ( ( MainThreshold )( float )( MinimumValue<-60> )( MaximumValue< 0> )( DefaultValue< -5> )( DisplayValueSuffix<' dB'> ) )        
        ( ( MainRatio     )( float )( MinimumValue<  1> )( MaximumValue<15> )( DefaultValue< 5> ) )
        ( ( SideThreshold )( float )( MinimumValue<-60> )( MaximumValue< 0> )( DefaultValue< -5> )( DisplayValueSuffix<' dB'> ) )
        ( ( SideRatio     )( float )( MinimumValue<  1> )( MaximumValue<15> )( DefaultValue< 5> ) )
    );

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////
    
    
    void setup  ( EngineSetup const &, Parameters const & );
    void process( ChannelData_AmPh & ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];

private:
    unsigned int num_bins_;

    float thresholdMain_;
    float thresholdSide_;
    float ratioMain_;
    float ratioSide_;
    float maxAmplitude_;

};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // tai
