////////////////////////////////////////////////////////////////////////////////
///
/// hashing.cpp
/// -----------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "hashing.hpp"

#include "le/utility/platformSpecifics.hpp"

////////////////////////////////////////////////////////////////////////////////
// Hashers
////////////////////////////////////////////////////////////////////////////////
// http://www.cs.mcgill.ca/~smcmur/hash https://svn.boost.org/svn/boost/sandbox/hash
// http://create.stephan-brumme.com/hash-library
// https://github.com/lyokato/cpp-cryptlite
// http://aldrin.co/crypto-primitives.html
// http://codereview.stackexchange.com/questions/13288/perform-sha-256-on-input
// http://www.zedwood.com/article/cpp-sha256-function
////////////////////////////////////////////////////////////////////////////////

#include <boost/hash/compute_digest.hpp>
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

LE_NOTHROWNOALIAS
Hash LE_FASTCALL_ABI createHash( std::uint8_t const * const pData, std::size_t const dataSize )
{
    // Smaller-type check /RTCc has to be disabled boost::hashes::detail::unbounded_shifter
    return boost::hashes::compute_digest_n<HashAlgorithm>( pData, dataSize );
}

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
