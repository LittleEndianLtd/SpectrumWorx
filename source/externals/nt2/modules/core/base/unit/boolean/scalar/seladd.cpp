//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 boolean toolbox - seladd/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of boolean components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 18/02/2011
///
#include <nt2/boolean/include/functions/seladd.hpp>
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


NT2_TEST_CASE_TPL ( seladd_real__3_0,  NT2_REAL_TYPES)
{

  using nt2::seladd;
  using nt2::tag::seladd_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<seladd_(T,T,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef typename boost::common_type<T,T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_EQUAL(seladd(T(0),T(1),T(2)), T(1));
  NT2_TEST_EQUAL(seladd(nt2::Nan<T>(),T(1),T(2)), T(3));
  NT2_TEST_EQUAL(seladd(nt2::Nan<T>(),nt2::Inf<T>(),nt2::Inf<T>()), nt2::Inf<r_t>());
  NT2_TEST_EQUAL(seladd(nt2::Nan<T>(),nt2::Minf<T>(),nt2::Minf<T>()), nt2::Minf<r_t>());
  NT2_TEST_EQUAL(seladd(nt2::Nan<T>(),nt2::Nan<T>(),nt2::Nan<T>()), nt2::Nan<r_t>());
  NT2_TEST_EQUAL(seladd(nt2::Nan<T>(),nt2::Zero<T>(),nt2::Zero<T>()), nt2::Zero<r_t>());
} // end of test for floating_

NT2_TEST_CASE_TPL ( seladd_integer__3_0,  NT2_INTEGRAL_TYPES)
{

  using nt2::seladd;
  using nt2::tag::seladd_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<seladd_(T,T,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef typename boost::common_type<T,T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_EQUAL(seladd(T(-1),T(1),T(2)), T(3));
  NT2_TEST_EQUAL(seladd(T(0),T(1),T(2)), T(1));
  NT2_TEST_EQUAL(seladd(nt2::Zero<T>(),nt2::Zero<T>(),nt2::Zero<T>()), nt2::Zero<r_t>());
} // end of test for integer_