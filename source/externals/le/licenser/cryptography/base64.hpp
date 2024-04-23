////////////////////////////////////////////////////////////////////////////////
///
/// \file base64.hpp
/// ----------------
///
/// Base64 encoding/decoding.
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef base64_hpp__917842AD_206C_4610_A60F_6A5D2A3E8CD8
#define base64_hpp__917842AD_206C_4610_A60F_6A5D2A3E8CD8
#pragma once
//------------------------------------------------------------------------------
#include <le/utility/abi.hpp>

#include <cstdint>
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

LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL_ABI decode(         char const * pInput, std::uint16_t inputSize, std::uint8_t * pOutput, std::uint16_t outputSize );
LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL_ABI encode( std::uint8_t const * pInput, std::uint16_t inputSize,         char * pOutput, std::uint16_t outputSize );

LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL_ABI decodeBufferLength( std::uint16_t encodedMessageLength );
LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL_ABI encodeBufferLength( std::uint16_t decodedMessageLength );

//------------------------------------------------------------------------------
} // namespace Base64
//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // base64_hpp
