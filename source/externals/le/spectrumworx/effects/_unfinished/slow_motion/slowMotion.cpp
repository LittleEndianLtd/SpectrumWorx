////////////////////////////////////////////////////////////////////////////////
///
/// slowMotion.cpp
/// --------------
///
/// Copyright (c) 2010. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "slowMotion.hpp"

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
// SlowMotion static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const SlowMotion::title      [] = "Slow Motion";
char const SlowMotion::description[] = "Limits magnitude change speed.";


////////////////////////////////////////////////////////////////////////////////
//
// SlowMotion UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

UI_NAME( SlowMotion::LimitRise ) = "Speed";
UI_NAME( SlowMotion::Threshold ) = "Threshold";
UI_NAME( SlowMotion::Step      ) = "Step";


////////////////////////////////////////////////////////////////////////////////
//
// SlowMotion::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void SlowMotion::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    num_bins_ = engineSetup.numberOfBins(); 
  
    float const limitR( myParameters.get<LimitRise>() / engineSetup.stepsPerSecond() );
    
    limitRise_ = std::pow( 10, ( limitR / ( 2.0f * 10.0f) ) ); // microBell    
    
    threshold_ = engineSetup.maximumAmplitude () * std::pow( 10.0f, myParameters.get<Threshold>() / 20.0f );

    step_ = myParameters.get<Step>() / 1000.0f;
}


////////////////////////////////////////////////////////////////////////////////
//
// SlowMotion::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void SlowMotion::process( ChannelState & cs, ChannelData_AmPh & data ) const
{
    float const epsilon( 1E-20f );
    float gain;

    // Initialize the buffer:
    if ( !cs.isInitialized )
    {
        std::memcpy( cs.magsPrev.begin(), data.amplitudes.begin(), num_bins_ * sizeof( float ) );
        cs.isInitialized = true;
    }
    
    // Get new target:
    for( unsigned int k( 0 ); k < num_bins_; ++k )
    {
        // If amplitude is higher make it target!
        if( data.amplitudes[k] > (cs.magsTargetNew[k]*step_) && data.amplitudes[k] > threshold_ )
        {
            cs.magsTargetNew[k] = data.amplitudes[k];
        }
        else
        {
            //cs.magsTargetNew[k] = 0.0f;
        }

        if( cs.reached[ k ] )
            cs.magsTarget[ k ] = cs.magsTargetNew[ k ];
    }

    for( unsigned int k( 0 ); k < num_bins_; ++k )
    {         
        //// If we reached the target, then reset it:
        //if( cs.magsPrev[k] > cs.magsTarget[k] /*&& data.amplitudes[k] < threshold_ */)
        //{
        //    // cs.magsTarget[k] = 0.0f;
        //    cs.reached[ k ] = true;
        //}


        if( data.amplitudes[k] < threshold_ )
            cs.reached[ k ] = true;
        else
        {            
            // Go slowly towards the target:            
            gain = cs.magsTarget[k] / ( cs.magsPrev[k] + epsilon ); 
            
            if( gain  > limitRise_ )
            {
                cs.reached[ k ] = false;
                data.amplitudes[k] = ( cs.magsPrev[k] + epsilon ) * limitRise_;
            }
            else
            {
                //cs.reached[ k ] = true;
            }
        } 
    }

    // Save new amplitudes for next comparison:
    std::memcpy( cs.magsPrev.begin(), data.amplitudes.begin(), num_bins_ * sizeof( float ) );
}


void SlowMotion::ChannelState::clear()
{
    magsPrev.clear();
    magsTarget.clear();
    magsTargetNew.clear();

    isInitialized = false;
}


//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
