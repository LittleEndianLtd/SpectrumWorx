////////////////////////////////////////////////////////////////////////////////
///
/// exaggeratorImpl.cpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "exaggeratorImpl.hpp"

#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"

#include <cmath>
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
// Exaggerator static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Exaggerator::title      [] = "Exaggerator";
char const Exaggerator::description[] = "Emphasize or flatten spectral peaks.";


////////////////////////////////////////////////////////////////////////////////
//
// Exaggerator UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Exaggerator::Exaggerate, "Intensity" )


////////////////////////////////////////////////////////////////////////////////
//
// ExaggeratorImpl::setup()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SIZE_BEGIN()

void LE_COLD ExaggeratorImpl::setup( IndexRange const &, Engine::Setup const & )
{
    //exaggerate_  = -1 + 5 * (parameters().get<Exaggerate>() / 128.f + 0.5f);

    // Map [-100, 0, 100] to [-1, +1, 4]. Got this range from original
    // plugin made by Dobson.
    float const & exaggerate ( parameters().get<Exaggerate>()                   );
    float const   scaleFactor( Math::isNegative( exaggerate ) ? 50.0f : 33.333f );
    exaggerate_ = exaggerate / scaleFactor + 1;
}

LE_OPTIMIZE_FOR_SIZE_END()

////////////////////////////////////////////////////////////////////////////////
//
// ExageratorImpl::process()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SPEED_BEGIN()

void LE_HOT ExaggeratorImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    auto const maxamp( Math::max( data.amps() ) );
    if ( !maxamp )
        return;

    float const exaggerate  ( exaggerate_ );
    float       normaliser  ( 1 / maxamp  );
    float       postTotalAmp( 0           );
    float       preTotalAmp ( 0           );

    for ( auto & amplitude : data.amps() )
    {
        preTotalAmp  += amplitude;
        amplitude     = std::pow( amplitude * normaliser, exaggerate );
        postTotalAmp += amplitude;
    }

    BOOST_ASSERT( postTotalAmp > 0 );
    normaliser = ( preTotalAmp / postTotalAmp ) / 2;

    Math::multiply( data.amps(), normaliser );
}

LE_OPTIMIZE_FOR_SPEED_END()

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
