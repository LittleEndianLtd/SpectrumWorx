////////////////////////////////////////////////////////////////////////////////
///
/// subOctaver.cpp
/// --------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "subOctaver.hpp"

#include "../../parameters/uiElements.hpp"
#include "common/platformSpecifics.hpp"
#include "../../math/dft/domainConversion.hpp"
#include "../../math/dft/fft.hpp"
#include "../../../ydsp.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms 
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// SubOctaver static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const SubOctaver::title      [] = "Sub Octaver";
char const SubOctaver::description[] = "Adds two sub octaves.";


////////////////////////////////////////////////////////////////////////////////
//
// SubOctaver UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

UI_NAME( SubOctaver::GainOrig        ) = "Dry";
UI_NAME( SubOctaver::GainOct1        ) = "Sub octave 1";
UI_NAME( SubOctaver::GainOct2        ) = "Sub octave 2";
UI_NAME( SubOctaver::CutoffFrequency ) = "Low pass";


////////////////////////////////////////////////////////////////////////////////
//
// SubOctaver::setup()
// ------------------
//
////////////////////////////////////////////////////////////////////////////////

void SubOctaver::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{
    num_bins_ = engineSetup.numberOfBins();

    gain0_ = Math::db2normalizedLinear( myParameters.get<GainOrig>() );
    gain1_ = Math::db2normalizedLinear( myParameters.get<GainOct1>() );
    gain2_ = Math::db2normalizedLinear( myParameters.get<GainOct2>() );

    ps1_.setup( engineSetup );
    ps2_.setup( engineSetup );
    
    cutoff_ = engineSetup.frequencyInHzToBin( myParameters.get<CutoffFrequency>() ); 
}


////////////////////////////////////////////////////////////////////////////////
//
// SubOctaver::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void SubOctaver::process( ChannelState & cs, ChannelData_AmPh2ReIm const data ) const
{
    {
        ChannelData_AmPh shiftedAmPh1;
        ChannelData_AmPh shiftedAmPh2;

        std::memcpy( shiftedAmPh1.amplitudes.begin(), data.input_.amplitudes.begin(), num_bins_ * sizeof( float ) );
        std::memcpy( shiftedAmPh1.phases    .begin(), data.input_.phases    .begin(), num_bins_ * sizeof( float ) );

        std::memcpy( shiftedAmPh2.amplitudes.begin(), data.input_.amplitudes.begin(), num_bins_ * sizeof( float ) );
        std::memcpy( shiftedAmPh2.phases    .begin(), data.input_.phases    .begin(), num_bins_ * sizeof( float ) );


        float pitchScale1 = Math::semitone2Interval12TET( -12.0f );
        float pitchScale2 = Math::semitone2Interval12TET( -24.0f );

        ps1_.process( pitchScale1, cs.cs1_, shiftedAmPh1 );
        ps2_.process( pitchScale2, cs.cs2_, shiftedAmPh2 );


        Math::amph2ReIm
        (
            shiftedAmPh1.amplitudes.begin(),
            shiftedAmPh1.phases    .begin(),
            shifted1Reals_.begin(),
            shifted1Imags_.begin(),
            num_bins_
        );

        Math::amph2ReIm
        (
            shiftedAmPh2.amplitudes.begin(),
            shiftedAmPh2.phases    .begin(),
            shifted2Reals_.begin(),
            shifted2Imags_.begin(),
            num_bins_
        );
    }
    
    Mult_Vec_Scal( &data.inputOutput_.reals()[ 0 ], num_bins_, gain0_ );
    Mult_Vec_Scal( &data.inputOutput_.imags()[ 0 ], num_bins_, gain0_ );

    Mult_Vec_Scal( &shifted1Reals_[ 0 ]           , cutoff_  , gain1_ );
    Mult_Vec_Scal( &shifted1Imags_[ 0 ]           , cutoff_  , gain1_ );

    Mult_Vec_Scal( &shifted2Reals_[ 0 ]           , cutoff_  , gain2_ );
    Mult_Vec_Scal( &shifted2Imags_[ 0 ]           , cutoff_  , gain2_ );


    for ( unsigned int k( 0 ); k < cutoff_; ++k )
    {
        data.inputOutput_.reals()[ k ] += shifted1Reals_[ k ] + shifted2Reals_[ k ];
        data.inputOutput_.imags()[ k ] += shifted1Imags_[ k ] + shifted2Imags_[ k ];
    }
}

void SubOctaver::ChannelState::clear()
{
    cs1_.clear();
    cs2_.clear();
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
