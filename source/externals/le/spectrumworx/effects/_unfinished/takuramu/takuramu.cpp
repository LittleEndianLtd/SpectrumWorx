////////////////////////////////////////////////////////////////////////////////
///
/// takuramu.cpp
/// ------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "takuramu.hpp"

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
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Takuramu static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Takuramu::title      [] = "Takuramu";
char const Takuramu::description[] = "Hideki's Takuramu.";


////////////////////////////////////////////////////////////////////////////////
//
// Takuramu UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

/// \todo Fix this.
///                                    (22.11.2009.) (Ivan Dokmanic)

char const UIElements<Takuramu::Pitch  >::name_[] = "Pitch"  ;
char const UIElements<Takuramu::Warp   >::name_[] = "Warp"   ;
char const UIElements<Takuramu::Formant>::name_[] = "Formant";

DISCRETE_VALUE_STRING( Takuramu, Formant, Yes   ) = "Yes"  ;
DISCRETE_VALUE_STRING( Takuramu, Formant, No    ) = "No"   ;


////////////////////////////////////////////////////////////////////////////////
//
// Takuramu::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void Takuramu::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    halfFFTSize_ = engineSetup.fftSize<unsigned int>() / 2;

    pitch_   = myParameters.get<Pitch  >();
    warp_    = myParameters.get<Warp   >();
    formant_ = myParameters.get<Formant>();
}


////////////////////////////////////////////////////////////////////////////////
//
// Takuramu::process()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void Takuramu::process( ChannelData_AmPh & data ) const
{
    //
    synthesizedMagns_.clear();
    synthesizedFreqs_.clear();
    peaks_           .clear();
    bandDiff_        .clear();
    
    // find peaks
    float largestPeak( 0 );
    for ( unsigned int k( 2 ); k < halfFFTSize_; ++k )
    {
        if ( ( data.amplitudes[ k ] > data.amplitudes[ k - 2 ] )
          && ( data.amplitudes[ k ] > data.amplitudes[ k - 1 ] )
          && ( data.amplitudes[ k ] > data.amplitudes[ k + 1 ] )
          && ( data.amplitudes[ k ] > data.amplitudes[ k + 2 ] ) )
        {
            peaks_[ k ] = data.amplitudes[ k ];
            if ( peaks_[ k ] > largestPeak )
            {
                largestPeak = peaks_[ k ];
            }
        }
    }
    
    peaks_[ 0 ] = data.amplitudes[ 0 ];
    peaks_[ halfFFTSize_ - 1 ] = data.amplitudes[ halfFFTSize_ - 1 ];

    if ( largestPeak <= 0 )
    {
        largestPeak = 1;
    }

    // Implementation note:
    //   (Alex): Find base frequency. Works kinda poopily. We can replace this
    // algo.
    //                                     (17.11.2009.) (Ivan Dokmanic)

    float        to            ( peaks_[ 1 ] );
    float        from          ( peaks_[ 1 ] );
    float        baseFreq      (      0      );
    unsigned int numberOfPeaks (      0      );
    unsigned int k1            (      2      );
    unsigned int k2            (      3      );

    while ( k1 < halfFFTSize_ && k2 < halfFFTSize_ )
    {
        while ( ( k2 + 1 < halfFFTSize_ ) && ( peaks_[ k2 ] == 0 ) )
            ++k2;
        assert( k1 < halfFFTSize_ && k2 < halfFFTSize_ );

        to = peaks_[ k2 ];
        for ( unsigned int k( k1 ); k < k2; ++k )
        {
            peaks_[ k ] = from + ( to + from ) * ( k - k1 ) / ( k2 - k1 );
        }
        bandDiff_[ numberOfPeaks++ ] = data.amplitudes[ k2 ] - data.amplitudes[ k1 ];

        k1 = k2++;
        from = to;
    }

    
    /// \todo Replace this terrible thing with some scal-vec multiplication
    /// from FrameWave of ACML.
    ///                                    (22.11.2009.) (Ivan Dokmanic)
    float invHalfFFTSize = 1.f / halfFFTSize_;
    for ( unsigned int k( 0 ); k < halfFFTSize_; k += 4 )
    {
        indices_[ k + 0 ] = ( k + 0 ) * invHalfFFTSize;
        indices_[ k + 1 ] = ( k + 1 ) * invHalfFFTSize;
        indices_[ k + 2 ] = ( k + 2 ) * invHalfFFTSize;
        indices_[ k + 3 ] = ( k + 3 ) * invHalfFFTSize;
    }

    const float inharm = std::pow( 2.f, warp_ / 100.f ); 
    /// \todo Move this to LE math.
    ///                                       (18.11.2009.) (Ivan Dokmanic)
    PowApprox_Vec_Vec( pitches_.begin(), indices_.begin(), halfFFTSize_, inharm );

    unsigned int idx;
    for ( unsigned int k( 0 ); k < halfFFTSize_; ++k )
    {
        idx = Math::round( pitches_[ k ] / basePitchScale_ );

        if ( idx < halfFFTSize_ )
        {
            float magn;
            if ( formant_ == Formant::Yes ) // normalize with peak value
            {
                /// \todo Thoroughly rewrite all this. Its very bad...
                /// (Alex's funny ways).
                ///                           (18.11.2009.) (Ivan Dokmanic)
                float tpeak = peaks_[ idx ];
                if ( tpeak == 0 ) tpeak = 1;
                magn = ( k < 14 ) ? data.amplitudes[ idx ] : data.amplitudes[ idx ] * largestPeak / tpeak;
                if ( largestPeak == 0 ) largestPeak = 1;
                magn = (k < 14) ? magn : magn * peaks_[ k ] / largestPeak;
            }
            else
            {
                magn = data.amplitudes[ idx ];
            }

            if ( magn > synthesizedMagns_[ k ] )
            {
                synthesizedMagns_[ k ] = magn;
                synthesizedFreqs_[ k ] = data.phases[ k ] * basePitchScale_;
            }

            // Fill empty bins with nearest neighbor. We could interpolate
            // linearly here.
            if ( ( synthesizedFreqs_[ k ] == 0 ) && ( k > 0 ) )
            {
                synthesizedFreqs_[ k ] = synthesizedFreqs_[ k - 1 ];
                synthesizedMagns_[ k ] = synthesizedMagns_[ k - 1 ];
            }
        }
    }

    std::memcpy( data.amplitudes.begin(), synthesizedMagns_.begin(), halfFFTSize_ * sizeof( float ) );
    std::memcpy( data.phases    .begin(), synthesizedFreqs_.begin(), halfFFTSize_ * sizeof( float ) );
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
