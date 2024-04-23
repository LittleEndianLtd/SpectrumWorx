//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2013   Domagoj Saric, Little Endian Ltd.
//
//          Distributed under the Boost Software License, Version 1.0.
//               See accompanying file LICENSE.txt or copy at
//                   http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef BOOST_SIMD_SDK_SIMD_META_AS_SIMD_HPP_INCLUDED
#define BOOST_SIMD_SDK_SIMD_META_AS_SIMD_HPP_INCLUDED

#include <boost/simd/sdk/simd/extensions/meta/tags.hpp>
#include <boost/simd/forward/aligned_array.hpp>
#include <boost/simd/sdk/simd/preprocessor/repeat.hpp>
#include <boost/simd/sdk/config/type_lists.hpp>
#include <boost/simd/sdk/config/types.hpp>
#include <boost/simd/sdk/config/compiler.hpp>
#include <boost/simd/sdk/config/arch.hpp>
#include <boost/preprocessor/comparison/equal.hpp> //...mrmlj...only for clang...
#include <boost/preprocessor/control/if.hpp>       //...mrmlj...only for clang...
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/dispatch/meta/as_integer.hpp>      //...mrmlj...only for neon logical...
#include <boost/dispatch/meta/na.hpp>
#include <boost/type_traits/is_fundamental.hpp>
#include <boost/utility/enable_if.hpp>

// Forward-declare logical
namespace boost { namespace simd
{
  template<class T>
  struct logical;
} }

namespace boost { namespace simd { namespace meta
{
  template<class T, class Extension, class Enable = void>
  struct as_simd
  {
    typedef dispatch::meta::na_ type;
  };

  template<std::size_t N, class T>
  struct as_simd<T, tag::simd_emulation_<N>, typename enable_if< is_fundamental<T> >::type>
  {
    typedef boost::simd::aligned_array<T, N / sizeof(T)> type;
  };


#if defined( __GNUC__ ) && !defined( BOOST_SIMD_DETECTED )
    #define BOOST_SIMD_NATIVE_GCC
#endif // __GNUC__

//...mrmlj...when using GCC/Clang native vectors the vectors for logical seem to
//...mrmlj...have to be of the same size as their corresponding 'value'/number
//...mrmlj...vectors so the default logical<T> implementation has to be skipped
#ifndef BOOST_SIMD_NATIVE_GCC
  template<std::size_t N, class T>
  struct as_simd<logical<T>, tag::simd_emulation_<N> >
       : as_simd<T, tag::simd_emulation_<N> >
  {
  };
#endif // BOOST_SIMD_NATIVE_GCC


////////////////////////////////////////////////////////////////////////////////
// GCC/Clang native vector support
////////////////////////////////////////////////////////////////////////////////

#ifdef BOOST_SIMD_NATIVE_GCC

template <typename T> struct builtin_gcc_type { typedef T type; };

#if defined( __ARM_NEON__ ) || defined( BOOST_SIMD_ARCH_ARM_64 )
    //...mrmlj...gcc generates noticeably slower code when unsigned is used
    //...mrmlj...in order to remove the casts from the affected intrinsics
    //...mrmlj...calls...
    template <std::size_t N, class T>
    struct as_simd<logical<T>, tag::simd_emulation_<N> > : as_simd< typename dispatch::meta::as_integer<T>::type, tag::simd_emulation_<N> > {};

