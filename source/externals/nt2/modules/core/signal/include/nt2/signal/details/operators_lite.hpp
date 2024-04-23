//==============================================================================
//          Copyright 2015 Domagoj Saric, Little Endian Ltd.
//
// Basic arithmetic and logical functionality native compiler vector types for
// use in simpler contexts that do not require the full Boost.SIMD/NT2 machinery
// in order to reduce the compile time and runtime overheads.
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef OPERATORS_LITE_HPP_INCLUDED
#define OPERATORS_LITE_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <boost/simd/sdk/simd/meta/as_simd.hpp>
//------------------------------------------------------------------------------
/// \note Clang (3.5) still chokes on operator^ even though it is supposed to
/// support it at least on x86 targets.
/// http://clang.llvm.org/docs/LanguageExtensions.html#vector-operations
///                                           (29.01.2015.) (Domagoj Saric)
#if defined( __GNUC__ ) && !defined( __clang__ )

    #define BOOST_SIMD_HAS_LITE_OPERATORS

    namespace boost
    {
    namespace simd
    {
    namespace details
    {
        using floatv4  = typename meta::as_simd<float           , BOOST_SIMD_DEFAULT_EXTENSION>::type;
        using doublev2 = typename meta::as_simd<double          , BOOST_SIMD_DEFAULT_EXTENSION>::type;
        using intv4    = typename meta::as_simd</*std::*/int32_t, BOOST_SIMD_DEFAULT_EXTENSION>::type;
    } // namespace details
    } // namespace simd
    } // namespace boost

#elif defined( _MSC_VER )

    #define BOOST_SIMD_HAS_LITE_OPERATORS

    BOOST_FORCEINLINE __m128  __vectorcall operator + ( __m128  const left, __m128  const right ) { return _mm_add_ps   ( left, right ); }
    BOOST_FORCEINLINE __m128  __vectorcall operator - ( __m128  const left, __m128  const right ) { return _mm_sub_ps   ( left, right ); }
    BOOST_FORCEINLINE __m128  __vectorcall operator * ( __m128  const left, __m128  const right ) { return _mm_mul_ps   ( left, right ); }
    BOOST_FORCEINLINE __m128  __vectorcall operator / ( __m128  const left, __m128  const right ) { return _mm_div_ps   ( left, right ); }
    BOOST_FORCEINLINE __m128  __vectorcall operator & ( __m128  const left, __m128  const right ) { return _mm_and_ps   ( left, right ); }
    BOOST_FORCEINLINE __m128  __vectorcall operator | ( __m128  const left, __m128  const right ) { return _mm_or_ps    ( left, right ); }
    BOOST_FORCEINLINE __m128  __vectorcall operator ^ ( __m128  const left, __m128  const right ) { return _mm_xor_ps   ( left, right ); }

    BOOST_FORCEINLINE __m128d __vectorcall operator + ( __m128d const left, __m128d const right ) { return _mm_add_pd   ( left, right ); }
    BOOST_FORCEINLINE __m128d __vectorcall operator - ( __m128d const left, __m128d const right ) { return _mm_sub_pd   ( left, right ); }
    BOOST_FORCEINLINE __m128d __vectorcall operator * ( __m128d const left, __m128d const right ) { return _mm_mul_pd   ( left, right ); }
    BOOST_FORCEINLINE __m128d __vectorcall operator / ( __m128d const left, __m128d const right ) { return _mm_div_pd   ( left, right ); }
    BOOST_FORCEINLINE __m128d __vectorcall operator & ( __m128d const left, __m128d const right ) { return _mm_and_pd   ( left, right ); }
    BOOST_FORCEINLINE __m128d __vectorcall operator | ( __m128d const left, __m128d const right ) { return _mm_or_pd    ( left, right ); }
    BOOST_FORCEINLINE __m128d __vectorcall operator ^ ( __m128d const left, __m128d const right ) { return _mm_xor_pd   ( left, right ); }

    BOOST_FORCEINLINE __m128i __vectorcall operator + ( __m128i const left, __m128i const right ) { return _mm_add_epi32( left, right ); }
    BOOST_FORCEINLINE __m128i __vectorcall operator - ( __m128i const left, __m128i const right ) { return _mm_sub_epi32( left, right ); }
    BOOST_FORCEINLINE __m128i __vectorcall operator * ( __m128i const left, __m128i const right ) { return _mm_mul_epi32( left, right ); }
    BOOST_FORCEINLINE __m128i __vectorcall operator & ( __m128i const left, __m128i const right ) { return _mm_and_si128( left, right ); }
    BOOST_FORCEINLINE __m128i __vectorcall operator | ( __m128i const left, __m128i const right ) { return _mm_or_si128 ( left, right ); }
    BOOST_FORCEINLINE __m128i __vectorcall operator ^ ( __m128i const left, __m128i const right ) { return _mm_xor_si128( left, right ); }

    namespace boost
    {
    namespace simd
    {
    namespace details
    {
        using floatv4  = __m128 ;
        using doublev2 = __m128d;
        using intv4    = __m128i;
    } // namespace details
    } // namespace simd
    } // namespace boost

