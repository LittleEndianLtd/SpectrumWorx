////////////////////////////////////////////////////////////////////////////////
///
/// musicalScales.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "musicalScales.hpp"

#include "le/math/conversion.hpp"
#include "le/math/constants.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"

#include "boost/assert.hpp"

#include <array>
#include <cstring>
#ifndef LE_MELODIFY_SDK_BUILD
#include <numeric>
#endif //LE_MELODIFY_SDK_BUILD 
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Music
{
//------------------------------------------------------------------------------

Scale::Scale()
{
#ifndef LE_MELODIFY_SDK_BUILD
    lastPitchScale_             =  1;
    centerTone_                 = -1;
    targetPitchChangeDirection_ =  0;
#endif // LE_MELODIFY_SDK_BUILD
}


void Scale::tonesUpdated( std::uint8_t const snappedTo, std::uint8_t const bypassed )
{
#ifndef NDEBUG
	auto const n( snappedTo + bypassed );
    BOOST_ASSERT( unsigned( n ) <= toneOffsets_.size() );
    BOOST_ASSERT( ( *std::max_element( toneOffsets_.begin(), toneOffsets_.begin() + n ) < 12 ) || ( n == 0 ) );
#endif // NDEBUG

    numberOfTones_    = snappedTo;
#ifdef LE_SW_SDK_BUILD
	numberOfBypassed_ = bypassed;
#else
    LE_ASSUME( bypassed == 0 );
#endif // LE_SW_SDK_BUILD

#if 0
    if ( snappedTo == 0 )
    {
        centerTone_                 = -1;
        targetPitchChangeDirection_ =  0;
    }
    else
#endif // 0
#ifndef LE_MELODIFY_SDK_BUILD
    {
        if ( snappedTo )
        {
            float const newCenterTone( std::accumulate( toneOffsets_.begin(), toneOffsets_.begin() + snappedTo, 0 ) / Math::convert<float>( snappedTo ) );
            LE_ASSUME( newCenterTone >= 0 );
            if ( newCenterTone != centerTone_ )
            {
                     if ( centerTone_   == -1          ) targetPitchChangeDirection_ =  0;
                else if ( newCenterTone >  centerTone_ ) targetPitchChangeDirection_ = +1;
                else if ( newCenterTone <  centerTone_ ) targetPitchChangeDirection_ = -1;
                centerTone_ = newCenterTone;
            }
        }
        else
        {
            centerTone_                 = -1;
            targetPitchChangeDirection_ =  0;
        }
    }
#endif // LE_SW_SDK_BUILD
}


LE_OPTIMIZE_FOR_SPEED_BEGIN()

float LE_HOT Scale::snap2Scale( float const freq, std::uint8_t const keyIndex ) const
{
	using namespace Math;

    struct PitchScaleRatio
    {
        float ratio   ;
        bool  inverted;
        bool operator<( PitchScaleRatio const & other ) const { return ratio < other.ratio; }
    };

#ifdef LE_MELODIFY_SDK_BUILD
    float const pitchScaleComparisonSource( 1 );
#else
    float const pitchScaleComparisonSource( lastPitchScale_ );
#endif // LE_MELODIFY_SDK_BUILD

    std::uint8_t const totalTones( numberOfTones() + numberOfBypassed() );
    LE_ASSUME( totalTones < 12 );
	std::array<PitchScaleRatio, 12> pitchScaleDeltas;
	for ( std::uint8_t n( 0 ); n < totalTones; ++n )
	{
        ToneOffsets::value_type const noteOffset( toneOffset( n ) );
        //...mrmlj...quick-doc: transform A-based scale to (lowest) C-based scale
		// move index in range [-9,2] for calculation of base frequency of current note (frequency in zeroth octave)
		int const noteBaseIndex( keyIndex + noteOffset - ( ( keyIndex + noteOffset + 9 ) / 12 ) * 12 );

		// note base frequency
		float const noteBaseFreq( 27.5f * semitone2Interval12TET( static_cast<float>( noteBaseIndex ) ) );

		// ratio between discovered pitch and note base frequency
		float const freqRatio( freq / noteBaseFreq );

		//...mrmlj...quick-doc: check two neighbouring octaves for the actual closest harmonic
		// closest note is note base frequency multiplied by power of 2 closest to frequency ratio
        std::uint8_t const lowerOctaveExponent( PositiveFloats::floor( Math::log2( freqRatio ) ) );
        float        const lowerOctaveRatio   ( convert<float>( 1 << lowerOctaveExponent ) ); LE_ASSUME( lowerOctaveRatio <= freqRatio );
        float        const lowerOctavePitch   ( lowerOctaveRatio * noteBaseFreq );
        float        const lowerPitchScale    ( lowerOctavePitch / freq         );
        float        const upperPitchScale    ( lowerPitchScale * 2             );
        float        const lowerRatio         ( pitchScaleComparisonSource / lowerPitchScale            );
        float        const upperRatio         ( upperPitchScale            / pitchScaleComparisonSource );
        if
        (
        #ifndef LE_MELODIFY_SDK_BUILD
            ( targetPitchChangeDirection_ > 0 ) ||
        #endif // LE_MELODIFY_SDK_BUILD
            ( upperRatio < lowerRatio )
        )
        {
            pitchScaleDeltas[ n ].ratio    = upperRatio;
            pitchScaleDeltas[ n ].inverted = false;
        }
        else
        {
            pitchScaleDeltas[ n ].ratio    = lowerRatio;
            pitchScaleDeltas[ n ].inverted = true;
        }
	}

	// find the note with minimum distance and return closest frequency
    auto const & minimumRatio( *std::min_element( pitchScaleDeltas.begin(), pitchScaleDeltas.begin() + totalTones ) );

#ifdef LE_SW_SDK_BUILD
    auto const nearestScaleToneIndex( static_cast<std::uint8_t>( &minimumRatio - &pitchScaleDeltas.front() ) );
    if ( nearestScaleToneIndex >= numberOfTones() )
    { // bypassed tone:
    #ifndef LE_MELODIFY_SDK_BUILD
        lastPitchScale_ = 1;
    #endif // LE_MELODIFY_SDK_BUILD
        return freq;
    }
#endif // LE_SW_SDK_BUILD

    float const newPitchScale
    (
        minimumRatio.inverted
            ? pitchScaleComparisonSource / minimumRatio.ratio
            : pitchScaleComparisonSource * minimumRatio.ratio
    );

#ifndef LE_MELODIFY_SDK_BUILD
    lastPitchScale_ = newPitchScale;
#endif // LE_MELODIFY_SDK_BUILD
    return newPitchScale * freq;
}

LE_OPTIMIZE_FOR_SPEED_END()


Scale::ToneOffsets::value_type Scale::toneOffset( std::uint8_t const i ) const
{
    BOOST_ASSERT( i < numberOfTones() + numberOfBypassed() );
    return toneOffsets_[ i ];
}

//------------------------------------------------------------------------------
} // namespace Music
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
