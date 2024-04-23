////////////////////////////////////////////////////////////////////////////////
///
/// operations.cpp
/// --------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "operations.hpp"

#include "../../parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Operations static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Operations::title      [] = "Operations";
char const Operations::description[] = "Math operations.";


////////////////////////////////////////////////////////////////////////////////
//
// Operations UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Operations::Weight   >::name_[] = "Side gain";
char const UIElements<Operations::Operation>::name_[] = "Operation";

DISCRETE_VALUE_STRING( Operations, Operation, Add    ) = "Main+Side";
DISCRETE_VALUE_STRING( Operations, Operation, Sub    ) = "Main-Side";
DISCRETE_VALUE_STRING( Operations, Operation, InvSub ) = "Side-Main";

////////////////////////////////////////////////////////////////////////////////
//
// Operations::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void Operations::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    weight_    = Math::db2normalizedLinear( myParameters.get<Weight>() );
    operation_ = myParameters.get<Operation>();    
    bins_      = engineSetup.workingRange( myParameters );      
}


////////////////////////////////////////////////////////////////////////////////
//
// Operations::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void Operations::process( ChannelData_ReIm & data ) const
{
    /// \todo Vectorize!!!
    ///                                    (07.12.2009.) (Ivan Dokmanic)
    
    switch(operation_)
    {
    case Operation::Add:
    
        for( InclusiveIndexRange k( bins_ ); k; ++k )
        {
            data.reals ()[ *k ] = data.reals ()[ *k ] + data.sideChannelReals ()[ *k ] * weight_;
            data.imags ()[ *k ] = data.imags ()[ *k ] + data.sideChannelImags ()[ *k ] * weight_;
        }
        break;
        
    case Operation::Sub:
        for( InclusiveIndexRange k( bins_ ); k; ++k )
        {            
            data.reals ()[ *k ] = data.reals ()[ *k ] - data.sideChannelReals ()[ *k ] * weight_;
            data.imags ()[ *k ] = data.imags ()[ *k ] - data.sideChannelImags ()[ *k ] * weight_;              
        }
        break;

    case Operation::InvSub:
        for( InclusiveIndexRange k( bins_ ); k; ++k )
        {
           data.reals ()[ *k ] = - data.reals ()[ *k ] + data.sideChannelReals ()[ *k ] * weight_;
           data.imags ()[ *k ] = - data.imags ()[ *k ] + data.sideChannelImags ()[ *k ] * weight_;                           
        }
        break;
    }
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
