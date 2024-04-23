////////////////////////////////////////////////////////////////////////////////
///
/// \file typeTraits.hpp
/// --------------------
///
///   Imports TR1 type traits into the std namespace if the compiler does not
/// provide true C++11 type traits.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef typeTraits_hpp__45496FE2_5F16_4115_8225_39355C7AB4D5
#define typeTraits_hpp__45496FE2_5F16_4115_8225_39355C7AB4D5
#pragma once
//------------------------------------------------------------------------------
#include "boost/tr1/detail/config_all.hpp"

#include <ciso646>

#ifdef _STLPORT_VERSION
    #error STLPort not supported - please use a more up to date C++ standard library implementation
#endif // _STLPORT_VERSION

// http://gcc.gnu.org/onlinedocs/libstdc++/manual/abi.html#abi.versioning
#if defined( __APPLE__ ) && !defined( _LIBCPP_VERSION ) && ( __GLIBCXX__ < 20110325 )

    /// \note GCC requires that a trimmed string be passed to BOOST_TR1_HEADER.
    ///                                       (27.09.2012.) (Domagoj Saric)
    #include BOOST_TR1_HEADER(type_traits)
    #include "boost/type_traits/make_signed.hpp"
    namespace std
    {
        using ::boost::make_signed;
        using namespace tr1;
    } // namespace std

    #define LE_AUX_TYPE_TRAITS_NAMESPACE_BEGIN() namespace std { namespace tr1 {
    #define LE_AUX_TYPE_TRAITS_NAMESPACE_END()   } }

#else

#include <type_traits>

#ifdef _MSC_VER
    #define LE_AUX_TYPE_TRAITS_NAMESPACE_BEGIN() namespace std { namespace tr1 {
    #define LE_AUX_TYPE_TRAITS_NAMESPACE_END()   } }
#else
    #define LE_AUX_TYPE_TRAITS_NAMESPACE_BEGIN() namespace std {
    #define LE_AUX_TYPE_TRAITS_NAMESPACE_END()   }
#endif

#endif // type traits impl

#include "abi.hpp"
//------------------------------------------------------------------------------

LE_AUX_TYPE_TRAITS_NAMESPACE_BEGIN()
#if defined( _LIBCPP_VERSION )
    template <typename T> using has_trivial_default_constructor = is_trivially_default_constructible<T>;
    template <typename T> using has_trivial_destructor          = is_trivially_destructible         <T>;
#elif ( __GLIBCXX__ >= 20110325 )
    template <typename T> using has_trivial_destructor          = is_trivially_destructible         <T>;
#elif defined( __clang__ )
    template <typename T> struct has_trivial_default_constructor : boost::mpl::bool_<__is_trivially_constructible( T )> {};
  //template <typename T> struct has_trivial_destructor          : boost::mpl::bool_<__has_trivial_destructor    ( T )> {};
#endif

#if defined( _LIBCPP_VERSION )
    template <typename T> struct is_trivially_default_constructible<T * LE_RESTRICT> : true_type {};
    template <typename T> struct is_trivially_destructible         <T * LE_RESTRICT> : true_type {};
#elif ( __GLIBCXX__ >= 20110325 )
    template <typename T> struct is_trivially_destructible         <T * LE_RESTRICT> : true_type {};
    template <typename T> struct has_trivial_default_constructor   <T * LE_RESTRICT> : true_type {};
#elif defined( __GLIBCXX__ ) && defined( BOOST_HAS_TR1_TYPE_TRAITS )
    template <typename T> struct has_trivial_destructor            <T * LE_RESTRICT> : true_type {};
#endif // STL

/// \note Workarounds for lack of restricted pointer support in most STL's.
/// Check if a third party workaround is already included.
///                                           (11.09.2013.) (Domagoj Saric)
#if !defined( BOOST_DISPATCH_RESTRICT ) || defined( BOOST_HAS_TR1_TYPE_TRAITS ) || defined( BOOST_HAS_CPP_0X )
template <typename T> struct is_pointer<T * LE_RESTRICT> : true_type {};
#endif
LE_AUX_TYPE_TRAITS_NAMESPACE_END()

#ifdef __clang__
template <typename T>
void * __attribute__(( nothrow )) operator new( std::size_t /*count*/, T * LE_RESTRICT * const pStorage ) { LE_ASSUME( pStorage ); return reinterpret_cast<void *>( reinterpret_cast<std::size_t>( pStorage ) ); }
#endif // __clang__

#undef LE_AUX_TYPE_TRAITS_NAMESPACE_BEGIN
#undef LE_AUX_TYPE_TRAITS_NAMESPACE_END

//------------------------------------------------------------------------------
#endif // typeTraits_hpp
