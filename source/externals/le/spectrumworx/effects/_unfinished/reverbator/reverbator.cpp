////////////////////////////////////////////////////////////////////////////////
///
/// reverbator.cpp
/// --------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "reverbator.hpp"

#include "../../parameters/uiElements.hpp"
#include "../../../ydsp.h"
#include "math/math.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Reverbator static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Reverbator::title      [] = "Reverbator";
char const Reverbator::description[] = "Multiple sound reflections.";


////////////////////////////////////////////////////////////////////////////////
//
// Reverbator UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Reverbator::Depth        >::name_[] = "Depth" ;
char const UIElements<Reverbator::Amount       >::name_[] = "Amount";
char const UIElements<Reverbator::Mode         >::name_[] = "Mode"  ;
//char const UIElements<Reverbator::StopFrequency>::name_[] = "Stop frequency";

DISCRETE_VALUE_STRING( Reverbator, Mode, Classic  ) = "Classic" ;
DISCRETE_VALUE_STRING( Reverbator, Mode, Takuramu ) = "Takuramu";


////////////////////////////////////////////////////////////////////////////////
//
// Reverbator::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void Reverbator::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    num_bins_ = engineSetup.numberOfBins ();
   
    depth_    = engineSetup.secondsToFrames( myParameters.get<Depth>()/1000.0f );
    amount_   = Math::db2normalizedLinear( myParameters.get<Amount>() );
    mode_     = myParameters.get<Mode  >();
    rightBin_ = 1 + engineSetup.normalizedFrequencyToBin( myParameters.get<StopFrequency>() );
}

////////////////////////////////////////////////////////////////////////////////
//
// Reverbator::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void Reverbator::process( ChannelState & cs, ChannelData_AmPh & data ) const
{
    // Frame counter:
    cs.frameCount++; if( cs.frameCount >= depth_ ) cs.frameCount = 0;
    
    
    if ( mode_ == Mode::Classic )
    {
        for ( unsigned int k( 0 ); k < rightBin_; ++k )
        {   
         // Udo Zolzer, "DAFx": y(n) = -g * x(n) + x(n-Delay) + g * y(n-Delay)                        
         // tempMag[ k ] = -factor * data.amplitudes[ k ] + cs.savedInMag[ k ] + factor * cs.savedOutMag[ k ];
            
            tempMag[ k ] =  + cs.savedInMag[ k ] + amount_ * ( cs.savedOutMag[ k ] - data.amplitudes[ k ]) ;
        } 
        for ( unsigned int k( rightBin_ ); k < num_bins_; ++k )
        {                
            tempMag[ k ] =  data.amplitudes[ k ];
        } 
        
        if( cs.frameCount == 0 )
        {
            std::memcpy( cs.savedInMag.begin() , data.amplitudes.begin(), num_bins_ * sizeof( float ) );
            std::memcpy( cs.savedOutMag.begin(), tempMag.begin()        , num_bins_ * sizeof( float ) );
        }
        
        // Output:
        std::memcpy( data.amplitudes.begin(), tempMag.begin(), num_bins_ * sizeof( float ) );        
    }   
    else
    if ( mode_ == Mode::Takuramu )
    {
      //const float factor =  1.0f - ( 1.0f / std::pow( 2.0f, amount_/10.0f ) );

        // "reverb"
        for ( unsigned int k( 0 ); k < rightBin_; ++k )
        {
          //cs.synthesizedFreqs2_[ k ] = cs.synthesizedFreqsB_[ k ] + ( data.phases    [ k ] - cs.synthesizedFreqs2_[ k ] ) * factor;
            cs.savedInMagPrev[ k ] = cs.savedInMag[ k ] + ( data.amplitudes[ k ] - cs.savedInMagPrev[ k ] ) * amount_;
        }
        for ( unsigned int k( rightBin_ ); k < num_bins_; ++k )
        {                
            cs.savedInMagPrev[ k ] =  data.amplitudes[ k ];
        }         
        
        if( cs.frameCount == 0 )
        {
            std::memcpy( cs.savedInMag.begin() , data.amplitudes.begin(), num_bins_ * sizeof( float ) );
        }
        
        // Output:
        std::memcpy( data.amplitudes.begin(), cs.savedInMagPrev.begin(), num_bins_ * sizeof( float ) );        
    }
    
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
