////////////////////////////////////////////////////////////////////////////////
///
/// math.cpp
/// --------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// General IEEE floating point, integer arithmetic, etc. information and tricks:
// http://chrishecker.com/images/f/fb/Gdmfp.pdf
// http://books.google.com/books?id=M2QYbTVd0VgC&pg=PA167&lpg=PA167&dq=game+programming+gems+IEEE&source=bl&ots=K4sV7CuLUW&sig=abo8s2JooXhBZ2VlElCJgv9wOg8&hl=en&ei=oZ85S5PmEIuImgPn9Oy6DQ&sa=X&oi=book%5Fresult&ct=result&resnum=2&ved=0CA8Q6AEwAQ#v=onepage&q=game%20programming%20gems%20IEEE&f=false
// http://musicdsp.org/archive.php?classid=5#273
// http://locklessinc.com/articles/sat_arithmetic
// http://pandorawiki.org/Floating_Point_Optimization
// http://www.altdevblogaday.com/2012/05/20/thats-not-normalthe-performance-of-odd-floats
// http://dbp-consulting.com/StrictAliasing.pdf
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "math.hpp"

#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/utility/intrinsics.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/simd/sdk/config/arch.hpp"

#include <boost/assert.hpp>
#include <boost/detail/endian.hpp>
#include <boost/integer.hpp>
#include <boost/integer_traits.hpp>

#include <cmath>
#include <chrono>
#include <cstdlib>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------
#ifdef _MSC_VER
    #pragma runtime_checks( "", off )
    #pragma check_stack   (     off )
#endif // BOOST_MSVC

LE_OPTIMIZE_FOR_SPEED_BEGIN()

namespace
{
    union Float32
    {
        float value;
        struct Bits
        {
            std::uint32_t mantissa: 23;
            std::uint32_t exponent:  8;
            std::uint32_t sign    :  1;
        } bits;
        bool          negative() const { return bits.sign          ; }
        std::int8_t   exponent() const { return bits.exponent - 127; }
        std::uint32_t mantissa() const { return bits.mantissa      ; }
    }; // union Float32
}

LE_NOTHROWNOALIAS
bool LE_FASTCALL makeBool( unsigned int boolean );


std::uint8_t abs( bool const value )
{
    BOOST_ASSERT_MSG( value == 0 || value == 1, "Invalid input" );
    return static_cast<std::uint8_t>( value );
}


bool isGreater( float const * LE_RESTRICT const pLeft, float const * LE_RESTRICT const pRight )
{
    /// \note The integer version is better for in-memory operands.
    ///                                       (13.01.2012.) (Domagoj Saric)
    LE_ASSUME( pLeft  );
    LE_ASSUME( pRight );

    int leftBits ( reinterpret_cast<int const &>( *pLeft  ) );
    int rightBits( reinterpret_cast<int const &>( *pRight ) );

    int const leftSign( leftBits >> 31 );
    leftBits  = (  leftBits ^  leftSign ) + (  leftSign & 0x80000001 );

    int const rightSign( rightBits >> 31 );
    rightBits = ( rightBits ^ rightSign ) + ( rightSign & 0x80000001 );

    auto const result( leftBits > rightBits );
    BOOST_ASSERT_MSG
    (
        ( result == ( *pLeft > *pRight ) ) || !std::isfinite( *pLeft ) || !std::isfinite( *pRight ),
        "Unexpected result"
    );
    return result;
}

namespace PositiveFloats
{
    // Float comparison
    // http://randydillon.org/Papers/2007/everfast.htm
    // http://www.cygnus-software.com/papers/comparingfloats/comparingfloats.htm
    // http://realtimecollisiondetection.net/blog/?p=89
    // http://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison

    bool isGreater( float const * LE_RESTRICT const pLeft, float const * LE_RESTRICT const pRight )
    {
        /// \note The integer version is better for in-memory operands.
        ///                                   (13.01.2012.) (Domagoj Saric)
        LE_ASSUME( pLeft  );
        LE_ASSUME( pRight );
        auto const result( reinterpret_cast<int const &>( *pLeft ) > reinterpret_cast<int const &>( *pRight ) );
        BOOST_ASSERT_MSG
        (
            ( result == ( *pLeft > *pRight ) ) || !std::isfinite( *pLeft ) || !std::isfinite( *pRight ),
            "Unexpected result"
        );
        return result;
    }

    bool isGreater( double const * LE_RESTRICT const pLeft, double const * LE_RESTRICT const pRight )
    {
        auto const result( reinterpret_cast<long long const &>( *pLeft ) > reinterpret_cast<long long const &>( *pRight ) );
        BOOST_ASSERT_MSG( result == ( *pLeft > *pRight ), "Unexpected result" );
        return result;
    }


