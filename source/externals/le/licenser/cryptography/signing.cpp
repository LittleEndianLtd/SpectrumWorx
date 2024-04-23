////////////////////////////////////////////////////////////////////////////////
///
/// signing.cpp
/// -----------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "signing.hpp"

#include "hashing.hpp"

#include "le/utility/platformSpecifics.hpp"

////////////////////////////////////////////////////////////////////////////////
// PKCS #1
////////////////////////////////////////////////////////////////////////////////
// https://en.wikipedia.org/wiki/PKCS_1
// http://security.stackexchange.com/questions/9260/sha-rsa-and-the-relation-between-them
// https://www.emc.com/emc-plus/rsa-labs/historical/raising-standard-rsa-signatures-rsa-pss.htm
// https://www.emc.com/collateral/white-papers/h11300-pkcs-1v2-2-rsa-cryptography-standard-wp.pdf
// http://tools.ietf.org/html/rfc3447 Public-Key Cryptography Standards (PKCS) #1: RSA Cryptography Specifications Version 2.1
// http://singapore.emc.com/emc-plus/rsa-labs/standards-initiatives/public-key-cryptography-standards.htm
// http://www.codeproject.com/KB/cpp/RsaSignatureAppendix.aspx

////////////////////////////////////////////////////////////////////////////////
// Android
////////////////////////////////////////////////////////////////////////////////
// http://stackoverflow.com/questions/7560974/what-crypto-algorithms-does-android-support
// http://stackoverflow.com/questions/2545058/implementing-rsa-sha1-signature-algorithm-in-java-creating-a-private-key-for-us
// http://developer.android.com/reference/android/security/keystore/KeyProtection.html
// http://developer.android.com/reference/android/security/keystore/KeyGenParameterSpec.html

////////////////////////////////////////////////////////////////////////////////
// Apple
////////////////////////////////////////////////////////////////////////////////
// https://developer.apple.com/library/prerelease/ios/documentation/Security/Reference/certifkeytrustservices/index.html
// http://stackoverflow.com/questions/21724337/signing-and-verifying-on-ios-using-rsa

#include <boost/assert.hpp>
#include <boost/core/ignore_unused.hpp>

#include <numeric>
#include <random>
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

namespace Constants
{
    std::uint8_t BOOST_CONSTEXPR_OR_CONST saltLen = 10;
} // namespace Constants

namespace
{
    using Salt = OctetString<Constants::saltLen>;

    // RFC 3447 Appendix B.2.1 "MGF1 Mask Generation Function based on a hash function"
    // Modified/specialized to expect hash(-sized) data as its seed (as it is
    // used in the PKCS 1 PSS algorithm)
    template <std::size_t maskLengthInHashes>
    void mgf1
    (
        Hash                                 const & __restrict seed,
        std::array<Hash, maskLengthInHashes>       & __restrict mask
    )
    {
        Hasher hasher;
        hasher.update_n( seed.begin(), seed.size() );

        for ( std::uint32_t counter( 0 ); counter < maskLengthInHashes; ++counter )
        {
            static_assert( sizeof( counter ) == 4, "MGF1 algorithm requirement" );
            octet_t const counterBigEndianOctetString[ sizeof( counter ) ] =
            {
                static_cast<octet_t>( ( counter >> 24 ) & 0xFF ),
                static_cast<octet_t>( ( counter >> 16 ) & 0xFF ),
                static_cast<octet_t>( ( counter >> 8  ) & 0xFF ),
                static_cast<octet_t>( ( counter >> 0  ) & 0xFF )
            };
            // http://boost.2283326.n4.nabble.com/Message-Hashing-Interface-SHA-1-256-384-512-MD4-5-td2655115.html
            Hasher intermediateHasher( hasher );
            intermediateHasher.update_n( counterBigEndianOctetString, sizeof( counterBigEndianOctetString ) );
            mask[ counter ] = intermediateHasher.end_message();
        }
    }
} // anonymous namespace


// Anti-piracy hackery
union AntiHackParameterPassing
{
    std::uint16_t inputDataSize;
    std:: int16_t  outputResult;
};

