//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_BENCH_MODULE "nt2 ieee toolbox - saturate/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// timing Test behavior of ieee components in simd mode
//////////////////////////////////////////////////////////////////////////////
#include <nt2/ieee/include/functions/saturate.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <nt2/include/constants/valmax.hpp>
#include <nt2/include/constants/valmin.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>

//////////////////////////////////////////////////////////////////////////////
// simd runtime benchmark for functor<saturate_> from ieee
//////////////////////////////////////////////////////////////////////////////
using nt2::tag::saturate_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, (V1) ,(V2))

namespace n1 {
  typedef nt2::uint8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(saturate_<uint16_t>,(RS(vT,nt2::Valmin<T>(),nt2::Valmax<T>())))
}
namespace n2 {
  typedef nt2::uint16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(saturate_<uint16_t>,(RS(vT,nt2::Valmin<T>(),nt2::Valmax<T>())))
}
namespace n3 {
  typedef nt2::uint32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(saturate_<uint16_t>,(RS(vT,nt2::Valmin<T>(),nt2::Valmax<T>())))
}
namespace n4 {
  typedef nt2::uint64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(saturate_<uint16_t>,(RS(vT,nt2::Valmin<T>(),nt2::Valmax<T>())))
}

#undef RS
