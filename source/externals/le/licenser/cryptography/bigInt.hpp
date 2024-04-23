////////////////////////////////////////////////////////////////////////////////
///
/// \file bigInt.hpp
/// ----------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef bigInt_hpp__4AD63631_5E57_429B_A9B3_6B7AA1A42EE2
#define bigInt_hpp__4AD63631_5E57_429B_A9B3_6B7AA1A42EE2
#pragma once
//------------------------------------------------------------------------------
#include <boost/config.hpp>
#ifdef BOOST_NO_EXCEPTIONS
    #define BOOST_NO_IOSTREAM
    #ifndef BOOST_MSVC
        #define try        if ( true  )
        #define catch( x ) if ( false )
    #endif // BOOST_MSVC
#endif // BOOST_NO_EXCEPTIONS
#include <boost/multiprecision/cpp_int.hpp>
#ifndef BOOST_MSVC
    #undef try
    #undef catch
#endif // BOOST_MSVC

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

namespace Detail
{
    using namespace boost::multiprecision;
#ifdef NDEBUG
    auto const runtime_checked = unchecked;
#else
    auto const runtime_checked =   checked;
#endif // NDEBUG
    template <std::uint16_t sizeInBits>
    using BigUInt = number<cpp_int_backend<sizeInBits, sizeInBits, unsigned_magnitude, runtime_checked, void>>;
    template <std::uint16_t sizeInBits>
    using BigInt  = number<cpp_int_backend<sizeInBits, sizeInBits,   signed_magnitude, runtime_checked, void>>;
} // namespace Detail

using Detail::BigUInt;
using Detail::BigInt ;

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // bigInt_hpp
