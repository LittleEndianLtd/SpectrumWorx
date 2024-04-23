////////////////////////////////////////////////////////////////////////////////
///
/// key.cpp
/// -------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// Open Source Cryptographic Libraries: http://www.homeport.org/~adam/crypto
// http://www.carljohansen.co.uk/bigint/bigintrsa.aspx
// http://www.oakcircle.com/xint_docs/ex_rsa.html
// http://code.google.com/p/librsa
// https://github.com/ouyoutouchmytralala/RSA
//------------------------------------------------------------------------------
#include "key.hpp"

#include "le/utility/platformSpecifics.hpp"
#include "le/utility/filesystem.hpp"

#include <boost/detail/endian.hpp>
#define BOOST_MP_RANDOM_HPP
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/multiprecision/miller_rabin.hpp>
#include <boost/random/independent_bits.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/search.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/utility/string_ref.hpp>

#if defined( _WIN32 ) && !defined( BOOST_NO_EXCEPTIONS )
#include <process.h>
#endif // _WIN32 && !BOOST_NO_EXCEPTIONS

#include <algorithm>
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

#ifdef BOOST_BIG_ENDIAN
    #error not implemented
#endif // BOOST_BIG_ENDIAN

LE_NOTHROWNOALIAS
void Key::load( uint4096_t & key, RawBytes const & rawLittleEndianKey )
{
    using namespace boost::multiprecision;
#if 0 // https://svn.boost.org/trac/boost/ticket/9235
    BOOST_VERIFY( &import_bits( key, rawLittleEndianKey.begin(), rawLittleEndianKey.end(), false ) == &key );
#else
    key.backend().resize( key.backend().internal_limb_count, 0 );
    std::copy( rawLittleEndianKey.begin(), rawLittleEndianKey.end(), reinterpret_cast<octet_t *>( key.backend().limbs() ) );
#endif
}

LE_NOTHROWNOALIAS
void Key::loadAsString( uint4096_t & number, RawBytes const & rawBigEndianNumber )
{
    using namespace boost::multiprecision;
#if 0 // https://svn.boost.org/trac/boost/ticket/9235
    BOOST_VERIFY( &import_bits( number, rawBigEndianNumber.begin(), rawBigEndianNumber.end(), true ) == &key );
#else
    number.backend().resize( number.backend().internal_limb_count, 0 );
    std::reverse_copy( rawBigEndianNumber.begin(), rawBigEndianNumber.end(), reinterpret_cast<octet_t *>( number.backend().limbs() ) );
#endif
}

LE_NOTHROWNOALIAS
void Key::store( uint4096_t const & key, RawBytes & rawLittleEndianKey )
{
    using namespace boost::multiprecision;
#if 0 // https://svn.boost.org/trac/boost/ticket/9235
    BOOST_VERIFY( export_bits( key, rawLittleEndianKey.begin(), 8, false ) == rawLittleEndianKey.end() );
#else
    auto const & rawBytes( *reinterpret_cast<RawBytes const *>( key.backend().limbs() ) );
    BOOST_VERIFY( std::copy( rawBytes.begin(), rawBytes.end(), rawLittleEndianKey.begin() ) == rawLittleEndianKey.end() );
#endif
}
LE_NOTHROWNOALIAS
void Key::storeAsString( uint4096_t const & number, RawBytes & rawBigEndianEndianNumber )
{
    using namespace boost::multiprecision;
#if 0 // https://svn.boost.org/trac/boost/ticket/9235
    BOOST_VERIFY( export_bits( number, rawBigEndianEndianNumber.begin(), 8, true ) == rawBigEndianEndianNumber.end() );
#else
    auto const & rawBytes( *reinterpret_cast<RawBytes const *>( number.backend().limbs() ) );
    BOOST_VERIFY( std::reverse_copy( rawBytes.begin(), rawBytes.end(), rawBigEndianEndianNumber.begin() ) == rawBigEndianEndianNumber.end() );
#endif
}

