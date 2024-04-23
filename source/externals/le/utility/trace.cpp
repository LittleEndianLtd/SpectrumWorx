////////////////////////////////////////////////////////////////////////////////
///
/// trace.cpp
/// ---------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "trace.hpp"

#include "abi.hpp"
#include "platformSpecifics.hpp"

#ifdef __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #import <Foundation/Foundation.h>
    #else
        #include <MacTypes.h>
        #include <mach-o/dyld.h>
        #include <sys/syslog.h>
    #endif // iOS
#endif // __APPLE__

#ifdef __ANDROID__
    #include "jni.hpp"

    #include <android/log.h>
#endif // __ANDROID__

#ifdef _WIN32
extern "C" __declspec( dllimport ) void __stdcall OutputDebugStringA( char const * lpOutputString );
#endif // _WIN32

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_WEAK_SYMBOL char const * Tracer::pTagString = "LE";

namespace
{
#ifdef __ANDROID__
    JNI::GlobalRef<> userMessageObject = nullptr;
    jmethodID        userMessageMethod = nullptr;

    LE_NOTHROWNOALIAS LE_COLD
    void LE_FASTCALL userMessage( char const * const pFormatString, va_list /*const*/ arglist )
    {
        if ( !userMessageMethod )
            return;
        BOOST_ASSERT( userMessageObject );
        char formattedMessage[ 512 ];
        /*auto const charactersWritten*/( /*std*/::vsnprintf( formattedMessage, sizeof( formattedMessage ), pFormatString, arglist ) );
        auto const pJNI( JNI::env() );
        auto const javaString( pJNI->NewStringUTF( formattedMessage ) );
        pJNI->CallVoidMethod( userMessageObject.get(), userMessageMethod, javaString );
    }
#elif TARGET_OS_IPHONE
    void (^userMessageCallback)( NSString * message ) = nullptr;

    LE_NOTHROWNOALIAS LE_COLD
    void LE_FASTCALL iOSLog( char const * const pFormatString, va_list /*const*/ arglist )
    {
        // https://developer.apple.com/library/ios/documentation/Cocoa/Conceptual/MemoryMgmt/Articles/mmAutoreleasePools.html
        @autoreleasepool
        {
            //@try
            {
                auto const formatString   ( [NSString stringWithCString: pFormatString encoding: NSASCIIStringEncoding] );
                auto const formattedString( [[[NSString alloc] initWithFormat: formatString arguments: arglist] autorelease] );
            #pragma clang diagnostic push
            #pragma clang diagnostic ignored "-Wformat-security"
                ::NSLog( formattedString );
            #pragma clang diagnostic pop
                if ( userMessageCallback )
                    userMessageCallback( formattedString );
            }
            //@catch ( ::NSException * ) {}
        }
    }
#endif // target
} // anonymous namespace

#ifdef __ANDROID__
    void * LE_FASTCALL Tracer_setUserMessageMethod( void *   const opaquePtr   ) { auto const previous( userMessageMethod   ); userMessageMethod   = static_cast<decltype( userMessageMethod   )>( opaquePtr ); return previous; }
#elif TARGET_OS_IPHONE
    void * LE_FASTCALL Tracer_setUserMessageMethod( void *   const opaquePtr   ) { auto const previous( userMessageCallback ); userMessageCallback = (decltype( userMessageCallback ))( opaquePtr ); return previous; }
#else
    void * LE_FASTCALL Tracer_setUserMessageMethod( void * /*const opaquePtr*/ ) { return nullptr; }
#endif // OS

