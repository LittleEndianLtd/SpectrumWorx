////////////////////////////////////////////////////////////////////////////////
///
/// \file reverbator.hpp
/// --------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef reverbator_hpp__98055B9A_6CDA_4627_B71A_DA318D536B68
#define reverbator_hpp__98055B9A_6CDA_4627_B71A_DA318D536B68
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
/// \class Reverbator
///
/// \ingroup Algorithms RC
///
/// \brief Creates reverbant sound. 
///
////////////////////////////////////////////////////////////////////////////////

class Reverbator
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////
    
    DISCRETE_VALUES_PARAMETER( Mode, (Classic) (Takuramu) )

    DEFINE_PARAMETERS
    (
        ( ( Mode   )                                                                                                              )            
        ( ( Depth  )( float        )( MinimumValue< 10> )( MaximumValue<1000> )( DefaultValue<150> ) ( DisplayValueSuffix<' ms'>) )    
      //( ( Amount )( unsigned int )( MinimumValue<  0> )( MaximumValue< 100> )( DefaultValue< 50> ) ( DisplayValueSuffix<' %'> ) )
        ( ( Amount )( int          )( MinimumValue<-60> )( MaximumValue<   0> )( DefaultValue<-12> ) ( DisplayValueSuffix<' dB'> ) )  
        ( ( StopFrequency  ) )
    );

    struct ChannelState 
    {
        unsigned int frameCount;                      
        
        Common::SSEAlignedHalfFFTBuffer savedInMag;
        Common::SSEAlignedHalfFFTBuffer savedOutMag;
        
        Common::SSEAlignedHalfFFTBuffer savedInMagPrev;
        
        void clear()
        {             
            savedInMag.clear();
            savedOutMag.clear();
            savedInMagPrev.clear();
            
            frameCount = 0;
        }
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

private:
    unsigned int num_bins_;
    unsigned int depth_   ;
    float        amount_  ;
    unsigned int rightBin_;
    Mode::Value  mode_    ;
    
    mutable Common::SSEAlignedHalfFFTBuffer tempMag;

};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // reverbator_hpp
