////////////////////////////////////////////////////////////////////////////////
///
/// \file pvImploderSide.hpp
/// ---------------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef pvImploderSide_hpp__991897E6_7752_46CB_8629_F9EF41FD7958
#define pvImploderSide_hpp__991897E6_7752_46CB_8629_F9EF41FD7958
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
/// \class PVImploderSide
///
/// \ingroup Algorithms Alpha
///
/// \brief Spectral implosion with glissando.  
///
////////////////////////////////////////////////////////////////////////////////

class PVImploderSide
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DEFINE_PARAMETERS
    (
        ( ( DecayMain )( unsigned int )( MinimumValue<   1> )( MaximumValue<200> )( DefaultValue< 50> )( DisplayValueSuffix<' s'> ) )
        ( ( GlissMain )(          int )( MinimumValue<-300> )( MaximumValue<300> )( DefaultValue<  0> )( DisplayValueSuffix<' \"/s'> ) )
        ( ( DecaySide )( unsigned int )( MinimumValue<   1> )( MaximumValue<200> )( DefaultValue< 50> )( DisplayValueSuffix<' s'> ) )
        ( ( GlissSide )(          int )( MinimumValue<-300> )( MaximumValue<300> )( DefaultValue<  0> )( DisplayValueSuffix<' \"/s'> ) )
    );

    struct ChannelState
    {   
        Common::SSEAlignedHalfFFTBuffer accumMagnMain_ ;
        Common::SSEAlignedHalfFFTBuffer accumPhaseMain_;     
        Common::SSEAlignedHalfFFTBuffer accumMagnSide_ ;
        Common::SSEAlignedHalfFFTBuffer accumPhaseSide_;     
        
        void clear() 
        {
            accumMagnMain_ .clear();
            accumPhaseMain_.clear();
            accumMagnSide_ .clear();
            accumPhaseSide_.clear();
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
    float        decayM_   ;
    float        glissM_   ;
    float        decayS_   ;
    float        glissS_   ;
    float        nyquist_ ;
    float        maxAmplitude_;    
    unsigned int num_bins_;
           
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // pvImploderSide_hpp

