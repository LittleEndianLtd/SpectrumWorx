//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_BENCH_MODULE "nt2 ieee toolbox - predecessor/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// timing Test behavior of ieee components in scalar mode
//////////////////////////////////////////////////////////////////////////////
#include <nt2/ieee/include/functions/predecessor.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>


//////////////////////////////////////////////////////////////////////////////
// scalar runtime benchmark for functor<predecessor_> from ieee
//////////////////////////////////////////////////////////////////////////////
using nt2::tag::predecessor_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, T(V1) ,T(V2))

namespace n1 {
  typedef float T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,T(-10),T(10))))
}
namespace n2 {
  typedef double T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,T(-10),T(10))))
}
namespace n3 {
  typedef nt2::uint8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100)))
}
namespace n4 {
  typedef nt2::uint16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100)))
}
namespace n5 {
  typedef nt2::uint32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100)))
}
namespace n6 {
  typedef nt2::uint64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100)))
}
namespace n7 {
  typedef nt2::int8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100)))
}
namespace n8 {
  typedef nt2::int16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100)))
}
namespace n9 {
  typedef nt2::int32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100)))
}
namespace n10 {
  typedef nt2::int64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100)))
}
namespace n11 {
  typedef float T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,T(-10),T(10)))(RS(iT,iT(2),iT(2))))
}
namespace n12 {
  typedef double T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,T(-10),T(10)))(RS(iT,iT(2),iT(2))))
}
namespace n13 {
  typedef nt2::uint8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100))(RS(iT,iT(2),iT(2))))
}
namespace n14 {
  typedef nt2::uint16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100))(RS(iT,iT(2),iT(2))))
}
namespace n15 {
  typedef nt2::uint32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100))(RS(iT,iT(2),iT(2))))
}
namespace n16 {
  typedef nt2::uint64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,0,100))(RS(iT,iT(2),iT(2))))
}
namespace n17 {
  typedef nt2::int8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100))(RS(iT,iT(2),iT(2))))
}
namespace n18 {
  typedef nt2::int16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100))(RS(iT,iT(2),iT(2))))
}
namespace n19 {
  typedef nt2::int32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100))(RS(iT,iT(2),iT(2))))
}
namespace n20 {
  typedef nt2::int64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(predecessor_,(RS(T,-100,100))(RS(iT,iT(2),iT(2))))
}

#undef RS
