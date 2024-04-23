////////////////////////////////////////////////////////////////////////////////
///
/// \file lexicalCast.hpp
/// ---------------------
///
/// String<->binary conversion routines.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef lexicalCast_hpp__432C7AD4_64B3_4058_BA65_5F4401951A08
#define lexicalCast_hpp__432C7AD4_64B3_4058_BA65_5F4401951A08
#pragma once
//------------------------------------------------------------------------------
#include "tchar.hpp"

#include "le/utility/platformSpecifics.hpp"

#include <cstdint>
#include <limits>
#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// Binary -> String
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( std:: int32_t , char * );
LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( long          , char * );
LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( std::uint32_t , char * );
LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( unsigned long , char * );
LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( float         , char * );
LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( double        , char * );

LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( float , std::uint8_t decimalPlaces, char * );
LE_NOTHROWNOALIAS unsigned int LE_FASTCALL lexical_cast( double, std::uint8_t decimalPlaces, char * );

inline unsigned int LE_FASTCALL lexical_cast( std:: int8_t  const value, char * const pBuffer ) { return lexical_cast( static_cast<std:: int32_t>( value ), pBuffer ); }
inline unsigned int LE_FASTCALL lexical_cast( std::uint8_t  const value, char * const pBuffer ) { return lexical_cast( static_cast<std::uint32_t>( value ), pBuffer ); }
inline unsigned int LE_FASTCALL lexical_cast( std::uint16_t const value, char * const pBuffer ) { return lexical_cast( static_cast<std::uint32_t>( value ), pBuffer ); }


template <typename T>
struct RequiredStringStorage
{
private:
    // Implementation note:
    //   numeric_limits<>::digits10 does not actually represent the maximum
    // amount of characters required to display a float value but the value
    // given for doubles is large enough for all our values.
    //                                        (04.10.2011.) (Domagoj Saric)
    // Implementation note:
    //   2 + std::numeric_limits<T>::digits10 gives the wrong result (8 instead
    // of 9) for floats.
    // http://connect.microsoft.com/VisualStudio/feedback/details/668921/std-numeric-limits-float-max-digits10-value-of-8-is-wrong-and-should-be-9
    // http://boost.2283326.n4.nabble.com/serialization-Serialisation-deserialisation-of-floating-point-values-td2604169i20.html
    //                                        (19.07.2011.) (Domagoj Saric)
    using value_type   = typename std::remove_const<typename std::remove_reference<T>::type>::type;
    using binary_type =  typename std::conditional
    <
        std::is_enum<value_type>::value,
        std::uint8_t,
        typename std::conditional
        <
            std::is_floating_point<value_type>::value,
            double,
            value_type
        >::type
    >::type;
    using numeric_limits = std::numeric_limits<binary_type>;
    static_assert( numeric_limits::is_specialized, "Internal inconsistency" );

public:
    static std::uint8_t const value = 2 + numeric_limits::digits * 3010 / 10000;
}; // struct RequiredStringStorage


////////////////////////////////////////////////////////////////////////////////
// String -> Binary
////////////////////////////////////////////////////////////////////////////////

template <typename T> T LE_FASTCALL lexical_cast( char const * );

template <> bool         LE_FASTCALL lexical_cast<bool        >( char const * valueString );
template <> int          LE_FASTCALL lexical_cast<int         >( char const * valueString );
template <> long         LE_FASTCALL lexical_cast<long        >( char const * valueString );
template <> unsigned int LE_FASTCALL lexical_cast<unsigned int>( char const * valueString );
template <> float        LE_FASTCALL lexical_cast<float       >( char const * valueString );
template <> double       LE_FASTCALL lexical_cast<double      >( char const * valueString );

template <> inline
std::uint8_t  LE_FASTCALL lexical_cast<std::uint8_t >( char const * const valueString ) { return static_cast<std::uint8_t >( lexical_cast<std::uint32_t>( valueString ) ); }
template <> inline
std::uint16_t LE_FASTCALL lexical_cast<std::uint16_t>( char const * const valueString ) { return static_cast<std::uint16_t>( lexical_cast<std::uint32_t>( valueString ) ); }

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // lexicalCast_hpp
