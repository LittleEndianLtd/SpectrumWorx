////////////////////////////////////////////////////////////////////////////////
///
/// \file vector.hpp
/// ----------------
///
/// \brief Common operations for vectorized data. Mostly SIMD optimized.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef vector_hpp__F5D72A06_E9E6_42D9_A4DB_4E7917819F41
#define vector_hpp__F5D72A06_E9E6_42D9_A4DB_4E7917819F41
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include <boost/range/iterator_range_core.hpp>

#include <cstdint>
#include <limits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

unsigned int alignIndex( unsigned int index   );
void *       align     ( void *       pointer );


////////////////////////////////////////////////////////////////////////////////
/// Range based interfaces (mostly unimplemented).
////////////////////////////////////////////////////////////////////////////////

using InputRange       = boost::iterator_range<float const * LE_RESTRICT>;
using OutputRange      = boost::iterator_range<float       * LE_RESTRICT>;
using InputOutputRange = OutputRange;


void copy( InputRange const &, OutputRange const & );

void clear    ( InputOutputRange );
void fill     ( InputOutputRange, float value );
void negate   ( InputOutputRange );
void negate   ( InputOutputRange, unsigned int stride );
void randomize( InputOutputRange );
void reverse  ( InputOutputRange );
void swap     ( InputOutputRange const &, InputOutputRange const & );

float const & min( InputRange const & );
float const & max( InputRange const & );

void add( InputRange const &, InputOutputRange const & );
void add( InputRange const &, float constant, OutputRange const & );

void multiply( InputRange       const &, InputRange       const &, OutputRange );
void multiply( InputRange       const &, InputOutputRange const &              );
void multiply( InputRange       const &, float multiplier        , OutputRange );
void multiply( InputOutputRange const &, float multiplier                      );

void addProduct( InputRange, InputRange, InputOutputRange );

void cosine( InputRange, OutputRange );
void sine  ( InputRange, OutputRange );
void sinCos( InputRange, OutputRange, OutputRange );

void amplitudes( InputRange reals, InputRange imags, OutputRange amplitudes );
void phases    ( InputRange reals, InputRange imags, OutputRange phases     );

void ln ( InputRange const &, OutputRange const & );
void ln ( InputOutputRange const & );
void exp( InputOutputRange const & );

void square    ( InputOutputRange );
void squareRoot( InputOutputRange );

float rms( InputRange const & );

void mix( InputRange const & amps, InputRange const & phases, InputOutputRange const & reals, InputOutputRange const & imags, float amPhWeight               );
void mix( InputRange const & amps, InputRange const & phases, InputOutputRange const & reals, InputOutputRange const & imags, float amPhGain, float reImGain );

void movingAverage         ( InputOutputRange const &             , unsigned int windowWidth );
void symmetricMovingAverage( InputRange       const &, OutputRange, unsigned int windowWidth );


////////////////////////////////////////////////////////////////////////////////
/// Iterator interfaces.
////////////////////////////////////////////////////////////////////////////////

void copy( float const * pBegin, float const * pBeginEnd, float * pDestination );

void clear  ( float * pBegin, float const * pEnd );
void fill   ( float * pBegin, float const * pEnd, float value );
void negate ( float * pBegin, float const * pEnd );
void reverse( float * pBegin, float const * pEnd );
void swap   ( float * pBegin, float const * pEnd, float * pDestination );

float const & LE_FASTCALL_ABI min( float const * pBegin, float const * pEnd );
float const & LE_FASTCALL_ABI max( float const * pBegin, float const * pEnd );

void add( float const * pInput,                 float * pInputOutput, float const * pOutputEnd );
void add( float const * pInput, float constant, float * pOutput     , float const * pOutputEnd );

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI multiply( float const * pFirstArray , float const * pSecondArray, float * pOutput, float const * pOutputEnd );
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI multiply( float const * pInput      , float * pInputOutput         , float const * pOutputEnd );
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI multiply( float scalar, float const * pInput      , float * pOutput, float const * pOutputEnd );
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI multiply( float scalar, float       * pInputOutput                 , float const * pOutputEnd );

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI addProduct( float const * pInput1, float const * pInput2, float * pInput3AndOutput, float const * pOutputEnd );

