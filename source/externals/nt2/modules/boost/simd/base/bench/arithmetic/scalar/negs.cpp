//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================


//////////////////////////////////////////////////////////////////////////////
// timing Test behavior of boost.simd.arithmetic components in scalar mode
//////////////////////////////////////////////////////////////////////////////
#include <boost/simd/arithmetic/include/functions/negs.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>


//////////////////////////////////////////////////////////////////////////////
// scalar runtime benchmark for functor<negs_> from boost.simd.arithmetic
//////////////////////////////////////////////////////////////////////////////
using boost::simd::tag::negs_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, T(V1) ,T(V2))

namespace n1 {
  typedef float T;

  NT2_TIMING(negs_,(RS(T,T(-100),T(100))))
}
namespace n2 {
  typedef double T;

  NT2_TIMING(negs_,(RS(T,T(-100),T(100))))
}
namespace n3 {
  typedef boost::simd::int8_t T;

  NT2_TIMING(negs_,(RS(T,T(-100),T(100))))
}
namespace n4 {
  typedef boost::simd::int16_t T;

  NT2_TIMING(negs_,(RS(T,T(-100),T(100))))
}
namespace n5 {
  typedef boost::simd::int32_t T;

  NT2_TIMING(negs_,(RS(T,T(-100),T(100))))
}
namespace n6 {
  typedef boost::simd::int64_t T;

  NT2_TIMING(negs_,(RS(T,T(-100),T(100))))
}

#undef RS
