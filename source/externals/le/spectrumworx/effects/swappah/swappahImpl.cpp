////////////////////////////////////////////////////////////////////////////////
///
/// swappahImpl.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "swappahImpl.hpp"

#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include <array>
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
// Swappah static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Swappah::title      [] = "Swappah";
char const Swappah::description[] = "Swaps three spectral bands.";


////////////////////////////////////////////////////////////////////////////////
//
// Swappah UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Swappah::BandLowMid , "Low-Mid border"  )
EFFECT_PARAMETER_NAME( Swappah::BandMidHigh, "Mid-High border" )
EFFECT_PARAMETER_NAME( Swappah::BandOrder  , "Swap order"      )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Swappah, BandOrder,
    (( LowHighMid, "Low-High-Mid" ))
    (( MidLowHigh, "Mid-Low-High" ))
    (( MidHighLow, "Mid-High-Low" ))
    (( HighLowMid, "High-Low-Mid" ))
    (( HighMidLow, "High-Mid-Low" ))
)


////////////////////////////////////////////////////////////////////////////////
//
// SwappahImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void SwappahImpl::setup( IndexRange const & workingRange, Engine::Setup const & )
{
    std::uint16_t const maxBinIndex( workingRange.size() - 1 );
    band1_ = maxBinIndex * parameters().get<BandLowMid >() / 100;
    band2_ = maxBinIndex * parameters().get<BandMidHigh>() / 100;

    if ( band1_ > band2_ )
        std::swap( band1_, band2_ );

    mode_.unpack( parameters().get<Mode>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// SwappahImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void SwappahImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    if ( mode_.magnitudes() ) swapBands( data.amps  () );
    if ( mode_.phases    () ) swapBands( data.phases() );
}


////////////////////////////////////////////////////////////////////////////////
//
// SwappahImpl::swapBands()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

void SwappahImpl::swapBands( DataRange const & data ) const
{
    using namespace Math;

    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( swapBuffer, Engine::real_t, data.size() );
    copy( data, swapBuffer );

    std::uint16_t const numBins( static_cast<std::uint16_t>( data.size() ) );
    std::uint16_t const lStart(      0 );
    std::uint16_t const mStart( band1_ );
    std::uint16_t const hStart( band2_ );
    std::uint16_t const lSize ( mStart  - lStart );
    std::uint16_t const mSize ( hStart  - mStart );
    std::uint16_t const hSize ( numBins - hStart );

    struct Band
    {
        std::uint16_t source;
        std::uint16_t size  ;
    };
    std::array<Band, 3> bands;

    switch ( parameters().get<BandOrder>().getValue() )
    {
        case BandOrder::LowHighMid: // LHM 132
            bands[ 0 ].source = lStart; bands[ 0 ].size = lSize;
            bands[ 1 ].source = hStart; bands[ 1 ].size = hSize;
            bands[ 2 ].source = mStart; bands[ 2 ].size = mSize;
            break;
        case BandOrder::MidLowHigh: // MLH 213
            bands[ 0 ].source = mStart; bands[ 0 ].size = mSize;
            bands[ 1 ].source = lStart; bands[ 1 ].size = lSize;
            bands[ 2 ].source = hStart; bands[ 2 ].size = hSize;
            break;
        case BandOrder::MidHighLow: // MHL 231
            bands[ 0 ].source = mStart; bands[ 0 ].size = mSize;
            bands[ 1 ].source = hStart; bands[ 1 ].size = hSize;
            bands[ 2 ].source = lStart; bands[ 2 ].size = lSize;
            break;
        case BandOrder::HighLowMid: // HLM 312
            bands[ 0 ].source = hStart; bands[ 0 ].size = hSize;
            bands[ 1 ].source = lStart; bands[ 1 ].size = lSize;
            bands[ 2 ].source = mStart; bands[ 2 ].size = mSize;
            break;
        case BandOrder::HighMidLow: // HML 321
            bands[ 0 ].source = hStart; bands[ 0 ].size = hSize;
            bands[ 1 ].source = mStart; bands[ 1 ].size = mSize;
            bands[ 2 ].source = lStart; bands[ 2 ].size = lSize;
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    std::uint16_t target( 0 );
    LE_DISABLE_LOOP_UNROLLING()
    for ( auto const & band : bands )
    {
        if ( band.source != target )
        {
            BOOST_ASSERT( band.source <  unsigned( swapBuffer.size() ) );
            BOOST_ASSERT( target      <= unsigned( data      .size() ) );
            copy( swapBuffer.begin() + band.source, data.begin() + target, band.size );
        }
        target += band.size;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
