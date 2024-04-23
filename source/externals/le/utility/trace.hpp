////////////////////////////////////////////////////////////////////////////////
///
/// \file trace.hpp
/// ---------------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef trace_hpp__0BA90AC6_C69C_428C_965B_915C2BD7781B
#define trace_hpp__0BA90AC6_C69C_428C_965B_915C2BD7781B
#pragma once
//------------------------------------------------------------------------------
#include "abi.hpp"

#ifdef __ANDROID__
#include <jni.h>
#elif defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ ) && defined( __OBJC__ )
#include <Block.h>

@class NSString;
#endif // __ANDROID__
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Tracer
///
/// \brief Utility logging class
///
/// <B>Android specific:</B> requires linking with the "log" (system-provided)
/// library.
///
////////////////////////////////////////////////////////////////////////////////

struct Tracer
{
    static LE_NOTHROWNOALIAS void message( char const * pFormatString, ... );
    static LE_NOTHROWNOALIAS void error  ( char const * pFormatString, ... );

    static char const * pTagString; ///< Tag or prefix string to be displayed before every log message.

#if defined( __ANDROID__ ) || defined( DOXYGEN_ONLY )
    /// <B>Android specific:</B> Registers a Java method to be used to marshal
    /// trace messages to the Java side (e.g. to be used in GUI code).
    /// \details The callback method must be a non-static member with the
    /// following signature: <VAR>void (String)</VAR>
    /// \return false if out-of-memory, true otherwise
    static LE_NOTHROW bool LE_FASTCALL_ABI setJavaCallback( JNIEnv &, jobject callbackObject, char const * callbackMethodName );
#endif // __ANDROID__

#if ( defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ ) && defined( __OBJC__ ) ) || defined( DOXYGEN_ONLY )
    /// <B>iOS specific:</B> Registers a block to be used to marshal trace
    /// messages to the ObjC side (e.g. to be used in GUI code).
    /// \details The callback method must be a non-static member with the
    /// following signature: <VAR>void (String)</VAR>
    /// \return false if out-of-memory, true otherwise
    static LE_NOTHROW bool LE_FASTCALL_ABI setObjCCallback( void (^)( NSString * message ) );
#endif // iOS
}; // struct Tracer

#ifndef NDEBUG

    #define LE_TRACE(                   formatString, ... ) LE::Utility::Tracer::message( formatString, ##__VA_ARGS__ )
    #define LE_TRACE_IF(     condition, formatString, ... ) if ( (condition) ) LE_TRACE( formatString, ##__VA_ARGS__ )
    #define LE_TRACE_RETURN( result   , formatString, ... ) ( LE_TRACE( formatString, ##__VA_ARGS__ ), result )

#else

    #define LE_TRACE(                   formatString, ... )
    #define LE_TRACE_IF(     condition, formatString, ... )
    #define LE_TRACE_RETURN( result   , formatString, ... ) result

#endif // _DEBUG

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // trace_hpp
