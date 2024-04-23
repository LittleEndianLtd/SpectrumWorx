//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_BENCH_MODULE "nt2 boost.simd.bitwise toolbox - rshl/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// timing Test behavior of boost.simd.bitwise components in scalar mode
//////////////////////////////////////////////////////////////////////////////
#include <boost/simd/bitwise/include/functions/rshl.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/simd/include/constants/valmax.hpp>
#include <boost/simd/include/constants/valmin.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <cmath>


//////////////////////////////////////////////////////////////////////////////
// scalar runtime benchmark for functor<rshl_> from boost.simd.bitwise
//////////////////////////////////////////////////////////////////////////////
using boost::simd::tag::rshl_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, T(V1) ,T(V2))

namespace n1 {
  typedef boost::simd::int8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n2 {
  typedef boost::simd::int16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n3 {
  typedef boost::simd::int32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n4 {
  typedef boost::simd::int64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n5 {
  typedef boost::simd::uint8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n6 {
  typedef boost::simd::uint16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n7 {
  typedef boost::simd::uint32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}
namespace n8 {
  typedef boost::simd::uint64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  NT2_TIMING(rshl_,(RS(T,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(iT,T(0),sizeof(T)*8-1)))
}

#undef RS