void sine  ( float const * pInput, float * pOutput, float const * pOutputEnd );
void cosine( float const * pInput, float * pOutput, float const * pOutputEnd );
void sinCos( float const * pInput, float const * pInputEnd, float * pSines, float * pCosines );

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI amplitudes( float const * pReals, float const * pImags, float * pAmplitudes, float const * pAmplitudesEnd );
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI phases    ( float const * pReals, float const * pImags, float * pPhases    , float const * pPhasesEnd     );

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI polar2rectangular( float const * pAmplitudes, float const * pPhases, float * pReals     , float * pImags , float const * pImagsEnd  );
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI rectangular2polar( float const * pReals     , float const * pImags , float * pAmplitudes, float * pPhases, float const * pPhasesEnd );

void ln ( float const * pInput, float * pOutput, float const * pOutputEnd );
void ln ( float * pInputOutput, float const * pOutputEnd );
void exp( float * pInputOutput, float const * pOutputEnd );

void square    ( float * pInputOutput, float const * pOutputEnd );
void squareRoot( float * pInputOutput, float const * pOutputEnd );

float rms( float const * pData, float const * pDataEnd );

void mix( float const * pInput1, float const * pInput2, float * pOutput, float const * pOutputEnd, float input1Weight                     );
void mix( float const * pInput1, float const * pInput2, float * pOutput, float const * pOutputEnd, float input1Weight, float input2Weight );


////////////////////////////////////////////////////////////////////////////////
/// "Pointers + size" based interfaces.
////////////////////////////////////////////////////////////////////////////////

void LE_FASTCALL_ABI copy( float const * pInput, float * pOutput, unsigned int numberOfElements );
void LE_FASTCALL_ABI move( float const * pInput, float * pOutput, unsigned int numberOfElements );

void LE_FASTCALL_ABI clear    ( float * pArray                     , unsigned int numberOfElements );
void LE_FASTCALL_ABI fill     ( float * pArray, float value        , unsigned int numberOfElements );
void LE_FASTCALL_ABI negate   ( float * pArray                     , unsigned int numberOfElements );
void LE_FASTCALL_ABI negate   ( float * pArray, unsigned int stride, unsigned int numberOfElements );
void LE_FASTCALL_ABI randomize( float * pArray                     , unsigned int numberOfElements );
void LE_FASTCALL_ABI reverse  ( float * pArray                     , unsigned int numberOfElements );
void LE_FASTCALL_ABI swap     ( float * pFirstArray, float * pSecondArray, unsigned int numberOfElements );

float const & LE_FASTCALL_ABI min( float const * pArray, unsigned int numberOfElements );
float const & LE_FASTCALL_ABI max( float const * pArray, unsigned int numberOfElements );

void add( float const * pInput, float * pInputOutput, unsigned int numberOfElements );
void add( float const * pInput, float constant, float * pOutput, unsigned int numberOfElements );

void multiply( float const * pFirstArray, float const * pSecondArray, float * pOutput     , unsigned int numberOfElements );
void multiply( float const * pInput     ,                             float * pInputOutput, unsigned int numberOfElements );
void multiply( float const * pInput, float scalar, float * pOutput, unsigned int numberOfElements );
void multiply( float * pInputOutput, float scalar                 , unsigned int numberOfElements );

void addProduct( float const * pInput1, float const * pInput2, float * pInput3AndOutput, unsigned int numberOfElements );

void sine  ( float const * pInput, float * pOutput                 , unsigned int numberOfElements );
void cosine( float const * pInput, float * pOutput                 , unsigned int numberOfElements );
void sinCos( float const * pInput, float * pSines, float * pCosines, unsigned int numberOfElements );

void LE_FASTCALL_ABI amplitudes( float const * LE_RESTRICT pReals, float const * LE_RESTRICT pImags, float * LE_RESTRICT pAmplitudes, std::uint16_t numberOfElements );
void LE_FASTCALL_ABI phases    ( float const * LE_RESTRICT pReals, float const * LE_RESTRICT pImags, float * LE_RESTRICT pPhases    , std::uint16_t numberOfElements );

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI polar2rectangular( float const * pAmplitudes, float const * pPhases, float * pReals, float * pImags, std::uint16_t numberOfElements );
LE_NOTHROWNOALIAS void LE_FASTCALL_ABI rectangular2polar( float const * pReals, float const * pImags, float * pAmplitudes, float * pPhases, std::uint16_t numberOfElements );

