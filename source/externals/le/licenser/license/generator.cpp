////////////////////////////////////////////////////////////////////////////////
///
/// generator.cpp
/// -------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if !defined( __ANDROID__ ) && !defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ )
#include "generator.hpp"

#include "le/licenser/cryptography/base64.hpp"
#include "le/licenser/cryptography/signing.hpp"
#include "le/licenser/license/predefinedItems.hpp"
#include "le/utility/filesystem.hpp"

#include <boost/config.hpp>
#if !defined( BOOST_MSVC ) || ( _MSC_FULL_VER > 190023419 )
    #define LE_USE_SPIRIT_X3
#endif // old MSVC

#ifdef _MSC_VER
    #pragma warning( disable : 4503 ) // Decorated name length exceeded, name was truncated.
#endif // _MSC_VER

#ifdef LE_USE_SPIRIT_X3
    #ifdef BOOST_NO_TYPEID
        #define BOOST_SPIRIT_X3_NO_RTTI
    #endif // BOOST_NO_TYPEID
    #pragma warning( push )
    #pragma warning( disable : 4100 ) // Unreferenced formal parameter.
    #pragma warning( disable : 4459 ) // Declaration of 'attr' hides global declaration.
    // http://ciere.com/cppnow15/x3_docs/index.html
    #include <boost/spirit/home/x3/auxiliary.hpp>
    #include <boost/spirit/home/x3/char.hpp>
    #include <boost/spirit/home/x3/directive.hpp>
    #include <boost/spirit/home/x3/operator.hpp>
    #include <boost/spirit/home/x3/core.hpp>
    #include <boost/spirit/home/x3/string.hpp>
    #pragma warning( pop )

    #define LE_MAKE_RULE( x ) x
#else
    #include <boost/spirit/home/qi/action.hpp>
    #include <boost/spirit/home/qi/auxiliary/eoi.hpp>
    #include <boost/spirit/home/qi/auxiliary/eol.hpp>
    #include <boost/spirit/home/qi/char/char.hpp>
    #include <boost/spirit/home/qi/directive/omit.hpp>
    #include <boost/spirit/home/qi/directive/raw.hpp>
    #include <boost/spirit/home/qi/operator.hpp>
    #include <boost/spirit/home/qi/parse.hpp>
    #include <boost/spirit/home/qi/string/lit.hpp>

    #define LE_MAKE_RULE( x ) boost::proto::deep_copy( x )
#endif

