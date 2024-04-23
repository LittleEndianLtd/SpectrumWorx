//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/bitwise/include/functions/rrol.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/simd/include/constants/valmax.hpp>
#include <boost/simd/include/constants/valmin.hpp>
#include <boost/dispatch/meta/as_integer.hpp>

typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;

//////////////////////////////////////////////////////////////////////////////
// simd runtime benchmark for functor<rrol_> from boost.simd.bitwise
//////////////////////////////////////////////////////////////////////////////
using boost::simd::tag::rrol_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, (V1) ,(V2))

namespace n1 {
  typedef boost::simd::int8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n2 {
  typedef boost::simd::int16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n3 {
  typedef boost::simd::int32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n4 {
  typedef boost::simd::int64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n5 {
  typedef boost::simd::uint8_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n6 {
  typedef boost::simd::uint16_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n7 {
  typedef boost::simd::uint32_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}
namespace n8 {
  typedef boost::simd::uint64_t T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  typedef boost::simd::native<iT,ext_t> viT;
  NT2_TIMING(rrol_,(RS(vT,boost::simd::Valmin<T>()/2,boost::simd::Valmax<T>()/2))(RS(viT,T(0),sizeof(T)*8-1)))
}

#undef RS
