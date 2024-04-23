//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/euler/include/functions/digamma.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <cmath>

using nt2::tag::digamma_;

#define RS(T,V1,V2) (T, T(V1) ,T(V2))

namespace n1 {
  typedef float T;
  NT2_TIMING(digamma_,(RS(T,T(-40),T(40))))
}
namespace n2 {
  typedef double T;
  NT2_TIMING(digamma_,(RS(T,T(-40),T(40))))
}
namespace n3 {
  typedef float T;
  NT2_TIMING(digamma_,(RS(T,T(1),T(2))))
}
namespace n4 {
  typedef double T;
  NT2_TIMING(digamma_,(RS(T,T(1),T(2))))
}
namespace n5 {
  typedef float T;
  NT2_TIMING(digamma_,(RS(T,T(2),T(20))))
}
namespace n6 {
  typedef double T;
  NT2_TIMING(digamma_,(RS(T,T(2),T(20))))
}
namespace n7 {
  typedef float T;
  NT2_TIMING(digamma_,(RS(T,T(20),T(40))))
}
namespace n8 {
  typedef double T;
  NT2_TIMING(digamma_,(RS(T,T(20),T(40))))
}


#undef RS
