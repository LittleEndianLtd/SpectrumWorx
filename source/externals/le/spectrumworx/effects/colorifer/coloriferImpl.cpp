////////////////////////////////////////////////////////////////////////////////
///
/// coloriferImpl.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "coloriferImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include <limits>
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
// Colorifer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Colorifer::title      [] = "Colorifer";
char const Colorifer::description[] = "Spectrum colour transfer.";


////////////////////////////////////////////////////////////////////////////////
//
// Colorifer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Colorifer::BandWidth         , "Shape width"         )
EFFECT_PARAMETER_NAME( Colorifer::SpectrumPreprocess, "Spectrum preprocess" )
EFFECT_PARAMETER_NAME( Colorifer::ReplacePhase      , "Replace phase"       )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Colorifer, SpectrumPreprocess,
    (( NotUsed    , "None"        ))
    (( SquareRoot , "Square root" ))
    (( Square     , "Square"      ))
    (( Exponential, "Exponent"    ))
)

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Colorifer, ReplacePhase,
    (( No , "No"  ))
    (( Yes, "Yes" ))
)


////////////////////////////////////////////////////////////////////////////////
//
// ColoriferImpl::setup()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void LE_COLD ColoriferImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    shapeWidth_ = std::min( workingRange.size(), engineSetup.frequencyInHzToBin( parameters().get<BandWidth>() ) );
}


////////////////////////////////////////////////////////////////////////////////
//
// ColoriferImpl::process()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SPEED_BEGIN()

void LE_HOT ColoriferImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    if ( shapeWidth_ == 0 )
        return;

    using namespace Math;

    auto       & main( data.main() );
    auto const & side( data.side() );

    if ( parameters().get<ReplacePhase>() == ReplacePhase::Yes )
        copy( side.phases(), main.phases() );

    auto const numberOfBins( data.numberOfBins()                               );
    auto const mode        ( parameters().get<SpectrumPreprocess>().getValue() );

    auto const & mainAmps( main.amps() );
    auto const & sideAmps( side.amps() );

    ReadOnlyDataRange x;
    ReadOnlyDataRange y;
    if ( mode == SpectrumPreprocess::NotUsed )
    {
        x = sideAmps;
        y = mainAmps;
    }
    else
    {
        BOOST_SIMD_ALIGNED_STACK_BUFFER( xStorage, Engine::real_t, numberOfBins );
        BOOST_SIMD_ALIGNED_STACK_BUFFER( yStorage, Engine::real_t, numberOfBins );
        x = xStorage;
        y = yStorage;
        copy( sideAmps.begin(), xStorage.begin(), numberOfBins ); //...mrmlj...no out-of-place vectorized squareRoot, square, exp...
        copy( mainAmps.begin(), yStorage.begin(), numberOfBins );

        LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();
        switch ( mode )
        {
            case SpectrumPreprocess::SquareRoot : squareRoot( /*data.side(),*/ xStorage ); squareRoot( /*data.main(),*/ yStorage ); break;
            case SpectrumPreprocess::Square     : square    ( /*data.side(),*/ xStorage ); square    ( /*data.main(),*/ yStorage ); break;
            case SpectrumPreprocess::Exponential: exp       ( /*data.side(),*/ xStorage ); exp       ( /*data.main(),*/ yStorage ); break;

            LE_DEFAULT_CASE_UNREACHABLE();
        }
    }

    auto shapeWidth( shapeWidth_  );
    auto binsLeft  ( numberOfBins );
    float       * LE_RESTRICT pAmps( mainAmps.begin() );
    float const * LE_RESTRICT pX   ( x       .begin() );
    float const * LE_RESTRICT pY   ( y       .begin() );
    LE_DISABLE_LOOP_UNROLLING()
    while ( binsLeft )
    {
        shapeWidth = std::min( binsLeft, shapeWidth );
        /// \note MSVC10 fails to merge two std::acumulate() calls into one loop
        /// so we perform accumulation manually.
        ///                                   (29.11.2012.) (Domagoj Saric)
        float X( std::numeric_limits<float>::epsilon() );
        float Y( std::numeric_limits<float>::epsilon() );
        auto blockCounter( shapeWidth );
        while ( blockCounter-- )
        {
            X += *pX++;
            Y += *pY++;
        }
        float const colour( X / Y );

        Math::multiply( pAmps, colour, shapeWidth );
        pAmps    += shapeWidth;
        binsLeft -= shapeWidth;
    }
}

LE_OPTIMIZE_FOR_SPEED_END()

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
