//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/bitwise/include/functions/twopower.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>

//////////////////////////////////////////////////////////////////////////////
// simd runtime benchmark for functor<twopower_> from boost.simd.bitwise
//////////////////////////////////////////////////////////////////////////////
using boost::simd::tag::twopower_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, (V1) ,(V2))

namespace n1 {
  typedef boost::simd::uint8_t T;

  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(twopower_,(RS(vT,0,sizeof(T)*8-1)))
}
namespace n2 {
  typedef boost::simd::uint16_t T;

  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(twopower_,(RS(vT,0,sizeof(T)*8-1)))
}
namespace n3 {
  typedef boost::simd::uint32_t T;

  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(twopower_,(RS(vT,0,sizeof(T)*8-1)))
}
namespace n4 {
  typedef boost::simd::uint64_t T;

  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(twopower_,(RS(vT,0,sizeof(T)*8-1)))
}

#undef RS
