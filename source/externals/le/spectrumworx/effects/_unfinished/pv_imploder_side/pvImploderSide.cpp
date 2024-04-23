////////////////////////////////////////////////////////////////////////////////
///
/// pvAccumulator.cpp
/// -----------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "pvImploderSide.hpp"

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
// PVImploderSide static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PVImploderSide::title      [] = "Side Imploder (pvd)";
char const PVImploderSide::description[] = "Combined implosion with glissando.";


////////////////////////////////////////////////////////////////////////////////
//
// PVImploderSide UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<PVImploderSide::DecayMain>::name_[] = "Decay main";
char const UIElements<PVImploderSide::GlissMain>::name_[] = "Glissando main" ;
char const UIElements<PVImploderSide::DecaySide>::name_[] = "Decay side";
char const UIElements<PVImploderSide::GlissSide>::name_[] = "Glissando side" ;


////////////////////////////////////////////////////////////////////////////////
//
// PVImploderSide::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void PVImploderSide::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    float minA; 
    float frametime;
    float T;
    float octPerSec;
    float octPerFrame;

    // User:
    nyquist_ = 0.95f * engineSetup.sampleRate<float>() / 2.0f;
    num_bins_ = engineSetup.numberOfBins();

    // Some helper vars:
    maxAmplitude_ = engineSetup.maximumAmplitude();
    minA = maxAmplitude_ * std::pow ( 10.0f, -120.0f / 20.0f );
    frametime = engineSetup.stepTime();// engineSetup.fftSize<float>() / engineSetup.sampleRate<float>() / engineSetup.windowOverlappingFactor<float>();

    // Decay time from 0 to -120 dB:

    // Main
    T = static_cast<float> ( myParameters.get<DecayMain>() );

    // ...transfer to frame decay constant (do the math!):
    decayM_ = std::exp( ( frametime / T ) * std::log( minA / maxAmplitude_ ) );

    // Side
    T = static_cast<float> ( myParameters.get<DecaySide>() );

    // ...transfer to frame decay constant (do the math!):
    decayS_ = std::exp( ( frametime / T ) * std::log( minA / maxAmplitude_ ) );

    // Get Cents per Second from input and transform to Octaves per Frame:

    // Main
    octPerSec = static_cast<float> (myParameters.get<GlissMain>()) / 12.0f / 100.0f;      
    octPerFrame = octPerSec * frametime; 
    glissM_ = std::pow( 2.0f, octPerFrame );   

    // Side
    octPerSec = static_cast<float> (myParameters.get<GlissSide>()) / 12.0f / 100.0f;      
    octPerFrame = octPerSec * frametime; 
    glissS_ = std::pow( 2.0f, octPerFrame );    
    
}


////////////////////////////////////////////////////////////////////////////////
//
// PVImploderSide::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void PVImploderSide::process( ChannelState & cs, ChannelData_AmPh & data ) const
{ 
    
    for( unsigned int i = 0; i < num_bins_; ++i )
    {
        float accumAmp;
        float accumPha;
        
        cs.accumMagnMain_ [i] *= decayM_;
        if( cs.accumMagnMain_ [i] > maxAmplitude_ )
            cs.accumMagnMain_ [i] = maxAmplitude_; 

        cs.accumPhaseMain_[i] *= glissM_;
        if( cs.accumPhaseMain_[i] >= nyquist_ ) 
            cs.accumMagnMain_[i] = 0.0f;

        cs.accumMagnSide_ [i] *= decayS_;
        if( cs.accumMagnSide_ [i] > maxAmplitude_ )
            cs.accumMagnSide_ [i] = maxAmplitude_;

        cs.accumPhaseSide_[i] *= glissS_;       
        if( cs.accumPhaseSide_[i] >= nyquist_ ) 
            cs.accumMagnSide_[i] = 0.0f;

        
        // Side update:
        if( data.sideChannelAmplitudes[i] > cs.accumMagnSide_[i] )    
        {
            cs.accumMagnSide_ [i]  = data.sideChannelAmplitudes[i];
            cs.accumPhaseSide_[i]  = data.sideChannelPhases    [i];
        }
        // Main update:
        if( data.amplitudes[i] > cs.accumMagnMain_[i] )    
        {
            cs.accumMagnMain_ [i]  = data.amplitudes[i];
            cs.accumPhaseMain_[i]  = data.phases    [i];
        }
        
        // Choose: 
        if( cs.accumMagnMain_[i] > cs.accumMagnSide_[i] )
        {
            accumAmp = cs.accumMagnMain_[i];
            accumPha = cs.accumPhaseMain_[i];
        }
        else
        {
            accumAmp = cs.accumMagnSide_[i];
            accumPha = cs.accumPhaseSide_[i];
        }
        
        // And update:
        if( data.amplitudes[i] < accumAmp )    
        {
            data.amplitudes[i] = accumAmp;
            data.phases    [i] = accumPha;        
        }
       
   }
}


//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
