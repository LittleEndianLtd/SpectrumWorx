////////////////////////////////////////////////////////////////////////////////
///
/// \file signing.hpp
/// -----------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef signing_hpp__BCB55DDE_2D0F_48E3_9753_65BASEF33433A
#define signing_hpp__BCB55DDE_2D0F_48E3_9753_65BASEF33433A
#pragma once
//------------------------------------------------------------------------------
#include "key.hpp"

#include <le/utility/abi.hpp>

#include <boost/range/iterator_range_core.hpp>

#include <array>
#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
/// \addtogroup Licenser
/// @{
namespace Lic
{
//------------------------------------------------------------------------------
namespace Cryptography /// \brief Basic cryptography required for the Licenser SDK
{
//------------------------------------------------------------------------------

using Signature = Key::RawBytes; ///< Byte array representing the signature

LE_NOTHROWNOALIAS
bool LE_FASTCALL_ABI verifySignature
(
    boost::iterator_range<char const *>         data,
    Signature                           const & signature,
    Key::RawBytes                       const & keyModulus
);

LE_NOTHROWNOALIAS
Signature LE_FASTCALL_ABI createSignature
(
    boost::iterator_range<char const *> data,
    Key::RawBytes const & keyModulus,
    Key::RawBytes const & keyPrivateExponent
);

LE_NOTHROWNOALIAS
Signature LE_FASTCALL_ABI createSignature
(
    char const        * data,
    std::size_t         dataLength,
    Key         const & key
);

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
/// @} // group Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // signing_hpp
