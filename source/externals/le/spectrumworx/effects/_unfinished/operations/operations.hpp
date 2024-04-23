////////////////////////////////////////////////////////////////////////////////
///
/// \file operations.hpp
/// --------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef operations_hpp__42479D97_2DB9_4249_87EF_851D727FAF71
#define operations_hpp__42479D97_2DB9_4249_87EF_851D727FAF71
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
/// \class Operations
///
/// \ingroup Algorithms RC
///
/// \brief Math operations between main and side channel.
///
////////////////////////////////////////////////////////////////////////////////

class Operations
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DISCRETE_VALUES_PARAMETER( Operation, ( Add )( Sub ) (InvSub) );

    DEFINE_PARAMETERS
    (
        ( ( Operation      ) )      
        ( ( Weight         ) ( float   )( MinimumValue<-24> )( MaximumValue<+24> )( DefaultValue<  0> ) ( DisplayValueSuffix<' dB'> )  )  
        ( ( StartFrequency ) )
        ( ( StopFrequency  ) )        
    );


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( EngineSetup const &, Parameters const & );
    void process( ChannelData_ReIm & ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];

private:
    float               weight_   ;
    Operation::Value    operation_;
    InclusiveIndexRange bins_     ;  
        
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // operations
