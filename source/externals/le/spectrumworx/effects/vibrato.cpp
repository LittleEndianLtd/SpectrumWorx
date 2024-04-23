////////////////////////////////////////////////////////////////////////////////
///
/// vibrato.cpp
/// -----------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "vibrato.hpp"

#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"

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

void VibratoEffect::setup( unsigned int const & periodInMilliseconds, Engine::Setup const & engineSetup )
{
    period_ = engineSetup.milliSecondsToSteps( periodInMilliseconds );
}

float VibratoEffect::calculateNewPitch( ChannelState & cs, CommonParameters::SpringType const springType, unsigned int const vibratoDepthInSemitones ) const
{
    using namespace Math::Constants;
    using Math::convert;

    unsigned int const period          ( period_                                                        );
    float        const frame           ( convert<float>( cs.frameCounter.nextValueFor( period ).first ) );
    float        const currentAmplitude( std::sin( twoPi * frame / convert<float>( period ) )           );

    float offsetAmplitude;

    switch ( springType.getValue() )
    {
        case CommonParameters::SpringType::Symmetric: offsetAmplitude =         currentAmplitude      ; break;
        case CommonParameters::SpringType::Up       : offsetAmplitude =   ( 1 + currentAmplitude ) / 2; break;
        case CommonParameters::SpringType::Down     : offsetAmplitude = - ( 1 + currentAmplitude ) / 2; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }

    float const scaledAmplitude( convert<float>( vibratoDepthInSemitones ) * offsetAmplitude );

    float const pitchScale( Math::cents2Interval12TET( scaledAmplitude ) );

    return pitchScale;
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
