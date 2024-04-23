//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
//////////////////////////////////////////////////////////////////////////////
// unit test behavior of arithmetic components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created by jt the 01/12/2010
///
/// The remainder() function computes the remainder of dividing x by y.
/// The return value is x-n*y, where n is the value x / y,
/// rounded to the nearest integer.  If the absolute value of x-n*y is 0.5,
/// n is chosen to be even. The drem() function does precisely the same thing.
#include <nt2/arithmetic/include/functions/remainder.hpp>
#include <nt2/include/functions/idivround.hpp>

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


NT2_TEST_CASE_TPL ( remainder_real__2_0,  NT2_REAL_TYPES)
{

  using nt2::remainder;
  using nt2::tag::remainder_;
  typedef typename nt2::meta::call<remainder_(T,T)>::type r_t;
  typedef typename boost::common_type<T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_ULP_EQUAL(remainder(nt2::Inf<T>(), nt2::Inf<T>()), nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::Minf<T>(), nt2::Minf<T>()), nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::Mone<T>(), nt2::Mone<T>()), nt2::Zero<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::Nan<T>(), nt2::Nan<T>()), nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::One<T>(), nt2::One<T>()), nt2::Zero<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::One<T>(),nt2::Zero<T>()), nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::Zero<T>(),nt2::Zero<T>()), nt2::Nan<T>(), 0);
} // end of test for floating_


NT2_TEST_CASE_TPL ( remainder_signed_int__2_0,  NT2_INTEGRAL_SIGNED_TYPES)
{

  using nt2::remainder;
  using nt2::tag::remainder_;
  typedef typename nt2::meta::call<remainder_(T,T)>::type r_t;
  typedef typename boost::common_type<T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_ULP_EQUAL(remainder(nt2::Mone<T>(), nt2::Mone<T>()), nt2::Zero<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::One<T>(), nt2::One<T>()), nt2::Zero<T>(), 0);
  NT2_TEST_ULP_EQUAL(remainder(nt2::Zero<T>(), nt2::Zero<T>()), nt2::Zero<T>(), 0);
} // end of test for signed_int_