    unsigned int ceil( float const value )
    {
        LE_ASSUME( value >= 0 );
        float const belowHalf( 0.45f ); //...mrmlj...
        int   const result   ( round( value + belowHalf ) );
        BOOST_ASSERT_MSG( std::ceil( value ) == static_cast<float>( result ), "Unexpected result" );
        return result;
    }


    unsigned int floor( float const value )
    {
        unsigned int const result( Math::truncate( value ) );
        //...mrmlj...need not hold when (mis)used for fast(er) phase mapping...
        //BOOST_ASSERT( std::floorf( value ) == static_cast<float>( result ) );
        return result;
    }


    float modulo( float const dividend, float const divisor )
    {
        LE_ASSUME( divisor != 0 );
    #if !( defined( BOOST_SIMD_ARCH_X86 ) && !defined( BOOST_SIMD_HAS_SSE_SUPPORT ) )
        //...mrmlj...signed so that it can be (mis)used for fast(er) phase mapping...
        /*unsigned*/ int const divisionFloor( truncate( dividend / divisor )         );
        float            const mod          ( dividend - ( divisionFloor * divisor ) );
        //...mrmlj...need not hold when (mis)used for fast(er) phase mapping...
        //BOOST_ASSERT( std::floor( value ) == static_cast<float>( result ) );
        return mod;
    #else
        return Math::modulo( dividend, divisor );
    #endif // BOOST_SIMD_HAS_SSE_SUPPORT
    }


    bool isZero( float const & value )
    {
        BOOST_ASSERT_MSG( std::isfinite( value ), "Invalid input" );
        unsigned int const & valueBits( reinterpret_cast<unsigned int const &>( value ) );
        return valueBits == 0;
    }
} // namespace PositiveFloats


int floor( float const value )
{
    // http://www.masm32.com/board/index.php?PHPSESSID=df3d20eef32d75578b6e4c0bf9b44819&action=printpage;topic=9515.0
    int const truncatedValue( truncate( value ) );
    int const result( truncatedValue - ( isNegative( value ) & ( truncatedValue != value ) ) );
    BOOST_ASSERT_MSG( std::floor( value ) == static_cast<float>( result ), "Unexpected result" );
    return result;
}


int ceil( float const value )
{
    // http://www.masm32.com/board/index.php?PHPSESSID=272a49f2a96ecb36c9a0b830e847c358&topic=9514.0

    float const valueX2( value * 2 );
    int const result( - ( round( - 0.5f - valueX2 ) >> 1 ) );
    BOOST_ASSERT_MSG( std::ceil( value ) == static_cast<float>( result ), "Unexpected result" );
    return result;
}


// http://ompf.org/forum/viewtopic.php?f=11&t=1271
// http://mubench.sourceforge.net/results.html
float modulo( float const dividend, float const divisor )
{
    LE_ASSUME( divisor != 0 );
#if !( defined( BOOST_SIMD_ARCH_X86 ) && !defined( BOOST_SIMD_HAS_SSE_SUPPORT ) )
    int   const divisionFloor( floor( dividend / divisor )            );
    float const mod          ( dividend - ( divisionFloor * divisor ) );
    // Implementation note:
    //   std::fmod() works with double precision so its internal divisionFloor
    // result can differ by one from ours when dividend / divisor is very close
    // to an integer. In these cases our routine will produce a small negative
    // mod result and will thus differ from the std::fmod() result so we skip
    // the below sanity check for those cases.
    //                                        (05.01.2011.) (Domagoj Saric)
    BOOST_ASSERT_MSG
    (
        nearEqual( mod, std::fmod( dividend, divisor ) ) ||
        (
            static_cast<int>( static_cast<float >( static_cast<float >( dividend ) / static_cast<float >( divisor ) ) ) !=
            static_cast<int>( static_cast<double>( static_cast<double>( dividend ) / static_cast<double>( divisor ) ) )
        ), "Broken modulo."
    );
    return mod;
#elif defined( __GNUC__ )
    return ::__builtin_fmodf( dividend, divisor );
#else // BOOST_SIMD_HAS_SSE_SUPPORT
    #pragma message ( "Add a fast version for this platform..." )
    return std::fmodf( dividend, divisor );
#endif // BOOST_SIMD_HAS_SSE_SUPPORT
}


int modulo( int const dividend, int const divisor )
{
    LE_ASSUME( divisor != 0 );
    return dividend % divisor;
}

unsigned int modulo( unsigned int const dividend, unsigned int const divisor )
{
    LE_ASSUME( divisor != 0 );
    return dividend % divisor;
}


std::uint32_t clamp( std::int32_t const value, std::uint32_t const lowerBound, std::uint32_t const upperBound )
{
    // http://stackoverflow.com/questions/427477/fastest-way-to-clamp-a-real-fixed-floating-point-value
    // http://graphics.stanford.edu/~seander/bithacks.html#IntegerMinOrMax
    // http://www.coranac.com/documents/bittrick
    // http://stackoverflow.com/questions/707370/clean-efficient-algorithm-for-wrapping-integers-in-c

    BOOST_ASSERT_MSG( lowerBound <= upperBound, "Invalid input" );

#ifdef BOOST_SIMD_HAS_SSE4_1_SUPPORT
    __m128i vectorResult( _mm_cvtsi32_si128 ( value ) );
    vectorResult = _mm_max_epi32( vectorResult, _mm_cvtsi32_si128( lowerBound ) );
    vectorResult = _mm_min_epu32( vectorResult, _mm_cvtsi32_si128( upperBound ) );
    return _mm_cvtsi128_si32( vectorResult );
#elif defined( _MSC_VER )
    //...mrmlj...MSVC generates bad code for the ternary operator...
         if ( value < static_cast<std::int32_t>( lowerBound ) ) return lowerBound;
    else if ( value > static_cast<std::int32_t>( upperBound ) ) return upperBound;
                                                                return value     ;
#else
    return std::min<std::uint32_t>( std::max<std::int32_t>( value, lowerBound ), upperBound );
#endif // _MSC_VER
}

std::uint64_t clamp( std::int64_t const value, std::uint64_t const lowerBound, std::uint64_t const upperBound ) { return std::min<std::uint64_t>( std::max<std::int64_t>( value, lowerBound ), upperBound ); }
std::uint16_t clamp( std::int16_t const value, std::uint16_t const lowerBound, std::uint16_t const upperBound ) { return std::min<std::uint16_t>( std::max<std::int16_t>( value, lowerBound ), upperBound ); }
std::uint8_t  clamp( std::int8_t  const value, std::uint8_t  const lowerBound, std::uint8_t  const upperBound ) { return std::min<std::uint8_t >( std::max<std::int8_t >( value, lowerBound ), upperBound ); }

std::uint16_t clamp( std::uint16_t const value, std::uint16_t const lowerBound, std::uint16_t const upperBound ) { return clamp( static_cast<std::int16_t>( value ), lowerBound, upperBound ); }

SplitFloat splitFloat( float const value )
{
    SplitFloat result;
    result.integer    = truncate( value );
    result.fractional = value - result.integer;
    return result;
}


bool equal( float const & left, float const & right )
{
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    LE_ASSUME( std::isfinite( left  ) );
    LE_ASSUME( std::isfinite( right ) );
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

#if defined( BOOST_SIMD_HAS_SSE2_SUPPORT ) && 0
    unsigned int const leftBits ( _mm_cvtsi128_si32( _mm_castps_si128( _mm_set_ss( left  ) ) ) );
    unsigned int const rightBits( _mm_cvtsi128_si32( _mm_castps_si128( _mm_set_ss( right ) ) ) );
    return leftBits == rightBits;
#elif defined( _MSC_VER )
    unsigned int const & leftBits ( reinterpret_cast<unsigned int const &>( left  ) );
    unsigned int const & rightBits( reinterpret_cast<unsigned int const &>( right ) );
    return leftBits == rightBits;
#else
    return left == right;
#endif
}

bool equal( float const & left, unsigned int const right )
{
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    LE_ASSUME( std::isfinite( left ) );
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

    float const rightFloat( convert<float>( right ) );

    unsigned int const & leftBits ( reinterpret_cast<unsigned int const &>( left       ) );
    unsigned int const & rightBits( reinterpret_cast<unsigned int const &>( rightFloat ) );

    return leftBits == rightBits;
}


bool LE_FASTCALL nearEqual( float const left, float const right )
{
    // Implementation note:
    //   See the previously listed float comparison related links.
    //                                        (05.01.2011.) (Domagoj Saric)

    int const leftBits ( reinterpret_cast<int const &>( left  ) );
    int const rightBits( reinterpret_cast<int const &>( right ) );

    int const lexicographicallyOrderedLeftArray [ 2 ] = { leftBits , static_cast<int>( 0x80000000 - leftBits  ) };
    int const lexicographicallyOrderedRightArray[ 2 ] = { rightBits, static_cast<int>( 0x80000000 - rightBits ) };

    int const lexicographicallyOrderedLeft ( lexicographicallyOrderedLeftArray [ isNegative( left  ) ] );
    int const lexicographicallyOrderedRight( lexicographicallyOrderedRightArray[ isNegative( right ) ] );

    /// \todo Reinvestigate this number and choose and document a meaningful
    /// value for our purposes.
    ///                                       (06.12.2010.) (Domagoj Saric)
    unsigned int const maximumDifferenceInULPs( 3000 );
    return Math::abs( lexicographicallyOrderedLeft - lexicographicallyOrderedRight ) < maximumDifferenceInULPs;
}

bool LE_FASTCALL nearEqual( float const left, unsigned int const right )
{
    float const rightFloat( convert<float>( right ) );

    int const leftBits ( reinterpret_cast<int const &>( left       ) );
    int const rightBits( reinterpret_cast<int const &>( rightFloat ) );

    int const lexicographicallyOrderedLeftArray [ 2 ] = { leftBits, static_cast<int>( 0x80000000 - leftBits ) };

    int const lexicographicallyOrderedLeft ( lexicographicallyOrderedLeftArray[ isNegative( left ) ] );
    int const lexicographicallyOrderedRight( rightBits                                               );

    /// \todo Reinvestigate this number and choose and document a meaningful
    /// value for our purposes.
    ///                                       (06.12.2010.) (Domagoj Saric)
    unsigned int const maximumDifferenceInULPs( 3000 );
    return Math::abs( lexicographicallyOrderedLeft - lexicographicallyOrderedRight ) < maximumDifferenceInULPs;
}


bool isZero( float const & value )
{
    //...mrmlj...avoid going to the SIMD unit...
    int   const & valueAbsoluteBits( reinterpret_cast<int   const &>( value             ) & 0x7FFFFFFF );
    float const & positive         ( reinterpret_cast<float const &>( valueAbsoluteBits )              );
    return PositiveFloats::isZero( positive );
}


bool isNegative( float const value )
{
    static_assert( sizeof( int ) == sizeof( float ), "Unexpected data sizes" );
#ifdef BOOST_SIMD_HAS_SSE2_SUPPORT
    auto const result( isNegative( _mm_cvtsi128_si32( _mm_castps_si128( _mm_set_ss( value ) ) ) ) );
#else
    auto const result( isNegative( reinterpret_cast<int const &>( value ) ) );
#endif // BOOST_SIMD_HAS_SSE2_SUPPORT
    BOOST_ASSERT_MSG( ( result == ( value < 0 ) ) || ( value == -0.0f ), "Unexpected result" );
    return result;
}

bool isNegative( int const value )
{
    std::uint8_t const valueNumberOfBits( sizeof( value ) * 8 );
    std::uint8_t const valueSign        ( reinterpret_cast<unsigned int const &>( value ) >> ( valueNumberOfBits - 1 ) );
    BOOST_ASSERT_MSG( valueSign == ( value < 0 ), "Unexpected result" );
    return reinterpret_cast<bool const &>( valueSign );
}

bool isNegative( unsigned int /*value*/ )
{
    return false;
}

#ifndef LE_HAS_NT2 // disabled: NT2 implementation@vector.cpp
LE_CONST_FUNCTION float LE_FASTCALL_ABI log2( float const value )
{
#if defined( __GNUC__ ) && !defined( __ANDROID__ )
    return ::__builtin_log2f( value );
#else
    return /*std*/::log2( value ) / LE::Math::Constants::ln2;
#endif
}
LE_CONST_FUNCTION float LE_FASTCALL_ABI exp2( float const value ) { return std::exp2( value ); }
#endif // disabled

LE_CONST_FUNCTION std::uint8_t LE_FASTCALL_ABI log2( int const value )
{
    BOOST_ASSERT_MSG( !isNegative( value ), "Invalid input" );
    return log2( static_cast<unsigned long>( value ) );
}

LE_CONST_FUNCTION std::uint8_t LE_FASTCALL_ABI log2( unsigned int const value )
{
    return firstSetBit( value );
}

LE_CONST_FUNCTION std::uint8_t LE_FASTCALL_ABI log2( unsigned long const value )
{
    BOOST_ASSERT_MSG( static_cast<unsigned int>( value ) == value, "Value out of range." ); //...mrmlj...
    return log2( static_cast<unsigned int>( value ) );
}


namespace PowerOfTwo
{
    unsigned int ceil( float const & value )
    {
        // http://stackoverflow.com/questions/466204/rounding-off-to-nearest-power-of-2
        // http://www.gamedev.net/community/forums/topic.asp?topic_id=229831

        unsigned int const & valueBits    ( reinterpret_cast<unsigned int const &>( value ) );
        unsigned int const   notPowerOfTwo( ( valueBits << 9 ) != 0 );
        unsigned int const   exponent
        (
            ( valueBits >> 23 ) // remove fractional part of the floating point number
             -
            127                 // subtract 127 (the bias) from the exponent
             +
            notPowerOfTwo       // add one to the exponent if the value was not a power of two
        );

        LE_ASSUME( exponent < ( sizeof( 1U ) * 8 ) );

        return 1U << exponent;
    }

    unsigned int floor( unsigned int const value )
    {
        return firstSetBit( value );
    }

    unsigned int round( unsigned int const value )
    {
        // http://en.wikipedia.org/wiki/Power_of_two#Algorithm_to_convert_any_number_into_nearest_power_of_two_number
        // http://stackoverflow.com/questions/1983303/using-bts-assembly-instruction-with-gcc-compiler
        // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=36473

        BOOST_ASSERT_MSG( value != 0, "Invalid input" );
        unsigned int const firstSetBitInValue( firstSetBit( value ) );
        unsigned int const isNextBitSet
        (
            #if defined( _MSC_VER ) && !defined( _XBOX )
                _bittest( reinterpret_cast<long const *>( &value ), ( firstSetBitInValue - 1 ) )
            #else
                firstSetBitInValue && ( ( value & ( 1U << ( firstSetBitInValue - 1 ) ) ) != 0 )
            #endif // _MSC_VER
        );

        unsigned int const exponent( firstSetBitInValue + isNextBitSet );

        LE_ASSUME( exponent < ( sizeof( 1U ) * 8 ) );

        return 1U << exponent;
    }

    std::uint8_t log2( unsigned int const value )
    {
        BOOST_ASSERT_MSG( isPowerOfTwo( value ), "Invalid input" );
        return firstSetBit( value );
    }
} // namespace PowerOfTwo


////////////////////////////////////////////////////////////////////////////////
//
// numberOfSetBits()
// -----------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Returns the number of bits set in the passed integer value.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

std::uint8_t numberOfSetBits( int const value )
{
    // Implementation note:
    //   http://tekpool.wordpress.com/category/bit-count.
    //                                        (11.05.2009.) (Domagoj Saric)
    auto const uCount
    (
        value
        - ( ( value >> 1 ) & 033333333333 )
        - ( ( value >> 2 ) & 011111111111 )
    );
    return static_cast<std::uint8_t>( ( ( uCount + ( uCount >> 3 ) ) & 030707070707 ) % 63 );
}


////////////////////////////////////////////////////////////////////////////////
//
// firstSetBit()
// -------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Returns the index of the first set MSB in the passed integer value.
/// Expects that the passed value is non-zero/has at least one bit set.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////
// http://stackoverflow.com/questions/364985/algorithm-for-finding-the-smallest-power-of-two-thats-greater-or-equal-to-a-give
////////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER ) && !defined( _XBOX )
    #pragma intrinsic( _BitScanReverse )
#endif // _MSC_VER

std::uint8_t firstSetBit( int const value )
{
    BOOST_ASSERT_MSG( value, "Invalid input" );
    #if defined( _MSC_VER )
        #ifdef _XBOX
            std::uint8_t const leadingZeroBits( _CountLeadingZeros( value ) );
            return ( sizeof( value ) * 8 ) - 1 - leadingZeroBits;
        #else
            unsigned long firstSetBitIndex;
            BOOST_VERIFY( _BitScanReverse( &firstSetBitIndex, value ) && "No bits set in the passed value." );
            return static_cast<std::uint8_t>( firstSetBitIndex );
        #endif
    #elif defined( __GNUC__ )
        std::uint8_t const leadingZeroBits( __builtin_clz( value ) );
        return ( sizeof( value ) * 8 ) - 1 - leadingZeroBits;
    #else // _MSC_VER
        #error not implemented.
    #endif // _MSC_VER
}


std::uint8_t lastSetBit( int const value )
{
    BOOST_ASSERT_MSG( value, "Invalid input" );
    #if defined( _MSC_VER )
        #ifdef _XBOX
            return static_cast<std::uint8_t>( ( sizeof( value ) * 8 ) - _CountLeadingZeros( ~value & ( value - 1 ) ) );
        #else
            unsigned long lastSetBitIndex;
            BOOST_VERIFY( _BitScanForward( &lastSetBitIndex, value ) && "No bits set in the passed value." );
            return static_cast<std::uint8_t>( lastSetBitIndex );
        #endif
    #elif defined( __GNUC__ )
        std::uint8_t const trailingZeroBits( __builtin_ctz( value ) );
        return ( sizeof( value ) * 8 ) - 1 - trailingZeroBits;
    #else // _MSC_VER
        #error not implemented.
    #endif // _MSC_VER
}


////////////////////////////////////////////////////////////////////////////////
//
// isPowerOfTwo()
// --------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Returns whether the passed value is a power-of-two value.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

bool isPowerOfTwo( unsigned int const value )
{
    // http://graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    bool const result( ( value & ( value - 1 ) ) == 0 );
    // If only one bit is set then it is certainly a power-of-two value.
    BOOST_ASSERT_MSG( result == ( numberOfSetBits( value ) == 1 ), "Power of two logic bug." );
    return result;
}

bool isPowerOfTwo( int const value )
{
    BOOST_ASSERT_MSG( !isNegative( value ), "Invalid input" );
    return isPowerOfTwo( static_cast<unsigned int>( value ) );
}

bool isPowerOfTwo( float const value )
{
    // http://cottonvibes.blogspot.com/2010/08/checking-if-float-is-power-of-2.html
    // http://cottonvibes.blogspot.com/2012/03/table-bitpacking-to-avoid-lookup-tables.html
    union FloatBits
    {
        struct Parts
        {
            std::uint32_t sign     :  1;
            std::uint32_t exponent :  8;
            std::uint32_t mantissa : 23;
        }; // struct Parts

        float         value;
        std::uint32_t bits ;
        Parts         parts;
    }; // union FloatBits

    FloatBits const bits = { value };
    return !bits.parts.sign && !bits.parts.mantissa && ( bits.parts.exponent >= 127 );
}


////////////////////////////////////////////////////////////////////////////////
//
// rangedRand()
// ------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates a random number in the [0, maximum) interval.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

namespace
{
    LE_OPTIMIZE_FOR_SPEED_BEGIN()

    // https://channel9.msdn.com/Events/GoingNative/2013/rand-Considered-Harmful
    // https://www.youtube.com/watch?v=45Oet5qjlms "PCG: A Family of Better Random Number Generators"
    // http://eternallyconfuzzled.com/arts/jsw_art_rand.aspx
    // http://www.pcg-random.org/other-rngs.html
    // http://www.boost.org/doc/libs/release/doc/html/boost_random/reference.html#boost_random.reference.generators
    // https://github.com/s9w/articles/blob/master/perf%20cpp%20random.md
    // http://stackoverflow.com/questions/1640258/need-a-fast-random-generator-for-c
    // http://stackoverflow.com/questions/1046714/what-is-a-good-random-number-generator-for-a-game
    // http://burtleburtle.net/bob/rand/smallprng.html
    // https://en.wikipedia.org/wiki/Xorshift
    // https://en.wikipedia.org/wiki/Talk%3AXorshift#32-bit_code_for_xorshift1024.2A_and_xorshift128.2B
    // http://xorshift.di.unimi.it

    // http://xorshift.di.unimi.it/xorshift128plus.c
    // http://www001.upp.so-net.ne.jp/isaku/en/dxor156.c.html
    // http://www.irrelevantconclusion.com/2012/02/pretty-fast-random-floats-on-ps3
    // http://www.reedbeta.com/blog/2013/01/12/quick-and-easy-gpu-random-numbers-in-d3d11

    // http://security.stackexchange.com/questions/47446/can-the-xor-of-two-rng-outputs-ever-be-less-secure-than-one-of-them

    static LE_ALIGN( 16 ) std::uint64_t rng_state[ 2 ];

    /// \note The 64 bit RNG is much slower in 32bit builds so we 'reduce'/limit
    /// its output width for those builds so that at least the ranged and
    /// floating point wrapper functions don't have to go through the slow
    /// emulated 64bit math path.
    ///                                       (06.10.2015.) (Domagoj Saric)
    using rand_t = std::size_t;

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4127 ) // Conditional expression is constant.
#endif // _MSC_VER
    LE_NOINLINE LE_HOT
    rand_t LE_FASTCALL xorshift128pRNG()
    {
        BOOST_ASSERT_MSG( rng_state[ 0 ] && rng_state[ 1 ], "RNG not seeded." );

        std::uint64_t       s1( rng_state[ 0 ] );
        std::uint64_t const s0( rng_state[ 1 ] );

        s1 ^= s1 << 23; // a
        s1  = s1 ^ s0 ^ ( s1 >> 17 ) ^ ( s0 >> 26 ); // b, c

        rng_state[ 0 ] = s0;
        rng_state[ 1 ] = s1;

        std::uint64_t const wideResult( s1 + s0 );
        if ( sizeof( std::size_t ) >= sizeof( rand_t ) )
            return wideResult;
        else
        {
            std::uint32_t const result( static_cast<std::uint32_t>( wideResult ) + ( wideResult >> 32 ) );
            return result;
        }
    }
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

    template <typename T>
    LE_HOT
    T LE_FASTCALL rangedRand( T const maximum )
    {
    #if 0 // std::rand() is usually slow and of poor quality
        // Implementation note:
        //   On some platforms (e.g. OS X) RAND_MAX is equal to INT_MAX which
        // makes it unsafe to implement this function simply as
        // return std::rand() * maximum / ( RAND_MAX + 1 );
        // because the intermediate result could overflow the int range.
        //                                    (16.05.2011.) (Domagoj Saric)
        using work_t = typename boost::uint_value_t<boost::integer_traits<T>::const_max * static_cast<std::uint64_t>( RAND_MAX )>::fast;

        work_t const randomMaximum( RAND_MAX );
        work_t const randomNumber( static_cast<unsigned>( std::rand() ) );
        LE_ASSUME( randomNumber >= 0 );
        LE_ASSUME( randomNumber <= randomMaximum );

        return static_cast<T>( randomNumber * maximum / ( randomMaximum + 1 ) );
    #else
        /// \note For now we intentionally go with the naive modulo approach
        /// because of the small ranges of random number we require vs the large
        /// value range of the RNG.
        /// http://c-faq.com/lib/randrange.html
        ///                                   (05.10.2015.) (Domagoj Saric)
        return static_cast<T>( xorshift128pRNG() % maximum );
    #endif
    }

    LE_OPTIMIZE_FOR_SPEED_END()
} // anonymous namespace

