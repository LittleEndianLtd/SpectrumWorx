////////////////////////////////////////////////////////////////////////////////
///
/// randomizer.cpp
/// --------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "randomizer.hpp"

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
// Randomizer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Randomizer::title      [] = "Randomizer";
char const Randomizer::description[] = "Random amplitude modification.";


////////////////////////////////////////////////////////////////////////////////
//
// Randomizer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<Randomizer::Intensity>::name_[] = "Max amplification";
char const UIElements<Randomizer::BlockSize>::name_[] = "Block size"       ;


////////////////////////////////////////////////////////////////////////////////
//
// Randomizer::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void Randomizer::setup( EngineSetup const & engineSetup, Randomizer::Parameters const & myParameters )
{      
    num_bins_         = engineSetup.numberOfBins();    
    maxMagCorrection_ = myParameters.get<Intensity>();
    blockSize_        = Math::convert<unsigned int>( Math::percentage2NormalizedLinear( myParameters.get<BlockSize>() ) * num_bins_ );
}                                                      


////////////////////////////////////////////////////////////////////////////////
//
// Randomizer::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void Randomizer::process( ChannelData_AmPh & data ) const
{
    for( unsigned int k = 0; k < ( num_bins_ - blockSize_ ); )
    {
      //factor = 1.0f + Math::rangedRand( maxMagCorrection_ ) / 100.0f  ;

        float const factor
        (
            Math::db2normalizedLinear
            (
                Math::rangedRand( maxMagCorrection_ )
            )
        );
        for ( unsigned int i = 0; i < blockSize_; ++i )
            data.amplitudes[ k++ ] *= factor;
    }   
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
