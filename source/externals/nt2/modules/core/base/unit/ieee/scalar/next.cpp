//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 ieee toolbox - next/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of ieee components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created by jt the 04/12/2010
///
#include <nt2/ieee/include/functions/next.hpp>
#include <nt2/include/functions/successor.hpp>
#include <nt2/include/constants/eps_related.hpp>

#include <boost/type_traits/is_same.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/meta/as_integer.hpp>
#include <nt2/sdk/meta/as_floating.hpp>
#include <nt2/sdk/meta/as_signed.hpp>
#include <nt2/sdk/meta/upgrade.hpp>
#include <nt2/sdk/meta/downgrade.hpp>
#include <nt2/sdk/meta/scalar_of.hpp>
#include <boost/dispatch/meta/as_floating.hpp>
#include <boost/type_traits/common_type.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/constant/constant.hpp>
#include <boost/core/ignore_unused.hpp>


NT2_TEST_CASE_TPL ( next_real__1_0,  NT2_REAL_TYPES)
{

  using nt2::next;
  using nt2::tag::next_;
  typedef typename nt2::meta::call<next_(T)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;

  // specific values tests
  NT2_TEST_EQUAL(next(nt2::Inf<T>()), nt2::Inf<r_t>());
  NT2_TEST_EQUAL(next(nt2::Minf<T>()), nt2::Valmin<r_t>());
  NT2_TEST_EQUAL(next(nt2::Mone<T>()), nt2::Mone<r_t>()+nt2::Eps<r_t>()/2);
  NT2_TEST_EQUAL(next(nt2::Nan<T>()), nt2::Nan<r_t>());
  NT2_TEST_EQUAL(next(nt2::One<T>()), nt2::One<r_t>()+nt2::Eps<r_t>());
  NT2_TEST_EQUAL(next(nt2::Valmax<T>()), nt2::Inf<r_t>());
  NT2_TEST_EQUAL(next(nt2::Zero<T>()), nt2::Zero<r_t>()+nt2::Bitincrement<T>());
} // end of test for real_

NT2_TEST_CASE_TPL ( next_unsigned_int__1_0,  NT2_UNSIGNED_TYPES)
{

  using nt2::next;
  using nt2::tag::next_;
  typedef typename nt2::meta::call<next_(T)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;

  // specific values tests
  NT2_TEST_EQUAL(next(nt2::One<T>()), nt2::Two<r_t>());
  NT2_TEST_EQUAL(next(nt2::Valmax<T>()), nt2::Valmin<r_t>());
  NT2_TEST_EQUAL(next(nt2::Zero<T>()), nt2::One<r_t>());
} // end of test for unsigned_int_

NT2_TEST_CASE_TPL ( next_signed_int__1_0,  NT2_INTEGRAL_SIGNED_TYPES)
{

  using nt2::next;
  using nt2::tag::next_;
  typedef typename nt2::meta::call<next_(T)>::type r_t;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;

  // specific values tests
  NT2_TEST_EQUAL(next(nt2::Mone<T>()), nt2::Zero<r_t>());
  NT2_TEST_EQUAL(next(nt2::One<T>()), nt2::Two<r_t>());
  NT2_TEST_EQUAL(next(nt2::Zero<T>()), nt2::One<r_t>());
} // end of test for signed_int_
