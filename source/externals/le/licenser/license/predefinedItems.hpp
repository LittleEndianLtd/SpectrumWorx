////////////////////////////////////////////////////////////////////////////////
///
/// \file predefinedItems.hpp
/// -------------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef predefinedItems_hpp__8DCAA6B9_89F7_47BE_B30F_466E1CA5FC92
#define predefinedItems_hpp__8DCAA6B9_89F7_47BE_B30F_466E1CA5FC92
#pragma once
//------------------------------------------------------------------------------
#include <boost/utility/string_ref.hpp>

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
/// \addtogroup Licenser
/// @{
namespace Lic
{
//------------------------------------------------------------------------------
namespace PredefinedItems /// \brief Predefined license items
{
//------------------------------------------------------------------------------
    using boost::string_ref;

    string_ref const rootElement = "LE.License";

    /// Data items that describe the entity that is being authorized to use the
    /// product (other than the elementName, these are specific to the Digital
    /// River ShareIt service).
    namespace LicenseeData
    {
        string_ref const elementName = "Licensee";

        string_ref const registeredName = "REG_NAME" ;
        string_ref const firstName      = "FIRSTNAME";
        string_ref const lastName       = "LASTNAME" ;

        string_ref const company = "COMPANY";
        string_ref const email   = "EMAIL"  ;
        string_ref const street  = "STREET" ;
        string_ref const city    = "CITY"   ;
        string_ref const state   = "STATE"  ;
        string_ref const country = "COUNTRY";
    } // namespace LicenseeData

    /// Data items that describe the product being authorized.
    namespace ProductData
    {
        string_ref const elementName           = "Product";

        string_ref const name                  = "Name"       ;
        string_ref const targetOS              = "OS"         ;
        string_ref const licenseType           = "LicenseType";

        string_ref const minimumTargetVersion  = "MinimumTargetVersion";
        string_ref const maximumTargetVersion  = "MaximumTargetVersion";

        string_ref const minimumUpgradeVersion = "MinimumUpgradeVersion";
        string_ref const maximumUpgradeVersion = "MaximumUpgradeVersion";
    } // namespace ProductData

    namespace LicenseTypes
    {
        string_ref const nfr            = "NFR"            ;
        string_ref const test           = "Test"           ;
        string_ref const fullCommercial = "Full commercial";
        string_ref const nonProfit      = "Non profit"     ;
    } // namespace LicenseTypes

    namespace OSTypes
    {
        string_ref const android = "Android" ;
        string_ref const iOS     = "iOS"     ;
        string_ref const macOSX  = "Mac OS X";
        string_ref const windows = "Windows" ;
        string_ref const upgrade = "upgrade" ;

        string_ref const currentOS =
        #if defined( _WIN32 )
            windows;
        #elif defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ )
            iOS;
        #elif defined( __APPLE__ )
            macOSX;
        #elif defined( __ANDROID__ )
            android;
        #endif // OS
    } // namespace OSTypes
//------------------------------------------------------------------------------
} // namespace PredefinedItems
//------------------------------------------------------------------------------
std::array<unsigned char const, 3> BOOST_CONSTEXPR_OR_CONST utf8BOM        {{ 0xEF, 0xBB, 0xBF }};
std::array<         char const, 4> BOOST_CONSTEXPR_OR_CONST xmlCommentBegin{{ '<', '!', '-', '-' }};
std::array<         char const, 3> BOOST_CONSTEXPR_OR_CONST xmlCommentEnd  {{ '-', '-', '>' }};
//------------------------------------------------------------------------------
/// @} // group Lic
//------------------------------------------------------------------------------
} // namespace Lic
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // predefinedItems_hpp
