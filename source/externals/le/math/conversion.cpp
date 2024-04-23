////////////////////////////////////////////////////////////////////////////////
///
/// conversion.cpp
/// --------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "conversion.hpp"

#include "le/math/math.hpp"
#include "le/math/constants.hpp"
#include "le/utility/intrinsics.hpp"

#include <cmath>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

#ifdef _MSC_VER
    #pragma runtime_checks( "", off )
    #pragma check_stack   ( off )
    #pragma optimize      ( "s", off )
    #pragma optimize      ( "t", on  )
#endif // BOOST_MSVC
LE_NOTHROWNOALIAS
bool LE_FASTCALL makeBool( unsigned int const boolean )
{
    BOOST_ASSERT_MSG( boolean == 0 || boolean == 1, "Boolean value not exactly 0 or 1" );
#if defined( BOOST_LITTLE_ENDIAN )
    bool const & result( reinterpret_cast<bool const &>( boolean ) );
#elif defined( BOOST_BIG_ENDIAN )
    typedef unsigned char bytes[ sizeof( boolean ) ];
    bytes const & input_bytes( reinterpret_cast<bytes const &>( boolean                              ) );
    bool  const & result     ( reinterpret_cast<bool  const &>( input_bytes[ sizeof( boolean ) - 1 ] ) );
#endif // BOOST_LITTLE_ENDIAN
    BOOST_ASSERT( static_cast<unsigned int>( result ) == boolean );
    return result;
}


bool Detail::convertToBool( float const source )
{
    /// \todo There is a problem of deciding how to treat floating point values
    /// with respect to conversion to bool. In general cases a simple comparison
    /// to zero as for all other types would be correct. However for normalised
    /// values (that we get from LFOs and automation) the value should actually
    /// be rounded (to 0 or 1). For now all our use cases fall into the later
    /// category so we specialize the float->bool conversion for this case.
    /// Reconsider how to solve this more cleanly. The Math module should
    /// preferably not know about any of this, this knowledge should probably be
    /// moved into parameter related functionality.
    ///                                       (07.07.2011.) (Domagoj Saric)
    BOOST_ASSERT( isNormalisedValue( source ) );
    int const result( Math::round( source ) );
    return makeBool( result );
}

bool Detail::convertToBool( double const source )
{
    BOOST_ASSERT( isNormalisedValue( static_cast<float>( source ) ) );
    int const result( Math::round( source ) );
    return makeBool( result );
}


////////////////////////////////////////////////////////////////////////////////
//
// dB2NormalisedLinear()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \ingroup UnitConversion Unit conversion
/// \brief Converts a value in dB's to a linear scale from 0 to 1 (inclusive).
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

#if defined( __APPLE__ ) || !defined( LE_HAS_NT2 ) // others NT2 implementation@vector.cpp
float LE_FASTCALL_ABI dB2NormalisedLinear( float         const dbValue ) { return std::pow( 10, dbValue / 20 ); }
float LE_FASTCALL_ABI dB2NormalisedLinear( std:: int8_t  const dbValue ) { return dB2NormalisedLinear( static_cast<float      >( dbValue ) ); }
float LE_FASTCALL_ABI dB2NormalisedLinear( std::uint8_t  const dbValue ) { return dB2NormalisedLinear( static_cast<std::int8_t>( dbValue ) ); }
float LE_FASTCALL_ABI dB2NormalisedLinear( std:: int16_t const dBValue ) { return dB2NormalisedLinear( static_cast<std::int8_t>( dBValue ) ); }
float LE_FASTCALL_ABI dB2NormalisedLinear( std::uint16_t const dBValue ) { return dB2NormalisedLinear( static_cast<std::int8_t>( dBValue ) ); }

////////////////////////////////////////////////////////////////////////////////
//
// normalisedLinear2dB()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \ingroup UnitConversion Unit conversion
/// \brief Converts a value in a linear scale from 0 to 1 (inclusive) to the dB
/// scale.
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

