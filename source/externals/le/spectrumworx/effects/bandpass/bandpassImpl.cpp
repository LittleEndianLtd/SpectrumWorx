////////////////////////////////////////////////////////////////////////////////
///
/// bandpassImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "bandpassImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/math/windows.hpp"
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
// BandGain UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Detail::BandGain::Attenuation, "Attenuation" )


////////////////////////////////////////////////////////////////////////////////
//
// Bandpass static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Bandpass::title      [] = "Bandpass";
char const Bandpass::description[] = "Band-pass filter.";


////////////////////////////////////////////////////////////////////////////////
//
// Bandstop static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Bandstop::title      [] = "Bandstop";
char const Bandstop::description[] = "Band-stop filter.";


////////////////////////////////////////////////////////////////////////////////
//
// Detail::BandGainImpl::setup()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////

void Detail::BandGainImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    using namespace Math;
    float const attenuation( dB2NormalisedLinear( - parameters().get<Attenuation>() ) );

    #ifdef LE_BAND_FILTER_USE_ENGINE_WINDOW
        // http://www.kemt.fei.tuke.sk/Predmety/KEMT421_DF/_materialy/Lectures/df_lesson_09.pdf
        // http://www.nicholson.com/rhn/dsp.html
        // https://ccrma.stanford.edu/~jos/sasp/Window_Method.html
        // https://ccrma.stanford.edu/~jos/sasp/Example_1_Low_Pass_Filtering.html
        // http://en.wikipedia.org/wiki/Window_function
        // http://www.mathworks.com/company/newsletters/news_notes/win00/filtering.html

        unsigned int const windowSize( engineSetup.numberOfBins() / 30 );

        unsigned int const slopeSize( windowSize / 2 );
        unsigned int const alignment( Utility::Constants::vectorAlignment / sizeof( float ) );

        unsigned int const leftAlignmentOffset ( ( workingRange.begin() - slopeSize ) % alignment );
        unsigned int const rightAlignmentOffset(   workingRange.end  ()               % alignment );

        unsigned int const currentWindowSize    ( static_cast<unsigned int>( downSlope_.size() ) * 2                                );
        unsigned int const currentLeftAlignment ( reinterpret_cast<std::size_t>( downSlope_.begin() ) / sizeof( float ) % alignment );
        unsigned int const currentRightAlignment( reinterpret_cast<std::size_t>( pUpSlope_          ) / sizeof( float ) % alignment );
        if
        (
            ( windowSize           != currentWindowSize     ) ||
            ( leftAlignmentOffset  != currentLeftAlignment  ) ||
            ( rightAlignmentOffset != currentRightAlignment ) ||
            ( attenuation_         != attenuation           )
        )
        {
            calculateWindow( DataRange( &window_[ 0 ], &window_[ windowSize ] ), engineSetup.windowFunction() );
            // Transform the window from the [0, 1] to [1, attenuation] range.
            multiply( window_.begin(), attenuation - 1, windowSize );
            add( window_.begin(), 1.0f, window_.begin(), windowSize );

            pUpSlope_ = window_.begin() + ( ( slopeSize + leftAlignmentOffset + alignment - 1 ) & ~( alignment - 1 ) ) + rightAlignmentOffset;
            move( window_.begin() + slopeSize, pUpSlope_, slopeSize );

            downSlope_ = DataRange( &window_[ leftAlignmentOffset ], &window_[ slopeSize + leftAlignmentOffset ] );
            move( window_.begin(), downSlope_.begin(), slopeSize );

            BOOST_ASSERT( downSlope_.end() <= pUpSlope_ );
        }
    #else
        (void)workingRange; (void)engineSetup;//...mrmlj...
    #endif // LE_BAND_FILTER_USE_ENGINE_WINDOW

    attenuation_ = attenuation;
}


////////////////////////////////////////////////////////////////////////////////
//
// BandpassImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void BandpassImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    using namespace Math;
#ifdef LE_BAND_FILTER_USE_ENGINE_WINDOW
    unsigned int const slopeSize( static_cast<unsigned int>( downSlope_.size() ) );

    unsigned int leftSize ( std::min<unsigned int>( data.beginBin()                           , slopeSize ) );
    unsigned int rightSize( std::min<unsigned int>( data.full().numberOfBins() - data.endBin(), slopeSize ) );

    float const * pWindowUpSlope( &pUpSlope_ [ slopeSize - leftSize ] );
    float       * pDataUpSlope  ( data.amps().begin() - leftSize      );

    float const * pWindowDownSlope( &downSlope_[ 0 ]  );
    float       * pDataDownSlope  ( data.amps().end() );

    multiply( attenuation_, data.full().amps().begin(), pDataUpSlope             );
    multiply( attenuation_, pDataDownSlope + rightSize, data.full().amps().end() );

    //...mrmlj...different alignment required for bandpass and bandstop...
    //multiply( pWindowUpSlope  , pDataUpSlope  , leftSize  );
    //multiply( pWindowDownSlope, pDataDownSlope, rightSize );
    
    while ( leftSize -- ) { *pDataUpSlope  ++ *= *pWindowUpSlope  ++; }
    while ( rightSize-- ) { *pDataDownSlope++ *= *pWindowDownSlope++; }
#else
    multiply( attenuation_, data.full().amps().begin(), data       .amps().begin() );
    multiply( attenuation_, data       .amps().end  (), data.full().amps().end  () );
#endif // LE_BAND_FILTER_USE_ENGINE_WINDOW
}


////////////////////////////////////////////////////////////////////////////////
//
// BandstopImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void BandstopImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
#ifdef LE_BAND_FILTER_USE_ENGINE_WINDOW
    unsigned int const slopeSize( static_cast<unsigned int>( downSlope_.size() ) );

    unsigned int const leftSize( std::min<unsigned int>( data.beginBin(), slopeSize ) );
    Math::multiply( downSlope_.end() - leftSize, data.amps().begin() - leftSize, leftSize );
#endif // LE_BAND_FILTER_USE_ENGINE_WINDOW

    Math::multiply( data.amps(), attenuation_ );

#ifdef LE_BAND_FILTER_USE_ENGINE_WINDOW
    unsigned int const rightSize( std::min<unsigned int>( data.full().numberOfBins() - data.endBin(), slopeSize ) );
    Math::multiply( pUpSlope_, data.amps().end(), rightSize );
#endif // LE_BAND_FILTER_USE_ENGINE_WINDOW
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
