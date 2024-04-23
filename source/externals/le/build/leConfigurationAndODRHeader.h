////////////////////////////////////////////////////////////////////////////////
//
// LittleEndian root ODR and ABI configuration header.
// ---------------------------------------------------
//
// Copyright � 2009 - 2016. LittleEndian Ltd. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef leConfigurationAndODRHeader_h__C79BE937_90BA_4DF1_9D66_5429633644F6
#define leConfigurationAndODRHeader_h__C79BE937_90BA_4DF1_9D66_5429633644F6
#ifdef _MSC_VER
    #pragma once
#endif // _MSC_VER
//------------------------------------------------------------------------------

#ifndef LE_CHECKED_BUILD
    // By default we use checked builds in all non-release builds.
    #ifdef NDEBUG
        #define LE_CHECKED_BUILD 0
    #else
        #define LE_CHECKED_BUILD 1
    #endif // NDEBUG
#endif  // LE_CHECKED_BUILD

// Include asserts in all "checked" builds.
#undef NDEBUG
#undef BOOST_DISABLE_ASSERTS
#if !LE_CHECKED_BUILD
    #define NDEBUG
    #define BOOST_DISABLE_ASSERTS
    #ifndef LE_PUBLIC_BUILD
        #define LE_PUBLIC_BUILD
    #endif // LE_PUBLIC_BUILD
#endif  // LE_CHECKED_BUILD


/// \note Automatically build SDK projects with "hidden"/"static"/"anonymous"
/// implementation details in order to enable simultaneous usage of multiple
/// SDKs which internally use the same functionality (which would otherwise
/// cause symbol clashes).
///                                           (06.10.2014.) (Domagoj Saric)
#ifdef LE_SW_SDK_BUILD
    #define LE_IMPL_NAMESPACE_BEGIN( namespaceName ) namespace namespaceName { namespace {
    #define LE_IMPL_NAMESPACE_END(   namespaceName ) } }
#else
    #define LE_IMPL_NAMESPACE_BEGIN( namespaceName ) namespace namespaceName {
    #define LE_IMPL_NAMESPACE_END(   namespaceName )   }
#endif // LE_SW_SDK_BUILD


/// \note A quick way to disable the requriement for boost::assertion_failed to
/// be defined in auxiliary projects when BOOST_ENABLE_ASSERT_HANDLER is
/// globally defined in the CMakeLists.txt file.
///                                           (15.11.2013.) (Domagoj Saric)
#ifdef LE_DISABLE_BOOST_ASSERT_HANDLER
    #undef BOOST_ENABLE_ASSERT_HANDLER
#endif // LE_DISABLE_BOOST_ASSERT_HANDLER


////////////////////////////////////////////////////////////////////////////////
//
// Operating system specifics.
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Microsoft Windows.
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
    #include "sdkddkver.h"

    #undef WINVER
    #undef _WIN32_WINNT
    #undef _WIN32_IE
    #undef NTDDI_VERSION
    #ifndef _WIN32_WINNT
        // If not specified, allow use of features specific to Windows Vista SP2 or later.
        #define WINVER        _WIN32_WINNT_VISTA
        #define _WIN32_WINNT  _WIN32_WINNT_VISTA
        #define _WIN32_IE     0x0900
        #define NTDDI_VERSION NTDDI_VISTASP2
    #endif // _WIN32_WINNT

    #ifndef LEB_INCLUDE_FULL_WINDOWS_HEADERS
        // Exclude rarely-used stuff from Windows headers.
        #define WIN32_LEAN_AND_MEAN

	    // We use std::min/std::max().
	    #define NOMINMAX
    #endif // LEB_INCLUDE_FULL_WINDOWS_HEADERS
#endif // _WIN32


