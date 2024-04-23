//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 arithmetic toolbox - idivide/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of arithmetic components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created by jt the 01/12/2010
///
#include <nt2/include/functions/idivide.hpp>
#include <nt2/include/functions/ceil.hpp>
#include <nt2/include/functions/toint.hpp>

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

#ifdef BOOST_MSVC
#pragma warning(disable: 4723) // potential divide by 0
#endif

NT2_TEST_CASE_TPL ( idivide_real__2_0,  NT2_REAL_TYPES)
{

  using nt2::idivide;
  using nt2::tag::idivide_;
  typedef T r_t;

  // specific values tests
  NT2_TEST_ULP_EQUAL(idivide(nt2::Four<T>(),nt2::Three<T>(), nt2::tag::ceil_()), nt2::Two<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Four<T>(),nt2::Zero<T>(), nt2::tag::ceil_()), nt2::Inf<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Inf<T>(), nt2::Inf<T>(), nt2::tag::ceil_()), nt2::Nan<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Minf<T>(), nt2::Minf<T>(), nt2::tag::ceil_()), nt2::Nan<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Mone<T>(), nt2::Mone<T>(), nt2::tag::ceil_()), nt2::One<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Mone<T>(),nt2::Zero<T>(), nt2::tag::ceil_()), nt2::Minf<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Nan<T>(), nt2::Nan<T>(), nt2::tag::ceil_()), nt2::Nan<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::One<T>(), nt2::One<T>(), nt2::tag::ceil_()), nt2::One<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::One<T>(),nt2::Zero<T>(), nt2::tag::ceil_()), nt2::Inf<r_t>(), 0);
  NT2_TEST_ULP_EQUAL(idivide(nt2::Zero<T>(),nt2::Zero<T>(), nt2::tag::ceil_()), nt2::Nan<r_t>(), 0);
} // end of test for floating_

// NT2_TEST_CASE_TPL ( idivide_unsigned_int__2_0,  NT2_UNSIGNED_TYPES)
// {

//   using nt2::idivide;
//   using nt2::tag::idivide_;
//   typedef typename nt2::meta::as_integer<T>::type iT;
//   typedef typename nt2::meta::call<idivide_(T,T)>::type r_t;
//   typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
//   typedef typename nt2::meta::upgrade<T>::type u_t;
//   typedef typename nt2::meta::as_integer<typename boost::common_type<T,T>::type>::type wished_r_t;


//   // return type conformity test
//   NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
//   std::cout << std::endl;
//   double ulpd;
//   ulpd=0.0;


//   // specific values tests
//   NT2_TEST_ULP_EQUAL(idivide(nt2::Four<T>(),nt2::Three<T>()), nt2::Two<r_t>(), 0);
//   NT2_TEST_ULP_EQUAL(idivide(nt2::Four<T>(),nt2::Zero<T>()), nt2::Valmax<r_t>(), 0);
//   NT2_TEST_ULP_EQUAL(idivide(nt2::One<T>(), nt2::One<T>()), nt2::One<r_t>(), 0);
// } // end of test for unsigned_int_

// NT2_TEST_CASE_TPL ( idivide_signed_int__2_0,  NT2_INTEGRAL_SIGNED_TYPES)
// {

//   using nt2::idivide;
//   using nt2::tag::idivide_;
//   typedef typename nt2::meta::as_integer<T>::type iT;
//   typedef typename nt2::meta::call<idivide_(T,T)>::type r_t;
//   typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
//   typedef typename nt2::meta::upgrade<T>::type u_t;
//   typedef typename nt2::meta::as_integer<typename boost::common_type<T,T>::type>::type wished_r_t;


//   // return type conformity test
//   NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
//   std::cout << std::endl;
//   double ulpd;
//   ulpd=0.0;

//   // specific values tests
//   NT2_TEST_ULP_EQUAL(idivide(nt2::Four<T>(),nt2::Three<T>()), nt2::Two<r_t>(), 0);
//   NT2_TEST_ULP_EQUAL(idivide(nt2::Four<T>(),nt2::Zero<T>()), nt2::Valmax<r_t>(), 0);
//   NT2_TEST_ULP_EQUAL(idivide(nt2::Mfour<T>(),nt2::Zero<T>()), nt2::Valmin<r_t>(), 0);
//   NT2_TEST_ULP_EQUAL(idivide(nt2::Mone<T>(), nt2::Mone<T>()), nt2::One<r_t>(), 0);
//   NT2_TEST_ULP_EQUAL(idivide(nt2::One<T>(), nt2::One<T>()), nt2::One<r_t>(), 0);
// } // end of test for signed_int_
