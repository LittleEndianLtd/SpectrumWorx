////////////////////////////////////////////////////////////////////////////////
///
/// \file rvalueReferences.hpp
/// --------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef rvalueReferences_hpp__D83532BA_1B66_4B90_BC66_20F6AE32F0A5
#define rvalueReferences_hpp__D83532BA_1B66_4B90_BC66_20F6AE32F0A5
#pragma once
//------------------------------------------------------------------------------
#if defined( __GNUC__ )
    #include <ciso646>
#endif

#if defined( _MSC_VER ) || defined( _LIBCPP_VERSION ) || ( defined( __ANDROID__ ) && defined( __GLIBCXX__ ) )
    #include <utility>
#else
    #include "typeTraits.hpp"

    #include "boost/config.hpp"
    #include "boost/type_traits/add_rvalue_reference.hpp"
#endif
//------------------------------------------------------------------------------

// Implementation note:
//   Clang on OS X (versions prior to Xcode 4.1 and Lion) and STLport on Android
// do not provide a C++0x STL implementation so we must add certain minimum
// functionality when compiling with -std=c++0x.
//                                            (30.08.2011.) (Domagoj Saric)
#if !( defined( _MSC_VER ) || defined( _LIBCPP_VERSION ) || ( defined( __ANDROID__ ) && defined( __GLIBCXX__ ) ) )
namespace std
{
#ifdef _STLPORT_VERSION
    #define LE_IDENTITY_TPL_NAME identity_aux
#else
    #define LE_IDENTITY_TPL_NAME identity
#endif

    template <class T>
    struct LE_IDENTITY_TPL_NAME
    {
        typedef T type;
        T const & operator()( T const & left ) const { return left; }
    };

    template <class T> inline
    T && forward( typename LE_IDENTITY_TPL_NAME<T>::type & arg ) { return static_cast<T &&>( arg ); }

    template <class T> inline
    typename remove_reference<T>::type && move( T && arg ) { return static_cast<typename remove_reference<T>::type &&>( arg ); }

    #undef LE_IDENTITY_TPL_NAME

    template <class T>
    typename boost::add_rvalue_reference<T>::type declval();
} // namespace std
#endif // Clang with 0x support with libstdc++, GCC with 0x support with STLPort...

//------------------------------------------------------------------------------
#endif // rvalueReferences_hpp