////////////////////////////////////////////////////////////////////////////////
//
// Build tool specifics.
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Microsoft Visual C++.
////////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER )

    #pragma once

    // Implementation note:
    //   A deficiency in CMake allows only MBCS or UNICODE Visual Studio builds.
    // To work around this we use the CMake default (MBCS) and then manually
    // undefine the relevant macro here (to get ASCII behaviour). Last tested
    // with CMake 2.8.2.
    //                                        (21.10.2010.) (Domagoj Saric)
    #ifdef _MBCS
        #undef _MBCS
    #endif // _MBCS

	#if ( _MSC_VER < 1400 ) || ( _MSC_VER > 1900 )
        #pragma message ( "WARNING: LEBuild was not tested with your version of MSVC..." )
	#endif

    #if ( defined( _M_IX86 ) && ( _M_IX86_FP == 1 ) )
        #define LE_HAS_SSE1
    #endif

    #if ( defined( _M_IX86 ) && ( _M_IX86_FP >= 2 ) ) || defined( _M_X64 )
        #define LE_HAS_SSE1
        #define LE_HAS_SSE2
    #endif

    #define _ATL_SECURE_NO_WARNINGS
    #define _CRT_SECURE_NO_WARNINGS
    #define _SCL_SECURE_NO_WARNINGS
    #define _CRT_DISABLE_PERFCRIT_LOCKS

    #define __STDC_WANT_SECURE_LIB__                      LE_CHECKED_BUILD
    #define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES         LE_CHECKED_BUILD
    #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES       LE_CHECKED_BUILD
    #define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT LE_CHECKED_BUILD
    #define _SECURE_ATL                                   LE_CHECKED_BUILD
    #ifndef _ITERATOR_DEBUG_LEVEL
        #define _SECURE_SCL                               LE_CHECKED_BUILD
        #define _ITERATOR_DEBUG_LEVEL                     LE_CHECKED_BUILD
    #endif // _ITERATOR_DEBUG_LEVEL
    #if defined( DEBUG ) || defined( _DEBUG )
        #define _HAS_ITERATOR_DEBUGGING                   LE_CHECKED_BUILD
        #if LE_CHECKED_BUILD
            #undef  _ITERATOR_DEBUG_LEVEL
            #define _ITERATOR_DEBUG_LEVEL 2
        #endif // LE_CHECKED_BUILD
    #endif  // DEBUG || _DEBUG

    #if !__STDC_WANT_SECURE_LIB__
        ////////////////////////////////////////////////////////////////////////
        //
        // Implementation note:
        //
        //   Certain headers and/or libraries do not properly use the
        // __STDC_WANT_SECURE_LIB__ macro and, for example, have hardcoded calls
        // to "secure CRT" functions or they only check for the
        // __STDC_SECURE_LIB__ (or the deprecated __GOT_SECURE_LIB__) macro and
        // expect the secure versions to exist if those macros are defined to a
        // sufficiently high version. Unfortunately the __STDC_SECURE_LIB__ can
        // not be predefined or undefined as the crtdefs.h header forcibly
        // defines it. This section provides a workaround for the mentioned
        // problems: forcibly includes crtdefs.h, undefines the two macros and
        // then defines several (pseudo) secure CRT function versions as
        // required by certain 'broken' headers/libraries.
        //                                           (25.02.2009.) (Domagoj)
        //
        ////////////////////////////////////////////////////////////////////////

        #include "crtdefs.h"
        #undef __STDC_SECURE_LIB__
        #undef __GOT_SECURE_LIB__

        // Implementation note:
        //   MSVC's CRT _wcstok_s_l function is defined even if
        // __STDC_SECURE_LIB__ is not and it references the wcstok_s function.
        //                                    (15.09.2011.) (Domagoj Saric)
        #define wcstok_s( strToken, strDelimit, context ) wcstok( strToken, strDelimit )

        // <required by Dinkumware STL's <xlocnum> header>
        #define sprintf_s _snprintf
        // </required by Dinkumware STL's <xlocnum> header>

        // <required by ATL>
        #include "string.h"
        #include "wchar.h"
        #define DEFINE_SECURE_MEMFUNCTION( nonSecureName, dataType )                                                                               \
        __inline errno_t nonSecureName##_s( dataType * const dest, size_t const numberOfElements, dataType const * const src, size_t const count ) \
        {                                                                                                                                          \
            (void)numberOfElements;                                                                                                                      \
            nonSecureName( dest, src, count );                                                                                                     \
            return 0;                                                                                                                              \
        }

        DEFINE_SECURE_MEMFUNCTION( memcpy  , void    )
        DEFINE_SECURE_MEMFUNCTION( memmove , void    )
        DEFINE_SECURE_MEMFUNCTION( wmemcpy , wchar_t )
        DEFINE_SECURE_MEMFUNCTION( wmemmove, wchar_t )

        #define DEFINE_SECURE_STRFUNCTION( nonSecureName, dataType )                                                                           \
        __inline errno_t nonSecureName##_s( dataType * const strDestination, size_t const numberOfElements, dataType const * const strSource ) \
        {                                                                                                                                      \
            (void)numberOfElements;                                                                                                                  \
            nonSecureName( strDestination, strSource );                                                                                        \
            return 0;                                                                                                                          \
        }

        DEFINE_SECURE_STRFUNCTION( strcpy , char    )
        DEFINE_SECURE_STRFUNCTION( wcscat , wchar_t )
        DEFINE_SECURE_STRFUNCTION( strcat , char    )
        DEFINE_SECURE_STRFUNCTION( wcscpy , wchar_t )

        #define DEFINE_SECURE_STRNFUNCTION DEFINE_SECURE_MEMFUNCTION
        DEFINE_SECURE_STRNFUNCTION( strncpy, char    )
        DEFINE_SECURE_STRNFUNCTION( wcsncpy, wchar_t )

        #undef DEFINE_SECURE_STRNFUNCTION
        #undef DEFINE_SECURE_STRFUNCTION
        #undef DEFINE_SECURE_MEMFUNCTION

        #define swprintf_s _snwprintf
        // </required by ATL>

    #endif  // !__STDC_WANT_SECURE_LIB__

	//   As we use a lot of heavy template (meta)programing it is actually
    // useful to instruct the MSVC++ compiler to be maximally aggressive with
    // inlining.
	#pragma inline_depth    ( 255 )
	#pragma inline_recursion( on  )

