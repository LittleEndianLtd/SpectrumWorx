//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/cauinv.hpp>
#include <nt2/include/functions/caucdf.hpp>
#include <nt2/include/constants/invpi.hpp>
#include <nt2/include/functions/linspace.hpp>
#include <nt2/include/functions/isulpequal.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/include/functions/oneplus.hpp>
#include <nt2/include/functions/abs.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/constant/constant.hpp>
#include <nt2/table.hpp>


NT2_TEST_CASE_TPL ( cauinv_real__2_0,  NT2_REAL_TYPES)
{

  using nt2::cauinv;
  using nt2::tag::cauinv_;
  // specific values tests
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Nan<T>()), nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::One<T>()), nt2::Inf<T>(), 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Zero<T>()), nt2::Minf<T>(), 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Half<T>()), nt2::Zero<T>(), 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Quarter<T>()), nt2::Mone<T>(), 0.5);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Quarter<T>()*3), nt2::One<T>(), 0.5);
} // end of test for floating_

NT2_TEST_CASE_TPL ( cauinv_2,  NT2_REAL_TYPES)
{

  using nt2::cauinv;
  using nt2::tag::cauinv_;
  using nt2::_;


  // specific values tests
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Nan<T>(), nt2::One<T>()), nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Zero<T>(), nt2::One<T>() ), nt2::Minf<T>()/2, 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::One<T>(), nt2::One<T>()), nt2::Inf<T>(), 0);
  NT2_TEST_ULP_EQUAL(cauinv(nt2::Half<T>(), nt2::One<T>()), nt2::One<T>(), 0);
 } // end of test for floating_

NT2_TEST_CASE_TPL ( cauinv_inv,  NT2_REAL_TYPES)
{

  using nt2::cauinv;
  using nt2::tag::cauinv_;
  using nt2::_;

  nt2::table<T> x = nt2::linspace(T(-10), T(10));
  nt2::table<T> s = nt2::linspace(T(1), T(10));
  nt2::table<T> z = nt2::cauinv(nt2::caucdf(x));
  nt2::table<T> ulps = T(5)+(nt2::abs(x));
  NT2_TEST_ULP_EQUAL(z, x, T(11.0));
  NT2_TEST(nt2::isulpequal(z, x, ulps));

} // end of test for floating_

