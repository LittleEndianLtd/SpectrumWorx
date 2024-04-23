//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/bitwise/include/functions/bitwise_andnot.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/include/constants/two.hpp>
#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/mone.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/minf.hpp>
#include <boost/simd/include/constants/nan.hpp>
#include <boost/simd/sdk/config.hpp>


NT2_TEST_CASE_TPL ( bitwise_andnot_real,  BOOST_SIMD_REAL_TYPES)
{

  using boost::simd::bitwise_andnot;
  using boost::simd::tag::bitwise_andnot_;
  typedef typename boost::dispatch::meta::call<bitwise_andnot_(T,T)>::type r_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
#ifndef BOOST_SIMD_NO_INVALIDS
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Inf<T>(), boost::simd::Inf<T>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Minf<T>(), boost::simd::Minf<T>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Nan<T>(), boost::simd::Nan<T>()), boost::simd::Zero<r_t>());
#endif
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::One<T>(),boost::simd::Zero<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Zero<T>(), boost::simd::Zero<T>()), boost::simd::Zero<r_t>());
} // end of test for floating_

NT2_TEST_CASE_TPL ( bitwise_andnot_integer,  BOOST_SIMD_INTEGRAL_TYPES)
{

  using boost::simd::bitwise_andnot;
  using boost::simd::tag::bitwise_andnot_;
  typedef typename boost::dispatch::meta::call<bitwise_andnot_(T,T)>::type r_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::One<T>(), boost::simd::One<T>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::One<T>(),boost::simd::Zero<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Zero<T>(), boost::simd::Zero<T>()), boost::simd::Zero<r_t>());
} // end of test for integer_


NT2_TEST_CASE_TPL(bitwise_andnot_mix, BOOST_SIMD_REAL_TYPES)
{
  using boost::simd::bitwise_andnot;
  using boost::simd::tag::bitwise_andnot_;
  typedef typename  boost::dispatch::meta::as_integer<T>::type siT;
  typedef typename  boost::dispatch::meta::as_integer<T, unsigned>::type uiT;

  // return type conformity test
  NT2_TEST_TYPE_IS(typename boost::dispatch::meta::call<bitwise_andnot_(T,uiT)>::type, T);
  NT2_TEST_TYPE_IS(typename boost::dispatch::meta::call<bitwise_andnot_(T,siT)>::type, T);
  NT2_TEST_TYPE_IS(typename boost::dispatch::meta::call<bitwise_andnot_(uiT, T)>::type, uiT);
  NT2_TEST_TYPE_IS(typename boost::dispatch::meta::call<bitwise_andnot_(siT, T)>::type, siT);

  // specific values tests
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Nan<T>(),boost::simd::Zero<uiT>()), boost::simd::Nan<T>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Nan<T>(), boost::simd::Zero<siT>()), boost::simd::Nan<T>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Mone<siT>(),boost::simd::Zero<T>()), boost::simd::Mone<siT>());
  NT2_TEST_EQUAL(bitwise_andnot(boost::simd::Valmax<uiT>(), boost::simd::Zero<T>()), boost::simd::Valmax<uiT>());
}
