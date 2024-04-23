////////////////////////////////////////////////////////////////////////////////
///
/// shapelessImpl.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "shapelessImpl.hpp"

#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <limits>
#include <numeric>
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
// Shapeless static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Shapeless::title      [] = "Shapeless";
char const Shapeless::description[] = "Spectrum shape transfer.";

/// \note Alex says: "Shapeless is a frequency shaper based on an algorithm.
/// Frequency Shaping is an audio dsp technique for tuning sounds to one
/// another. The spectral magnitudes of a signal will be maintained, but the
/// frequencies will be substituted with those from a reference signal. Coarse
/// is a shape factor with a wide range: 0...256 bins. Fine is a shape factor
/// with smaller range: only 0...64 bins.  Total shape factor is Coarse + Fine,
/// i.e. 320 bins max. Invert swaps channels: input 1 becomes input 2 and vice
/// versa. For best results try pad or texture sounds and use in Phase Vocoder
/// domain.
/// Effect comments: This is applying shaping envelope to magnitudes.
/// See doShape function."
///                                    (08.03.2010.) (Danijel Domazet)
///


////////////////////////////////////////////////////////////////////////////////
//
// Shapeless UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Shapeless::Width, "Shape width" )

////////////////////////////////////////////////////////////////////////////////
//
// ShapelessImpl::setup()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void ShapelessImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    width_ = engineSetup.frequencyInHzToBin( parameters().get<Width>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// ShapelessImpl::process()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

void ShapelessImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    auto const width( width_ );
    if ( !width )
        return;

    while ( data )
    {
        auto const currentWidth( std::min( data.size(), width ) );
        auto *       LE_RESTRICT pMagnOut     ( data.main().amps().begin() );
        auto *       LE_RESTRICT pMagnShape   ( data.side().amps().begin() );
        data.advance_begin( currentWidth );
        auto * const LE_RESTRICT pMagnOutEnd  ( data.main().amps().begin() );
        auto * const LE_RESTRICT pMagnShapeEnd( data.side().amps().begin() );

        float const inOut  ( std::accumulate( pMagnOut  , pMagnOutEnd  , std::numeric_limits<float>::epsilon() ) );
        float const inShape( std::accumulate( pMagnShape, pMagnShapeEnd, std::numeric_limits<float>::epsilon() ) );

        float const amplt( inOut / inShape );

        while ( pMagnOut != pMagnOutEnd )
        {
            *pMagnOut++ = amplt * *pMagnShape++;
        }
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
