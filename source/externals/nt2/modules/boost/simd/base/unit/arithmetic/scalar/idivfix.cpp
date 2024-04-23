//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/arithmetic/include/functions/idivfix.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/zero.hpp>
#include <boost/simd/include/constants/mzero.hpp>
#include <boost/simd/include/constants/mone.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/minf.hpp>
#include <boost/simd/include/constants/nan.hpp>
#include <boost/simd/include/constants/valmax.hpp>
#include <boost/simd/include/constants/valmin.hpp>
#include <boost/simd/sdk/config.hpp>

NT2_TEST_CASE_TPL ( idivfix_real,  BOOST_SIMD_REAL_TYPES)
{

  using boost::simd::idivfix;
  using boost::simd::tag::idivfix_;
  typedef typename boost::dispatch::meta::call<idivfix_(T,T)>::type r_t;
  typedef typename boost::dispatch::meta::as_integer<T>::type wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
#ifndef BOOST_SIMD_NO_INVALIDS
  NT2_TEST_EQUAL(idivfix(boost::simd::Inf<T>(), boost::simd::Inf<T>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::Minf<T>(), boost::simd::Minf<T>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::Nan<T>(), boost::simd::Nan<T>()), boost::simd::Zero<r_t>());
#endif
 NT2_TEST_EQUAL(idivfix(boost::simd::Mone<T>(), boost::simd::Mone<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::Mone<T>(),boost::simd::Zero<T>()), boost::simd::Minf<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(), boost::simd::One<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(),boost::simd::Zero<T>()), boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(),boost::simd::Mzero<T>()), boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::Zero<T>(),boost::simd::Zero<T>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(idivfix(T(4),T(3)), r_t(1));
  NT2_TEST_EQUAL(idivfix(T(-4),T(-3)), r_t(1));
  NT2_TEST_EQUAL(idivfix(T(-4),T(3)), r_t(-1));
  NT2_TEST_EQUAL(idivfix(T(4),T(-3)), r_t(-1));
} // end of test for floating_

NT2_TEST_CASE_TPL ( idivfix_unsigned_int,  BOOST_SIMD_UNSIGNED_TYPES)
{

  using boost::simd::idivfix;
  using boost::simd::tag::idivfix_;
  typedef typename boost::dispatch::meta::call<idivfix_(T,T)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(idivfix(T(4),T(3)), T(1));
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(), boost::simd::One<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(), boost::simd::Zero<T>()), boost::simd::Valmax<r_t>());
} // end of test for unsigned_int_

NT2_TEST_CASE_TPL ( idivfix_signed_int,  BOOST_SIMD_INTEGRAL_SIGNED_TYPES)
{

  using boost::simd::idivfix;
  using boost::simd::tag::idivfix_;
  typedef typename boost::dispatch::meta::call<idivfix_(T,T)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);

  // specific values tests
  NT2_TEST_EQUAL(idivfix(T(4),T(3)), r_t(1));
  NT2_TEST_EQUAL(idivfix(T(-4),T(-3)), r_t(1));
  NT2_TEST_EQUAL(idivfix(T(-4),T(3)), r_t(-1));
  NT2_TEST_EQUAL(idivfix(T(4),T(-3)), r_t(-1));
  NT2_TEST_EQUAL(idivfix(boost::simd::Mone<T>(), boost::simd::Mone<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(), boost::simd::One<T>()), boost::simd::One<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::One<T>(), boost::simd::Zero<T>()), boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(idivfix(boost::simd::Mone<T>(), boost::simd::Zero<T>()), boost::simd::Valmin<r_t>());
} // end of test for signed_int_
