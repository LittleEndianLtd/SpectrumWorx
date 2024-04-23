//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/arithmetic/include/functions/toints.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/minf.hpp>
#include <boost/simd/include/constants/nan.hpp>
#include <boost/simd/include/constants/zero.hpp>
#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/mone.hpp>
#include <boost/simd/include/constants/valmax.hpp>
#include <boost/simd/include/constants/valmin.hpp>
#include <boost/simd/include/functions/ldexp.hpp>

NT2_TEST_CASE_TPL ( toints_real,  BOOST_SIMD_REAL_TYPES)
{

  using boost::simd::toints;
  using boost::simd::tag::toints_;
  typedef typename boost::dispatch::meta::call<toints_(T)>::type      r_t;
  typedef typename boost::dispatch::meta::as_integer<T, signed>::type wished_r_t;


  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(toints(T(2)*boost::simd::Valmax<r_t>()),  boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(toints(T(2)*boost::simd::Valmin<r_t>()),  boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(toints(T(1.5)*boost::simd::Valmax<r_t>()),  boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(toints(T(1.5)*boost::simd::Valmin<r_t>()),  boost::simd::Valmin<r_t>());


  NT2_TEST_EQUAL(toints(boost::simd::Inf<T>()),  boost::simd::Inf<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::Minf<T>()), boost::simd::Minf<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::Mone<T>()), boost::simd::Mone<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::Nan<T>()),  boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::One<T>()),  boost::simd::One<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::Zero<T>()), boost::simd::Zero<r_t>());

  T v = T(1);
  wished_r_t iv = 1;
  int N = sizeof(T)*8-1;
  for(int i=0; i < N ; i++, v*= 2, iv <<= 1)
  {
    NT2_TEST_EQUAL(toints(v), iv);
    NT2_TEST_EQUAL(toints(-v), -iv);
  }
  NT2_TEST_EQUAL(toints(boost::simd::ldexp(boost::simd::One<T>(), N)), boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::ldexp(boost::simd::One<T>(), N+1)), boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(toints(-boost::simd::ldexp(boost::simd::One<T>(), N+1)), boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(toints(-boost::simd::ldexp(boost::simd::One<T>(), N+1)), boost::simd::Valmin<r_t>());

} // end of test for floating_

NT2_TEST_CASE_TPL ( toints_unsigned_int,  BOOST_SIMD_UNSIGNED_TYPES)
{

  using boost::simd::toints;
  using boost::simd::tag::toints_;
  typedef typename boost::dispatch::meta::call<toints_(T)>::type      r_t;
  typedef typename boost::dispatch::meta::as_integer<T, signed>::type wished_r_t;


  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(toints(boost::simd::One<T>()),  boost::simd::One<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::Zero<T>()), boost::simd::Zero<r_t>());
} // end of test for unsigned_int_

NT2_TEST_CASE_TPL ( toints_signed,  BOOST_SIMD_INTEGRAL_SIGNED_TYPES)
{

  using boost::simd::toints;
  using boost::simd::tag::toints_;
  typedef typename boost::dispatch::meta::call<toints_(T)>::type      r_t;
  typedef typename boost::dispatch::meta::as_integer<T, signed>::type wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(toints(boost::simd::Mone<T>()), boost::simd::Mone<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::One<T>()),  boost::simd::One<r_t>());
  NT2_TEST_EQUAL(toints(boost::simd::Zero<T>()), boost::simd::Zero<r_t>());
} // end of test for signed_int_
