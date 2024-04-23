////////////////////////////////////////////////////////////////////////////////
///
/// \file tchar.hpp
/// ---------------
///
/// Selected (Microsoft's) tchar.h bits for compilers that do not provide them.
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef tchar_hpp__5137b405_2BF4_4714_8BB3_8F3342D64267
#define tchar_hpp__5137b405_2BF4_4714_8BB3_8F3342D64267
#pragma once
//------------------------------------------------------------------------------
#include "boost/utility/string_ref.hpp"
#ifdef _MSC_VER
    #include "tchar.h"
#else
    #include <cstdlib>
    #include <cstring>

    #ifdef _UNICODE
        typedef wchar_t TCHAR;

        #define _T( x ) L##x

        #define _itot       _itow
        #define _ltot       _ltow
        #define _ultot      _ultow

        #define _tcscpy     wcscpy
        #define _tcslen     wcslen
        #define _tcscmp     wcscmp
        #define _tcsrchr    wcsrchr

        #define _stprintf   swprintf
        #define _sntprintf  _snwprintf
    #else
        typedef char TCHAR;

        #define _T( x ) x

        #define _itot       _itoa
        #define _ltot       _ltoa
        #define _ultot      _ultoa

        #define _tcscpy     strcpy
        #define _tcslen     strlen
        #define _tcscmp     strcmp
        #define _tcsrchr    strrchr

        #define _stprintf   sprintf
        #define _sntprintf  snprintf
    #endif
#endif // _MSC_VER

#ifdef _WIN32
    extern "C" __declspec( dllimport ) int __cdecl wsprintfA( __out char    *, __in __format_string char    const *, ... );
    extern "C" __declspec( dllimport ) int __cdecl wsprintfW( __out wchar_t *, __in __format_string wchar_t const *, ... );
    #define LE_INT_SPRINTFA ::wsprintfA
    #define LE_INT_SPRINTFW ::wsprintfW
#else // _WIN32
    #define LE_INT_SPRINTFA std::sprintf
    #define LE_INT_SPRINTFW std::swprintf
#endif // _WIN32

#ifdef _UNICODE
    #define LE_INT_SPRINTF LE_INT_SPRINTFW
#else
    #define LE_INT_SPRINTF LE_INT_SPRINTFA
#endif // _UNICODE


//...mrmlj...quick-fixes for Boost.Range restrict support...
#include "platformSpecifics.hpp"
#ifdef LE_HAS_NT2
#include "boost/dispatch/meta/is_iterator.hpp"
#endif // LE_HAS_NT2
#include "boost/type_traits/detail/yes_no_type.hpp"
namespace boost
{
#ifdef BOOST_DISPATCH_NO_RESTRICT
    template <typename T> struct is_pointer;
    template <typename T> struct is_pointer  <T       * LE_RESTRICT> : std::true_type {};
    template <typename T> struct is_pointer  <T const * LE_RESTRICT> : std::true_type {};
#endif // BOOST_DISPATCH_NO_RESTRICT
    template <typename T> struct remove_const;
    template <typename T> struct remove_const<T       * LE_RESTRICT const> { typedef T       * LE_RESTRICT type; };
    template <typename T> struct remove_const<T const * LE_RESTRICT const> { typedef T const * LE_RESTRICT type; };

    namespace range_detail
    {
        using type_traits::yes_type;
        using type_traits::no_type;

        yes_type is_string_impl           ( char    const * LE_RESTRICT );
        yes_type is_string_impl           ( wchar_t const * LE_RESTRICT );
        yes_type is_char_ptr_impl         ( char          * LE_RESTRICT );
        yes_type is_const_char_ptr_impl   ( char    const * LE_RESTRICT );
        yes_type is_wchar_t_ptr_impl      ( wchar_t       * LE_RESTRICT );
        yes_type is_const_wchar_t_ptr_impl( wchar_t const * LE_RESTRICT );

        //inline bool is_char_ptr( char       * LE_RESTRICT ) { return true; }
        //inline bool is_char_ptr( char const * LE_RESTRICT ) { return true; }
    } // namespace 'range_detail'

    //inline char       * LE_RESTRICT range_begin( char       * LE_RESTRICT const & pString ) { return pString                         ; }
    //inline char       * LE_RESTRICT range_end  ( char       * LE_RESTRICT const & pString ) { return pString + std::strlen( pString ); }
    //inline char const * LE_RESTRICT range_begin( char const * LE_RESTRICT const & pString ) { return pString                         ; }
    //inline char const * LE_RESTRICT range_end  ( char const * LE_RESTRICT const & pString ) { return pString + std::strlen( pString ); }
} // namespace boost

LE_COLD
inline bool operator==( boost::string_ref const & left, boost::string_ref const & right )
{
    return
        ( left.size() == right.size() ) &&
        ( std::memcmp( left.begin(), right.begin(), right.size() ) == 0 );
}

//------------------------------------------------------------------------------
#endif // tchar_hpp
