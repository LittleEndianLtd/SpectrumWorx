//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/trigonometric/include/functions/rem_pio2.hpp>
#include <nt2/trigonometric/constants.hpp>
#include <nt2/constant/constant.hpp>

#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/meta/scalar_of.hpp>
#include <boost/type_traits/is_same.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

NT2_TEST_CASE_TPL ( rem_pio2_real__1_0,  NT2_REAL_TYPES)
{

  using nt2::rem_pio2;
  using nt2::tag::rem_pio2_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<rem_pio2_(T)>::type r_t;
  typedef typename nt2::meta::call<rem_pio2_(T)>::type wished_r_t;

  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;

  // specific values tests
  typedef typename boost::fusion::result_of::value_at_c<r_t,0>::type r_t0;
  typedef typename boost::fusion::result_of::value_at_c<r_t,1>::type r_t1;
  typedef typename boost::fusion::result_of::value_at_c<r_t,2>::type r_t2;
  {
    r_t res = rem_pio2(nt2::Zero<T>());
    NT2_TEST_ULP_EQUAL( boost::fusion::get<0>(res), nt2::Zero<r_t0>(), 1);
    NT2_TEST_ULP_EQUAL( boost::fusion::get<1>(res), nt2::Zero<r_t1>(), 1);
    NT2_TEST_ULP_EQUAL( boost::fusion::get<2>(res), nt2::Zero<r_t2>(), 1);
    T xr;
    iT n = rem_pio2(nt2::Zero<T>(), xr);
    NT2_TEST_ULP_EQUAL( xr, nt2::Zero<T>(), 1);
    NT2_TEST_ULP_EQUAL( n, nt2::Zero<iT>(), 1);

  }
} // end of test for floating_

NT2_TEST_CASE_TPL ( rem_pio2_targeted,  NT2_REAL_TYPES)
{

  using nt2::rem_pio2;
  using nt2::tag::rem_pio2_;
  typedef typename nt2::meta::as_integer<T>::type iT;

  iT n;
  T x = nt2::Pio_2<T>(), xr;
  n = rem_pio2(x, xr, nt2::meta::as_<nt2::big_>());
  NT2_TEST_ULP_EQUAL( xr, nt2::Zero<T>(), 1);
  NT2_TEST_ULP_EQUAL( n, nt2::Zero<iT>(), 1);

  n = rem_pio2(x, xr, nt2::meta::as_<nt2::medium_>());
  NT2_TEST_ULP_EQUAL( xr, nt2::Zero<T>(), 1);
  NT2_TEST_ULP_EQUAL( n, nt2::Zero<iT>(), 1);

  n = rem_pio2(x, xr, nt2::meta::as_<nt2::small_>());
  NT2_TEST_ULP_EQUAL( xr, nt2::Zero<T>(), 1);
  NT2_TEST_ULP_EQUAL( n, nt2::Zero<iT>(), 1);

  n = rem_pio2(x, xr, nt2::meta::as_<nt2::very_small_>());
  NT2_TEST_ULP_EQUAL( xr, nt2::Zero<T>(), 1);
  NT2_TEST_ULP_EQUAL( n, nt2::Zero<iT>(), 1);
}



