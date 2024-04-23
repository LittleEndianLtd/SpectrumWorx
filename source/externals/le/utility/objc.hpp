////////////////////////////////////////////////////////////////////////////////
///
/// \file objc.hpp
/// --------------
///
/// ObjC <-> C++ interop utilities.
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef objc_hpp__F939BC5B_C31E_4B2E_952D_83EBCE908813
#define objc_hpp__F939BC5B_C31E_4B2E_952D_83EBCE908813
#pragma once
#ifdef __OBJC__
//------------------------------------------------------------------------------
#include "abi.hpp"

@class NSString;
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------
namespace ObjC
{
//------------------------------------------------------------------------------

LE_NOTHROW NSString * LE_FASTCALL_ABI asciiString( char const * c_str );
LE_NOTHROW NSString * LE_FASTCALL_ABI utf8String ( char const * c_str );

LE_NOTHROW NSString * LE_FASTCALL_ABI copyASCIIString( char const * c_str );
LE_NOTHROW NSString * LE_FASTCALL_ABI copyUTF8String ( char const * c_str );

//------------------------------------------------------------------------------
} // namespace ObjC
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // __OBJC__
#endif // objc_hpp
