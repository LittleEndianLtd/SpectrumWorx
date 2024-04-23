////////////////////////////////////////////////////////////////////////////////
///
/// \file randomizer.hpp
/// --------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef randomizer_hpp__B839419D_D8BD_4287_BA5D_1263030A22EA
#define randomizer_hpp__B839419D_D8BD_4287_BA5D_1263030A22EA
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
/// \class Randomizer
///
/// \ingroup Algorithms RC
///
/// \brief Randomly attenuates amplitudes.
///
////////////////////////////////////////////////////////////////////////////////

/// \todo Randomizer would make sense in a "New phase-vocoder technique..." 
/// presented by Laroche and Dolson in their 2000 paper, where only peaks would 
/// be shifted to random locations.
///                                    (15.02.2010.) (Danijel Domazet)

class Randomizer
{
public: // LE::Algorithm interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DEFINE_PARAMETERS
    (
      //( ( Intensity )( float )( MinimumValue<  1> )( MaximumValue<200> )( DefaultValue< 50> ) ( DisplayValueSuffix<' %'> ) ) 
        ( ( Intensity )( float )( MinimumValue<  0> )( MaximumValue<+24> )( DefaultValue<  0> ) ( DisplayValueSuffix<' dB'>  )  )  
        ( ( BlockSize )( float )( MinimumValue<  0> )( MaximumValue< 10> )( DefaultValue<  5> ) ( DisplayValueSuffix<' bw%'> ) )
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
    /// size of the source data to copy
    unsigned int blockSize_;
    /// 
    float maxMagCorrection_;    
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // randomizer_hpp