#else

    #include <boost/simd/operator/functions/bitwise_and.hpp>
    #include <boost/simd/operator/functions/bitwise_or.hpp>
    #include <boost/simd/operator/functions/bitwise_xor.hpp>
    #include <boost/simd/operator/functions/complement.hpp>
    #include <boost/simd/operator/functions/divides.hpp>
    #include <boost/simd/operator/functions/is_less.hpp>
    #include <boost/simd/operator/functions/is_greater_equal.hpp>
    #include <boost/simd/operator/functions/minus.hpp>
    #include <boost/simd/operator/functions/modulo.hpp>
    #include <boost/simd/operator/functions/multiplies.hpp>
    #include <boost/simd/operator/functions/plus.hpp>
    #include <boost/simd/operator/functions/unary_minus.hpp>

    #include <boost/simd/sdk/config/arch.hpp>
    #include <boost/simd/sdk/simd/extensions.hpp>
    #include <boost/simd/sdk/simd/native.hpp>

#endif // compiler w/ or w/o native operator support for vector types

//------------------------------------------------------------------------------
#ifdef BOOST_SIMD_HAS_LITE_OPERATORS
    #include <boost/dispatch/meta/value_of.hpp>
    #include <boost/simd/sdk/meta/cardinal_of.hpp>
    #include <boost/simd/sdk/simd/meta/vector_of.hpp>
    #include <boost/simd/sdk/simd/native.hpp>

    #if defined( _MSC_VER ) && defined( BOOST_SIMD_HAS_SSE2_SUPPORT )
        #include "emmintrin.h"
    #endif