void ln ( float const * pInput, float * pOutput, unsigned int numberOfElements );
void ln ( float * pInputOutput, unsigned int numberOfElements );
void exp( float * pInputOutput, unsigned int numberOfElements );

void square    ( float * pInputOutput, unsigned int numberOfElements );
void squareRoot( float * pInputOutput, unsigned int numberOfElements );

float rms( float const * pData, unsigned int numberOfElements );

void mix( float const * pInput1, float const * pInput2, float * pOutput, float input1Weight, unsigned int numberOfElements );

void LE_FASTCALL_ABI interleave  ( float const * LE_RESTRICT const * LE_RESTRICT pInputs, float * LE_RESTRICT                     pOutput , std::uint16_t numberOfElements, std::uint8_t numberOfChannels );
void LE_FASTCALL_ABI deinterleave( float const * LE_RESTRICT                     pInput , float * LE_RESTRICT const * LE_RESTRICT pOutputs, std::uint16_t numberOfElements, std::uint8_t numberOfChannels );


/// \note These may be used in both AudioIO and SW and so have to be in the
/// header (until LE.Math is separated into a shared library).
///                                           (05.02.2016.) (Domagoj Saric)
LE_OPTIMIZE_FOR_SPEED_BEGIN()

// http://blog.frama-c.com/index.php?post/2013/10/09/Overflow-float-integer
// http://stackoverflow.com/questions/9832430/arm-saturate-signed-int-to-unsigned-byte
// http://stackoverflow.com/questions/24546927/behavior-of-arm-neon-float-integer-conversion-with-overflow
LE_FORCEINLINE LE_HOT LE_NOTHROWNOALIAS void LE_FASTCALL
convertSample( float const LE_GNU_SPECIFIC( & __restrict ) input, std::int16_t & LE_GNU_SPECIFIC( __restrict ) output )
{
#if 0 // slower & not autovectorized
    static auto BOOST_CONSTEXPR_OR_CONST scale( static_cast<float>( std::numeric_limits<std::int16_t>::max() ) );
    output = Math::convert<std::int16_t>( Math::clamp( input, -1.0f, +1.0f ) * scale );
#else
    static auto BOOST_CONSTEXPR_OR_CONST scale( static_cast<float>( std::numeric_limits<std::int32_t>::max() ) );
    /// \note We can use a plain static_cast/truncation here because the one bit
    /// bit of precision lost here does not matter (we discard all of the lower
    /// 16 bits anyway + floats have only 24 bits of precision to begin with).
    ///                                       (19.02.2016.) (Domagoj Saric)
    output = static_cast<std::int16_t>( static_cast<std::int32_t>( input * scale ) >> 16 );
#endif
}

LE_FORCEINLINE LE_HOT LE_NOTHROWNOALIAS void LE_FASTCALL
convertSample( std::int16_t const LE_GNU_SPECIFIC( & __restrict ) input, float & LE_GNU_SPECIFIC( __restrict ) output )
{
    static float BOOST_CONSTEXPR_OR_CONST scale( -1.0f / std::numeric_limits<std::int16_t>::min() );
    output = input * scale;
}

LE_FORCEINLINE LE_HOT LE_NOTHROWNOALIAS void LE_FASTCALL
convertSample( std::int32_t const LE_GNU_SPECIFIC( & __restrict ) input, float & LE_GNU_SPECIFIC( __restrict ) output )
{
    static float BOOST_CONSTEXPR_OR_CONST scale( -1.0f / std::numeric_limits<std::int32_t>::min() );
    output = input * scale;
}

template <typename Input, typename Output>
LE_HOT LE_NOTHROWNOALIAS void LE_FASTCALL
convertSamples( Input const * LE_RESTRICT pInput, Output * LE_RESTRICT pOutput, std::uint32_t samples )
{
#ifdef __clang__
    #pragma clang loop vectorize( enable ) interleave( enable )
#endif // __clang__
    while ( samples-- )
        convertSample( *pInput++, *pOutput++ );
}

template <typename Sample>
LE_HOT LE_NOTHROWNOALIAS void LE_FASTCALL
convertSamples( Sample const * LE_RESTRICT const pInput, Sample * LE_RESTRICT const pOutput, std::uint32_t const samples )
{
    std::copy_n( pInput, samples, pOutput );
}

LE_OPTIMIZE_FOR_SPEED_END()

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // vector_hpp
