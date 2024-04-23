////////////////////////////////////////////////////////////////////////////////
///
/// \file morphologicalMutations.hpp
/// --------------------------------
///
/// Copyright (c) 2010. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef morphologicalMutations_hpp__21670A92_C064_425C_B548_1BE9DFA38E88
#define morphologicalMutations_hpp__21670A92_C064_425C_B548_1BE9DFA38E88
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
/// \class MorphologicalMutations
///
/// \ingroup Algorithms RC
///
/// \brief Morphological mutations.
///
////////////////////////////////////////////////////////////////////////////////

class MorphologicalMutations
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DISCRETE_VALUES_PARAMETER( Mode, (USIM)(UUIM)(ISIM)(IUIM)(LCM)( VM ) )

    DEFINE_PARAMETERS
    (
        ( ( Mode           )                                                                                                              )
        ( ( IrregularRange )( unsigned int )( MinimumValue<  0> )( MaximumValue<100> )( DefaultValue< 50> )( DisplayValueSuffix<' bw%'> ) )
        ( ( Attenuate      )( float        )( MinimumValue<-24> )( MaximumValue<+24> )( DefaultValue<  0> )( DisplayValueSuffix<' dB' > ) )  
        ( ( UniformGain    )( float        )( MinimumValue<-48> )( MaximumValue<+48> )( DefaultValue<  0> )( DisplayValueSuffix<' dB' > ) )
    );

    struct ChannelState
    {
        Common::SSEAlignedHalfFFTBuffer Sj_;
        Common::SSEAlignedHalfFFTBuffer Tj_;
        Common::SSEAlignedHalfFFTBuffer Mj_; 

        void clear()
        {
            Sj_.clear();
            Tj_.clear();
            Mj_.clear();
        }   
    };


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( EngineSetup const &, Parameters const & );
    void process( ChannelState &, ChannelData_AmPh & ) const;

private: 
    void normalize( float * data, unsigned int size ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];

private:
    unsigned int num_bins_      ;
    unsigned int irregularRange_;

    float        uniformGain_;
    float        attenuate_  ;
    Mode::Value  mode_       ;
    
    mutable Common::SSEAlignedHalfFFTBuffer irrTrue_; 
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // morphologicalMutations_hpp