#endif // BOOST_SIMD_HAS_LITE_OPERATORS
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace simd
{
//------------------------------------------------------------------------------
namespace meta
{
//------------------------------------------------------------------------------
template <typename Vector> struct operator_only_lite_vector { typedef          Vector              type; };
template <typename Vector> struct full_vector               { typedef          Vector              type; };
template <typename Vector> struct compiler_vector           { typedef typename Vector::native_type type; };

#ifdef BOOST_SIMD_HAS_LITE_OPERATORS
    template <> struct operator_only_lite_vector<typename boost::simd::meta::vector_of<float           , 4>::type> { typedef details::floatv4  type; };
    template <> struct operator_only_lite_vector<typename boost::simd::meta::vector_of<double          , 2>::type> { typedef details::doublev2 type; };
    template <> struct operator_only_lite_vector<typename boost::simd::meta::vector_of</*std::*/int32_t, 4>::type> { typedef details::intv4    type; };

    template <> struct full_vector<details::floatv4 > : boost::simd::meta::vector_of<float           , 4>::type {};
    template <> struct full_vector<details::doublev2> : boost::simd::meta::vector_of<double          , 2>::type {};
    template <> struct full_vector<details::intv4   > : boost::simd::meta::vector_of</*std::*/int32_t, 4>::type {};

    template <> struct compiler_vector<details::floatv4 > { typedef details::floatv4  type; };
    template <> struct compiler_vector<details::doublev2> { typedef details::doublev2 type; };
    template <> struct compiler_vector<details::intv4   > { typedef details::intv4    type; };

    template <> struct cardinal_of<details::floatv4 > : boost::mpl::size_t<4> {};
    template <> struct cardinal_of<details::doublev2> : boost::mpl::size_t<2> {};
    template <> struct cardinal_of<details::intv4   > : boost::mpl::size_t<4> {};
#endif // BOOST_SIMD_HAS_LITE_OPERATORS
//------------------------------------------------------------------------------
} // namespace meta
//------------------------------------------------------------------------------

template <typename Scalar, typename Extension, typename Enable>
Scalar * scalars( native<Scalar, Extension, Enable> & vector ) { return vector. data(); }
template <typename Scalar, typename Extension, typename Enable>
Scalar * scalars( native<Scalar, Extension, Enable> * vector ) { return vector->data(); }

template <typename Scalar, typename Extension, typename Enable>
typename native<Scalar, Extension, Enable>::native_type       & compiler_vector( native<Scalar, Extension, Enable>       & vector ) { return vector.data_; }
template <typename Scalar, typename Extension, typename Enable>
typename native<Scalar, Extension, Enable>::native_type const & compiler_vector( native<Scalar, Extension, Enable> const & vector ) { return vector.data_; }

#ifdef BOOST_SIMD_HAS_LITE_OPERATORS
    BOOST_FORCEINLINE float            * scalars( details::floatv4  & vector ) { return reinterpret_cast<float            *>( &vector ); }
    BOOST_FORCEINLINE float            * scalars( details::floatv4  * vector ) { return reinterpret_cast<float            *>(  vector ); }
    BOOST_FORCEINLINE double           * scalars( details::doublev2 & vector ) { return reinterpret_cast<double           *>( &vector ); }
    BOOST_FORCEINLINE double           * scalars( details::doublev2 * vector ) { return reinterpret_cast<double           *>(  vector ); }
    BOOST_FORCEINLINE /*std::*/int32_t * scalars( details::intv4    & vector ) { return reinterpret_cast</*std::*/int32_t *>( &vector ); }
    BOOST_FORCEINLINE /*std::*/int32_t * scalars( details::intv4    * vector ) { return reinterpret_cast</*std::*/int32_t *>(  vector ); }

    BOOST_FORCEINLINE details::floatv4        & compiler_vector( details::floatv4        & vector ) { return vector; }
    BOOST_FORCEINLINE details::floatv4  const & compiler_vector( details::floatv4  const & vector ) { return vector; }
    BOOST_FORCEINLINE details::doublev2       & compiler_vector( details::doublev2       & vector ) { return vector; }
    BOOST_FORCEINLINE details::doublev2 const & compiler_vector( details::doublev2 const & vector ) { return vector; }
    BOOST_FORCEINLINE details::intv4          & compiler_vector( details::intv4          & vector ) { return vector; }
    BOOST_FORCEINLINE details::intv4    const & compiler_vector( details::intv4    const & vector ) { return vector; }
#endif // BOOST_SIMD_HAS_LITE_OPERATORS

//------------------------------------------------------------------------------
} // namespace simd
//------------------------------------------------------------------------------
#ifdef BOOST_SIMD_HAS_LITE_OPERATORS
namespace dispatch
{
//------------------------------------------------------------------------------
namespace meta
{
//------------------------------------------------------------------------------
template <> struct value_of<simd::details::floatv4 > { typedef float            type; };
template <> struct value_of<simd::details::doublev2> { typedef double           type; };
template <> struct value_of<simd::details::intv4   > { typedef /*std::*/int32_t type; };
//------------------------------------------------------------------------------
} // namespace meta
//------------------------------------------------------------------------------
} // namespace dispatch
//------------------------------------------------------------------------------
#endif // BOOST_SIMD_HAS_LITE_OPERATORS
} // namespace boost
//------------------------------------------------------------------------------
#endif // OPERATORS_LITE_HPP_INCLUDED