LE_NOTHROWNOALIAS
void Tracer::error( char const * const pFormatString, ... )
{
    va_list arglist;
    va_start( arglist, pFormatString );
#if defined( __ANDROID__ )
    ::__android_log_vprint( ANDROID_LOG_ERROR, pTagString, pFormatString, arglist );
    userMessage( pFormatString, arglist );
#elif ( TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR )
    iOSLog( pFormatString, arglist );
#else
    auto const bufferSize( 1024 );
    char formattedError[ bufferSize ];
    char * pFormattedError( &formattedError[ 1 ] );
    std::strcpy( pFormattedError, pTagString );
    pFormattedError += std::strlen( pFormattedError );
    std::strcpy( pFormattedError, ", ERROR: " );
    pFormattedError += std::strlen( pFormattedError );
    unsigned int const tagCharacters( static_cast<unsigned int>( pFormattedError - ( formattedError + 1 ) ) );
    int const charactersWritten( /*std*/::vsnprintf( pFormattedError, bufferSize - tagCharacters, pFormatString, arglist ) );
    LE_ASSUME( charactersWritten > 0 || ( charactersWritten == 0 && *pFormatString == '\0' ) );
    #if defined( __APPLE__ )
        // https://developer.apple.com/library/mac/documentation/Darwin/Reference/ManPages/man3/syslog.3.html
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wformat-security"
        ::syslog( LOG_ERR, &formattedError[ 1 ] );
        #pragma clang diagnostic pop
        formattedError[ 0 ] = static_cast<unsigned char>( tagCharacters + charactersWritten );
        ::DebugStr( static_cast<ConstStr255Param>( static_cast<void const *>( formattedError ) ) );
    #else
        std::fputs( &formattedError[ 1 ], stderr );
        pFormattedError[ charactersWritten + 0 ] = '\n';
        pFormattedError[ charactersWritten + 1 ] = '\0';
        ::OutputDebugStringA( &formattedError[ 1 ] );
    #endif
#endif // platform/compiler
    va_end( arglist );
}

LE_NOTHROWNOALIAS
void Tracer::message( char const * const pFormatString, ... )
{
    va_list arglist;
    va_start( arglist, pFormatString );
#if defined( __ANDROID__ )
    ::__android_log_vprint( ANDROID_LOG_INFO, pTagString, pFormatString, arglist );
    userMessage( pFormatString, arglist );
#elif ( TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR )
    iOSLog( pFormatString, arglist );
#else
    unsigned int const bufferSize( 1024 );
    char formattedMessage[ bufferSize ];
    int const charactersWritten( /*std*/::vsnprintf( formattedMessage, bufferSize, pFormatString, arglist ) );
    LE_ASSUME( charactersWritten > 0 );
    std::puts( formattedMessage );
    #if defined( __APPLE__ )
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wformat-security"
        va_start( arglist, pFormatString );
        ::vsyslog( LOG_INFO, pFormatString, arglist );
        #pragma clang diagnostic pop
    #else
        formattedMessage[ charactersWritten + 0 ] = '\n';
        formattedMessage[ charactersWritten + 1 ] = '\0';
        ::OutputDebugStringA( formattedMessage );
    #endif
#endif // platform/compiler
    va_end( arglist );
}


#ifdef __ANDROID__
LE_NOTHROW
bool LE_FASTCALL_ABI Tracer::setJavaCallback( JNIEnv & jni, jobject const callbackObject, char const * const callbackMethodName )
{
    jclass const callbackClass( jni.GetObjectClass( callbackObject ) );                                BOOST_ASSERT( callbackClass     );
    userMessageMethod = jni.GetMethodID( callbackClass, callbackMethodName, "(Ljava/lang/String;)V" ); BOOST_ASSERT( userMessageMethod );
    userMessageObject = JNI::globalReference( callbackObject );
    return BOOST_LIKELY( userMessageObject != nullptr );
}
#elif TARGET_OS_IPHONE
LE_NOTHROW
bool LE_FASTCALL_ABI Tracer::setObjCCallback( void( ^newCallback )( NSString * message ) )
{
    Block_release( userMessageCallback );
    userMessageCallback = Block_copy( newCallback );
    return BOOST_LIKELY( userMessageCallback != nullptr );
}
#endif // os

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