#elif defined( __GNUC__  ) // compiler

    // http://boost.sourceforge.net/doc/html/boost_tr1/config.html
    // http://boost.2283326.n4.nabble.com/TR1-and-BOOST-HAS-GCC-TR1-td3337508.html
    #define BOOST_HAS_TR1
    #define BOOST_HAS_GCC_TR1

    #if defined( __SSE__ )
        #define LE_HAS_SSE1
        // Implementation note:
        //   When compiling for the iOS simulator __SSE__ and __SSE2__ get
        // defined without __MMX__ which causes compilation errors.
        //                                    (28.11.2011.) (Domagoj Saric)
        #if !defined( __MMX__ )
            #define __MMX__
        #endif
    #endif

    #if defined( __SSE2__ )
        #define LE_HAS_SSE2
    #endif

#else

    #error Your compiler is not supported by the Little Endian build system.

#endif  // compiler


#ifdef __APPLE__
    #include <cstddef>
    #if !defined( _LIBCPP_VERSION ) && ( __GLIBCXX__ < 20110325 )
        namespace std { typedef void const * const nullptr_t; }
    #endif // old stdlibc++
#endif // __APPLE__


////////////////////////////////////////////////////////////////////////////////
//
// 3rd party library specifics.
// ----------------------------
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Boost.
////////////////////////////////////////////////////////////////////////////////

#define BOOST_NO_IOSTREAM
#ifdef NDEBUG
    #define BOOST_NO_TYPEID // implies BOOST_NO_RTTI
#endif // NDEBUG

#ifndef BOOST_EXCEPTION_DISABLE
    #define BOOST_EXCEPTION_DISABLE
#endif // BOOST_EXCEPTION_DISABLE

#define BOOST_COMMON_TYPE_DONT_USE_TYPEOF

#define BOOST_LEXICAL_CAST_ASSUME_C_LOCALE

#define BOOST_PHOENIX_NO_PREDEFINED_TERMINALS
#define BOOST_SPIRIT_NO_PREDEFINED_TERMINALS
#define BOOST_SPIRIT_MAX_LOCALS_SIZE 3
#define BOOST_SPIRIT_USE_PHOENIX_V3
#define BOOST_KARMA_NUMERICS_LOOP_UNROLL 0
#define SPIRIT_ARGUMENTS_LIMIT 3
#define SPIRIT_ATTRIBUTES_LIMIT 3
#define SPIRIT_NUMERICS_LOOP_UNROLL 1

// boost/detail/endian.hpp configuration
#if defined( __ANDROID__ ) && !( defined( __BIG_ENDIAN__ ) || defined( __LITTLE_ENDIAN__ ) )
    // http://stackoverflow.com/questions/6212951/endianness-of-android-ndk
    #include "sys/endian.h"
    #if _BYTE_ORDER == BIG_ENDIAN
        #define __BIG_ENDIAN__
    #else
        #define __LITTLE_ENDIAN__
    #endif // _BYTE_ORDER
