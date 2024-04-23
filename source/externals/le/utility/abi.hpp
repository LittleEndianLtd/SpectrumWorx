////////////////////////////////////////////////////////////////////////////////
///
/// \file abi.hpp
/// -------------
///
///   A collection of complier/platform specific ABI defining macros.
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef abi_hpp__10DEE3C0_20E4_4B36_B0A8_B02ADEFD7F38
#define abi_hpp__10DEE3C0_20E4_4B36_B0A8_B02ADEFD7F38
#pragma once
//------------------------------------------------------------------------------
#ifdef __ANDROID__
#include "sys/cdefs.h"
#endif // __ANDROID__
//------------------------------------------------------------------------------

#ifndef __cplusplus
    #error LE SDKs are C++ libraries and have to be compiled with a C++ compiler
#endif // __cplusplus

#if defined( _MSC_VER ) && !defined( __clang__ )

    #if _MSC_VER < 1600
        #error LE SDKs require MSVC10 SP1 (Visual Studio 2010) or later
    #endif // _MSC_VER

    #define LE_NOTHROW                __declspec( nothrow                  )
    #define LE_NOEXCEPT
    #define LE_NOALIAS                __declspec( noalias                  )
    #define LE_NOTHROWNOALIAS         __declspec( nothrow noalias          )
    #define LE_RESTRICTNOALIAS        __declspec( restrict noalias         )
    #define LE_NOTHROWRESTRICTNOALIAS __declspec( nothrow restrict noalias )
    #define LE_CONST_FUNCTION         LE_NOALIAS
    #define LE_PURE_FUNCTION          LE_NOALIAS

    #define LE_CDECL        __cdecl
    #define LE_FASTCALL_ABI __fastcall

    #define LE_RESTRICT __restrict

    #define LE_OVERRIDE override
    #define LE_SEALED   sealed

    #define LE_DLL_EXPORT __declspec( dllexport )
    #define LE_DLL_IMPORT __declspec( dllimport )

    #define LE_ASSUME( condition ) __assume( condition )

#elif defined( __GNUC__ )

    #if __cplusplus < 201103L
        #error LE SDKs require GCC 4.7+ or Clang 3.2+ with enabled C++11 support (-std=c++11)
    #endif // __cplusplus

    #define LE_NOTHROW                __attribute__(( nothrow ))
#ifdef __clang__
    #define LE_NOEXCEPT               LE_NOTHROW
#else
    #define LE_NOEXCEPT               noexcept
#endif // __clang__
    #define LE_NOALIAS                //__declspec( noalias )
    #define LE_NOTHROWNOALIAS         LE_NOTHROW                         LE_NOALIAS
    #define LE_RESTRICTNOALIAS        __attribute__(( malloc ))          LE_NOALIAS
    #define LE_NOTHROWRESTRICTNOALIAS __attribute__(( nothrow, malloc )) LE_NOALIAS
    #define LE_CONST_FUNCTION         __attribute__(( const )) // http://lwn.net/Articles/285332
    #define LE_PURE_FUNCTION          __attribute__(( pure  ))

    #ifdef __i386__
        #define LE_CDECL        __attribute__(( cdecl ))
        #define LE_FASTCALL_ABI __attribute__(( regparm( 3 ), stdcall ))
    #elif ( defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7__ ) ) && !defined( __aarch64__ )
        #define LE_CDECL
        #define LE_FASTCALL_ABI __attribute__(( pcs( "aapcs-vfp" ) ))
    #else
        #define LE_CDECL
        #define LE_FASTCALL_ABI
    #endif // __i386__

    #define LE_RESTRICT __restrict__

    #if defined( __clang__ ) || ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) >= 47 )
        #define LE_OVERRIDE override
        #define LE_SEALED   final
    #else
        #define LE_OVERRIDE
        #define LE_SEALED
    #endif

    #define LE_DLL_EXPORT __attribute__(( visibility( "default" ) ))
    #define LE_DLL_IMPORT __attribute__(( visibility( "default" ) ))

    #define LE_ASSUME( condition ) do { if ( !( condition ) ) __builtin_unreachable(); } while ( 0 )

#else

    #error LE unsupported compiler

#endif

//------------------------------------------------------------------------------
#endif // abi_hpp
