////////////////////////////////////////////////////////////////////////////////
///
/// \file hashing.hpp
/// -----------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef hashing_hpp__E9A2463E_AEB1_4FF1_B914_CF3B69712D33
#define hashing_hpp__E9A2463E_AEB1_4FF1_B914_CF3B69712D33
#pragma once
//------------------------------------------------------------------------------
#include <le/utility/abi.hpp>

#pragma warning( push )
#pragma warning( disable : 4100 ) // Unreferenced formal parameter.
#pragma warning( disable : 4245 ) // Conversion from 'int' to 'unsigned char'.
#pragma warning( disable : 4554 ) // Check operator precedence for possible error; use parentheses to clarify precedence.
#define BOOST_HASH_NO_OPTIMIZATION
#include <boost/hash/sha2.hpp>
#pragma warning( pop )
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

// http://en.wikipedia.org/wiki/SHA_hash_functions

using HashAlgorithm = boost::hashes::sha2<256>;
using Hasher        = HashAlgorithm::stream_hash<8>::type;
using Hash          = HashAlgorithm::digest_type;

LE_NOTHROWNOALIAS
Hash LE_FASTCALL_ABI createHash( std::uint8_t const *, std::size_t dataSize );

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // hashing_hpp
