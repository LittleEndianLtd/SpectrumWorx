//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/arithmetic/include/functions/abs.hpp>
#include <nt2/sdk/complex/complex.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <complex>
#include <cmath>

//////////////////////////////////////////////////////////////////////////////
// scalar runtime benchmark for functor<abs_> from arithmetic
//////////////////////////////////////////////////////////////////////////////
using nt2::tag::abs_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, T(V1) ,T(V2))

namespace n1 {
  typedef std::complex<float> T;
  NT2_TIMING(abs_,(RS(T,T(-100, -100),T(100, 100))))
}
namespace n2 {
  typedef std::complex<double> T;
  NT2_TIMING(abs_,(RS(T,T(-100, -100),T(100, 100))))
}


#undef RS
