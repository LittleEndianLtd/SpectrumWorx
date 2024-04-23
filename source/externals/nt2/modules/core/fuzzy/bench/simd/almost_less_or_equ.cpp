//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_BENCH_MODULE "nt2 fuzzy toolbox - almost_less_or_equ/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// timing Test behavior of fuzzy components in simd mode
//////////////////////////////////////////////////////////////////////////////
#include <nt2/fuzzy/include/functions/almost_less_or_equ.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>
typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;

//////////////////////////////////////////////////////////////////////////////
// simd runtime benchmark for functor<almost_less_or_equ_> from fuzzy
//////////////////////////////////////////////////////////////////////////////
using nt2::tag::almost_less_or_equ_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, (V1) ,(V2))

namespace n1 {
  typedef float T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(almost_less_or_equ_,(RS(vT,T(-10),T(10)))(RS(vT,T(-10),T(10)))(RS(viT,iT(0),iT(10))))
}
namespace n2 {
  typedef double T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(almost_less_or_equ_,(RS(vT,T(-10),T(10)))(RS(vT,T(-10),T(10)))(RS(viT,iT(0),iT(10))))
}

#undef RS