    #if defined( __clang__ )
        template <> struct as_simd<double, tag::simd_emulation_<16> > { typedef double type __attribute__((vector_size(16), may_alias)); };
        #define BOOST_SIMD_AUX_BUILTIN_VECTOR( scalar_t, sizeof_vector )                                            \
            BOOST_PP_IF                                                                                             \
            (                                                                                                       \
                BOOST_PP_EQUAL( sizeof_vector, 16 ),                                                                \
                typedef __attribute__((neon_vector_type(sizeof_vector/sizeof(scalar_t)), may_alias)) scalar_t type, \
                typedef scalar_t type __attribute__((vector_size(sizeof_vector), may_alias))                        \
            )
    #else // GCC
        template <> struct builtin_gcc_type<boost::simd::int8_t  > { typedef __builtin_neon_qi  type; };
        template <> struct builtin_gcc_type<boost::simd::uint8_t > { typedef __builtin_neon_uqi type; };
        template <> struct builtin_gcc_type<boost::simd::int16_t > { typedef __builtin_neon_hi  type; };
        template <> struct builtin_gcc_type<boost::simd::uint16_t> { typedef __builtin_neon_uhi type; };
        template <> struct builtin_gcc_type<boost::simd::int32_t > { typedef __builtin_neon_si  type; };
        template <> struct builtin_gcc_type<boost::simd::uint32_t> { typedef __builtin_neon_usi type; };
        template <> struct builtin_gcc_type<boost::simd::int64_t > { typedef __builtin_neon_di  type; };
        template <> struct builtin_gcc_type<boost::simd::uint64_t> { typedef __builtin_neon_udi type; };
        template <> struct builtin_gcc_type<float                > { typedef __builtin_neon_sf  type; };
    #endif // GCC/Clang
#else // generic
    template <std::size_t N, class T>
    struct as_simd<logical<T>, tag::simd_emulation_<N> > : as_simd< typename dispatch::meta::as_integer<T, unsigned>::type, tag::simd_emulation_<N> > {};
#endif // target CPU

#ifndef BOOST_SIMD_AUX_BUILTIN_VECTOR
    #define BOOST_SIMD_AUX_BUILTIN_VECTOR( scalar_t, sizeof_vector )  \
        typedef typename builtin_gcc_type<scalar_t>::type type __attribute__((vector_size(sizeof_vector), may_alias));
#endif // BOOST_SIMD_AUX_BUILTIN_VECTOR


  // Some GCC and Clang versions require full specializations
  #define M0(r,n,t)                           \
  template<>                                  \
  struct as_simd<t, tag::simd_emulation_<n> > \
  {                                           \
    BOOST_SIMD_AUX_BUILTIN_VECTOR( t, n );    \
  };                                          \
  /**/

//...mrmlj...clang chokes on doubles with neon...
#if defined( __clang__ ) && ( defined( __ARM_NEON__ ) || defined( BOOST_SIMD_ARCH_ARM_64 ) )
  #define M1(z,n,t) BOOST_PP_SEQ_FOR_EACH(M0, n, (boost::simd::uint64_t)(boost::simd::int64_t)(boost::simd::uint32_t)(boost::simd::int32_t)(float)(boost::simd::uint16_t)(boost::simd::int16_t)(boost::simd::uint8_t)(boost::simd::int8_t))
// GCC bug with 64-bit integer types on i686
// Also affects GCC 4.8.x on x86-64
#elif defined(BOOST_SIMD_COMPILER_GCC) && defined(BOOST_SIMD_ARCH_X86) && ((__GNUC__ == 4 && __GNUC_MINOR__ == 8) || !defined(BOOST_SIMD_ARCH_X86_64))
  #define M1(z,n,t) BOOST_PP_SEQ_FOR_EACH(M0, n, BOOST_SIMD_SPLITABLE_TYPES(double))
#else
  #define M1(z,n,t) BOOST_PP_SEQ_FOR_EACH(M0, n, BOOST_SIMD_TYPES)
#endif

  M0(0, 1, boost::simd::int8_t)
  M0(0, 1, boost::simd::uint8_t)
  M0(0, 2, boost::simd::int8_t)
  M0(0, 2, boost::simd::uint8_t)
  M0(0, 2, boost::simd::int16_t)
  M0(0, 2, boost::simd::uint16_t)
  M0(0, 4, boost::simd::int8_t)
  M0(0, 4, boost::simd::uint8_t)
  M0(0, 4, boost::simd::int16_t)
  M0(0, 4, boost::simd::uint16_t)
  M0(0, 4, boost::simd::int32_t)
  M0(0, 4, boost::simd::uint32_t)
  M0(0, 4, float)
  BOOST_SIMD_PP_REPEAT_POWER_OF_2_FROM(8, M1, ~)

  #undef M0
  #undef M1

  #undef BOOST_SIMD_AUX_BUILTIN_VECTOR

#endif // BOOST_SIMD_NATIVE_GCC
} } }

#endif