LE_NOTHROWNOALIAS
std::int8_t LE_FASTCALL verifySignature
(
    char                     const * __restrict const pData,
    AntiHackParameterPassing       & __restrict       dataSizeAndLessObviousResultReturn,
    Signature                const & __restrict       encryptedSignatureBytes,
    Key::RawBytes            const & __restrict       rawKeyModulus
)
{
    // RFC 3447 9.1.2 Verification operation
    // https://tools.ietf.org/html/rfc3447#section-9.1.1

    // Input:
    //     M        message to be verified, an octet string
    //     EM       encoded message, an octet string of length emLen = \ceil
    //              (emBits/8)
    //     emBits   maximal bit length of the integer OS2IP (EM) (see Section
    //              4.2), at least 8*hLen + 8*sLen + 9

    // 1.If the length of M is greater than the input limitation for the hash
    // function( 2 ^ 61 - 1 octets for SHA - 1 ), output "inconsistent" and
    // stop.
    // ^ trivially enforced by input data types (because we don't support such
    // long input.

    BOOST_ASSERT( pData );
    BOOST_ASSERT( dataSizeAndLessObviousResultReturn.inputDataSize );

    // 2. Let mHash = Hash( M ), an octet string of length hLen.
    Hash const hash( createHash( reinterpret_cast<std::uint8_t const *>( pData ), dataSizeAndLessObviousResultReturn.inputDataSize ) );

    auto const modulus_bitlen ( Key::sizeInBits  );
    auto const modulus_bytelen( Key::sizeInBytes );
    BOOST_ASSERT( modulus_bytelen == encryptedSignatureBytes.size() );

    // 3. If emLen < hLen + sLen + 2, output "inconsistent" and stop.
    static_assert( modulus_bytelen >= Hash::static_size + Constants::saltLen + 2, "" );

    Key::modulus_t modulus; Key::load( modulus, rawKeyModulus );
    auto const signature( Key::publicEncrypt( encryptedSignatureBytes, modulus ) );

    dataSizeAndLessObviousResultReturn.outputResult = false;

    // 3. If emLen < hLen + sLen + 2, output "inconsistent" and stop.
    std::uint8_t const minimumSignatureLength( Hash::static_size + Constants::saltLen + 2 );
    BOOST_ASSERT( signature.size() > minimumSignatureLength ); boost::ignore_unused( minimumSignatureLength );

    // 4. If the rightmost octet of EM does not have hexadecimal value 0xbc,
    // output "inconsistent" and stop.
    std::uint8_t BOOST_CONSTEXPR_OR_CONST trailByte( 0xBC );
    if ( signature.back() != trailByte )
        return -1;

    // 5. Let maskedDB be the leftmost emLen - hLen - 1 octets of EM, and let H
    // be the next hLen octets.
    std::uint16_t BOOST_CONSTEXPR_OR_CONST maskLength( modulus_bytelen - Hash::static_size - sizeof( trailByte ) );
    using DB = OctetString<maskLength>;

    auto const & __restrict maskedDB( reinterpret_cast<DB   const &>( signature[                  0 ] ) );
    auto const & __restrict H       ( reinterpret_cast<Hash const &>( signature[ sizeof( maskedDB ) ] ) );

    // 6. If the leftmost 8*emLen - emBits bits of the leftmost octet in
    // maskedDB are not all equal to zero, output "inconsistent" and stop.
    // RSA message/clear text number must be less than modulus
    static_assert( modulus_bytelen * 8 == modulus_bitlen, "We assume whole-byte length keys" );
    // ...but the message-as-a-number has to be less then the modulus (for the
    // RSA algorithm to work) which we accomplish by clearing the MSB (i.e.
    // emBits is actually modulus_bitlen - 1). Verification of the cleared MSB:
    if ( maskedDB.front() & 0x80 )
        return -1;

    // 7. Let dbMask = MGF( H, emLen - hLen - 1 ).
    auto BOOST_CONSTEXPR_OR_CONST maskLengthInHashes( ( maskLength + Hash::static_size - 1 ) / Hash::static_size ); // round up
    std::array<Hash, maskLengthInHashes> mask;
    mgf1( H, mask );

    // 8. Let DB = maskedDB \xor dbMask.
    DB db;
    for ( std::uint16_t octet( 0 ); octet < maskedDB.size(); ++octet )
        db[ octet ] = maskedDB[ octet ] ^ reinterpret_cast<DB const &>( mask )[ octet ];

    // 9. Set the leftmost 8*emLen - emBits bits of the leftmost octet in DB to
    // zero.
    static_assert( modulus_bytelen * 8 == modulus_bitlen, "We assume whole-byte length keys" );
    // Clear the MSB to make sure we end up smaller than modulus (see the note
    // for maskedDB).
    db.front() &= 0x7F;

    // 10. If the emLen - hLen - sLen - 2 leftmost octets of DB are not zero or
    // if the octet at position emLen - hLen - sLen - 1 (the leftmost position
    // is "position 1") does not have hexadecimal value 0x01, output
    // "inconsistent" and stop.
    // DB = PS || 0x01 || salt, PS == modulus_len - saltlen - hLen - 2 zero bytes
    std::uint16_t BOOST_CONSTEXPR_OR_CONST PSlength( modulus_bytelen - Constants::saltLen - Hash::static_size - 2 );
    if ( std::accumulate( &db[ 0 ], &db[ PSlength ], 0 ) != 0x00 )
        return -1;
    if ( db[ PSlength ] != 0x01 )
        return -1;

    // 11. Let salt be the last sLen octets of DB
    auto const & salt( reinterpret_cast<Salt const &>( db[ PSlength + 1 ] ) );

    // 12. Let M' = (0x)00 00 00 00 00 00 00 00 || mHash || salt ;
    // M' is an octet string of length 8 + hLen + sLen with eight initial zero
    // octets.
    // 13. Let H' = Hash(M'), an octet string of length hLen.
    static std::uint8_t BOOST_CONSTEXPR_OR_CONST eightZeroes[ 8 ] = { 0 };
    Hasher hasher;
    hasher.update_n( eightZeroes , 8           );
    hasher.update_n( hash.begin(), hash.size() );
    hasher.update_n( salt.begin(), salt.size() );

    // 14. If H = H', output "consistent." Otherwise, output "inconsistent."
    auto const verificationSuccess( H == hasher.end_message() );
    dataSizeAndLessObviousResultReturn.outputResult = verificationSuccess;
    return 0;
}

