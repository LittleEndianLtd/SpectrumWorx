////////////////////////////////////////////////////////////////////////////////
///
/// centroidExtractorImpl.cpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "centroidExtractorImpl.hpp"

#include "le/analysis/peak_detector/peakDetector.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"

#include <utility>
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
// CentroidExtractor static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const CentroidExtractor::title      [] = "Centroid";
char const CentroidExtractor::description[] = "Adaptive bandpass filter.";


////////////////////////////////////////////////////////////////////////////////
//
// CentroidExtractor UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( CentroidExtractor::Mode       , "Filter center" )
EFFECT_PARAMETER_NAME( CentroidExtractor::Bandwidth  , "Bandwidth"     )
EFFECT_PARAMETER_NAME( CentroidExtractor::Attenuation, "Border slope"  )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    CentroidExtractor, Mode,
    (( Centroid, "Centroid" ))
    (( Peak    , "Peak"     ))
    (( Dominant, "Dominant" ))
)


////////////////////////////////////////////////////////////////////////////////
//
// CentroidExtractorImpl::setup()
// ------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void CentroidExtractorImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    /// \note The actual bandwidth is clamped to the working range inside the
    /// process() member function.
    ///                                       (27.09.2012.) (Domagoj Saric)
    bandwidth_     = engineSetup.frequencyInHzToBin(   parameters().get<Bandwidth  >() );
    amplification_ = Math::dB2NormalisedLinear     ( - parameters().get<Attenuation>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// CentroidExtractorImpl::process()
// --------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void CentroidExtractorImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & setup ) const
{
    auto const bandwidth( bandwidth_ );

    if ( bandwidth == 0 )
        return;

    IndexRange::value_type desiredCentralBin;
    switch ( parameters().get<Mode>().getValue() )
    {
        case Mode::Centroid: desiredCentralBin = centroid( data.full().amps()            ); break;
        case Mode::Peak    : desiredCentralBin = maxPeak ( data.full().amps(), setup     ); break;
        case Mode::Dominant: desiredCentralBin = dominant( data.full().amps(), setup, cs ); break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    using namespace Math;

    IndexRange::signed_value_type const desiredLeftBin( desiredCentralBin - bandwidth );
    IndexRange::       value_type const desiredRightBin( desiredCentralBin + bandwidth );

    BOOST_ASSERT_MSG( ( desiredCentralBin - desiredLeftBin    + 1 ) > 0, "Assumed bin order breached." ); //...mrmlj...add a better message...
    BOOST_ASSERT_MSG( ( desiredRightBin   - desiredCentralBin + 1 ) > 0, "Assumed bin order breached." );

    auto const dataBeginBin( data.beginBin() );
    auto const dataEndBin  ( data.endBin  () );

    auto const lBin      ( clamp( desiredLeftBin   , dataBeginBin, dataEndBin ) );
    auto const rBin      ( clamp( desiredRightBin  , dataBeginBin, dataEndBin ) );
    auto const centralBin( clamp( desiredCentralBin, dataBeginBin, dataEndBin ) );

    // Remove what's outside the bandwith range:
    clear(  data.amps().begin()               , &data.full().amps()[ lBin ] );
    clear( &data.full().amps()[ rBin - 1 ] + 1,  data.amps().end()          );

    // Filter the rest:
    float const amplification ( amplification_                                                             );
    float const slopeIncrement( ( 1 - amplification ) / convert<float>( bandwidth + 1 )                    );
    float const slopeOffset   ( std::max( 0.0f, convert<float>( lBin - desiredLeftBin ) ) * slopeIncrement ); // Clang calls the vector version if just 0 is used...
    float const slopeStart    ( amplification + slopeOffset                                                );

    float slope( slopeStart );

    for ( auto & amp : Engine::subRange( data.full().amps(), lBin, centralBin ) )
    {
        amp   *= slope         ;
        slope += slopeIncrement;
    }

    BOOST_ASSERT( nearEqual( slope, 1 - slopeIncrement ) || ( centralBin < desiredCentralBin ) || ( centralBin == lBin && slope == slopeStart ) );
    slope = 1;

    for ( auto & amp : Engine::subRange( data.full().amps(), centralBin, rBin ) )
    {
        amp   *= slope         ;
        slope -= slopeIncrement;
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// CentroidExtractorImpl::centroid()
// ---------------------------------
//
// http://en.wikipedia.org/wiki/Spectral_centroid
//
////////////////////////////////////////////////////////////////////////////////

IndexRange::value_type CentroidExtractorImpl::centroid( ReadOnlyDataRange amplitudes ) const
{
    float binmag( 0 );
    float mag   ( std::numeric_limits<float>::epsilon() );

    float binCounter( 0 );
    while ( amplitudes )
    {
        float const amp( amplitudes.front() );
        binmag += binCounter++ * amp;
        mag    +=                amp;
        amplitudes.advance_begin( 1 );
    }

    float const centroid( binmag / mag );
    return Math::convert<IndexRange::value_type>( centroid );
}


////////////////////////////////////////////////////////////////////////////////
//
// CentroidExtractorImpl::dominant()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////

IndexRange::value_type CentroidExtractorImpl::dominant( ReadOnlyDataRange const & amplitudes, Engine::Setup const & engineSetup, ChannelState & cs ) const
{
    using namespace Math;
    float const c  ( PitchDetector::findPitch( amplitudes, cs, 50, 50*2*2*2*2*2, engineSetup ) );
    float const bin( c * convert<float>( amplitudes.size() ) / engineSetup.sampleRate<float>() );
    return convert<IndexRange::value_type>( bin );
}


////////////////////////////////////////////////////////////////////////////////
//
// CentroidExtractorImpl::maxPeak()
// --------------------------------
//
////////////////////////////////////////////////////////////////////////////////

IndexRange::value_type CentroidExtractorImpl::maxPeak( ReadOnlyDataRange const & amplitudes, Engine::Setup const & engineSetup ) const
{
    // Find and sort peaks...
    PeakDetector pd;
    pd.setZeroDecibelValue     ( engineSetup.maximumAmplitude()                                     );
    pd.setStrengthThreshold    ( 0                                                                  );
    pd.findPeaksAndStrengthSort( amplitudes.begin(), static_cast<std::uint16_t>( amplitudes.size() ) );

    // ...and get strongest peak which is the first one:
    return pd.getPeak( 0 )->maxPos;
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
