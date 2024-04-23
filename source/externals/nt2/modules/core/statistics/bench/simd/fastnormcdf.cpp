//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/statistics/include/functions/fastnormcdf.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>


//////////////////////////////////////////////////////////////////////////////
// simd runtime benchmark for functor<fastnormcdf_> from statistics
//////////////////////////////////////////////////////////////////////////////
using nt2::tag::fastnormcdf_;

//////////////////////////////////////////////////////////////////////////////
// range macro
//////////////////////////////////////////////////////////////////////////////
#define RS(T,V1,V2) (T, (V1) ,(V2))

namespace n1 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(fastnormcdf_,(RS(vT,T(-10),T(10))))
}
// namespace n2 {
//   typedef double T;
//   typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
//   NT2_TIMING(fastnormcdf_,(RS(vT,T(-10),T(10))))
// }

#undef RS
