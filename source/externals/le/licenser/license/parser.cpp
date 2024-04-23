////////////////////////////////////////////////////////////////////////////////
///
/// parser.cpp
/// ----------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "parser.hpp"

#include "predefinedItems.hpp"

#include "le/licenser/cryptography/base64.hpp"
#include "le/licenser/cryptography/signing.hpp"

#include "le/utility/filesystem.hpp"

#include <boost/range/iterator_range_core.hpp>
#include <boost/range/algorithm/search.hpp>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Lic
{
//------------------------------------------------------------------------------

LE_NOTHROWNOALIAS
void License::parse
(
    unsigned char               const * const inMemoryLicense,
    std::uint16_t                       const licenseDataLength,
    Cryptography::Key::RawBytes const &       keyModulus
) BOOST_NOEXCEPT
{
    boost::string_ref const license( reinterpret_cast<char const *>( inMemoryLicense ), licenseDataLength );
    auto const xmlSize( license.find( { &xmlCommentBegin[ 0 ], xmlCommentBegin.size() } ) );
    BOOST_ASSERT_MSG( xmlSize != license.npos, "Invalid license file - no signature" );

    auto const pEncodedSignatureBegin( license.begin() + xmlSize + xmlCommentBegin.size() );
    auto const pEncodedSignatureEnd  ( license.end()             - xmlCommentEnd  .size() );

    Cryptography::Signature signature;
    BOOST_VERIFY_MSG
    (
        Cryptography::Base64::decode
        (
            pEncodedSignatureBegin, static_cast<std::uint16_t>( pEncodedSignatureEnd - pEncodedSignatureBegin ),
            &signature[ 0 ]       , static_cast<std::uint16_t>( signature.size()                              )
        )
            ==
        signature.size(),
        "Invalid license - signature decoding failed"
    );

    auto const verificationSuccess
    (
        Cryptography::verifySignature
        (
            boost::make_iterator_range( &license[ utf8BOM.size() ], &license[ xmlSize ] ),
            signature,
            keyModulus
        )
    );

    /// \note We have to copy the license XML data because of RapidXML's
    /// destructive parsing. rapidxml::xml_document<>::parse also requires null
    /// terminated input.
    ///                                       (21.10.2013.) (Domagoj Saric)
    /// \note We reserve an 8kB RAPIDXML_STATIC_POOL_SIZE which 'should be
    /// enough' for any 'sane' license file. A std::bad_alloc in copyString() or
    /// parse() below would almost certainly be a sign of a tampered-with
    /// license so we 'let it crash' in that case and mark this function as
    /// noexcept.
    ///                                       (25.04.2016.) (Domagoj Saric)
    auto xmlLicense( licenseXML_.copyString( { license.begin(), xmlSize + sizeof( '\0' ) } ) );
    const_cast<char &>/*no mutable string_ref*/( xmlLicense.back() ) = static_cast<char>( !verificationSuccess ); // null-terminate
    /// \note Hacker trap.
    ///                                       (21.03.2016.) (Domagoj Saric)
    /// \note Correct array size required for release ARM builds with Clang
    /// 3.8 from Android NDK r11c (otherwise it 'miscompiles' the out-of-bounds
    /// access - parsing always succeeds).
    ///                                       (21.03.2016.) (Domagoj Saric)
    char const * const xmlBeginnings[] = { xmlLicense.begin(), /*bogus 'crash on me' pointer*/&license[ xmlSize ] };

    licenseXML_.parse( /*...mrmlj...no mutable string_ref*/const_cast<char *>( xmlBeginnings[ !verificationSuccess ] ) );

    pRoot_ = static_cast<Utility::XML::Element const *>( licenseXML_.first_node() );
    BOOST_ASSERT_MSG( pRoot_->name() == PredefinedItems::rootElement, "Invalid license XML root element" );
}

LE_NOTHROWNOALIAS
bool License::parse
(
    char                        const * const onDiskLicense,
    Cryptography::Key::RawBytes const &       keyModulus
) BOOST_NOEXCEPT
{
    auto const licenseFile
    (
        Utility::File::map<Utility::AbsolutePath>( onDiskLicense )
    );
    if ( BOOST_UNLIKELY( !licenseFile ) )
        return false;
    parse( reinterpret_cast<unsigned char const *>( licenseFile.begin() ), static_cast<std::uint16_t>( licenseFile.size() ), keyModulus );
    return true;
}

LE_NOTHROWNOALIAS Utility::XML::Element const & License::licenseeData() const { return *dataNode( PredefinedItems::LicenseeData::elementName ); }
LE_NOTHROWNOALIAS Utility::XML::Element const & License::productData () const { return *dataNode( PredefinedItems::ProductData ::elementName ); }

LE_NOTHROWNOALIAS Utility::XML::Element const * License::dataNode( boost::string_ref const elementName ) const { return pRoot_->child( elementName ); }

LE_NOTHROWNOALIAS boost::string_ref License::licenseeData( boost::string_ref const itemName ) const { return Utility::XML::value( *licenseeData().child( itemName ) ); }
LE_NOTHROWNOALIAS boost::string_ref License::productData ( boost::string_ref const itemName ) const { return Utility::XML::value( *productData ().child( itemName ) ); }

//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