LE_NOTHROWNOALIAS
Key::BigEndianRawBytesRange Key::bigEndianBytes( uint4096_t const & key )
{
    auto const & rawBytes( littleEndianBytes( key ) );
    return { rawBytes.rbegin(), rawBytes.rend() };
}
LE_NOTHROWNOALIAS
Key::RawBytes const & Key::littleEndianBytes( uint4096_t const & key ) { return *reinterpret_cast<RawBytes const *>( key.backend().limbs() ); }


namespace
{
    using Data = boost::iterator_range<char const *>;
    LE_NOTHROW
    void LE_FASTCALL doLoad
    (
        Key::uint4096_t   &       number,
        boost::string_ref   const numberName,
        boost::string_ref   const nextNumberName,
        Data              &       pemData
    )
    {
        static std::array<char, 4> BOOST_CONSTEXPR_OR_CONST nonNumberChars{{ ' ', ':', '\r', '\n' }};
        static std::array<char, 2> BOOST_CONSTEXPR_OR_CONST hexPrefix     {{ '0', 'x'             }};
        std::array<char, LE_ARR_SZ( hexPrefix ) + ( Key::sizeInBytes + /*OpenSSL often prepends a zero byte*/ 1 ) * /*two hex-chars per byte*/ 2 + sizeof( '\0' )> numberString;

        auto const pInputBegin( boost::range::search( pemData                           , numberName     ) + numberName.length() );
        auto const pInputEnd  ( boost::range::search( Data( pInputBegin, pemData.end() ), nextNumberName )                       );

        *std::copy_if
        (
            pInputBegin,
            pInputEnd  ,
            boost::range::copy( hexPrefix, numberString.begin() ),
            []( char const value ) { return boost::range::find( nonNumberChars, value ) == nonNumberChars.end(); }
        ) = '\0';
        number.assign( &numberString[ 0 ] );

        pemData = Data( pInputEnd, pemData.end() );
    }
    LE_NOTHROW
    void LE_FASTCALL doLoad
    (
        Key::public_exponent_t &       number,
        boost::string_ref        const numberName,
        boost::string_ref        const nextNumberName,
        Data                   &       pemData
    )
    {
        auto const pInputBegin( boost::range::search( pemData                           , numberName     ) + numberName.length() );
        auto const pInputEnd  ( boost::range::search( Data( pInputBegin, pemData.end() ), nextNumberName )                       );
        number = std::atoi( pInputBegin );

        pemData = Data( pInputEnd, pemData.end() );
    }

} // anonymous namespace

LE_NOTHROWNOALIAS
bool Key::load( char const * const pemFile )
{
    auto const pem( Utility::File::map<Utility::AbsolutePath>( pemFile ) );
    if ( !pem )
        return false;

    std::uint32_t pubExp;

    Data data( pem );
    doLoad( modulus        , "modulus:"        , "publicExponent:" , data );
    doLoad( pubExp         , "publicExponent:" , "privateExponent:", data );
    doLoad( privateExponent, "privateExponent:", "prime1:"         , data );

    BOOST_ASSERT( pubExp == publicExponent );

    return true;
}

LE_OPTIMIZE_FOR_SPEED_BEGIN()
LE_NOTHROWNOALIAS LE_HOT
Key::RawBytes LE_FASTCALL_ABI Key::publicEncrypt( RawBytes const & __restrict clearTextMessage, modulus_t const & __restrict modulus )
{
    using namespace boost::multiprecision;

    uint4096_t       messageAsNumber;  loadAsString( messageAsNumber, clearTextMessage          ); BOOST_ASSERT( modulus > messageAsNumber  ); BOOST_ASSERT( gcd( messageAsNumber, modulus ) == 1 );
    uint4096_t const encryptedMessage( powm        ( messageAsNumber, publicExponent, modulus ) ); BOOST_ASSERT( modulus > encryptedMessage );

    RawBytes result; storeAsString( encryptedMessage, result ); return result;
}

