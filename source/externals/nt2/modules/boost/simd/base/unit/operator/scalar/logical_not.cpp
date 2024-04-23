//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/operator/include/functions/logical_not.hpp>
#include <boost/simd/sdk/simd/logical.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/sdk/config.hpp>
#include <boost/simd/sdk/simd/io.hpp>

#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/mone.hpp>
#include <boost/simd/include/constants/zero.hpp>
#include <boost/simd/include/constants/three.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/minf.hpp>
#include <boost/simd/include/constants/nan.hpp>
#include <boost/simd/include/constants/valmax.hpp>

NT2_TEST_CASE_TPL ( logical_not_integer,  BOOST_SIMD_INTEGRAL_TYPES)
{

  using boost::simd::logical_not;
  using boost::simd::tag::logical_not_;
  typedef typename boost::dispatch::meta::call<logical_not_(T)>::type r_t;
  typedef typename boost::simd::meta::as_logical<T>::type wished_r_t;


  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(logical_not(boost::simd::False< boost::simd::logical<T> >()), boost::simd::True<r_t>());
  NT2_TEST_EQUAL(logical_not(boost::simd::True< boost::simd::logical<T> >()), boost::simd::False<r_t>());
} // end of test for integer_

NT2_TEST_CASE_TPL ( logical_not_real,  BOOST_SIMD_REAL_TYPES)
{

  using boost::simd::logical_not;
  using boost::simd::tag::logical_not_;
  typedef typename boost::dispatch::meta::call<logical_not_(T)>::type r_t;
  typedef typename boost::simd::meta::as_logical<T>::type wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(logical_not(boost::simd::False< boost::simd::logical<T> >()), boost::simd::True<r_t>());
  NT2_TEST_EQUAL(logical_not(boost::simd::True< boost::simd::logical<T> >()), boost::simd::False<r_t>());
} // end of test for real_