#ifdef _MSC_VER
#include <malloc.h>
#else
#include <alloca.h>
#endif // _MSC_VER
#include <stdexcept>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Lic
{
//------------------------------------------------------------------------------

LE_NOTHROWNOALIAS
Generator::Generator()
    : root( PredefinedItems::rootElement )
{
    licenseXML.append_node( &root );
}

// Unicode/UTF8:
//
// http://losingfight.com/blog/2006/07/28/wchar_t-unsafe-at-any-size
// http://unicode.org/faq/utf_bom.html
// http://en.wikipedia.org/wiki/Byte_order_mark
// http://www.joelonsoftware.com/articles/Unicode.html
// http://lists.macosforge.org/pipermail/macports-dev/2009-February/007195.html
// http://msdn.microsoft.com/en-us/library/5z097dxa.aspx
// http://stackoverflow.com/questions/688760/how-to-create-a-utf-8-string-literal-in-visual-c-2008
// http://site.icu-project.org ICU - International Components for Unicode

#ifdef _MSC_VER
    #pragma warning( disable : 4503 ) // Decorated name length exceeded, name was truncated.
#endif // _MSC_VER
namespace
{
    using Range = boost::iterator_range<char const *>;

    #ifdef LE_USE_SPIRIT_X3
        using unused_parameter_t = std::false_type;
    #else
        using unused_parameter_t = boost::spirit::qi::unused_type;
    #endif // LE_USE_SPIRIT_X3

    unused_parameter_t const dummy{};
} // anonymous namespace

struct key_action
{
    template <typename Context>
    void operator()( Context const & ctx ) const { (*this)( _attr( ctx ), dummy, dummy ); } // x3
    void operator()( Range const keyRange, unused_parameter_t, unused_parameter_t ) const // x2
    {
        lastKey = { keyRange.begin(), keyRange.size() };
    }

    boost::string_ref & __restrict lastKey;
}; // struct key_action

struct value_action
{
    template <typename Context>
    void operator()( Context const & ctx ) const { (*this)( _attr( ctx ), dummy, dummy ); } // x3
    void operator()( Range const valueRange, unused_parameter_t, unused_parameter_t ) const // x2
    {
        if ( !valueRange )
            return;
        BOOST_ASSERT( !lastKey.empty() );
        auto & xml( generator.licenseXML );
        auto const key  ( xml.copyString( lastKey ) );
        auto const value( xml.copyString( {valueRange.begin(), valueRange.size()} ) );
        auto & parentNode( *generator.root.last_node() );
    #if 1 // use elements for items
        auto & __restrict element( *xml.allocate_node( rapidxml::node_element, key.begin(), value.begin(), key.size(), value.size() ) );
        parentNode.append_node( &element );
    #else // use attributes for items
        auto & __restrict attribute( *xml.allocate_attribute( key.begin(), value.begin(), key.size(), value.size() ) );
        parentNode.append_attribute( &attribute );
    #endif
    }

    boost::string_ref const & __restrict lastKey  ;
    Generator               & __restrict generator;
}; // struct value_action

void Generator::parseIniFile( char const * const filePath, boost::string_ref const licenseDataTitle )
{
    auto const configFile( Utility::File::map<Utility::AbsolutePath>( filePath ) );
    if ( !configFile )
        throw std::runtime_error( "Failed to open ini file" );

    auto & node( *licenseXML.allocate_node( rapidxml::node_element, licenseDataTitle.begin(), nullptr, licenseDataTitle.length() ) );
    root.append_node( &node );

    // Alternatives:
    // Boost.PropertyTree
    // https://github.com/brofield/simpleini

#ifdef LE_USE_SPIRIT_X3
    namespace x3 = boost::spirit::x3;

    using x3::eoi  ;
    using x3::eol  ;
    using x3::char_;
    using x3::lit  ;
    using x3::omit ;
    using x3::raw  ;
    using x3::blank;

    using x3::parse;
#else // LE_USE_SPIRIT_X3
    namespace qi = boost::spirit::qi;

    qi::       eoi_type   const eoi  ;
    qi::       eol_type   const eol  ;
    qi::       char_type  const char_;
    qi::       lit_type   const lit  ;
    qi::       omit_type  const omit ;
    qi::       raw_type   const raw  ;
    qi::ascii::blank_type const blank;

    using qi::parse;
#endif // LE_USE_SPIRIT_X3

    auto const eol_space( LE_MAKE_RULE( omit[ ( *blank >> ( +eol | eoi ) ) ] ) );

    auto const equal_sign       ( LE_MAKE_RULE( *blank >> '=' >> *blank                                       ) );
    auto const item_key_string  ( LE_MAKE_RULE( omit[ *blank ] >> raw[ +( char_ - equal_sign ) ]              ) );
    auto const item_value_string( LE_MAKE_RULE(                   raw[ *( char_ - eol_space  ) ] >> eol_space ) );

    auto       first( configFile.begin() );
    auto const  last( configFile.end  () );

    if ( std::equal( utf8BOM.begin(), utf8BOM.end(), reinterpret_cast<unsigned char const *>( &*first ) ) )
        std::advance( first, utf8BOM.size() );

    boost::string_ref lastKey;
    if
    (
        !parse
        (
            first, last,
            +(
                item_key_string  [ (key_action  { lastKey        }) ]
                >> equal_sign >>
                item_value_string[ (value_action{ lastKey, *this }) ]
            )
        )
    )
        throw std::runtime_error( "Ini parsing error @ " + std::string( first, last ) );
}


std::string Generator::generateXML() { return licenseXML.print(); }


void Generator::generateLicense
(
    std::ostream            & outputStream,
    Cryptography::Key const & key
)
{
    auto const xmlText( generateXML() );

    auto const signature( Cryptography::createSignature( &xmlText[ 0 ], xmlText.length(), key ) );

    namespace Base64 = Cryptography::Base64;

    auto const signatureLength    ( static_cast<std::uint16_t>( signature.size() ) );
    auto const encodedBufferLength( Base64::encodeBufferLength( signatureLength ) );
    auto const pEncodedSignature  ( static_cast<char *>( alloca( encodedBufferLength ) ) );
    auto const encodedLength      ( Base64::encode( &signature[ 0 ], signatureLength, pEncodedSignature, encodedBufferLength ) );
    BOOST_ASSERT( encodedLength <= encodedBufferLength );

    outputStream.write( reinterpret_cast<char const *>( &utf8BOM[ 0 ] ), utf8BOM.size() );
    outputStream << xmlText;
    outputStream.write( &xmlCommentBegin[ 0 ], xmlCommentBegin.size() );
    outputStream.write( pEncodedSignature    , encodedLength          );
    outputStream.write( &xmlCommentEnd  [ 0 ], xmlCommentEnd  .size() );
}


#if 0 // todo
struct RequiredItem
{
    boost::string_ref const key  ;
    boost::string_ref       value;
};
#endif

//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // !Android && !iOS
