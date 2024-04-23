//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 operator toolbox - complement/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of operator components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 18/02/2011
///
#include <nt2/operator/include/functions/complement.hpp>
#include <nt2/include/functions/shift_left.hpp>

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


NT2_TEST_CASE_TPL ( complement_real__1_0,  NT2_REAL_TYPES)
{

  using nt2::complement;
  using nt2::tag::complement_;
  typedef typename nt2::meta::call<complement_(T)>::type r_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_EQUAL(complement(nt2::Nan<T>()), nt2::Zero<r_t>());
  NT2_TEST_EQUAL(complement(nt2::Zero<T>()), nt2::Nan<r_t>());
} // end of test for floating_

NT2_TEST_CASE_TPL ( complement_integer__1_0,  NT2_INTEGRAL_TYPES)
{

  using nt2::complement;
  using nt2::tag::complement_;
  typedef typename nt2::meta::call<complement_(T)>::type r_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_EQUAL(complement(nt2::Mone<T>()), nt2::Zero<r_t>());
  NT2_TEST_EQUAL(complement(nt2::One<T>()), nt2::shli(nt2::Mone<r_t>(),1));
  NT2_TEST_EQUAL(complement(nt2::Three<T>()), nt2::shli(nt2::Mone<r_t>(),2));
  NT2_TEST_EQUAL(complement(nt2::Zero<T>()), nt2::Mone<r_t>());
} // end of test for integer_
