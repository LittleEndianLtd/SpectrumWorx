////////////////////////////////////////////////////////////////////////////////
///
/// \file parser.hpp
/// ----------------
///
/// License parser/loader component.
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parser_hpp__3992CC8A_A7FB_4C07_8674_D49DE94A3203
#define parser_hpp__3992CC8A_A7FB_4C07_8674_D49DE94A3203
#pragma once
//------------------------------------------------------------------------------
#include <le/licenser/cryptography/key.hpp>
#include <le/utility/abi.hpp>
#include <le/utility/xml.hpp>

#include <boost/config.hpp>
#include <boost/utility/string_ref.hpp>

#include <cstdint>
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( LE_SDK_NO_AUTO_LINK )
    #ifdef _WIN64
        #pragma comment( lib, "LE_Licenser_SDK_Win64_x86-64_SSE2.lib" )
    #else // _WIN32
        #pragma comment( lib, "LE_Licenser_SDK_Win32_x86-32_SSE2.lib" )
    #endif // _WIN32/64
#endif // _MSC_VER && !LE_SDK_NO_AUTO_LINK
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
/// \addtogroup Licenser
/// \brief Core Licenser SDK components
/// @{
//------------------------------------------------------------------------------
namespace Lic /// \brief Licenser SDK root namespace
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class License
///
/// \brief License parser
///
////////////////////////////////////////////////////////////////////////////////

class License
{
public:
    /// \name Parsing
    /// @{
    /// <B>Effect:</B> Parse the in-memory or on-disk license file data and verify its signature with the public <VAR>keyModulus</VAR>.<BR>
    /// <B>Preconditions:</B>License data has to be valid (i.e. not tampered with - it has to match the signature), otherwise the function crashes (this by design, as an antihacking device).<BR>
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI parse( unsigned char const * inMemoryLicense, std::uint16_t licenseDataLength, Cryptography::Key::RawBytes const & keyModulus ) BOOST_NOEXCEPT;
    /// \overload
    /// \return false if the function fails to open the file <VAR>onDiskLicense</VAR>
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI parse( char const *          onDiskLicense                                   , Cryptography::Key::RawBytes const & keyModulus ) BOOST_NOEXCEPT;
    /// @}

    /// \name Access to the predefined "Licensee" and "Product" data nodes.
    /// <B>Preconditions:</B>
    ///  - a successful call to parse()
    ///  - the parsed license must include those nodes (otherwise the functions will crash).<BR>
    /// @{
    LE_NOTHROWNOALIAS Utility::XML::Element const & LE_FASTCALL_ABI licenseeData() const;
    LE_NOTHROWNOALIAS Utility::XML::Element const & LE_FASTCALL_ABI productData () const;

    LE_NOTHROWNOALIAS boost::string_ref LE_FASTCALL_ABI licenseeData( boost::string_ref itemName ) const;
    LE_NOTHROWNOALIAS boost::string_ref LE_FASTCALL_ABI productData ( boost::string_ref itemName ) const;
    /// @}

    /// \name Generic data access
    /// <B>Preconditions:</B> a successful call to parse().
    /// \return pointer to the XML::Element with the given name or nullptr if the
    /// node is not present in the license.
    LE_NOTHROWNOALIAS Utility::XML::Element const * LE_FASTCALL_ABI dataNode( boost::string_ref elementName ) const;
    /// @}

private:
    Utility::XML::Document                    licenseXML_;
    Utility::XML::Element  const * __restrict pRoot_     ;
}; // class License

//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
/// @} // group Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parser_hpp
