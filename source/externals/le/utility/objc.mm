////////////////////////////////////////////////////////////////////////////////
///
/// objc.mm
/// -------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "objc.hpp"

#include "platformSpecifics.hpp"

#import <Foundation/Foundation.h>

#include <cstdint>
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

namespace
{
    LE_COLD
    NSString * LE_FASTCALL wrapString( char const * __restrict const c_str, std::uint32_t const encoding )
    {
        /// \note Using a CFString with 'toll-free bridging' as no equivalent
        /// method exists in NSString.
        ///                                   (18.04.2014.) (Domagoj Saric)
        ::CFStringRef const cfString
        (
            ::CFStringCreateWithCStringNoCopy
            (
                nullptr, c_str, encoding, kCFAllocatorNull
            )
        );
        return (__bridge NSString *)cfString;
    }
    LE_COLD
    NSString * LE_FASTCALL copyString( char const * __restrict const c_str, std::uint32_t const encoding )
    {
        return [NSString stringWithCString : c_str encoding: encoding];
    }
} // anonymous namespace

LE_NOTHROW NSString * LE_FASTCALL_ABI     asciiString( char const * __restrict const c_str ) { return wrapString( c_str, kCFStringEncodingASCII ); }
LE_NOTHROW NSString * LE_FASTCALL_ABI     utf8String ( char const * __restrict const c_str ) { return wrapString( c_str, kCFStringEncodingUTF8  ); }

LE_NOTHROW NSString * LE_FASTCALL_ABI copyASCIIString( char const * __restrict const c_str ) { return copyString( c_str, NSASCIIStringEncoding ); }
LE_NOTHROW NSString * LE_FASTCALL_ABI copyUTF8String ( char const * __restrict const c_str ) { return copyString( c_str, NSUTF8StringEncoding  ); }

//------------------------------------------------------------------------------
} // namespace ObjC
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
