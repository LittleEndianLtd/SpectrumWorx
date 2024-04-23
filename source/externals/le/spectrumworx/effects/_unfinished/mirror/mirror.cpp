////////////////////////////////////////////////////////////////////////////////
///
/// mirror.cpp
/// ----------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "mirror.hpp"

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
// Mirror static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Mirror::title      [] = "Mirror";
char const Mirror::description[] = "Mirrors bands across the spectrum.";


////////////////////////////////////////////////////////////////////////////////
//
// Mirror UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Mirror::Width >::name_[] = "Step size" ;


////////////////////////////////////////////////////////////////////////////////
//
// Mirror::setup()
// ---------------
//
////////////////////////////////////////////////////////////////////////////////

void Mirror::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    width_ = myParameters.get<Width>() * engineSetup.numberOfBins () / 100;
    mode_  = myParameters.get<Mode >();
    bins_  = engineSetup.workingRange( myParameters );
}


////////////////////////////////////////////////////////////////////////////////
//
// Mirror::process()
// -----------------
//
////////////////////////////////////////////////////////////////////////////////

void Mirror::process( ChannelData_AmPh & data ) const
{
    if( !width_ )
        return;

    for
    (
        unsigned int bin( bins_.begin() );
        ( bin + 2 * width_ ) <= bins_.end();
        bin += 2 * width_
    )
    {
         switch ( mode_ )
        {
            case Mode::Magnitudes:
                mirror( data.amplitudes.begin(), bin );
                break;
            case Mode::Phases:
                mirror( data.phases.begin(), bin );
                break;
            case Mode::Both:
                mirror( data.amplitudes.begin(), bin );
                mirror( data.phases    .begin(), bin );
                break;
        }
    }  
}


////////////////////////////////////////////////////////////////////////////////
//
// Mirror::mirror()
// ----------------
//
////////////////////////////////////////////////////////////////////////////////

void Mirror::mirror( float * const data, unsigned int const start ) const
{
    unsigned int first       = start;
    unsigned int last        = start + width_ - 1;
    unsigned int destination = last + 1; 

    std::reverse_copy( &data[ first ], &data[ last ], &data[ destination ] );
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
