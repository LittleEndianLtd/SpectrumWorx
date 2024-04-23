//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_BENCH_MODULE "nt2 euler toolbox - beta/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// timing Test behavior of euler components in scalar mode
//////////////////////////////////////////////////////////////////////////////
#include <nt2/euler/include/functions/beta.hpp>
#include <nt2/include/constants/eps.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>


//////////////////////////////////////////////////////////////////////////////
// scalar runtime benchmark for functor<beta_> from euler
//////////////////////////////////////////////////////////////////////////////
using nt2::tag::beta_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, T(V1) ,T(V2))

namespace n1 {
  typedef float T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(beta_,(RS(T,nt2::Eps<T>(),T(1)))(RS(T,nt2::Eps<T>(),T(1))))
}
namespace n2 {
  typedef double T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(beta_,(RS(T,nt2::Eps<T>(),T(1)))(RS(T,nt2::Eps<T>(),T(1))))
}

#undef RS
