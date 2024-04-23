////////////////////////////////////////////////////////////////////////////////
///
/// tai.cpp
/// -------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "tai.hpp"

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
// Tai static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Tai::title      [] = "Tai";
char const Tai::description[] = "Product of frequency domain compressed signals.";


////////////////////////////////////////////////////////////////////////////////
//
// Tai UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Tai::MainRatio    >::name_[] = "Ratio main";
char const UIElements<Tai::SideRatio    >::name_[] = "Ratio side";
char const UIElements<Tai::MainThreshold>::name_[] = "Threshold main";
char const UIElements<Tai::SideThreshold>::name_[] = "Threshold side";


////////////////////////////////////////////////////////////////////////////////
//
// Tai::setup()
// ------------
//
////////////////////////////////////////////////////////////////////////////////

void Tai::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    num_bins_      = engineSetup.numberOfBins    ();
    maxAmplitude_  = engineSetup.maximumAmplitude();

    ratioMain_     = myParameters.get<MainRatio    >();
    ratioSide_     = myParameters.get<SideRatio    >();
    thresholdMain_ = myParameters.get<MainThreshold>();
    thresholdSide_ = myParameters.get<SideThreshold>();
}

////////////////////////////////////////////////////////////////////////////////
//
// Tai::process()
// --------------
//
////////////////////////////////////////////////////////////////////////////////

void Tai::process( ChannelData_AmPh & data ) const
{        
    float sideAmplitude; 

    for ( unsigned int k( 0 ); k < num_bins_; ++k )
    {               
        // Transfer magnitudes to decibel scale, take global Max as a reference:
        float magMain = 20.0f * log10f( data.amplitudes           [ k ]/maxAmplitude_);
        float magSide = 20.0f * log10f( data.sideChannelAmplitudes[ k ]/maxAmplitude_);
        
        // If magnitude higher than threshold, "compress" it:        
        if( magMain > thresholdMain_  )
            magMain = thresholdMain_ + ( magMain - thresholdMain_ )/ratioMain_; 
            
        if( magSide > thresholdSide_  )
            magSide = thresholdSide_ + ( magSide - thresholdSide_ )/ratioSide_;                  

        // Main channel back from dB
        data.amplitudes[ k ] = maxAmplitude_ * std::pow( 10, magMain/20.0f );        
        // Side channel back from dB
        sideAmplitude        = maxAmplitude_ * std::pow( 10, magSide/20.0f );   

        // AND FINALY COMBINE TWO COMPRESSED SIGNALS:
        data.amplitudes[ k ] *= sideAmplitude;
    }  

}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE

