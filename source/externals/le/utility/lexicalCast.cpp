////////////////////////////////////////////////////////////////////////////////
///
/// lexicalCast.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "lexicalCast.hpp"

#include "platformSpecifics.hpp"

#if defined( _MSC_VER )
    #define LE_USE_SPIRIT
#elif defined( __GNUC__ )
    // Implementation note:
    //   On *NIX platforms we link dynamically with the CRT so we just use the
    // sprintf function.
    //                                        (12.12.2011.) (Domagoj Saric)
    //#define LE_USE_SPIRIT
#endif // compiler

#ifdef LE_USE_SPIRIT
    #pragma warning( disable : 4100 ) // Unreferenced formal parameter.
    #pragma warning( disable : 4127 ) // Conditional expression is constant.
    #pragma warning( disable : 4244 ) // Conversion from 'long' to 'float', possible loss of data.
    #pragma warning( disable : 4702 ) // Unreachable code.

    #include "boost/spirit/home/karma/generate.hpp"
    #include "boost/spirit/home/karma/numeric/int.hpp"
    #include "boost/spirit/home/karma/numeric/real.hpp"
    #include "boost/spirit/home/qi/numeric/int.hpp"
    #include "boost/spirit/home/qi/numeric/real.hpp"
#endif // LE_USE_SPIRIT

#include <cstdio>
#ifndef NDEBUG
    #include <cctype>
    #include <cmath>
#endif // NDEBUG
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

// http://code.google.com/p/stringencoders/source/browse/trunk/src/modp_numtoa.c
// http://www.dreamincode.net/code/snippet2482.htm
// http://www.piumarta.com/software/fcvt

LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( std::int32_t const value, char * const buffer )
{
    return LE_INT_SPRINTFA( buffer, "%d", value );
}
LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( long const value, char * const buffer )
{
    return lexical_cast( static_cast<std::int32_t>( value ), buffer );
}
LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( std::uint32_t const value, char * const buffer )
{
    return LE_INT_SPRINTFA( buffer, "%u", value );
}
LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( unsigned long const value, char * const buffer )
{
    return lexical_cast( static_cast<std::uint32_t>( value ), buffer );
}

#ifdef LE_USE_SPIRIT
namespace
{
    ////////////////////////////////////////////////////////////////////////////
    /// \internal
    /// \struct LERealGeneratePolicies
    ////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct LERealGeneratePolicies : boost::spirit::karma::real_policies<T>
    {
        using Base = boost::spirit::karma::real_policies<T>;

        LERealGeneratePolicies( std::uint8_t const precision = sizeof( T ) == sizeof( float ) ? 4 : 9 ) : precision_( precision ) {}

        static int floatfield( T ) { return fmtflags::fixed; }

        unsigned precision( T ) const { return precision_; }

        template <typename OutputIterator>
        static bool LE_FASTCALL dot( OutputIterator & sink, T const n, std::uint8_t const precision )
        {
            if ( precision && n )
                return Base::dot( sink, n, precision );
            else
                return true;
        }

        template <typename OutputIterator>
        bool LE_FASTCALL fraction_part( OutputIterator & sink, T const n, std::uint8_t const actualPrecision, std::uint8_t const precision ) const
        {
            LE_ASSUME( precision       == precision_ );
            LE_ASSUME( actualPrecision <= precision  );
            if ( actualPrecision )
                return Base::fraction_part( sink, n, actualPrecision, precision );
            else
            {
                LE_ASSUME( n == 0 );
                return true;
            }
        }

        template <typename CharEncoding, typename Tag, typename OutputIterator>
        static bool nan( OutputIterator & /*sink*/, T /*n*/, bool /*force_sign*/ ) { LE_UNREACHABLE_CODE(); return false; }

        template <typename CharEncoding, typename Tag, typename OutputIterator>
        static bool inf( OutputIterator & sink, T const n, bool /*force_sign*/ )
        {
            //LE_UNREACHABLE_CODE(); return false;
            return Base::inf<CharEncoding, Tag, OutputIterator>( sink, n, true );
        }

        std::uint8_t const precision_;
    }; // struct LERealGeneratePolicies

    template <typename T>
    LE_NOTHROWNOALIAS unsigned int LE_FASTCALL generateFloat( T const value, std::uint8_t const precision, char * const pBuffer )
    {
        char * pOutput( pBuffer );
        BOOST_VERIFY
        (
            boost::spirit::karma::generate
            (
                pOutput,
                boost::spirit::karma::real_generator<T, LERealGeneratePolicies<T>>( precision ),
                value
            )
        );
        *pOutput = '\0';
        return static_cast<unsigned int>( pOutput - pBuffer );
    }
} // anonymous namespace
#endif // LE_USE_SPIRIT

LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( float  const value, char * const buffer ) { return lexical_cast( value, 4, buffer ); }
LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( double const value, char * const buffer ) { return lexical_cast( value, 9, buffer ); }
LE_NOTHROWNOALIAS LE_COLD
unsigned int LE_FASTCALL lexical_cast( float const value, std::uint8_t const decimalPlaces, char * const buffer )
{
#ifdef LE_USE_SPIRIT
    return generateFloat<float>( value, decimalPlaces, buffer );
#else
    return lexical_cast( static_cast<double>( value ), decimalPlaces, buffer );
#endif // LE_USE_SPIRIT
}
LE_NOTHROWNOALIAS LE_COLD LE_NOINLINE
unsigned int LE_FASTCALL lexical_cast( double const value, std::uint8_t const decimalPlaces, char * const buffer )
{
#ifdef LE_USE_SPIRIT
    return lexical_cast( static_cast<float>( value ), std::min<std::uint8_t>( decimalPlaces, 4 ), buffer );
#else
    char const format[] = { '%', '.', static_cast<char>( '0' + decimalPlaces ), 'f', '\0' };
    unsigned int totalCharactersWritten( std::sprintf( buffer, format, value ) );
    if ( decimalPlaces )
    {
        /// \note Trim trailing zeros.
        ///                                   (15.12.2011.) (Domagoj Saric)
        char       *       pEnd( buffer + totalCharactersWritten );
        char const * const pDot( pEnd - decimalPlaces - 1        );
        BOOST_ASSERT( *pEnd == '\0'                            );
        BOOST_ASSERT( *pDot ==  '.' || !std::isfinite( value ) );
        while ( ( pEnd != pDot ) && ( *--pEnd == '0' ) ) {}
        pEnd += ( pEnd != pDot );
        BOOST_ASSERT( *pEnd == '0' || *pEnd == '.' || *pEnd == '\0' );
        *pEnd = '\0';
        totalCharactersWritten = static_cast<unsigned int>( pEnd - buffer );
    }
    else
    {
        BOOST_ASSERT( std::isalnum( buffer[ totalCharactersWritten - 1 ] ) );
    }
    BOOST_ASSERT_MSG
    (
        ( std::abs( lexical_cast<float>( buffer ) - static_cast<float>( value ) ) < ( 1 / std::pow( 10.0f, decimalPlaces ) ) ) ||
        !std::isfinite( value ),
        "Zero trimming broken."
    );
    return totalCharactersWritten;
#endif // LE_USE_SPIRIT
}


template <> LE_COLD
bool LE_FASTCALL lexical_cast<bool>( char const * const valueString )
{
    BOOST_ASSERT( valueString[ 0 ] == '0'  || valueString[ 0 ] == '1'                            );
    BOOST_ASSERT( valueString[ 1 ] == '\0' || valueString[ 1 ] == '"' || valueString[ 1 ] == '<' );
    std::uint8_t const value( valueString[ 0 ] - '0' );
    LE_ASSUME( ( value == 0 ) || ( value == 1 ) );
    return reinterpret_cast<bool const &>( value );
}

template <> LE_COLD
int LE_FASTCALL lexical_cast<int>( char const * valueString )
{
#ifdef LE_USE_SPIRIT
    using namespace boost::spirit;
    unsigned const maxDigits( std::numeric_limits<int>::digits10 );
    int result;
    BOOST_VERIFY(( qi::extract_int<int, 10, 1, maxDigits>::call( valueString, valueString + maxDigits, result ) ));
    return result;
#else
    return std::atoi( valueString );
#endif // LE_USE_SPIRIT
}

template <> LE_COLD long         LE_FASTCALL lexical_cast<long        >( char const * const valueString ) { return lexical_cast<int>( valueString ); }
template <> LE_COLD unsigned int LE_FASTCALL lexical_cast<unsigned int>( char const * const valueString ) { return lexical_cast<int>( valueString ); }

namespace
{
#ifdef LE_USE_SPIRIT
    ////////////////////////////////////////////////////////////////////////////
    /// \internal
    /// \struct LERealParsePolicies
    ////////////////////////////////////////////////////////////////////////////

    template <typename T>
    struct LERealParsePolicies : boost::spirit::qi::real_policies<T>
    {
        static bool const allow_leading_dot = false;

        template <typename Iterator>
        static bool parse_exp( Iterator & first, Iterator const & last )
        {
            BOOST_ASSERT( first == last || ( *first != 'e' && *first != 'E' ) );
            return false;
        }

        template <typename Iterator>
        static bool parse_exp_n( Iterator &, Iterator const &, int & ) { LE_UNREACHABLE_CODE(); }

        template <typename Iterator, typename Attribute>
        static bool parse_nan( Iterator & first, Iterator const & last, Attribute & )
        {
            BOOST_ASSERT( first == last || ( *first != 'n' && *first != 'N' ) );
            return false;
        }

        template <typename Iterator, typename Attribute>
        static bool parse_inf( Iterator & first, Iterator const & last, Attribute & )
        {
            BOOST_ASSERT( first == last || ( *first != 'i' && *first != 'I' ) );
            return false;
        }
    };
#endif // LE_USE_SPIRIT

    double lexical_cast_double_worker( char const * & pValueString );

    float lexical_cast_float_worker( char const * & pValueString )
    {
    #ifdef LE_USE_SPIRIT
        using namespace boost::spirit;
        float result;
        typedef qi::detail::real_impl<float, LERealParsePolicies<float>> Parser;
        //BOOST_VERIFY( qi::parse( pValueString, pValueString + RequiredStringStorage<float>::value, qi::real_parser<float, LERealParsePolicies<float>>(), result ) );
        BOOST_VERIFY( Parser::parse( pValueString, pValueString + RequiredStringStorage<float>::value, result, LERealParsePolicies<float>() ) );
        return result;
    #else
        return static_cast<float>( lexical_cast_double_worker( pValueString ) );
    #endif // LE_USE_SPIRIT
    }

    double lexical_cast_double_worker( char const * & pValueString )
    {
    #ifdef LE_USE_SPIRIT
        return lexical_cast_float_worker( pValueString );
    #else
        char * pEnd;
        double const result( std::strtod( pValueString, &pEnd ) );
        pValueString = pEnd;
        return result;
    #endif // LE_USE_SPIRIT
    }
}

template <> LE_COLD
float  LE_FASTCALL lexical_cast<float >( char const * valueString ) { return lexical_cast_float_worker ( valueString ); }
template <> LE_COLD
double LE_FASTCALL lexical_cast<double>( char const * valueString ) { return lexical_cast_double_worker( valueString ); }

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
