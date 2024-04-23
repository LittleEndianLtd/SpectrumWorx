////////////////////////////////////////////////////////////////////////////////
///
/// \file dissonancizer.hpp
/// -----------------------
///
/// Dissonancizer.
///
/// Copyright (c) 2010. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef dissonancizer_hpp__E7B417ED_D7F2_4B45_9F13_ADF61109F4E9
#define dissonancizer_hpp__E7B417ED_D7F2_4B45_9F13_ADF61109F4E9
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
#include "common/buffers.hpp"
#include "utils/peak_detector/peakDetector.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Dissonancizer
///
/// \ingroup Algorithms Alpha
///
/// \brief Adds maximaly dissonant harmonic.
///
////////////////////////////////////////////////////////////////////////////////

class Dissonancizer
{
public: // LE::Algorithm interface.
    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DISCRETE_VALUES_PARAMETER( TargetCreation, ( Sum ) ( Avg )( Rep ) );
    
    DEFINE_PARAMETERS
    (
        ( ( Strength       ) ( float )( MinimumValue<  0> )( MaximumValue< 30> )( DefaultValue< 10> ) ( DisplayValueSuffix<' dB'> ) ) 
        ( ( RemoveOriginal ) ( bool )( DefaultValue< false > ) )
        ( ( TargetCreation )                                   )
    );


  void setup  ( EngineSetup const &, Parameters const & );
  void process( ChannelData_AmPh & ) const;

private: 
  void dissonancize( ChannelData_AmPh & data, Peak const * ) const;

public: // Algorithm traits.
  static bool const canUseTwoInputs = false;

public: 
  static char const title      [];
  static char const description[];

private:
    unsigned int num_bins_ ;
    float        strength_ ;

    // Implementation note:
    //   The peakDetector instance is an internal implementation detail and
    // does not constitute algorithm state.
    //                                        (25.02.2010.) (Domagoj Saric)
    mutable peakDetector  pd             ;    
    bool                  removeOriginal_;
    TargetCreation::Value targetCreation_;
    
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // dissonancizer_hpp

