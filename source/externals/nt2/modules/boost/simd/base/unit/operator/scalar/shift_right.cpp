//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/operator/include/functions/shift_right.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/sdk/config.hpp>
#include <boost/simd/sdk/simd/io.hpp>

#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/mone.hpp>
#include <boost/simd/include/constants/zero.hpp>
#include <boost/simd/include/constants/two.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/minf.hpp>
#include <boost/simd/include/constants/nan.hpp>
#include <boost/simd/include/constants/mzero.hpp>

NT2_TEST_CASE_TPL ( shift_right_integer,  BOOST_SIMD_INTEGRAL_TYPES)
{

  using boost::simd::shift_right;
  using boost::simd::tag::shift_right_;
  typedef typename boost::dispatch::meta::as_integer<T>::type iT;
  typedef typename boost::dispatch::meta::call<shift_right_(T,iT)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(shift_right(boost::simd::One<T>(),boost::simd::One<iT>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(shift_right(boost::simd::One<T>(),boost::simd::Zero<iT>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(shift_right(boost::simd::Zero<T>(),boost::simd::One<iT>()), boost::simd::Zero<r_t>());
} // end of test for integer_

NT2_TEST_CASE_TPL ( shift_right_real,  BOOST_SIMD_REAL_TYPES)
{
  using boost::simd::shift_right;
  using boost::simd::tag::shift_right_;
  typedef typename boost::dispatch::meta::as_integer<T>::type iT;
  typedef typename boost::dispatch::meta::call<shift_right_(T,iT)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(shift_right(boost::simd::One<T>(),boost::simd::Zero<iT>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(shift_right(boost::simd::Zero<T>(),boost::simd::One<iT>()), boost::simd::Zero<r_t>());
} // end of test for floating_