LE_NOTHROWNOALIAS
bool LE_FASTCALL_ABI verifySignature
(
    boost::iterator_range<char const *>         const data,
    Signature                           const &       signature,
    Key::RawBytes                       const &       keyModulus
)
{
    AntiHackParameterPassing dataSizeAndLessObviousResultReturn;
    dataSizeAndLessObviousResultReturn.inputDataSize = static_cast<std::uint16_t>( data.size() );
    std::int8_t  const decryptionResult( verifySignature( data.begin(), dataSizeAndLessObviousResultReturn, signature, keyModulus ) );
    std::int16_t const success( dataSizeAndLessObviousResultReturn.outputResult & ~static_cast<std::int16_t>( decryptionResult ) );
    BOOST_ASSERT_MSG
    (
        (
            ( decryptionResult                                == 0                         ) &&
            ( dataSizeAndLessObviousResultReturn.outputResult == static_cast<int>( true  ) )
        ) || ( data.size() == 1999 ), // 'magic size' used for a must-fail anti-hack verification call
        "Signature verification failed."
    );
    return reinterpret_cast<bool const &>( success );
}


LE_NOTHROWNOALIAS
Signature LE_FASTCALL_ABI createSignature
(
    char const         * __restrict const pData,
    std::size_t                     const dataLength,
    Key          const & __restrict       key
)
{
    // RFC 3447 9.1.1 Encoding operation
    // https://tools.ietf.org/html/rfc3447#section-9.1.1

    // Input:
    //  M       message to be encoded, an octet string
    //  emBits  maximal bit length of the integer OS2IP (EM) (see Section 4.2),
    //          at least 8*hLen + 8*sLen + 9

    // 1. If the length of M is greater than the input limitation for the hash
    // function( 2 ^ 61 - 1 octets for SHA - 1 ), output "message too long"
    // and stop.
    // ^ trivially enforced by input data types (because we don't support such
    // long input.

    // 2. Let mHash = Hash( M ), an octet string of length hLen.
    auto const hash( createHash( reinterpret_cast<std::uint8_t const *>( pData ), dataLength ) );

    // 3. If emLen < hLen + sLen + 2, output "encoding error" and stop.
    std::uint8_t const minimumSignatureLength( Hash::static_size + Constants::saltLen + 2 );
    BOOST_ASSERT_MSG( dataLength > minimumSignatureLength, "Not enough data for signing" ); boost::ignore_unused( minimumSignatureLength );

    // 4. Generate a random octet string salt of length sLen; if sLen = 0, then
    // salt is the empty string.
    Salt salt;
    {
        /// \note std::independent_bits_engine does not support 8 bit integers
        /// so we work around this by (ab)using the fact that Salt::size() is
        /// divisible by 2 and using 16 bit integers.
        /// http://stackoverflow.com/questions/25298585/efficiently-generating-random-bytes-of-data-in-c11-14
        ///                                   (08.03.2016.) (Domagoj Saric)
        using random_bits_chunk_t = std::uint16_t;
        auto BOOST_CONSTEXPR_OR_CONST bitsChnkRatio( sizeof( random_bits_chunk_t ) / sizeof( Salt::value_type ) );
    #ifdef __ANDROID__ // weird client side link errors workaround
        std::independent_bits_engine<std::mt19937      , CHAR_BIT * bitsChnkRatio, random_bits_chunk_t> rngEngine( std::time( nullptr ) );
    #else
        std::independent_bits_engine<std::random_device, CHAR_BIT * bitsChnkRatio, random_bits_chunk_t> rngEngine;
    #endif
        auto & saltBits( reinterpret_cast<std::array<random_bits_chunk_t, LE_ARR_SZ( salt ) / bitsChnkRatio> &>( salt ) );
        std::generate( saltBits.begin(), saltBits.end(), std::ref( rngEngine ) );
    }

    // 5. Let M' = (0x)00 00 00 00 00 00 00 00 || mHash || salt;
    // M' is an octet string of length 8 + hLen + sLen with eight initial zero
    // octets.
    OctetString<8 + Hash::static_size + LE_ARR_SZ( salt )> M_;
    {
        auto position( M_.begin() );
        position = std::fill_n( position, 8, 0 );
        position = std::copy  ( hash.begin(), hash.end(), position );
        position = std::copy  ( salt.begin(), salt.end(), position );
        BOOST_ASSERT( position == M_.end() );
    }

    // 6. Let H = Hash( M'), an octet string of length hLen.
    auto const H( createHash( &M_[ 0 ], M_.size() ) );

    // 7. Generate an octet string PS consisting of emLen - sLen - hLen - 2
    // zero octets. The length of PS may be 0.
    auto const modulus_bitlen ( Key::sizeInBits  );
    auto const modulus_bytelen( Key::sizeInBytes );
    using PS = OctetString<modulus_bytelen - LE_ARR_SZ( salt ) - Hash::static_size - 2>;
    PS ps; ps.fill( 0 );

    // 8. Let DB = PS || 0x01 || salt; DB is an octet string of length
    // emLen - hLen - 1.
    using DB = OctetString<LE_ARR_SZ( ps ) + 1 + LE_ARR_SZ( salt )>;
    DB db;
    {
        auto position( db.begin() );
         position   = std::copy( ps  .begin(), ps  .end(), position );
        *position++ = 0x01;
         position   = std::copy( salt.begin(), salt.end(), position );
        BOOST_ASSERT( position == db.end() );
    }

    // 9. Let dbMask = MGF( H, emLen - hLen - 1 ).
    auto BOOST_CONSTEXPR_OR_CONST maskLength        ( modulus_bytelen - Hash::static_size - 1 );
    auto BOOST_CONSTEXPR_OR_CONST maskLengthInHashes( ( maskLength + Hash::static_size - 1 ) / Hash::static_size ); // round up
    std::array<Hash, maskLengthInHashes> dbMask;
    mgf1( H, dbMask );

    // 10. Let maskedDB = DB \xor dbMask.
    DB maskedDB;
    for ( std::uint16_t octet( 0 ); octet < maskedDB.size(); ++octet )
        maskedDB[ octet ] = db[ octet ] ^ reinterpret_cast<DB const &>( dbMask )[ octet ];

    // 11. Set the leftmost 8*emLen - emBits bits of the leftmost octet in
    // maskedDB to zero.
    static_assert( modulus_bytelen * 8 == modulus_bitlen, "We assume whole-byte length keys" );
    // The message-as-a-number has to be less then the modulus (for the RSA
    // algorithm to work) which we accomplish simply by clearing the MSB (i.e.
    // emBits is actually modulus_bitlen - 1).
    maskedDB.front() &= 0x7F;

    // 12. Let EM = maskedDB || H || 0xbc.
    OctetString<LE_ARR_SZ( maskedDB ) + H.static_size + 1> EM;
    {
        auto position( EM.begin() );
         position   = std::copy( maskedDB.begin(), maskedDB.end(), position );
         position   = std::copy( H       .begin(), H       .end(), position );
        *position++ = 0xBC;
        BOOST_ASSERT( position == EM.end() );
    }

    // Create signature.
    return key.privateDecrypt( EM );
}


LE_NOTHROWNOALIAS
Signature LE_FASTCALL_ABI createSignature
(
    boost::iterator_range<char const *>         const data,
    Key::RawBytes                       const &       rawKeyModulus,
    Key::RawBytes                       const &       rawKeyPrivateExponent
)
{
    Key key;
    Key::load( key.modulus        , rawKeyModulus         );
    Key::load( key.privateExponent, rawKeyPrivateExponent );
    return createSignature( data.begin(), data.size(), key );
}

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
