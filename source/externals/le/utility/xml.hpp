////////////////////////////////////////////////////////////////////////////////
///
/// \file xml.hpp
/// -------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef xml_hpp__7955132E_E25A_4EB2_A9C6_AD39B005BA72
#define xml_hpp__7955132E_E25A_4EB2_A9C6_AD39B005BA72
#pragma once
//------------------------------------------------------------------------------
#include "abi.hpp"

#include <boost/config.hpp>
#include <boost/utility/string_ref.hpp>

#define RAPIDXML_NO_STREAMS
#define RAPIDXML_STATIC_POOL_SIZE  8 * 1024
#define RAPIDXML_DYNAMIC_POOL_SIZE RAPIDXML_STATIC_POOL_SIZE
#if defined( BOOST_NO_EXCEPTIONS )
    #define RAPIDXML_NO_EXCEPTIONS
#endif // BOOST_NO_EXCEPTIONS

#include <rapidxml.hpp>

#ifdef __GNUC__
// http://sourceforge.net/p/rapidxml/bugs/16
// http://stackoverflow.com/questions/14113923/rapidxml-print-header-has-undefined-methods
namespace rapidxml
{
namespace internal
{
    template<class OutIt, class Ch> OutIt print_children        (OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_attributes      (OutIt out, const xml_node<Ch> *node, int flags            );
    template<class OutIt, class Ch> OutIt print_data_node       (OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_cdata_node      (OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_element_node    (OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_declaration_node(OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_comment_node    (OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_doctype_node    (OutIt out, const xml_node<Ch> *node, int flags, int indent);
    template<class OutIt, class Ch> OutIt print_pi_node         (OutIt out, const xml_node<Ch> *node, int flags, int indent);
} // namespace internal
} // namespace rapidxml
#endif // GCC/Clang

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4100 ) // Unreferenced formal parameter.
#endif // _MSC_VER
#include <rapidxml_print.hpp>
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

#ifndef BOOST_NO_EXCEPTIONS
#include <string>
#endif // BOOST_NO_EXCEPTIONS
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
/// \addtogroup Utility
/// @{
namespace Utility
{
//------------------------------------------------------------------------------
/// \addtogroup XML
/// \brief XML utilities
/// @{
//------------------------------------------------------------------------------
namespace XML /// \brief XML utilities
{
//------------------------------------------------------------------------------

using boost::string_ref;

// https://www.w3.org/XML/Datamodel.html
// http://www.w3schools.com/xml/default.asp
// http://stackoverflow.com/questions/132564/whats-the-difference-between-an-element-and-a-node-in-xml

/// XML attribute
using Attribute = rapidxml::xml_attribute<>;

/// Base XML object
using Object = rapidxml::xml_base<>;

/// \name Name/value access
/// \brief All XML objects <em>are</em> <VAR>XML::Object</VAR>s making these
/// functions usable with both Elements and Attributes.
/// @{
LE_NOTHROWNOALIAS inline string_ref name ( Object const & object ) { return { object.name (), object.name_size () }; }
LE_NOTHROWNOALIAS inline string_ref value( Object const & object ) { return { object.value(), object.value_size() }; }
/// @}

/// XML element node
class Element : public rapidxml::xml_node<>
{
public:
    LE_NOTHROWNOALIAS Element();
    LE_NOTHROWNOALIAS Element( string_ref name );

    /// @{
    /// \name Name/value access
    string_ref name () const { return XML::name ( *this ); }
    string_ref value() const { return XML::value( *this ); }

    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setName ( string_ref );
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setValue( string_ref );
    /// @}

    /// @{
    /// \name Subelement access
    LE_NOTHROWNOALIAS Element   const * LE_FASTCALL_ABI child    ( string_ref name ) const;
    LE_NOTHROWNOALIAS Attribute const * LE_FASTCALL_ABI attribute( string_ref name ) const;
    LE_NOTHROWNOALIAS Attribute       * LE_FASTCALL_ABI attribute( string_ref name )      ;
    /// @}
}; // class Node

/// XML document or root node
class Document : public rapidxml::xml_document<>
{
public:
    void parse( char * string );

    LE_NOTHROWNOALIAS Element const * LE_FASTCALL_ABI element( string_ref name ) const;

    string_ref copyString( string_ref ) const;

#ifndef BOOST_NO_EXCEPTIONS
    std::string print() const
    {
        std::string xmlText;
        rapidxml::print( std::back_inserter( xmlText ), *this );
        return /*std::move inhibits NRVO*/( xmlText );
    }
#endif // BOOST_NO_EXCEPTIONS

    LE_NOTHROW
    char * print( char * pBuffer, std::uint16_t bufferLength ) const;
}; // class Document

//------------------------------------------------------------------------------
} // namespace XML
//------------------------------------------------------------------------------
/// @} // group XML
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
/// @} // group Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // xml_hpp
