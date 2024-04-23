////////////////////////////////////////////////////////////////////////////////
///
/// ahAhImpl.cpp
/// ------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "ahAhImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/parameters/uiElements.hpp"

#include <algorithm>
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
// AhAh static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const AhAh::title      [] = "Ah-ah";
char const AhAh::description[] = "Wah ah ah...";


////////////////////////////////////////////////////////////////////////////////
//
// AhAh UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( AhAh::Center  , "Center (LFO me!)" )
EFFECT_PARAMETER_NAME( AhAh::Width   , "Width"            )
EFFECT_PARAMETER_NAME( AhAh::Strength, "Strength"         )


////////////////////////////////////////////////////////////////////////////////
//
// AhAhImpl::setup()
// -----------------
//
////////////////////////////////////////////////////////////////////////////////

void AhAhImpl::setup( IndexRange const & indexRange, Engine::Setup const & engineSetup )
{
    using namespace Math;

    gain_ = dB2NormalisedLinear( parameters().get<Strength>() );

    auto const centre( engineSetup.frequencyInHzToBin( parameters().get<Center>() ) );
    auto const width ( engineSetup.frequencyInHzToBin( parameters().get<Width >() ) );

    auto const halfWidth( width / 2 );

    // Implementation note:
    //   The angular frequency is calculated so that we get the first half
    // period of a sine across the width of the filter.
    //                                        (21.10.2011.) (Domagoj Saric)
    using namespace Math::Constants;
    omega_ = pi / convert<float>( halfWidth * 2 );
    BOOST_ASSERT( omega_ == twoPi / 2.0f / convert<float>( halfWidth * 2 ) );

    IndexRange::signed_value_type userBeginBin( centre - halfWidth     );
    IndexRange::       value_type userEndBin  ( centre + halfWidth + 1 );

    beginBin_ = std::max<IndexRange::signed_value_type>( userBeginBin, indexRange.begin() );
    endBin_   = std::min                               ( userEndBin  , indexRange.end  () );
    endBin_   = std::max                               ( endBin_     , beginBin_          );

    offsetFromUserRange_ = convert<float>( beginBin_ - userBeginBin );
}


////////////////////////////////////////////////////////////////////////////////
//
// AhAhImpl::process()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void AhAhImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    DataRange const workingRange( Engine::subRange( data.full().amps(), beginBin_, endBin_ ) );

    float const omega( omega_ );
    float const gain ( gain_  );

    float i( offsetFromUserRange_ );
    for ( auto & amp : workingRange )
    {
        // http://courses.engr.illinois.edu/ece420/handouts/audio.pdf
        float const sine( std::sin( omega * i++ ) );
        BOOST_ASSERT( sine >= -1E-6 );
        float const currentGain( ( ( gain - 1 ) * sine ) + 1 );
        amp *= currentGain;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