LE_NOTHROWNOALIAS LE_HOT
Key::RawBytes LE_FASTCALL_ABI Key::privateDecrypt( RawBytes const & __restrict encryptedMessage ) const
{
    using namespace boost::multiprecision;

    /// \todo Optimise: https://svn.boost.org/trac/boost/ticket/12022
    ///                                       (24.02.2016.) (Domagoj Saric)
    uint4096_t       messageAsNumber;  loadAsString( messageAsNumber, encryptedMessage           ); BOOST_ASSERT( modulus > messageAsNumber  );
    uint4096_t const decryptedMessage( powm        ( messageAsNumber, privateExponent, modulus ) ); BOOST_ASSERT( modulus > decryptedMessage );

    RawBytes message; storeAsString( decryptedMessage, message ); return message;
}
LE_OPTIMIZE_FOR_SPEED_END()


#if defined( _WIN32 ) && !defined( BOOST_NO_EXCEPTIONS )
namespace
{
    BOOST_NORETURN
    void reportRuntimeError( char const * const description ) { throw std::runtime_error( description ); }
} // anonymous namespace

void Key::generateWithOpenSSL( boost::string_ref const keyName )
{
    char outputDERFileName[ /*MAX_PATH*/260 ];
    {
        std::strcpy( boost::copy( keyName, outputDERFileName ), ".der" );

        char keygenBits  [ 32 ]; std::sprintf( keygenBits  , "rsa_keygen_bits:%hu" , Key::sizeInBits     );
        char keygenPubExp[ 32 ]; std::sprintf( keygenPubExp, "rsa_keygen_pubexp:%u", Key::publicExponent );

        intptr_t const generate_result
        (
            ::_spawnl
            (
                _P_WAIT,
                "openssl.exe",
                "openssl.exe",
                "genpkey"    ,
                "-algorithm" , "RSA",
                "-out"       , outputDERFileName,
                "-outform"   , "DER",
                "-pkeyopt"   , keygenBits,
                "-pkeyopt"   , keygenPubExp,
                nullptr
            )
        );
        if ( generate_result != EXIT_SUCCESS )
        {
            reportRuntimeError
            (
                ( generate_result == - 1 )
                    ? "Unable to run OpenSSL.exe."
                    : "OpenSSL key generation failed."
            );
        }
    }

    char outputPEMFileName[ /*MAX_PATH*/260 ];
    {
        std::strcpy( boost::copy( keyName, outputPEMFileName ), ".pem" );
        intptr_t const make_pem_result
        (
            ::_spawnl
            (
                _P_WAIT,
                "openssl.exe",
                "openssl.exe",
                "rsa"        ,
                "-inform"    , "DER",
                "-in"        , outputDERFileName,
                "-outform"   , "PEM",
                "-out"       , outputPEMFileName,
                "-text"      ,
                nullptr
            )
        );
        if ( make_pem_result != EXIT_SUCCESS )
        {
            reportRuntimeError
            (
                ( make_pem_result == - 1 )
                    ? "Unable to run OpenSSL.exe."
                    : "OpenSSL DER-to-PEM conversion failed."
            );
        }
    }

    if ( !load( outputPEMFileName ) )
        reportRuntimeError( "Failed to open OpenSSL generated PEM key file." );

    BOOST_ASSERT( verify() );
}
#endif // _WIN32 && !BOOST_NO_EXCEPTIONS

#if !defined( __ANDROID__ ) && !defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ )
LE_OPTIMIZE_FOR_SPEED_BEGIN()

namespace
{
    using uint_half_t   = BigUInt<Key::sizeInBits / 2>;
    using uint_double_t = BigUInt<Key::sizeInBits * 2>;
    using signed_t      = BigInt <Key::sizeInBits>;

