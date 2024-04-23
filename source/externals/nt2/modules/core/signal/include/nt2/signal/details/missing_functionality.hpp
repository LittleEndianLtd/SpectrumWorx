//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2015   Domagoj Saric, Little Endian Ltd.
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef MISSING_FUNCTIONALITY_HPP_INCLUDED
#define MISSING_FUNCTIONALITY_HPP_INCLUDED
//------------------------------------------------------------------------------
#include "operators_lite.hpp" // for compiler_vector()

#include <boost/simd/memory/functions/extract.hpp>
#include <boost/simd/memory/functions/insert.hpp>
#include <boost/simd/memory/functions/load.hpp>
#include <boost/simd/memory/functions/make.hpp>
#include <boost/simd/memory/functions/splat.hpp>
#include <boost/simd/memory/functions/store.hpp>
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
#include <boost/simd/operator/functions/shift_left.hpp>
#include <boost/simd/operator/functions/shift_right.hpp>
#include <boost/simd/operator/functions/unary_minus.hpp>
#include <boost/simd/preprocessor/make_helper.hpp>
#include <boost/simd/swar/functions/details/shuffle.hpp>
#include <boost/simd/swar/functions/deinterleave_first.hpp>
#include <boost/simd/swar/functions/deinterleave_second.hpp>
#include <boost/simd/swar/functions/interleave_first.hpp>
#include <boost/simd/swar/functions/interleave_second.hpp>
#include <boost/simd/swar/functions/reverse.hpp>

#include <boost/simd/memory/iterator.hpp>
#include <boost/simd/sdk/config/arch.hpp>
#include <boost/simd/sdk/simd/extensions.hpp>
#include <boost/simd/sdk/simd/native.hpp>

#include <boost/type_traits/is_const.hpp>

#if defined( __ARM_NEON__ ) || defined( BOOST_SIMD_ARCH_ARM_64 )
#include "arm_neon.h"
#endif // ARM Neon
//------------------------------------------------------------------------------
#if defined( _MSC_VER )

    #define BOOST_NOTHROW_NOALIAS __declspec( nothrow noalias )
    #if ( _MSC_VER >= 1800 ) && ( defined( _M_X64 ) || ( _M_IX86_FP >= 2 ) )
        #define BOOST_FASTCALL __vectorcall
    #else
        #define BOOST_FASTCALL __fastcall
    #endif // _MSC_VER
    #define BOOST_UNREACHABLE_CODE()  BOOST_ASSERT_MSG( false    , "This code should not be reached." ); __assume( false     )
    #define BOOST_ASSUME( condition ) BOOST_ASSERT_MSG( condition, "Assumption broken."               ); __assume( condition )

#elif defined( __GNUC__ )

    /// \note The 'pure' (and especially 'const') attribute(s) seem to be too
    /// strict to mimic the MSVC 'noalias' attribute (which allows first level
    /// indirections).
    ///                                       (09.07.2012.) (Domagoj Saric)
    #define BOOST_NOTHROW_NOALIAS __attribute__(( nothrow ))
    #if defined( BOOST_SIMD_ARCH_X86 ) && !defined( BOOST_SIMD_ARCH_X86_64 )
        #if defined( BOOST_SIMD_HAS_SSE_SUPPORT ) && !defined( __clang__ )
            #define BOOST_FASTCALL __attribute__(( regparm( 3 ), sseregparm ))
        #else
            #define BOOST_FASTCALL __attribute__(( regparm( 3 ) ))
        #endif // __clang__
    #endif // BOOST_SIMD_ARCH_X86

    // http://en.chys.info/2010/07/counterpart-of-assume-in-gcc
    // http://nondot.org/sabre/LLVMNotes/BuiltinUnreachable.txt
    #if ( __clang_major__ >= 2 ) || ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) >= 45 )
        #define BOOST_UNREACHABLE_CODE()  BOOST_ASSERT_MSG( false    , "This code should not be reached." ); __builtin_unreachable()
        #if defined( __clang__ ) || ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) < 46 ) || ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) > 47 )
            // Broken/pessimization in GCC 4.6:
            //  https://bugs.launchpad.net/gcc-linaro/+bug/1020601
            //  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=50385
            //  http://gcc.gnu.org/ml/gcc-patches/2012-07/msg00254.html
            //  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=49054
            #define BOOST_ASSUME( condition ) BOOST_ASSERT_MSG( condition, "Assumption broken." ); do { if ( !( condition ) ) __builtin_unreachable(); } while ( 0 )
        #endif
    #endif

    #define BOOST_HOT  __attribute__(( hot  ))
    #define BOOST_COLD __attribute__(( cold ))

