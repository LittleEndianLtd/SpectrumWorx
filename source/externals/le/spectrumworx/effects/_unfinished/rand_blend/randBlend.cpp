////////////////////////////////////////////////////////////////////////////////
///
/// randBlend.cpp
/// -------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "randBlend.hpp"

#include "../../parameters/uiElements.hpp"
#include "../../../ydsp.h"
#include "math/conversion.hpp"
#include "../../../fastmath.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// RandBlend static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const RandBlend::title      [] = "Rand Blend";
char const RandBlend::description[] = "Random frequency blend.";


////////////////////////////////////////////////////////////////////////////////
//
// RandBlend UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<RandBlend::Mode  >::name_[] = "Morph mode"     ;
char const UIElements<RandBlend::Amount>::name_[] = "Amount"         ;
char const UIElements<RandBlend::Range >::name_[] = "Range"          ;

DISCRETE_VALUE_STRING( RandBlend, Mode, Replace  ) = "Replace"       ;
DISCRETE_VALUE_STRING( RandBlend, Mode, Blend    ) = "Blend"         ;
DISCRETE_VALUE_STRING( RandBlend, Mode, BlendInv ) = "Inverse blend" ;

/// \note Alex says: "Turnupeer – another adaptive interpolation (blend) module.
/// It builds a special grid based on spectral bin relations and smoothly 
/// transitions one input to another. Factor is the same here as in Morpheus – 
/// transition factor. There are only 5 modes for this module: modes 0 - 3 are 
/// exactly same as in the Morpheus, and mode 4 is same as mode 5: it 
/// interpolates magnitude linearly and performs angular interpolation for 
/// phases.	Highly recommended for pads or «texture» sounds. Noticeable spectral 
/// artifacts may be introduced when using some sources. 
/// Can be used in Phase Vocoder domain, as well. 
/// Algorithm comments:  This is variation of Morpheus, morph happens if 
/// generated grid allows us. I.e. we don't detect peaks but select bins to 
/// replace them randomly in the beginning but with update their relations in 
/// grid during processing. This is sort of very basic adaptive harmonics 
/// detection."
///                                    (09.03.2010.) (Danijel Domazet)
///

////////////////////////////////////////////////////////////////////////////////
//
// RandBlend::setup()
// ------------------
//
////////////////////////////////////////////////////////////////////////////////

void RandBlend::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{
    num_bins_ = engineSetup.numberOfBins();

    mode_   = myParameters.get<Mode >();
    range_  = myParameters.get<Range>() * num_bins_ / 100;
    amount_ = Math::percentage2NormalizedLinear( myParameters.get<Amount>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// RandBlend::process()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void RandBlend::process( ChannelData_AmPh & data ) const
{      
    for ( unsigned int k( 0 ); k < range_; ++k )
    {
        unsigned int const x( Math::rangedRand( num_bins_ ) ); 

        switch ( mode_ )
        {
            case Mode::Replace:// replace main with side               
                data.amplitudes[ x ] = amount_ * data.sideChannelAmplitudes[ x ];
                data.phases    [ x ] = data.sideChannelPhases    [ x ];
            break;

            case Mode::Blend: // blend 
                data.amplitudes[ x ] +=  amount_ * ( data.sideChannelAmplitudes[ x ] - data.amplitudes[ x ] );
                if( amount_ > 0.5 )
                    data.phases[ x ]  = data.sideChannelPhases[ x ];                
            break;

            case Mode::BlendInv: // blend inverse
                data.amplitudes[ x ] +=  amount_ * ( - data.sideChannelAmplitudes[ x ] + data.amplitudes[ x ] );
                if( amount_ < 0.5 )
                    data.phases[ x ]  = data.sideChannelPhases[ x ];
                break;
                        
            default:
                break;
        }        
    }   

}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
