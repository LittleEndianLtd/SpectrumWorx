//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 bitwise toolbox - ror/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of bitwise components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 18/02/2011
///
#include <nt2/bitwise/include/functions/ror.hpp>
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


NT2_TEST_CASE_TPL ( ror_integer__2_0,  NT2_INTEGRAL_TYPES)
{

  using nt2::ror;
  using nt2::tag::ror_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<ror_(T,iT)>::type r_t;
  typedef T wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;



  // specific values tests
  NT2_TEST_EQUAL(ror(T(2),T(2)), r_t(nt2::One<T>()<<(sizeof(T)*8-1)));
  NT2_TEST_EQUAL(ror(nt2::Mone<T>(),T(1)), nt2::Mone<T>());
  NT2_TEST_EQUAL(ror(nt2::Mone<T>(),T(5)), nt2::Mone<T>());
  NT2_TEST_EQUAL(ror(nt2::One<T>(),T(1)), r_t(nt2::One<T>()<<(sizeof(T)*8-1)));
  NT2_TEST_EQUAL(ror(nt2::Zero<T>(),T(1)), nt2::Zero<T>());
} // end of test for integer_