#endif // compiler

#ifndef BOOST_NOTHROW_NOALIAS
    #define BOOST_NOTHROW_NOALIAS
#endif // BOOST_NOTHROW_NOALIAS

#ifndef BOOST_FASTCALL
    #define BOOST_FASTCALL
#endif // BOOST_FASTCALL

#ifndef BOOST_HOT
    #define BOOST_HOT
    #define BOOST_COLD
#endif // BOOST_HOT&COLD

#ifndef BOOST_UNREACHABLE_CODE
    #define BOOST_UNREACHABLE_CODE() BOOST_ASSERT_MSG( false, "This code should not be reached." )
#endif // BOOST_UNREACHABLE_CODE

/// \note We want assume to have verify semantics (i.e. the condition expression
/// should be evaluated in all builds.
///                                           (13.11.2012.) (Domagoj Saric)
#ifndef BOOST_ASSUME
    #define BOOST_ASSUME( condition ) BOOST_VERIFY( condition && "Assumption broken." )
#endif // BOOST_ASSUME
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace simd
{
//------------------------------------------------------------------------------
namespace ext
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// GCC/Clang native vector operators for emulation
//
////////////////////////////////////////////////////////////////////////////////

/// \note Using the below native specializations with Clang 3.3 and 3.4 when
/// targeting the x86 architecture causes assertion failures in mask2logical()
/// during FFT twiddle factor calculation (might have 'something' to do with the
/// complement and/or extract operations). For this reason we skip/disable these
/// specializations when Boost.SIMD/NT2 already provides its own
/// (BOOST_SIMD_DETECTED is defined).
///                                           (05.05.2014.) (Domagoj Saric)
#if !defined( BOOST_SIMD_DETECTED )

#if ( __GNUC__ >= 4 )

#if ( __GNUC_MINOR__ >= 5 ) || defined( __clang__ )
  #define BOOST_SIMD_HAS_VECTORIZABLE_EMULATION

  BOOST_DISPATCH_IMPLEMENT( minus_      , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) - compiler_vector( a1 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( plus_       , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) + compiler_vector( a1 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( multiplies_ , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) * compiler_vector( a1 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( divides_    , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) / compiler_vector( a1 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( modulo_     , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) % compiler_vector( a1 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( unary_minus_, tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL(1) { return -compiler_vector( a0 ); }
  };
  //...mrmlj....
  typedef native<unsigned int, BOOST_SIMD_DEFAULT_EXTENSION>::native_type _ivec;
  BOOST_DISPATCH_IMPLEMENT( bitwise_and_, tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { _ivec const result( (_ivec const &)compiler_vector( a0 ) & (_ivec const &)compiler_vector( a1 ) ); return (result_type const &)result; }
  };
  BOOST_DISPATCH_IMPLEMENT( bitwise_or_ , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { _ivec const result( (_ivec const &)compiler_vector( a0 ) | (_ivec const &)compiler_vector( a1 ) ); return (result_type const &)result; }
  };
  BOOST_DISPATCH_IMPLEMENT( bitwise_xor_, tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { _ivec const result( (_ivec const &)compiler_vector( a0 ) ^ (_ivec const &)compiler_vector( a1 ) ); return (result_type const &)result; }
  };
  BOOST_DISPATCH_IMPLEMENT( complement_ , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL(1) { _ivec const result( ~( (_ivec const &)compiler_vector( a0 ) ) ); return (result_type const &)result; }
  };
  BOOST_DISPATCH_IMPLEMENT( make_       , tag::cpu_, (A0), ((target_<simd_<single_<A0>, BOOST_SIMD_DEFAULT_EXTENSION > >)) )
  {
    BOOST_SIMD_MAKE_BODY(4) { return (typename result_type::native_type){ a0, a1, a2, a3 }; }
  };
#endif // GCC 4.5+

#if ( __GNUC_MINOR__ >= 6 ) || defined( __clang__ )
  // shifts by scalar
  BOOST_DISPATCH_IMPLEMENT( shift_left_ , tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) << compiler_vector( a1 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( shift_right_, tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >))((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
    typedef A0 result_type; BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return compiler_vector( a0 ) >> compiler_vector( a1 ); }
  };
#endif // GCC 4.6+

#if ( __GNUC_MINOR__ >= 7 ) || defined( __clang__ )
  // operators ==, !=, <, <=, >, >=
  // splat
#endif // GCC 4.7+

#if ( __GNUC_MINOR__ >= 8 ) || ( defined( __clang__ ) && !defined( BOOST_SIMD_ARCH_ARM ) )
  // subscript access
  /// \note GCC should support subscript access since version 4.6 but they
  /// "forgot" to add this to the C++ frontend:
  /// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51033
  /// http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53094
  ///                                         (17.12.2012.) (Domagoj Saric)
  /// \note Clang has subscript operator support but not for ARM NEON. It does
  /// not seem to work even when the __vector_size__ syntax is used. ...mrmlj...it does not seem to work at all (even for x86)...
  /// http://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors
  /// http://clang-developers.42468.n3.nabble.com/Native-vectors-types-GCC-vs-NEON-td4035283.html
  ///                                         (28.10.2013.) (Domagoj Saric)

  BOOST_DISPATCH_IMPLEMENT( extract_, tag::cpu_, (A0)(A1), ((simd_<single_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(scalar_< integer_<A1> >) )
  {
    typedef float result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const & a0, A1 const a1 ) const { return compiler_vector( a0 )[ static_cast<unsigned int>( a1 ) ]; }
  };
  BOOST_DISPATCH_IMPLEMENT( extract_, tag::cpu_, (A0)(A1), ((simd_<uint32_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(scalar_< integer_<A1> >) )
  {
    typedef boost::uint32_t result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const & a0, A1 const a1 ) const { return compiler_vector( a0 )[ static_cast<unsigned int>( a1 ) ]; }
  };
  BOOST_DISPATCH_IMPLEMENT( extract_, tag::cpu_, (A0)(A1), ((simd_<int32_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(scalar_< integer_<A1> >) )
  {
    typedef boost::int32_t result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const & a0, A1 const a1 ) const { return compiler_vector( a0 )[ static_cast<unsigned int>( a1 ) ]; }
  };

  BOOST_DISPATCH_IMPLEMENT( insert_, tag::cpu_, (A0)(A1)(A2), (scalar_< arithmetic_<A0> >)((simd_< single_<A1>, BOOST_SIMD_DEFAULT_EXTENSION >))(scalar_< integer_<A2> >) )
  {
    typedef void result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const a0, A1 & a1, A2 const a2 ) const { compiler_vector( a1 )[ static_cast<unsigned int>( a2 ) ] = a0; }
  };
  BOOST_DISPATCH_IMPLEMENT( insert_, tag::cpu_, (A0)(A1)(A2), (scalar_< arithmetic_<A0> >)((simd_< uint32_<A1>, BOOST_SIMD_DEFAULT_EXTENSION >))(scalar_< integer_<A2> >) )
  {
    typedef void result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const a0, A1 & a1, A2 const a2 ) const { compiler_vector( a1 )[ static_cast<unsigned int>( a2 ) ] = a0; }
  };
  BOOST_DISPATCH_IMPLEMENT( insert_, tag::cpu_, (A0)(A1)(A2), (scalar_< arithmetic_<A0> >)((simd_< int32_<A1>, BOOST_SIMD_DEFAULT_EXTENSION >))(scalar_< integer_<A2> >) )
  {
    typedef void result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const a0, A1 & a1, A2 const a2 ) const { compiler_vector( a1 )[ static_cast<unsigned int>( a2 ) ] = a0; }
  };
#endif // GCC 4.8+

#endif // __GNUC__

#endif // !defined( BOOST_SIMD_DETECTED )


////////////////////////////////////////////////////////////////////////////////
//
// ARM NEON
//
// http://software.intel.com/en-us/blogs/2012/12/12/from-arm-neon-to-intel-mmxsse-automatic-porting-solution-tips-and-tricks
// http://software.intel.com/en-us/blogs/2011/08/18/understanding-x86-vs-arm-memory-alignment-on-android
//
////////////////////////////////////////////////////////////////////////////////

#if defined( __ARM_NEON__ ) || defined( BOOST_SIMD_ARCH_ARM_64 )
  BOOST_DISPATCH_IMPLEMENT( reverse_, tag::cpu_, (A0), ((simd_<arithmetic_<A0>, BOOST_SIMD_DEFAULT_EXTENSION >)) )
  {
      typedef A0 result_type;
      BOOST_SIMD_FUNCTOR_CALL(1)
      {
          float32x4_t const half_reversed_vector( vrev64q_f32  ( a0 )                   );
          float32x2_t const result_hi           ( vget_low_f32 ( half_reversed_vector ) );
          float32x2_t const result_lo           ( vget_high_f32( half_reversed_vector ) );
          float32x4_t const result              ( vcombine_f32 ( result_lo, result_hi ) );
          return result;
      }
  };
  BOOST_DISPATCH_IMPLEMENT( deinterleave_first_ , tag::cpu_, (A0)(A1), ((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>))((simd_<single_<A1>,BOOST_SIMD_DEFAULT_EXTENSION>)) )
  {
    typedef A0 result_type;
    //...mrmlj...assume that (de)interleave will always be called in first+second
    //...mrmlj...pairs and that the compiler will issue only a single instruction
    //...mrmlj...(GCC 4.6 does but does not quite generate optimal code for
    //...mrmlj..."N-element vectors/structs")
    //...mrmlj...http://gcc.gnu.org/bugzilla/show_bug.cgi?id=48941
    //...mrmlj...http://gcc.gnu.org/bugzilla/show_bug.cgi?id=51980
    result_type operator()(float32x4_t const a0, float32x4_t const a1) const { return vuzpq_f32( a0, a1 ).val[ 0 ]; }
  };
  BOOST_DISPATCH_IMPLEMENT( deinterleave_second_, tag::cpu_, (A0)(A1), ((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>))((simd_<single_<A1>,BOOST_SIMD_DEFAULT_EXTENSION>)) )
  {
    typedef A0 result_type;
    result_type operator()(float32x4_t const a0, float32x4_t const a1) const { return vuzpq_f32( a0, a1 ).val[ 1 ]; }
  };
  BOOST_DISPATCH_IMPLEMENT( interleave_first_   , tag::cpu_, (A0)(A1), ((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>))((simd_<single_<A1>,BOOST_SIMD_DEFAULT_EXTENSION>)) )
  {
    typedef A0 result_type;
    result_type operator()(float32x4_t const a0, float32x4_t const a1) const { return vzipq_f32( a0, a1 ).val[ 0 ]; }
  };
  BOOST_DISPATCH_IMPLEMENT( interleave_second_  , tag::cpu_, (A0)(A1), ((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>))((simd_<single_<A1>,BOOST_SIMD_DEFAULT_EXTENSION>)) )
  {
    typedef A0 result_type;
    result_type operator()(float32x4_t const a0, float32x4_t const a1) const { return vzipq_f32( a0, a1 ).val[ 1 ]; }
  };
  // ** splat **
  BOOST_DISPATCH_IMPLEMENT( splat_, tag::cpu_, (A0)(A1), (scalar_< fundamental_<A0> >)((target_< simd_< single_<A1>, BOOST_SIMD_DEFAULT_EXTENSION > >)) )
  {
    typedef typename A1::type result_type;
    result_type operator()(A0 const a0, A1) const { return vmovq_n_f32( a0 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( splat_, tag::cpu_, (A0)(A1), (scalar_< fundamental_<A0> >)((target_< simd_< uint32_<A1>, BOOST_SIMD_DEFAULT_EXTENSION > >)) )
  {
    typedef typename A1::type result_type;
    result_type operator()(A0 const a0, A1) const { return vmovq_n_u32( a0 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( splat_, tag::cpu_, (A0)(A1), (scalar_< fundamental_<A0> >)((target_< simd_< int32_<A1>, BOOST_SIMD_DEFAULT_EXTENSION > >)) )
  {
    typedef typename A1::type result_type;
    result_type operator()(A0 const a0, A1) const { return vmovq_n_s32( a0 ); }
  };
  // ** is_less **
  BOOST_DISPATCH_IMPLEMENT( is_less_, tag::cpu_, (A0), ((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>))((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>)) )
  {
    typedef typename meta::as_logical<A0>::type result_type;
    //...mrmlj...see the note in as_simd around the as_integer<> meta-call...
    BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return vreinterpretq_s32_u32( vcltq_f32( a0, a1 ) ); }
  };
  // ** is_greater **
  BOOST_DISPATCH_IMPLEMENT( is_greater_equal_, tag::cpu_, (A0), ((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>))((simd_<single_<A0>,BOOST_SIMD_DEFAULT_EXTENSION>)) )
  {
    typedef typename meta::as_logical<A0>::type result_type;
    BOOST_SIMD_FUNCTOR_CALL_REPEAT(2) { return vreinterpretq_s32_u32( vcgeq_f32( a0, a1 ) ); }
  };
  // ** load **
  BOOST_DISPATCH_IMPLEMENT( load_, tag::cpu_, (A0)(A1), (iterator_< scalar_< single_<A0> > >)((target_< simd_< single_<A1>, BOOST_SIMD_DEFAULT_EXTENSION > >)) )
  {
    typedef typename A1::type result_type;
    result_type operator()( A0 const a0, A1 ) const { return vld1q_f32( a0 ); }
  };
  BOOST_DISPATCH_IMPLEMENT( load_, tag::cpu_, (A0)(A1)(A2), (iterator_< scalar_< single_<A0> > >)(scalar_< fundamental_<A1> >)((target_< simd_< single_<A2>, BOOST_SIMD_DEFAULT_EXTENSION > >)) )
  {
    typedef typename A2::type result_type;
    inline result_type operator()( A0 const a0, A1 const a1, A2 ) const { return vld1q_f32( a0 + a1 ); }
  };
  // ** store **
  BOOST_DISPATCH_IMPLEMENT( store_, tag::cpu_, (A0)(A1), ((simd_<single_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(iterator_< scalar_< single_<A1> > >) )
  {
    typedef A0 result_type;
    BOOST_SIMD_FUNCTOR_CALL(2) { vst1q_f32( a1, a0 ); return a0; }
  };
  // ** extract **
  BOOST_DISPATCH_IMPLEMENT( extract_, tag::cpu_, (A0)(A1), ((simd_<single_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(mpl_integral_< scalar_< integer_<A1> > >) )
  {
    typedef float result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const & a0, A1 ) const { return vgetq_lane_f32( a0, A1::value ); }
  };
  BOOST_DISPATCH_IMPLEMENT( extract_, tag::cpu_, (A0)(A1), ((simd_<uint32_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(mpl_integral_< scalar_< integer_<A1> > >) )
  {
    typedef boost::uint32_t result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const & a0, A1 ) const { return vgetq_lane_u32( a0, A1::value ); }
  };
  BOOST_DISPATCH_IMPLEMENT( extract_, tag::cpu_, (A0)(A1), ((simd_<int32_<A0>, BOOST_SIMD_DEFAULT_EXTENSION>))(mpl_integral_< scalar_< integer_<A1> > >) )
  {
    typedef boost::int32_t result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const & a0, A1 ) const { return vgetq_lane_s32( a0, A1::value ); }
  };
  // ** insert **
  BOOST_DISPATCH_IMPLEMENT( insert_, tag::cpu_, (A0)(A1)(A2), (scalar_< arithmetic_<A0> >)((simd_< single_<A1>, BOOST_SIMD_DEFAULT_EXTENSION >))(mpl_integral_< scalar_< integer_<A2> > >) )
  {
    typedef void result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const a0, A1 & a1, A2 ) const { a1 = vsetq_lane_f32( a0, a1, A2::value ); }
  };
  BOOST_DISPATCH_IMPLEMENT( insert_, tag::cpu_, (A0)(A1)(A2), (scalar_< arithmetic_<A0> >)((simd_< uint32_<A1>, BOOST_SIMD_DEFAULT_EXTENSION >))(mpl_integral_< scalar_< integer_<A2> > >) )
  {
    typedef void result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const a0, A1 & a1, A2 ) const { a1 = vsetq_lane_u32( a0, a1, A2::value ); }
  };
  BOOST_DISPATCH_IMPLEMENT( insert_, tag::cpu_, (A0)(A1)(A2), (scalar_< arithmetic_<A0> >)((simd_< int32_<A1>, BOOST_SIMD_DEFAULT_EXTENSION >))(mpl_integral_< scalar_< integer_<A2> > >) )
  {
    typedef void result_type;
    BOOST_FORCEINLINE result_type operator()( A0 const a0, A1 & a1, A2 ) const { a1 = vsetq_lane_s32( a0, a1, A2::value ); }
  };

#endif // __ARM_NEON__

//------------------------------------------------------------------------------
} // namespace ext
//------------------------------------------------------------------------------
namespace details
{
//------------------------------------------------------------------------------

    ////////////////////////////////////////////////////////////////////////////
    // Shuffles
    ////////////////////////////////////////////////////////////////////////////

#if defined( BOOST_SIMD_HAS_SSE_SUPPORT )
    template <> BOOST_FORCEINLINE __m128 shuffle<0, 1, 0, 1>( __m128 const lower, __m128 const upper ) { return _mm_movelh_ps( lower, upper ); }
    template <> BOOST_FORCEINLINE __m128 shuffle<2, 3, 2, 3>( __m128 const lower, __m128 const upper ) { return _mm_movehl_ps( upper, lower ); }

    template <> BOOST_FORCEINLINE __m128 shuffle<0, 0, 1, 1>( __m128 const single_vector ) { return _mm_unpacklo_ps( single_vector, single_vector ); }
    template <> BOOST_FORCEINLINE __m128 shuffle<2, 2, 3, 3>( __m128 const single_vector ) { return _mm_unpackhi_ps( single_vector, single_vector ); }

    #ifdef BOOST_SIMD_HAS_SSE2_SUPPORT
    template <> BOOST_FORCEINLINE __m128 shuffle<0, 1, 2, 3>( __m128 const lower, __m128 const upper ) { return _mm_castpd_ps( _mm_move_sd( _mm_castps_pd( upper ), _mm_castps_pd( lower ) ) ); }
    #endif // BOOST_SIMD_HAS_SSE2_SUPPORT

    #ifdef BOOST_SIMD_HAS_SSE3_SUPPORT
    template <> BOOST_FORCEINLINE __m128 shuffle<0, 1, 0, 1>( __m128 const single_vector ) { return _mm_castpd_ps( _mm_movedup_pd( _mm_castps_pd( single_vector ) ) ); }
    template <> BOOST_FORCEINLINE __m128 shuffle<0, 0, 2, 2>( __m128 const single_vector ) { return _mm_moveldup_ps( single_vector ); }
    template <> BOOST_FORCEINLINE __m128 shuffle<1, 1, 3, 3>( __m128 const single_vector ) { return _mm_movehdup_ps( single_vector ); }
    #endif // BOOST_SIMD_HAS_SSE3_SUPPORT

    //...zzz...to be continued...
    //_mm_insert_* + _mm_extract_*
    //_mm_blend_*
    //_mm_move_s*
    //_mm_unpackhi_*
    //_mm_unpacklo_*
#elif defined( __clang__ )
    // Clang's builtin shuffle
    template
    <
        unsigned int lower_i0, unsigned int lower_i1,
        unsigned int upper_i0, unsigned int upper_i1,
        typename Vector
    >
    BOOST_FORCEINLINE Vector shuffle( Vector const & lower, Vector const & upper )
    {
        return __builtin_shufflevector( compiler_vector( lower ), compiler_vector( upper ), 0 + lower_i0, 0 + lower_i1, 4 + upper_i0, 4 + upper_i1 );
    }

    template
    <
        unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3,
        typename Vector
    >
    BOOST_FORCEINLINE Vector shuffle( Vector const & vector )
    {
        return shuffle<i0, i1, i2, i3>( vector, vector );
    }
#elif defined( __GNUC__ ) && ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) >= 47 ) && !defined( __clang__ )
    // GCC's builtin shuffle
    typedef int shuffle_mask_t __attribute__(( vector_size( 16 ) ));
    template
    <
        unsigned int lower_i0, unsigned int lower_i1,
        unsigned int upper_i0, unsigned int upper_i1,
        typename Vector
    >
    BOOST_FORCEINLINE Vector shuffle( Vector const & lower, Vector const & upper )
    {
        /// \note GCC4.8 can crash here.
        /// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57509
        ///                                   (01.10.2014.) (Domagoj Saric)
        shuffle_mask_t constexpr mask = { 0 + lower_i0, 0 + lower_i1, 4 + upper_i0, 4 + upper_i1 };
        return __builtin_shuffle( compiler_vector( lower ), compiler_vector( upper ), mask );
    }

    template
    <
        unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3,
        typename Vector
    >
    BOOST_FORCEINLINE Vector shuffle( Vector const & vector )
    {
        shuffle_mask_t constexpr mask = { i0, i1, i2, i3 };
        return __builtin_shuffle( compiler_vector( vector ), mask );
    }
#elif ( defined( __ARM_NEON__ ) || defined( BOOST_SIMD_ARCH_ARM_64 ) ) && !( defined( __clang__ ) && defined( BOOST_SIMD_ARCH_ARM_64 ) )
    // NEON shuffle
    // Clang missing vtbl2_u8 for arm64 - "...and yes, support for ARM64 NEON is patchy at the moment..." https://groups.google.com/forum/#!topic/llvm-dev/Pdztvvs--yU
    typedef native<float, BOOST_SIMD_DEFAULT_EXTENSION>::native_type builtin_vector_t;
    template
    <
        unsigned int lower_i0, unsigned int lower_i1,
        unsigned int upper_i0, unsigned int upper_i1
    >
    BOOST_FORCEINLINE builtin_vector_t shuffle( builtin_vector_t const lower, builtin_vector_t const upper )
    {
        static uint8x8_t const indices_lower =
        {
            lower_i0 * 4 + 0, lower_i0  * 4 + 1, lower_i0  * 4 + 2, lower_i0  * 4 + 3,
            lower_i1 * 4 + 0, lower_i1  * 4 + 1, lower_i1  * 4 + 2, lower_i1  * 4 + 3,
        };
        static uint8x8_t const indices_upper =
        {
            upper_i0 * 4 + 0, upper_i0  * 4 + 1, upper_i0  * 4 + 2, upper_i0  * 4 + 3,
            upper_i1 * 4 + 0, upper_i1  * 4 + 1, upper_i1  * 4 + 2, upper_i1  * 4 + 3,
        };

        uint8x8_t const result_lower_bits( vtbl2_u8( (uint8x8x2_t const &)/*vreinterpretq_u8_f32*/( lower ), indices_lower ) );
        uint8x8_t const result_upper_bits( vtbl2_u8( (uint8x8x2_t const &)/*vreinterpretq_u8_f32*/( upper ), indices_upper ) );

        return vcombine_f32( vreinterpret_f32_u8( result_lower_bits ), vreinterpret_f32_u8( result_upper_bits ) );
    }
    template <unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3>
    BOOST_FORCEINLINE
    builtin_vector_t shuffle( builtin_vector_t const vector )
    {
        return shuffle<i0, i1, i2, i3>( vector, vector );
    }
    template <>
    builtin_vector_t shuffle<1, 0, 3, 2>( builtin_vector_t const vector ) { return vrev64q_f32( vector ); }
#else // generic shuffle
    template
    <
        unsigned int lower_i0, unsigned int lower_i1,
        unsigned int upper_i0, unsigned int upper_i1,
        typename Vector
    >
    BOOST_FORCEINLINE
    Vector shuffle( Vector const & lower, Vector const & upper )
    {
        return make<Vector>
        (
            lower[ lower_i0 ],
            lower[ lower_i1 ],
            upper[ upper_i0 ],
            upper[ upper_i1 ]
        );
    }

    template
    <
        unsigned int i0, unsigned int i1, unsigned int i2, unsigned int i3,
        typename Vector
    >
    BOOST_FORCEINLINE
    Vector shuffle( Vector const & vector )
    {
        return shuffle<i0, i1, i2, i3>( vector, vector );
    }
#endif // BOOST_SIMD_HAS_SSE_SUPPORT

//------------------------------------------------------------------------------
} // namespace details
//------------------------------------------------------------------------------
} // namespace simd
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#endif // MISSING_FUNCTIONALITY_HPP_INCLUDED
