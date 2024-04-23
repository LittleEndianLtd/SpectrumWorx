////////////////////////////////////////////////////////////////////////////////
///
/// \file transientsExtractor.hpp
/// -----------------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef transientsExtractor_hpp__A0A0BA5A_6F3F_4E4F_B1F1_2D2DCB7E2438
#define transientsExtractor_hpp__A0A0BA5A_6F3F_4E4F_B1F1_2D2DCB7E2438
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
/// \class TransientsExtractor
///
/// \ingroup Algorithms RC
///
/// \brief Extracts transients.
///
////////////////////////////////////////////////////////////////////////////////

class TransientsExtractor
{
public: // LE::Algorithm interface.
  ////////////////////////////////////////////////////////////////////////////
  // Parameters
  ////////////////////////////////////////////////////////////////////////////

  DEFINE_PARAMETERS
  (
  //( ( THD         )( float )( MinimumValue<  0> )( MaximumValue< 10> )( DefaultValue<  5> )( RangeValuesDenominator<10> ) )
    ( ( THD         )( float )( MinimumValue<  0> )( MaximumValue<100> )( DefaultValue< 50> )( DisplayValueSuffix<' %'>  ) )
    ( ( Sign        )( float )( MinimumValue<-10> )( MaximumValue< 10> )( DefaultValue< 10> )( RangeValuesDenominator<10> ) )
    ( ( StartFrequency ) )
    ( ( StopFrequency  ) )
  );
  
  struct ChannelState 
  {
    Common::SSEAlignedHalfFFTBuffer prevReals_;
    Common::SSEAlignedHalfFFTBuffer prevImags_;

    Common::SSEAlignedHalfFFTBuffer processedPrevReals_;
    Common::SSEAlignedHalfFFTBuffer processedPrevImags_;
    
    void clear()
    {
        prevReals_.clear();
        prevImags_.clear();
        processedPrevReals_.clear();
        processedPrevImags_.clear();    
    }      
  };


  void setup  ( EngineSetup const &, Parameters const & );
  void process( ChannelState & , ChannelData_ReIm & ) const;

public: // Algorithm traits.
  static bool const canUseTwoInputs = false;

public: 
  static char const title      [];
  static char const description[];

private:
    float sign_     ;
    float threshold_;

    InclusiveIndexRange workingRange_;

};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // transientsExtractor_hpp