float LE_FASTCALL_ABI normalisedLinear2dB( float const linearNormalisedValue )
{
    BOOST_ASSERT_MSG( linearNormalisedValue >= 0, "Value out of range." );
    return 20 * log10( linearNormalisedValue );
}

double LE_FASTCALL_ABI normalisedLinear2dB( double const linearNormalisedValue )
{
    BOOST_ASSERT_MSG( linearNormalisedValue >= 0, "Value out of range." );
    return 20 * log10( linearNormalisedValue );
}


float LE_FASTCALL_ABI normalisedPower2dB( float const linearPowerValue )
{
    BOOST_ASSERT_MSG( linearPowerValue >= 0, "Value out of range." );
    return 10 * log10( linearPowerValue );
}


float LE_FASTCALL_ABI dB2NormalisedPower( float const dBValue )
{
    return std::pow( 10, dBValue / 10 );
}
#endif // (for broken)Apple Clang || !LE_HAS_NT2

////////////////////////////////////////////////////////////////////////////////
//
// normalisedLinear2Percentage()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \ingroup UnitConversion Unit conversion
/// \brief Converts a value in a linear scale from 0 to 1 (inclusive) to the
/// percentage scale.
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

LE_CONST_FUNCTION float LE_FASTCALL normalisedLinear2Percentage( float const normalisedFloatValue )
{
    // Implementation note:
    //   We do not assert that the passed 'normalisedFloatValue' is indeed
    // normalised as we want to accept values representing for example 120% or
    // -60%.
    //                                        (16.02.2010.) (Domagoj Saric)
    return 100 * normalisedFloatValue;
}

LE_CONST_FUNCTION double LE_FASTCALL normalisedLinear2Percentage( double const normalisedFloatValue ) { return 100 * normalisedFloatValue; }


LE_CONST_FUNCTION float LE_FASTCALL semitone2Interval12TET( float const semitones )
{
  //float const tonesPerOctave                        (  8 );
    float const semitonesPerOctave                    ( 12 );
  //float const frequencyMultiplicationFactorPerOctave(  2 );
    return Math::exp2( semitones / semitonesPerOctave );
}


LE_CONST_FUNCTION float LE_FASTCALL cents2Interval12TET( float        const cents ) { return Math::exp2( cents / 1200 ); }
LE_CONST_FUNCTION float LE_FASTCALL cents2Interval12TET( std::int16_t const cents ) { return cents2Interval12TET( static_cast<float>( cents ) ); }


LE_CONST_FUNCTION float LE_FASTCALL octaves2Interval12TET( std::int8_t const octaves )
{
    float const pitchScale( convert<float>( 1 << abs( octaves ) ) );
    return ( octaves < 0 ) ? ( 1 / pitchScale ) : ( pitchScale );
}


LE_CONST_FUNCTION float LE_FASTCALL interval12TET2Semitone( float const interval ) { return 12 * log2( interval ); }


/*
    Linear 2 Log:

    for (var i = 0; i <= 100; i++)
    {
        var value = Math.floor(-900 + 1000*Math.exp(i/10.857255959));
        document.write(value + "<br>");
    }


    function logslider(value)
    {
        // value will be between 0 and 100
        var min = 0;  var max = 100;
        // The result should be between 100 an 10000000
        var minv = Math.log(100);
        var maxv = Math.log(10000000);
        // calculate adjustment factor
        var scale = (maxv-minv) / (max-min);
        return Math.exp(minv + scale*(value-min));
    }

    The resulting values match a logarithmic scale:
    js> logslider(0);100.00000000000004js> logslider(10);316.22776601683825js> logslider(20);1000.0000000000007js> logslider(40);10000.00000000001js> logslider(60);100000.0000000002js> logslider(100);10000000.000000006

    http://software.intel.com/en-us/articles/integrating-fast-math-libraries-for-the-intel-pentiumr-4-processor
*/

#ifdef _MSC_VER
    #pragma optimize( "", on )
#endif // _MSC_VER

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} //namespace LE
//------------------------------------------------------------------------------
