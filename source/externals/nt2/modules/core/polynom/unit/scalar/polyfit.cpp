//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/polyfit.hpp>
#include <nt2/include/functions/polyval.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/table.hpp>
#include <nt2/include/functions/tie.hpp>

NT2_TEST_CASE_TPL ( plev_1_0,  NT2_REAL_TYPES)
{

  using nt2::polyfit;
  using nt2::tag::polyfit_;
  nt2::table<T> x =  nt2::_(T(1), T(3));
  nt2::table<T> p =  nt2::_(T(1), T(3));
  nt2::table<T> y(nt2::of_size(1, 3));
  y(1) = T(6);
  y(2) = T(11);
  y(3) = T(18);
  nt2::table<T> p1 =polyfit(x, y);
  NT2_TEST_ULP_EQUAL(polyval(p1, x), y, 0.5);

  nt2::table<T> p2 =polyfit(x, y, 2);
  NT2_TEST_ULP_EQUAL(polyval(p2, x), y, 0.5);

  nt2::table<T> r;
  T df, normr;
  nt2::table<T> mu;
  nt2::tie(p, r, df, normr, mu) = polyfit(x, y);
  NT2_TEST_ULP_EQUAL(polyval(p, (x-mu(1))/mu(2)), y, 10);


  //////////////////////////////////////////////////////
  // TODO This does not work s being a structure defined in polyfit.hpp
  //   nt2::polyfit_infos<T> s;
  //   nt2::tie(p, s) = polyfit(x, y);
  //   NT2_DISPLAY(p);
  //   NT2_DISPLAY(s.r);
  //   NT2_DISPLAY(s.df);
  //    NT2_DISPLAY(s.normr);
  //////////////////////////////////////////////////////

}



NT2_TEST_CASE_TPL ( plevl_real__2_0,  NT2_REAL_TYPES)
{

  using nt2::polyfit;
  using nt2::tag::polyfit_;
  nt2::table<T> x =  reshape(nt2::_(T(1), T(4)), nt2::of_size(2, 2));
  nt2::table<T> p =  nt2::_(T(1), T(3));
  nt2::table<T> y =  nt2::polyval(p, x);
  nt2::table<T> r;
  T df, normr;
  nt2::table<T> mu;
  nt2::tie(p, r, df, normr, mu) = polyfit(x, y, 2);
  NT2_TEST_ULP_EQUAL(polyval(p, (x-mu(1))/mu(2)), y, 10);
}

NT2_TEST_CASE_TPL ( plevl_real__3_0,  NT2_REAL_TYPES)
{

  using nt2::polyfit;
  using nt2::tag::polyfit_;

  T a [] = {0.0, 1.0, 2.0, 3.0,  4.0,  5.0};
  T b [] = {0.0, 0.8, 0.9, 0.1, -0.8, -1.0};
  T c [] = {0.087037037037037245923, -0.81349206349206493183, 1.6931216931216956922, -0.039682539682541172199 };
  nt2::table<T> x(nt2::of_size(1,6), a+0, a+6);
  nt2::table<T> y(nt2::of_size(1,6), b+0, b+6);
  nt2::table<T> zz(nt2::of_size(1,4), c+0, c+4);
  nt2::table<T> z = polyfit(x, y, 3);
  NT2_DISPLAY(z);
  NT2_DISPLAY(zz);
  NT2_TEST_ULP_EQUAL(z, zz, 500);
}