void LE_COLD rngSeed()
{
    char stack;
    rng_state[ 0 ] = std::chrono::system_clock::now().time_since_epoch().count();
    rng_state[ 1 ] = reinterpret_cast<std::size_t>( &stack );

    //include the CRT version for 3rd party code?
    //std::srand( static_cast<unsigned int>( std::time( 0 ) ) );
}

std::uint32_t LE_FASTCALL rangedRand( std::uint32_t const maximum ) { return rangedRand<std::uint32_t>( maximum ); }
std::uint16_t LE_FASTCALL rangedRand( std::uint16_t const maximum ) { return rangedRand<std::uint16_t>( maximum ); }

////////////////////////////////////////////////////////////////////////////////
//
// rangedRand()
// ------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates a random number in the [minimum, maximum] interval.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

float LE_FASTCALL rangedRand( float const minimum, float const maximum )
{
    LE_ASSUME( maximum >= minimum );
    auto const result( minimum + rangedRand( maximum - minimum ) );
    LE_ASSUME( result >= minimum );
    LE_ASSUME( result <= maximum );
    return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// rangedRand()
// ------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates a random number in the [minimum, maximum) interval.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

std::int32_t LE_FASTCALL rangedRand( std::int32_t const minimum, std::uint32_t const maximum )
{
    LE_ASSUME( static_cast<signed>( maximum ) > minimum );
    std::int32_t const result( minimum + rangedRand( maximum - minimum ) );
    LE_ASSUME( result >=                      minimum   );
    LE_ASSUME( result <= static_cast<signed>( maximum ) );
    return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// rangedRand()
// ------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates a random number in the [0, maximum] interval.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

float LE_FASTCALL rangedRand( float const maximum ) { return normalisedRand() * maximum; }


////////////////////////////////////////////////////////////////////////////////
//
// normalisedRand()
// ----------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates a random number in the [0, 1] interval.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

float LE_FASTCALL LE_HOT normalisedRand()
{
    auto const result( static_cast<float>( static_cast<double>( xorshift128pRNG() ) / std::numeric_limits<rand_t>::max() ) );
    LE_ASSUME( result >= 0 );
    LE_ASSUME( result <= 1 );
    return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// FPUDisableDenormalsGuard::FPUDisableDenormalsGuard()
// ----------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Saves the current FPU control word and enables FTZ and/or DAZ.
///
////////////////////////////////////////////////////////////////////////////////
/// Related reading:
/// http://software.intel.com/en-us/articles/x87-and-sse-floating-point-assists-in-ia-32-flush-to-zero-ftz-and-denormals-are-zero-daz
/// https://code.google.com/p/no-more-denormals
/// http://www.cplusplus.com/reference/cfenv/fesetenv FE_DFL_DISABLE_SSE_DENORMS_ENV
/// http://linux.die.net/man/3/fesetround
////////////////////////////////////////////////////////////////////////////////

namespace
{
    unsigned int getFPUControlWord()
    {
    #if defined( _MSC_VER )
        return ::_controlfp( 0, 0 );
    #elif defined( BOOST_SIMD_HAS_SSE_SUPPORT )
        return _mm_getcsr();
    #elif defined( BOOST_SIMD_ARCH_ARM ) && !defined( __SOFTFP__ ) && !defined( __aarch64__ )
        // https://www.raspberrypi.org/forums/viewtopic.php?f=72&t=34664
        // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/BCFHFBGA.html FPSCR, the floating-point status and control register
        // http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0068b/BCFIDDIG.html
        // http://sourceware.org/ml/binutils/2006-12/msg00196.html
        unsigned int fpscr;
        __asm__( "vmrs %0, fpscr" : "=r" (fpscr) );
        return fpscr;
    #elif defined( __aarch64__ )
        // https://chromium.googlesource.com/chromium/blink/+/72fef91ac1ef679207f51def8133b336a6f6588f/Source/platform/audio/DenormalDisabler.h?autodive=0%2F%2F%2F
        unsigned int fpcr;
        asm volatile( "mrs %[fpcr], FPCR" : [fpcr] "=r" (fpcr) );
        return fpcr;
    #else
        return 0;
    #endif // arch
    }
}

FPUDisableDenormalsGuard::FPUDisableDenormalsGuard()
    :
    originalFloatingPointControlWord_( getFPUControlWord() )
{
#if defined( _MSC_VER )
    ::_controlfp( _DN_FLUSH, _MCW_DN );
#elif defined( BOOST_SIMD_HAS_SSE_SUPPORT )
    _mm_setcsr( ( originalFloatingPointControlWord_ & ~_MM_FLUSH_ZERO_MASK & ~_MM_EXCEPT_MASK ) | _MM_EXCEPT_UNDERFLOW | _MM_FLUSH_ZERO_ON | ( 1 << 6 ) );
#elif defined( BOOST_SIMD_ARCH_ARM ) && !defined( __SOFTFP__ ) && !defined( __aarch64__ )
    unsigned int const fpscr( originalFloatingPointControlWord_ | (1 << 24) );
    __asm__( "vmsr fpscr,%0" : :"ri" ( fpscr ) );
#elif defined( __aarch64__ )
    unsigned int const fpcr( originalFloatingPointControlWord_ | (1 << 24) );
    asm volatile( "msr FPCR, %[src]" : : [src] "r" (fpcr) );
#endif // arch
}


FPUDisableDenormalsGuard::~FPUDisableDenormalsGuard()
{
#if defined( _MSC_VER )
    BOOST_VERIFY( ::_controlfp( originalFloatingPointControlWord_, _MCW_DN ) == originalFloatingPointControlWord_ );
#elif defined( BOOST_SIMD_HAS_SSE_SUPPORT )
    _mm_setcsr( originalFloatingPointControlWord_ );
#elif defined( BOOST_SIMD_ARCH_ARM ) && !defined( __SOFTFP__ ) && !defined( __aarch64__ )
    __asm__( "vmsr fpscr,%0" : :"ri" (originalFloatingPointControlWord_) );
#elif defined( __aarch64__ )
    asm volatile( "msr FPCR, %[src]" : : [src] "r" (originalFloatingPointControlWord_) );
#endif // arch
}


////////////////////////////////////////////////////////////////////////////////
//
// FPUExceptionsGuard::FPUExceptionsGuard()
// ----------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
/// Related reading:
/// http://msdn.microsoft.com/en-us/library/aa289157(VS.71).aspx#floapoint_topic8
/// http://www.dinkumware.com/manuals/?manual=compleat&page=fenv.html
////////////////////////////////////////////////////////////////////////////////

FPUExceptionsGuard::FPUExceptionsGuard()
#ifdef _MSC_VER
    :
    originalFloatingPointControlWord_( ::_control87( 0, 0 ) )
#endif // _MSC_VER
{
#ifdef _MSC_VER
    BOOST_ASSERT( ( originalFloatingPointControlWord_ & EM_AMBIGUOUS ) == 0 );
    ::_clear87();
#endif // _MSC_VER
}


FPUExceptionsGuard::~FPUExceptionsGuard()
{
#ifdef _MSC_VER
    ::_clear87();

    // Reset the control word.
    ::_control87( originalFloatingPointControlWord_, exceptionsMask );
#endif // _MSC_VER
}


FPUExceptionsEnabler::FPUExceptionsEnabler()
{
#ifdef _MSC_VER
    // Set the exception masks OFF/turn exceptions on.
    ::_controlfp( 0, exceptionsMask );
#endif // _MSC_VER
}


FPUExceptionsDisabler::FPUExceptionsDisabler()
{
#ifdef _MSC_VER
    // Set the exception masks ON/turn exceptions off.
    ::_controlfp( static_cast<unsigned int>( -1 ), exceptionsMask );
#endif // _MSC_VER
}

LE_OPTIMIZE_FOR_SPEED_END()

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#if LE_SW_GUI
namespace juce
{
    float jmin( float const & value1, float const & value2 ) { return std::min( value1, value2 ); }
    float jmax( float const & value1, float const & value2 ) { return std::max( value1, value2 ); }
    float jlimit( float const & lowerLimit, float const & upperLimit, float const & valueToConstrain )
    {
        return LE::Math::clamp( valueToConstrain, lowerLimit, upperLimit );
    }
}
#endif // LE_SW_GUI
