////////////////////////////////////////////////////////////////////////////////
///
/// base64.cpp
/// ----------
///
/// Base64 encoding/decoding. Based on code by Matt Gallagher:
/// http://cocoawithlove.com/2009/06/base64-encoding-options-on-mac-and.html
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "base64.hpp"

#include "le/utility/platformSpecifics.hpp"

#include <boost/assert.hpp>
#include <boost/core/ignore_unused.hpp>

#include <array>
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Lic
{
//------------------------------------------------------------------------------
namespace Cryptography
{
//------------------------------------------------------------------------------
namespace Base64
{
//------------------------------------------------------------------------------

namespace Constants
{
    // Mapping from 6 bit pattern to ASCII character.
    LE_WEAK_SYMBOL_CONST
    std::array<char, 64 + 1> const encodeLookup =
        { "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" };

    // Marker for "masked-out" areas of the base64DecodeLookup mapping.
    std::uint8_t const xx( 0xFF );

    // Mapping from ASCII character to 6 bit pattern.
    LE_WEAK_SYMBOL_CONST
    std::array<std::uint8_t const, 256> const decodeLookup =
    {{
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 62, xx, xx, xx, 63, 
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, xx, xx, xx, xx, xx, xx, 
        xx,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, xx, xx, xx, xx, xx, 
        xx, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
        xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, xx, 
    }};

    // Fundamental sizes of the binary and base64 encode/decode units in bytes
    std::uint8_t const binaryUnitSize( 3 );
    std::uint8_t const base64UnitSize( 4 );
} // namespace Constants


////////////////////////////////////////////////////////////////////////////////
//
// decode()
// --------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \return Number of bytes decoded.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS
std::uint16_t LE_FASTCALL_ABI decode
(
	char          const * __restrict const inputBuffer,
    std::uint16_t                    const inputLength,
    std::uint8_t        * __restrict const outputBuffer,
    std::uint16_t                    const outputLength
)
{
    using namespace Constants;

    /// \note We do not use (or support) '=' padded Base64 data and so do not
    /// expect/require that input length be divisible by 4.
    ///                                       (10.03.2016.) (Domagoj Saric)
    //BOOST_ASSERT_MSG( inputLength % 4 == 0, "Invalid base64 input" );

    /// \note Quick-hack: support exact input buffer lengths (decodeBufferLength
    /// can be off by base64UnitSize - 1 due to a 'content dependent' result).
    ///                                       (10.03.2016.) (Domagoj Saric)
    auto const requiredOutputBufferSize( ( decodeBufferLength( inputLength ) / base64UnitSize ) * base64UnitSize );
    BOOST_ASSERT_MSG( outputLength <  inputLength             , "Invalid buffers"       );
    BOOST_ASSERT_MSG( outputLength >= requiredOutputBufferSize, "Invalid output buffer" );
    if ( outputLength < requiredOutputBufferSize )
        return 0; //...mrmlj...

	auto       inputPosition ( inputBuffer                 );
	auto const inputEnd      ( inputPosition + inputLength );
	auto       outputPosition( outputBuffer                );
	while ( inputPosition != inputEnd )
	{
		// Accumulate 4 valid characters.
		std::array<std::uint8_t, base64UnitSize> accumulated;
        std::uint8_t accumulateIndex( 0 );
		while ( inputPosition != inputEnd )
		{
            auto const inputCharacter( *inputPosition++ ); LE_ASSUME( inputCharacter > 0 );
			auto const decodedValue  ( decodeLookup[ static_cast<std::uint8_t>( inputCharacter ) ] );
            // Assume no padding or CRLFs were added.
            LE_ASSUME( decodedValue != xx );
            accumulated[ accumulateIndex++ ] = decodedValue;
            if ( accumulateIndex == base64UnitSize )
                break;
		}

		// Store the 6 bits from each of the 4 characters as 3 bytes.
        /// \todo Investigate std::uint8_t overflows for the last two bytes.
        ///                                   (24.08.2011.) (Domagoj Saric)
        std::array<std::uint8_t, binaryUnitSize> decoded;
		decoded[ 0 ] =   ( accumulated[ 0 ] << 2 ) | ( accumulated[ 1 ] >> 4 )         ;
        decoded[ 1 ] = ( ( accumulated[ 1 ] << 4 ) | ( accumulated[ 2 ] >> 2 ) ) & 0xFF;
        decoded[ 2 ] = ( ( accumulated[ 2 ] << 6 ) | ( accumulated[ 3 ]      ) ) & 0xFF;
        LE_ASSUME( accumulateIndex >= 2 ); // Wikipedia claims at most two padding characters...
        std::uint8_t const newCharacters( accumulateIndex - 1 );
        LE_ASSUME( newCharacters < 4 );
        for ( std::uint8_t i( 0 ); i < newCharacters; ++i )
            *outputPosition++ = decoded[ i ];
	}

    auto const decodedSize( static_cast<std::uint16_t>( outputPosition - outputBuffer ) );
    BOOST_ASSERT( decodedSize <= outputLength );
    boost::ignore_unused( outputLength );
    return decodedSize;
}


////////////////////////////////////////////////////////////////////////////////
//
// base64Encode()
// --------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \return Number of bytes encoded.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS
std::uint16_t LE_FASTCALL_ABI encode
(
    std::uint8_t  const * __restrict const inputBuffer ,
    std::uint16_t                    const inputLength ,
    char                * __restrict const outputBuffer,
    std::uint16_t                    const outputLength
)
{
    using namespace Constants;

    auto const requiredOutputBufferSize( encodeBufferLength( inputLength ) );
    BOOST_ASSERT_MSG( outputLength > inputLength              , "Invalid buffers"       );
    BOOST_ASSERT_MSG( outputLength >= requiredOutputBufferSize, "Invalid output buffer" );
    if ( outputLength < requiredOutputBufferSize )
        return 0; //...mrmlj...

	auto       inputPosition ( inputBuffer                 );
	auto const inputEnd      ( inputPosition + inputLength );
	auto       outputPosition( outputBuffer                );
	while ( inputPosition < ( inputEnd - ( binaryUnitSize - 1 ) ) )
	{
		// Inner loop: turn 48 bytes into 64 base64 characters
		*outputPosition++ = encodeLookup[  (* inputPosition       & 0xFC) >> 2 ];
		*outputPosition++ = encodeLookup[ ((* inputPosition       & 0x03) << 4) | ((*(inputPosition + 1) & 0xF0) >> 4) ];
		*outputPosition++ = encodeLookup[ ((*(inputPosition + 1 ) & 0x0F) << 2) | ((*(inputPosition + 2) & 0xC0) >> 6) ];
		*outputPosition++ = encodeLookup[ *(  inputPosition + 2 ) & 0x3F ];

        inputPosition += binaryUnitSize;
	}

    auto const tailSize( static_cast<std::uint16_t>( inputEnd - inputPosition ) );
    switch ( tailSize )
    {
        case 0:
            break;

        case 1:
            *outputPosition++ = encodeLookup[ (*inputPosition & 0xFC) >> 2 ];
            *outputPosition++ = encodeLookup[ (*inputPosition & 0x03) << 4 ];
            break;

        case 2:
            *outputPosition++ = encodeLookup[  (* inputPosition      & 0xFC) >> 2 ];
            *outputPosition++ = encodeLookup[ ((* inputPosition      & 0x03) << 4) | ((*(inputPosition + 1) & 0xF0) >> 4) ];
            *outputPosition++ = encodeLookup[  (*(inputPosition + 1) & 0x0F) << 2 ];
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    auto const encodedSize( static_cast<std::uint16_t>( outputPosition - outputBuffer ) );
    BOOST_ASSERT( encodedSize <= outputLength );
    return encodedSize;
}


LE_NOTHROWNOALIAS
std::uint16_t LE_FASTCALL_ABI decodeBufferLength( std::uint16_t const encodedMessageLength )
{
    using namespace Constants;
    std::uint16_t const requiredOutputBufferSize
    (
	    ( encodedMessageLength + base64UnitSize - 1 ) / base64UnitSize  // round up divison
		    *
        binaryUnitSize
    );
    return requiredOutputBufferSize;
}

LE_NOTHROWNOALIAS
std::uint16_t LE_FASTCALL_ABI encodeBufferLength( std::uint16_t const decodedMessageLength )
{
    using namespace Constants;
    std::uint16_t const requiredOutputBufferSize
    (
        ( decodedMessageLength + binaryUnitSize - 1 ) / binaryUnitSize  // round up divison
            *
        base64UnitSize
    );
    return requiredOutputBufferSize;
}

//------------------------------------------------------------------------------
} // namespace Base64
//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