#elif defined( _XBOX ) && !defined( __BIG_ENDIAN__ )
    #define __BIG_ENDIAN__
#endif // platform

// boost\tr1\detail\config_all.hpp configuration
#if defined( __ANDROID__ )
    #define BOOST_TR1_GCC_INCLUDE_PATH include
#elif defined( _XBOX )
    #define BOOST_TR1_DETAIL_CONFIG_ALL_HPP_INCLUDED
    #include <cstddef>
    #include <cstdlib>
    #define BOOST_TR1_STD_HEADER(name) <../xbox/name>
    #define BOOST_TR1_STD_CHEADER(name) BOOST_TR1_STD_HEADER(name)
    #define BOOST_HAS_CPP_0X
    #include BOOST_TR1_STD_HEADER(utility)
    #include <boost/tr1/detail/config.hpp>
#endif // _XBOX or __ANDROID__

/// \note Boost does not yet provide a BOOST_NO_CXX11_HDR_ATOMIC macro:
/// http://lists.boost.org/Archives/boost/2012/06/194554.php
/// http://www.boost.org/doc/libs/release/libs/config/doc/html/boost_config/boost_macro_reference.html
///                                           (10.10.2013.) (Domagoj Saric)
#include <ciso646>
#if defined( _MSC_VER )
    #if _MSC_VER == 1900
        #define BOOST_COMPILER_CONFIG "le/build/boost_compiler_config_msvc.hpp"
    #endif // MSVC14
#elif defined( _LIBCPP_VERSION )
#elif defined( __ANDROID__     ) && defined( __ARM_ARCH_5TE__ ) // libstdc++'s <atomic> does not seem to support ARMv5 (fails to compile)...
    #define BOOST_NO_CXX11_HDR_ATOMIC
#elif defined( __GLIBC__ ) || defined( __GLIBCPP__ ) || defined( __GLIBCXX__ ) || defined( __clang__ ) //...mrmlj...no __GLIBC*__ on OSX?
    #if ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) < 46 )
        #define BOOST_NO_CXX11_HDR_ATOMIC
    #endif
#endif // STL version


////////////////////////////////////////////////////////////////////////////////
// boost::throw_exception()
////////////////////////////////////////////////////////////////////////////////

// http://llvm.org/releases/3.6.0/tools/clang/docs/ReleaseNotes.html#the-exceptions-macro
#ifndef __clang__
    #define __has_feature( x ) 0
#endif // __clang__
#if defined( __cplusplus ) && ( ( defined( _MSC_VER ) && !defined( _CPPUNWIND ) ) || ( defined( __GNUC__ ) && ( !defined( __EXCEPTIONS ) || !__has_feature( cxx_exceptions ) ) ) )
    #ifndef BOOST_NO_EXCEPTIONS
        #define BOOST_NO_EXCEPTIONS
    #endif
    #include <cassert>
    namespace boost
    {
        template <class E>
        #if defined( _MSC_VER )
            __declspec( noreturn )
        #elif defined( __GNUC__ )
            __attribute__(( noreturn ))
        #endif
        inline void throw_exception( E const & /*e*/ )
        {
            assert( !"Exception!" );
            #if defined( _MSC_VER )
                __assume( false );
            #elif defined( __GNUC__ )
                __builtin_unreachable();
            #else
                ::abort();
            #endif
        }
    } // namespace boost
#elif !defined( JUCE_STRING_UTF_TYPE ) && defined( __cplusplus ) //...mrmlj...quick-hack detection of JUCE builds...
    /// \note Avoid the overhead of boost::throw_exception.
    ///                                       (30.08.2013.) (Domagoj Saric)
    #include "boost/throw_exception.hpp"
    #undef BOOST_THROW_EXCEPTION
    #define BOOST_THROW_EXCEPTION( x ) throw x
#endif // !JUCE


/// \note Import the fixes/workarounds for Boost.Range's lack of restricted
/// pointer support from NT2.
///                                           (11.09.2013.) (Domagoj Saric)
#if defined( LE_HAS_NT2 )
    #include "boost/dispatch/meta/is_iterator.hpp"
#endif // LE_HAS_NT2

//------------------------------------------------------------------------------
#endif // leConfigurationAndODRHeader_h
