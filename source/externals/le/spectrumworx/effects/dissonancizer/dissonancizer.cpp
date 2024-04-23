////////////////////////////////////////////////////////////////////////////////
///
/// dissonancizer.cpp
/// -----------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "dissonancizer.hpp"

#include "../../parameters/uiElements.hpp"
#include "common/platformSpecifics.hpp"

//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms 
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Dissonancizer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Dissonancizer::title      [] = "Dissonancizer";
char const Dissonancizer::description[] = "Adds maximaly dissonant harmonic.";


////////////////////////////////////////////////////////////////////////////////
//
// Dissonancizer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Dissonancizer::Strength >::name_[]      = "Peak strength"  ;
char const UIElements<Dissonancizer::RemoveOriginal>::name_[] = "Remove original";
char const UIElements<Dissonancizer::TargetCreation>::name_[] = "Target creation";

DISCRETE_VALUE_STRING( Dissonancizer, TargetCreation, Sum  ) = "Sum";
DISCRETE_VALUE_STRING( Dissonancizer, TargetCreation, Avg  ) = "Average";
DISCRETE_VALUE_STRING( Dissonancizer, TargetCreation, Rep  ) = "Replace";


////////////////////////////////////////////////////////////////////////////////
//
// Dissonancizer::setup()
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

void Dissonancizer::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{      
    // User control
    strength_ = myParameters.get<Strength >();
   
    removeOriginal_ = myParameters.get<RemoveOriginal>();
    targetCreation_ = myParameters.get<TargetCreation>();    
    
    // Engine info
    num_bins_ = engineSetup.numberOfBins();

    // ...
    pd.setStrengthThreshold( strength_ );

}


////////////////////////////////////////////////////////////////////////////////
//
// Dissonancizer::process()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////

void Dissonancizer::process( ChannelData_AmPh & data ) const
{ 
    // find peaks in input:
    // pd.setZeroDecibelValue( maxAmplitude );

    //pd.findPeeksAndStrengthSort( data.amplitudes.begin(), num_bins_ ); 
    pd.findPeeksAndStrengthSortAndEstimateFrequency( data.amplitudes.begin(), num_bins_, 44100 ); 
   
    const Peak* peak;
    int num_peaks; 
    int i;

    num_peaks = pd.getNumPeeks(); 
    
    // Add dissonant harmonic to each found peak:
    for( i = 0; i < num_peaks; i++ )
    {
        peak = pd.getPeek( i );

        dissonancize( data, peak ); 
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Dissonancizer::harmonize()
// -------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void Dissonancizer::dissonancize( ChannelData_AmPh & data, Peak const * pPeak ) const
{
    unsigned int start, stop, i, j; 

    // !
    j = pPeak->start_pos + static_cast<unsigned int> ( 2.27f * std::pow( static_cast<float>(pPeak->start_pos), 0.477f ) + 0.5f );

    start = pPeak->start_pos;
    stop  = pPeak->stop_pos;
    

    // Copy the complete pPeak to another location:
    for( i = start; i <= stop; i++ )
    {        
        if( j < num_bins_ )
        {
            switch ( targetCreation_ )
            {
                case TargetCreation::Sum:
                {
                    // Target amplitude is averaged with existing one in order to avoid 
                    // saturation:
                    data.amplitudes[ j ] = ( data.amplitudes[ i ] + data.amplitudes[ j ] );
                }
                break;                 
                case TargetCreation::Avg:
                {
                    // Target amplitude is averaged with existing one in order to avoid 
                    // saturation:
                    data.amplitudes[ j ] = ( data.amplitudes[ i ] + data.amplitudes[ j ] ) / 2.0f;
                }
                break;            
                case TargetCreation::Rep:
                {
                    data.amplitudes[ j ] = data.amplitudes[ i ];
                }
                break;                
            }                 
        }                        
        
        if( removeOriginal_ )
        {
            data.amplitudes[ i ] = 0.0f;
            data.phases    [ i ] = 0.0f;
        }

        j++;
    }  
    
    /// \todo Implement phase correction according to Laroche&Dolson. Without
    ///       phase correction this algo is not complete.
    ///                                    (26.02.2010.) (Danijel Domazet)
    ///

}


//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
