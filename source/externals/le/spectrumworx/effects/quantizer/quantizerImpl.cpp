////////////////////////////////////////////////////////////////////////////////
///
/// quantizerImpl.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "quantizerImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Quantizer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Quantizer::title      [] = "Quantizer";
char const Quantizer::description[] = "Quantize the spectrum.";


////////////////////////////////////////////////////////////////////////////////
//
// Quantizer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Quantizer::Width  , "Width"   )
EFFECT_PARAMETER_NAME( Quantizer::Origami, "Origami" )


////////////////////////////////////////////////////////////////////////////////
//
// QuantizerImpl::setup()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void QuantizerImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    chunkSize_ = std::min( engineSetup.frequencyInHzToBin( parameters().get<Width>() ), workingRange.size() );
    origami_   = Math::percentage2NormalisedLinear( parameters().get<Origami>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// QuantizerImpl::process()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

void QuantizerImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    if ( chunkSize_ < 2 ) 
        return;

    /*
    switch(mode_)
    {
        case Mode::Magnitudes:
            quantize( data.amps().begin() );
            break;
            
        case Mode::Phases: 
            quantize( data.phases    .begin() );
            break;
            
        case Mode::Both: 
            quantize( data.amps().begin() );
            quantize( data.phases    .begin() );
            break;
     }
     */

    quantize( data.amps() );
}


////////////////////////////////////////////////////////////////////////////////
//
// QuantizerImpl::quantize()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

void QuantizerImpl::quantize( DataRange amps ) const
{
    auto const chunkSize( chunkSize_ );

    float const origamiByChunkSize( origami_ / Math::convert<float>( chunkSize ) );

    while ( amps )
    {
        auto const currentChunkSize( std::min( static_cast<IndexRange::value_type>( amps.size() ), chunkSize ) );

        float * pCurrentChunkValue( amps.begin() );

        amps.advance_begin( currentChunkSize );

        float const magStart( *pCurrentChunkValue   );
        float const magEnd  ( *( amps.begin() - 1 ) );

        float const dm( ( magEnd - magStart ) * origamiByChunkSize );

        float mag( magStart );
        while ( pCurrentChunkValue < amps.begin() )
        {
            *pCurrentChunkValue++  = mag;
            mag                   += dm;
        }
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