    LE_NOTHROWNOALIAS LE_HOT
    uint_half_t LE_FASTCALL generate_prime()
    {
        // https://en.wikipedia.org/wiki/Primality_test
        // http://www.boost.org/doc/libs/release/libs/multiprecision/doc/html/boost_multiprecision/tut/primetest.html
        // https://en.wikipedia.org/wiki/Safe_prime

        using MainRNG   = std::random_device;
        using TestRNG   = std::mt19937;
        using RNGEngine = boost::random::independent_bits_engine<MainRNG, Key::sizeInBits / 2, uint_half_t>;

        LE_ALIGN( 16 ) RNGEngine   rngEngine;
        LE_ALIGN( 16 ) TestRNG     testRNG( const_cast<MainRNG &>( rngEngine.base() )() );
        LE_ALIGN( 16 ) uint_half_t prime;
        do
        {
            prime = rngEngine();
        }
        while
        (
            !boost::multiprecision::miller_rabin_test(   prime          , 25, testRNG ) &&
            !boost::multiprecision::miller_rabin_test( ( prime - 1 ) / 2, 25, testRNG )
        );
        return prime;
    }

    LE_NOTHROWNOALIAS LE_HOT
    Key::uint4096_t LE_FASTCALL calculateD( Key::uint4096_t const & __restrict phi )
    {
        // http://stackoverflow.com/questions/3344759/how-to-calculate-the-modular-multiplicative-inverse-of-a-number-in-the-context-o
        std::uint32_t const e        ( Key::publicExponent );
        std::uint32_t const phi_mod_e( phi % e             );

        std::uint32_t k    ( 0 );
        std::uint32_t total( 1 );
        while ( total != 0 )
        {
            total = ( total + phi_mod_e ) % e;
            ++k;
        }
        return Key::uint4096_t{ ( uint_double_t( k ) * phi + 1 ) / e };
    }
} // anonymous namespace


LE_NOTHROWNOALIAS LE_HOT
void LE_FASTCALL_ABI Key::generate()
{
    // https://en.wikipedia.org/wiki/RSA_(cryptosystem)#Key_generation
    // http://stackoverflow.com/questions/3209665/for-rsa-how-do-i-calculate-the-secret-exponent/3209797#3209797
    // https://en.wikipedia.org/w/index.php?title=Extended_Euclidean_algorithm&oldid=580163370#Recursive_method
    // http://codeforces.com/blog/entry/2667
    // http://stackoverflow.com/questions/12826114/euclids-extended-algorithm-c
    // http://www.cypherspace.org/rsa/rsa-keygen.html

    using namespace boost::multiprecision;

    uint4096_t const p  ( generate_prime()  );
    uint4096_t const q  ( generate_prime()  );
    uint4096_t const n  ( p * q             );
    uint4096_t const phi( n - ( p + q - 1 ) );
    auto       const e  ( publicExponent    );
    uint4096_t const d  ( calculateD( phi ) );
    BOOST_ASSERT_MSG
    (
        p != q,
        "Internal inconsistency: invalid RSA prime factors"
    );
    BOOST_ASSERT_MSG
    (
        ( 1 < e ) && ( e < phi ) && ( gcd( e, phi ) == 1 ),
        "Internal inconsistency: invalid RSA public exponent"
    );
    BOOST_ASSERT_MSG
    (
        ( uint_double_t( d ) * e ) % phi == 1,
        "Internal inconsistency: invalid RSA private exponent"
    );
    this->modulus         = n;
    this->privateExponent = d;

    BOOST_ASSERT( verify() );
}
LE_OPTIMIZE_FOR_SPEED_END()

LE_NOTHROWNOALIAS
bool LE_FASTCALL_ABI Key::verify() const
{
    RawBytes randomBytes( reinterpret_cast<RawBytes const &>( *this ) ); randomBytes[ 0 ] &= 0x7F;
    auto const encryptedBytes( publicEncrypt ( randomBytes, modulus ) );
    auto const decryptedBytes( privateDecrypt( encryptedBytes       ) );
    return decryptedBytes == randomBytes;
}
#endif // !Android && !iOS

#ifdef __clang__
/// \note Client side link error quick-fix.
///                                           (24.03.2016.) (Domagoj Saric)
Key::public_exponent_t const Key::publicExponent;
#endif // __clang__

//------------------------------------------------------------------------------
} // namespace Cryptography
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
