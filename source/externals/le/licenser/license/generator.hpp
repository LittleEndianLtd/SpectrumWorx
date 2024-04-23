////////////////////////////////////////////////////////////////////////////////
///
/// \file generator.hpp
/// -------------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef generator_hpp__D22D2C17_B9D6_449A_932A_EB9902053938
#define generator_hpp__D22D2C17_B9D6_449A_932A_EB9902053938
#pragma once
//------------------------------------------------------------------------------
#if !defined( __ANDROID__ ) && !defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ )
#include <boost/config.hpp>
#ifndef BOOST_NO_EXCEPTIONS

#include <le/licenser/cryptography/key.hpp>
#include <le/utility/xml.hpp>

#include <boost/utility/string_ref.hpp>

#include <string>
#include <ostream>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
/// \addtogroup Licenser
/// @{
namespace Lic
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Generator
///
/// \brief License generator
///
/// Parses input license data, converts it to XML, formats and signs it and
/// finally writes the license to an output stream.<BR>
/// <em>(included only for desktop platforms and when building with exceptions
/// on)</em>
///
////////////////////////////////////////////////////////////////////////////////

class Generator
{
public:
    LE_NOTHROWNOALIAS
    Generator();

    /// <B>Effect:</B> Parses the ini file at <VAR>filePath</VAR> and stores the data items into a new XML element <VAR>licenseDataTitle</VAR>.<BR>
    /// \throws std::bad_alloc     out of memory
    /// \throws std::runtime_error failure to open or parse the ini file
    void parseIniFile( char const * filePath, boost::string_ref const licenseDataTitle );

    /// <B>Effect:</B> Creates a text XML representation of the so far parsed ini data.<BR>
    /// \throws std::bad_alloc out of memory
    std::string generateXML();

    /// <B>Effect:</B> Creates a text XML representation of the so far parsed ini data, signs it with <VAR>key</VAR>, formats it and writes it to <VAR>outputStream</VAR>.<BR>
    /// \throws std::bad_alloc         out of memory
    /// \throws std::ios_base::failure error writting to <VAR>outputStream</VAR>
    void generateLicense( std::ostream & outputStream, Cryptography::Key const & key );

private: friend struct value_action;
    Utility::XML::Document licenseXML;
    Utility::XML::Element  root      ;
}; // class Generator

//------------------------------------------------------------------------------
} // namespace Lic
/// @} // group Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // !BOOST_NO_EXCEPTIONS
#endif // !Android && !iOS
#endif // generator_hpp
