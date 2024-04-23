////////////////////////////////////////////////////////////////////////////////
///
/// \file key.hpp
/// -------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef key_hpp__EDC67A72_E5B2_4CFE_A9DD_C616E65D2177
#define key_hpp__EDC67A72_E5B2_4CFE_A9DD_C616E65D2177
#pragma once
//------------------------------------------------------------------------------
#include "bigInt.hpp"

#include <le/utility/abi.hpp>

#include <boost/assert.hpp>
#include <boost/config.hpp>

#if defined( _WIN32 ) && !defined( BOOST_NO_EXCEPTIONS )
#include <boost/utility/string_ref_fwd.hpp>
#endif // _WIN32 && !BOOST_NO_EXCEPTIONS

#include <array>
#include <cstdint>
#include <iterator>
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

/// \name Basic types
/// @{
using octet_t = std::uint8_t; ///< \brief Crypto-speak for a byte.

template <std::uint16_t length>
using OctetString = std::array<octet_t, length>; ///< \brief A string of octets.
/// @}


////////////////////////////////////////////////////////////////////////////////
///
/// \class Key
///
/// \brief RSA key-pair
///
/// http://en.wikipedia.org/wiki/Public-key_cryptography <BR>
/// http://en.wikipedia.org/wiki/RSA_(cryptosystem) <BR>
////////////////////////////////////////////////////////////////////////////////

struct Key
{
    /// \name Basic types
    /// @{
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST sizeInBits  = 4096          ;
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST sizeInBytes = sizeInBits / 8;

    using uint4096_t         = BigUInt<sizeInBits>;
    using modulus_t          = uint4096_t         ;
    using private_exponent_t = uint4096_t         ;
    using public_exponent_t  = std::uint32_t      ;

           modulus_t                                   modulus                 ;
    static public_exponent_t  BOOST_CONSTEXPR_OR_CONST publicExponent = 0x10001; // = 65537 = 2^16 + 1
           private_exponent_t                          privateExponent         ;

    using RawBytes = OctetString<sizeInBytes>;
    /// @}

    /// \name Loading/deserialization/construction
    /// @{
    LE_NOTHROWNOALIAS
    bool load( char const * pemFile );

    LE_NOTHROWNOALIAS
    static void load( uint4096_t & number, RawBytes const & rawLittleEndianKey );

    template <typename RawKeyContainer>
    static bool load( uint4096_t & number, RawKeyContainer const & rawLittleEndianKey )
    {
        if ( rawLittleEndianKey.size() != sizeInBytes )
            return false;
        load( number, reinterpret_cast<RawBytes const &>( rawLittleEndianKey[ 0 ] ) );
        return true;
    }
    LE_NOTHROWNOALIAS
    static void loadAsString( uint4096_t & number, RawBytes const & rawBigEndianNumber );
    /// @}

    /// \name Serialization
    /// @{
    LE_NOTHROWNOALIAS
    static void store( uint4096_t const & number, RawBytes & rawLittleEndianNumber );
    LE_NOTHROWNOALIAS
    static void storeAsString( uint4096_t const & number, RawBytes & rawBigEndianEndianNumber );

    template <typename RawKeyContainer>
    static bool store( uint4096_t const & number, RawKeyContainer & rawLittleEndianNumber )
    {
        if ( rawLittleEndianNumber.size() != sizeInBytes )
            return false;
        store( number, reinterpret_cast<RawBytes &>( rawLittleEndianNumber[ 0 ] ) );
        return true;
    }
    /// @}

    /// \name Raw bytes access
    /// @{
    using BigEndianRawBytesRange = boost::iterator_range<RawBytes::const_reverse_iterator>;
    LE_NOTHROWNOALIAS static BigEndianRawBytesRange bigEndianBytes   ( uint4096_t const & key );
    LE_NOTHROWNOALIAS static RawBytes const &       littleEndianBytes( uint4096_t const & key );
    /// @}

    /// \name Encryption/decryption
    /// @{
    LE_NOTHROWNOALIAS static RawBytes LE_FASTCALL_ABI publicEncrypt ( RawBytes const & message, modulus_t const & modulus )      ;
    LE_NOTHROWNOALIAS        RawBytes LE_FASTCALL_ABI privateDecrypt( RawBytes const & message                            ) const;
    /// @}

    /// \name Generation
    /// (included only for desktop platforms)
    /// @{
#if ( defined( _WIN32 ) && !defined( BOOST_NO_EXCEPTIONS ) ) || defined( DOXYGEN_ONLY )
    /// <B>Effect:</B> Generates an RSA key pair using the external OpenSSL binary. Writes the generated key pair in <VAR>name</VAR> .DER and .PEM files.<BR>
    /// <B>Preconditions:</B>The OpenSSL binary must be on the path.<BR>
    /// \throws std::runtime_error  <em>"OpenSSL not found" or "OpenSSL failure" or "failed to load the generated key from disk" (what() returns the appropariate description).</em>
    void generateWithOpenSSL( boost::string_ref name );
#endif // _WIN32 && !BOOST_NO_EXCEPTIONS
#if !defined( __ANDROID__ ) && !defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ )
    /// \details
    /// <B>Effect:</B> Generates an RSA keypair using internal logic. Does not require OpenSSL and cannot fail but is significantly slower and does not write the generated key pair to disk.<BR>
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI generate();
    /// \return Whether the object contains a validly geneated RSA key pair.
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI verify() const;
#endif // !Android && !iOS
    /// @}
}; // struct Key

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // key_hpp
