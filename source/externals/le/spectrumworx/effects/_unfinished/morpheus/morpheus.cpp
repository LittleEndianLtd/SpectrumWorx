////////////////////////////////////////////////////////////////////////////////
///
/// morpheus.cpp
/// ------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "morpheus.hpp"

#include "../../parameters/uiElements.hpp"
#include "math/math.hpp"
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
// Morpheus static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Morpheus::title      [] = "Morpheus";
char const Morpheus::description[] = "Adaptive spectral morphing.";

/// \note Alex says: "Morpheus is an adaptive spectral blend that may actually 
///  be called a "morph". It does not perform a timbre transformation but 
///  transitions made with this module are very smooth.
///  Factor controls transition from 0 (full Source) to 1 (full Target). The 
///  module has 7 modes, which basically are just different transition (morph) 
///  combinations. 
///  Mode 0 is direct, and replaces magnitudes from input 1 with those from 
///  input 2.  
///  In Mode 1, phase is replaced as in mode 0, but magnitudes are linearly 
///  interpolated.  
///  Mode 2 is similar to mode 2, but magnitudes are replaced and phases are 
///  linearly interpolated.  
///  In Mode 3 both magnitudes and phases are linearly blended.  
///  Mode 4 is the same as mode 3 but uses a special exponential table for 
///  linear blend weights.  Mode 5 interpolates magnitudes linearly and uses a 
///  special angular interpolation for phases. Mode 6 is almost the same as 
///  mode 5 but interpolates the magnitudes with an exponential table.  
///  
///  Fastest knob is a quality control: if yes – the module uses fast sort code 
///  instead of a slower one but with higher precision. It is recommended that 
///  this be turned off if you are processing offline. Use it in Phase Vocoder 
///  domain also.
///  
///  Algorithm comments: Main algorithm here is a find peaks with maximum energy
///  and try to blend around them first. This algorithm can be expanded to find 
///  real energy peaks (or harmonics) and make predictions that this peak is 
///  actually harmonic and not transient or random peak. I read in dafx papers, 
///  that people doing Neural network for harmonic extraction. I'll provide you 
///  guys with papers and research that i didn't finished on this area."
///  
///                                    (21.01.2010.) (Danijel Domazet)
////////////////////////////////////////////////////////////////////////////////
//
// Morpheus UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Morpheus::Mode1 >::name_[] = "Target";
char const UIElements<Morpheus::Mode2 >::name_[] = "Mode";
char const UIElements<Morpheus::Amount>::name_[] = "Amount";
char const UIElements<Morpheus::Range >::name_[] = "Coverage";

DISCRETE_VALUE_STRING( Morpheus, Mode1, AbsDiff  ) = "Main-Side (abs)";
DISCRETE_VALUE_STRING( Morpheus, Mode1, MainDiff ) = "Main-Side"  ;
DISCRETE_VALUE_STRING( Morpheus, Mode1, SideDiff ) = "Side-Main"  ;
DISCRETE_VALUE_STRING( Morpheus, Mode1, Sum      ) = "Main+Side"  ;
DISCRETE_VALUE_STRING( Morpheus, Mode1, Side     ) = "Side"       ;
DISCRETE_VALUE_STRING( Morpheus, Mode1, Main     ) = "Main"       ;

DISCRETE_VALUE_STRING( Morpheus, Mode2, Replace  ) = "Replace"       ;
DISCRETE_VALUE_STRING( Morpheus, Mode2, Blend    ) = "Blend"         ;
DISCRETE_VALUE_STRING( Morpheus, Mode2, BlendInv ) = "Blend inverse" ;


////////////////////////////////////////////////////////////////////////////////
//
// Morpheus::setup()
// -----------------
//
////////////////////////////////////////////////////////////////////////////////

void Morpheus::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    num_bins_ = engineSetup.numberOfBins();
    
    mode1_  = myParameters.get<Mode1>();
    mode2_  = myParameters.get<Mode2>();
    range_  = myParameters.get<Range>() * num_bins_ / 100;
    amount_ = Math::percentage2NormalizedLinear( myParameters.get<Amount>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// Morpheus::process()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void Morpheus::process( ChannelData_AmPh & data ) const
{
    for ( unsigned int k( 0 ); k < num_bins_; ++k )
    {
        peaks_[ k ].pos = k;

        switch ( mode1_ )
        {
            case Mode1::AbsDiff: // main stronger than side or side stronger than main, difference!
                peaks_[ k ].val = std::abs( data.amplitudes[ k ] - data.sideChannelAmplitudes[ k ] );           
                break;

            case Mode1::MainDiff: // main stronger than side
                peaks_[ k ].val = (   data.amplitudes[ k ] - data.sideChannelAmplitudes[ k ] );   
                break;

            case Mode1::SideDiff: // side stronger than main
                peaks_[ k ].val = ( - data.amplitudes[ k ] + data.sideChannelAmplitudes[ k ] );   
                break;

            case Mode1::Sum: // sum of both
                peaks_[ k ].val = (   data.amplitudes[ k ] + data.sideChannelAmplitudes[ k ] );   
                break;

            case Mode1::Side: // side
                peaks_[ k ].val = data.sideChannelAmplitudes[ k ];   
                break;

            case Mode1::Main: // main
                peaks_[ k ].val = data.amplitudes[ k ];   
                break;

            default:
                break;
        }
    }

    // Sort the peaks:    
    std::sort( &peaks_[ 0 ], (&peaks_[ num_bins_ - 1] + 1) );
    

    // How many strongest bins we apply? That's range. 
    
    for ( unsigned int k( 0 ); k < range_; ++k )
    {
        unsigned int x = peaks_[ num_bins_ - 1 - k ].pos; 

        switch ( mode2_ )
        {
            case Mode2::Replace:// replace main with side               
                data.amplitudes[ x ] = amount_ * data.sideChannelAmplitudes[ x ];
                data.phases    [ x ] = data.sideChannelPhases    [ x ];
            break;

            case Mode2::Blend: // blend 
                data.amplitudes[ x ] +=  amount_ * ( data.sideChannelAmplitudes[ x ] - data.amplitudes[ x ] );
                if( amount_ > 0.5 )
                    data.phases[ x ]  = data.sideChannelPhases[ x ];                
            break;

            case Mode2::BlendInv: // blend inverse
                data.amplitudes[ x ] +=  amount_ * ( - data.sideChannelAmplitudes[ x ] + data.amplitudes[ x ] );
                if( amount_ < 0.5 )
                    data.phases[ x ]  = data.sideChannelPhases[ x ];
                break;
                        
            default:
                break;
        }        
    }

    // Blending formula:
    // 
    // out = x * carrier + ( 1 - x ) * blender
    //
    // x = 1 => out = carrier
    // x = 0 => out = blender
    //

    // data.amplitudes[k] +=  amount_ * ( data.sideChannelAmplitudes[k] - data.amplitudes[k] );
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
