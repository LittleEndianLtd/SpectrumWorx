////////////////////////////////////////////////////////////////////////////////
///
/// morphologicalMutations.cpp
/// --------------------------
///
/// Copyright (c) 2010. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "morphologicalMutations.hpp"

#include "../../parameters/uiElements.hpp"
#include "math/math.hpp"
#include "../../../fastmath.h"
#include "fwSignal.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// MorphologicalMutations static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const MorphologicalMutations::title      [] = "Mormut";
char const MorphologicalMutations::description[] = "Morphological mutations.";


////////////////////////////////////////////////////////////////////////////////
//
// MorphologicalMutations UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<MorphologicalMutations::IrregularRange>::name_[] = "Irregular range";
char const UIElements<MorphologicalMutations::UniformGain   >::name_[] = "Uniform gain"   ;
char const UIElements<MorphologicalMutations::Attenuate     >::name_[] = "Gain"           ;
char const UIElements<MorphologicalMutations::Mode          >::name_[] = "Mode"           ;

DISCRETE_VALUE_STRING( MorphologicalMutations, Mode, USIM) = "Uniform signed"    ;
DISCRETE_VALUE_STRING( MorphologicalMutations, Mode, UUIM) = "Uniform unsigned"  ;
DISCRETE_VALUE_STRING( MorphologicalMutations, Mode, IUIM) = "Irregular unsigned";
DISCRETE_VALUE_STRING( MorphologicalMutations, Mode, ISIM) = "Irregular signed"  ;
DISCRETE_VALUE_STRING( MorphologicalMutations, Mode, LCM ) = "Linear contour"    ; 
DISCRETE_VALUE_STRING( MorphologicalMutations, Mode, VM  ) = "Value"             ;


////////////////////////////////////////////////////////////////////////////////
//
// MorphologicalMutations::setup()
// -------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void MorphologicalMutations::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{      
    num_bins_ = engineSetup.numberOfBins();

    irregularRange_ = myParameters.get<IrregularRange>() * num_bins_ / 100;
    uniformGain_    = Math::db2normalizedLinear( myParameters.get<UniformGain>() );
    attenuate_      = Math::db2normalizedLinear( myParameters.get<Attenuate  >() );
    mode_           = myParameters.get<Mode>();
}


////////////////////////////////////////////////////////////////////////////////
//
// MorphologicalMutations::process()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void MorphologicalMutations::process( ChannelState & cs, ChannelData_AmPh & data ) const
{
    float Smag, Tmag, Ssgn, Tsgn, Sint, Tint, Mi=0.0f; // S - main channel, T - side channel
    
    switch (mode_)
    {       
        // Irregular:
        case Mode::IUIM: 
        case Mode::ISIM: 
        case Mode::LCM:            
            irrTrue_.clear();
            for ( unsigned int k( 0 ); k < irregularRange_; ++k )
            {
                unsigned int const x( Math::rangedRand( num_bins_ ) );
                irrTrue_[ x ] = 1.0f;
            }
            break;  
        // Uniform:           
        case Mode::UUIM: 
        case Mode::USIM: 
        case Mode::VM:                  
            break; 
    }

    for ( unsigned int k( 0 ); k < num_bins_; ++k )
    {
        Sint = data.amplitudes           [k] - cs.Sj_[k];
        Tint = data.sideChannelAmplitudes[k] - cs.Tj_[k];
        
        //Sint *= 0.15f;
        //Tint *= 0.15f;
                
        Smag = fabs( Sint );
        Tmag = fabs( Tint );
        
        if(       data.amplitudes[k]  > cs.Sj_[k] )
            Ssgn =  1.0f;
        else if ( data.amplitudes[k] == cs.Sj_[k] )
            Ssgn =  0.0f; 
        else
            Ssgn = -1.0f;                       
                        
        if(       data.sideChannelAmplitudes[k]  > cs.Sj_[k] )
            Tsgn =  1.0f;
        else if ( data.sideChannelAmplitudes[k] == cs.Sj_[k] )
            Tsgn =  0.0f; 
        else
            Tsgn = -1.0f; 


        switch (mode_)
        {
            case Mode::LCM: 
                if( irrTrue_[ k ] )
                    Mi = cs.Mj_[k] + Tsgn * Smag;
                else
                    Mi = cs.Mj_[k] + Ssgn * Smag;
                break;
                
            case Mode::IUIM: 
                if( irrTrue_[ k ] )
                    Mi = cs.Mj_[k] + Ssgn * Tmag; 
                else
                    Mi = cs.Mj_[k] + Ssgn * Smag;
                break;
                
            case Mode::ISIM: 
                if( irrTrue_[ k ] )
                    Mi = cs.Mj_[k] + Tsgn * Tmag; 
                else
                    Mi = cs.Mj_[k] + Ssgn * Smag;
                break;
                
            case Mode::UUIM: 
                Mi = cs.Mj_[k] + Ssgn * (Smag + uniformGain_ * fabs(Tmag - Smag)); 
                break;
            case Mode::USIM: 
                Mi = cs.Mj_[k] + Sint + uniformGain_ * (Tint - Sint); 
                break; 
            case Mode::VM: 
                Mi = data.amplitudes[k] + uniformGain_ * ( pow (data.sideChannelAmplitudes[k] - data.amplitudes[k], 5 )); 
                break; 
        }
        
        // Save data for next frame:
        cs.Sj_[k] = data.amplitudes[k];
        cs.Tj_[k] = data.sideChannelAmplitudes[k];
        
        // If Mj is saved here, then it's without normalization - unstable!
        // cs.Mj_[k] = Mi;
          
        // ...               
        Mi *= attenuate_; 
        
        if( Mi < 0 ) Mi = 0; 
        /// \todo Do some kind of normalization here to avoid ovreflow&underflow.
        ///                                    (11.03.2010.) (Danijel Domazet)
        ///
        
        // That's it:
        data.amplitudes[k] = Mi;        
        
        // If saved here Mj includes normalization; more stable solution.
        cs.Mj_[k] = data.amplitudes[k]; 
                
        /// \todo Handle phase here in some way.
        ///                                    (11.03.2010.) (Danijel Domazet)
        ///        
    }
        
    // Normalize: 
    // normalize( data.amplitudes.begin(), num_bins_ );
    
    // Save normalized? 
    // std::memcpy( cs.Mj_.begin(),data.amplitudes.begin(), num_bins_ * sizeof( float ) );
    
}


////////////////////////////////////////////////////////////////////////////////
//
// MorphologicalMutations::normalize()
// -----------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void MorphologicalMutations::normalize( float * const data, unsigned int const size ) const
{
    float const min( *(std::min_element( &data[ 0 ], &data[ size ] ) ) );

    if ( min < 0 )
        fwsAddC_32f_I( min, data, size ); 
}


//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
