////////////////////////////////////////////////////////////////////////////////
///
/// xml.cpp
/// -------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "xml.hpp"

#include "platformSpecifics.hpp"

#include <boost/assert.hpp>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------
namespace XML
{
//------------------------------------------------------------------------------

void Document::parse( char * const string )
{
    // Implementation note:
    //   Entity translation is required to support 'items' with XML 'special
    //   characters' (e.g. "&").
    //                                        (15.11.2010.) (Domagoj Saric)
    auto const defaultRapidXMLOptions =
        ( rapidxml::parse_fastest & ~rapidxml::parse_no_entity_translation )
    #ifndef NDEBUG
        | rapidxml::parse_declaration_node
        | rapidxml::parse_comment_nodes
        | rapidxml::parse_doctype_node
        | rapidxml::parse_pi_nodes
        | rapidxml::parse_validate_closing_tags
    #endif
        ;

    rapidxml::xml_document<>::parse<defaultRapidXMLOptions>( string );
}


string_ref Document::copyString( string_ref const source ) const
{
    return { const_cast<Document &>( *this ).allocate_string( source.begin(), source.length() ), source.length() };
}


LE_NOTHROWNOALIAS Element   const * Document::element ( string_ref const name ) const { return static_cast<Element const &>( static_cast<rapidxml::xml_node<> const &>( *this ) ).child( name ); }

LE_NOTHROWNOALIAS Element   const * Element::child    ( string_ref const name ) const { return static_cast<Element   const *>( first_node     ( name.begin(), name.length() ) ); }
LE_NOTHROWNOALIAS Attribute       * Element::attribute( string_ref const name )       { return static_cast<Attribute       *>( first_attribute( name.begin(), name.length() ) ); }
LE_NOTHROWNOALIAS Attribute const * Element::attribute( string_ref const name ) const { return const_cast<Element &>( *this ).attribute( name ); }

LE_NOTHROWNOALIAS void Element::setName ( string_ref const name  ) { static_cast<Object &>( *this ).name ( name .begin(), name .size() ); }
LE_NOTHROWNOALIAS void Element::setValue( string_ref const value ) { static_cast<Object &>( *this ).value( value.begin(), value.size() ); }

LE_NOTHROWNOALIAS Element::Element() : rapidxml::xml_node<>( rapidxml::node_element ) {}

LE_NOTHROWNOALIAS
Element::Element( string_ref const name )
    :
    rapidxml::xml_node<>( rapidxml::node_element )
{
    BOOST_ASSERT( *name.end() == '\0' );
    Object::name( name.begin(), name.size() );
}

//------------------------------------------------------------------------------
} // namespace XML
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#if defined( RAPIDXML_NO_EXCEPTIONS )
#ifdef _MSC_VER
    extern "C" LE_WEAK_FUNCTION void rapidxml_parse_error_handler( char const * /*const what*/, void * /*const where*/ ) { std::terminate(); }
    #pragma comment( linker, "/alternatename:?parse_error_handler@rapidxml@@YAXPEBDPEAX@Z=rapidxml_parse_error_handler" )
#else
    LE_WEAK_FUNCTION void rapidxml::parse_error_handler( char const * /*const what*/, void * /*const where*/ ) { std::terminate(); }
#endif // _MSC_VER
#endif // RAPIDXML_NO_EXCEPTIONS
//------------------------------------------------------------------------------
