//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 boolean toolbox - select/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of boolean components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 18/02/2011
///
#include <nt2/boolean/include/functions/select.hpp>
#include <nt2/predicates/include/functions/is_nez.hpp>

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


NT2_TEST_CASE_TPL ( select_real__3_0,  NT2_REAL_TYPES)
{

  using nt2::select;
  using nt2::tag::select_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<select_(T,T,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;


  // specific values tests
  NT2_TEST_EQUAL(select(nt2::False< nt2::logical<T> >(),nt2::One<T>(),nt2::Two<T>()), nt2::Two<r_t>());
  NT2_TEST_EQUAL(select(nt2::True< nt2::logical<T> >(),nt2::One<T>(),nt2::Two<T>()), nt2::One<r_t>());
} // end of test for real_

NT2_TEST_CASE_TPL ( select_integer__3_0,  NT2_INTEGRAL_TYPES)
{

  using nt2::select;
  using nt2::tag::select_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<select_(T,T,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;


  // specific values tests
  NT2_TEST_EQUAL(select(nt2::False< nt2::logical<T> >(),nt2::One<T>(),nt2::Two<T>()), nt2::Two<r_t>());
  NT2_TEST_EQUAL(select(nt2::True< nt2::logical<T> >(),nt2::One<T>(),nt2::Two<T>()), nt2::One<r_t>());
} // end of test for integer_
