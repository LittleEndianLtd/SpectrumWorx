//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::sum1 function"

#include <nt2/table.hpp>
#include <nt2/include/functions/nearest.hpp>
#include <nt2/include/functions/linspace.hpp>
#include <nt2/include/functions/isequal.hpp>
#include <boost/core/ignore_unused.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>

NT2_TEST_CASE_TPL( nearest, NT2_REAL_TYPES )
{
  using nt2::_;
  nt2::table<T> x =  nt2::linspace(T(1),  T(4), 4);
  nt2::table<T> y =  nt2::linspace(T(2),  T(8), 4);
  nt2::table<T> xi=  nt2::linspace(T(0),  T(5), 11);
  T tr0[] = {2 ,2 ,2 ,4 ,4 ,6 ,6 ,8 ,8 ,8 ,8};
  nt2::table<T> r0(nt2::of_size(1, 11), &tr0[0], &tr0[11]);
  T nan =  nt2::Nan<T>();
  nt2::table<T> r1 = r0;  r1(_(1, 2)) = nan; r1(_(10, 11)) = nan;
  nt2::table<T> r2 = r0;  r2(_(1, 2)) = T(33); r2(_(10, 11))= T(33);
  NT2_DISPLAY(x);
  NT2_DISPLAY(y);
  NT2_DISPLAY(xi);
  nt2::table<T> yi =nt2::nearest(x, y, xi);
  std::cout << "extrap " <<  false <<  " extrapval " << "-" << std::endl;
  NT2_DISPLAY(yi);
  NT2_TEST_EQUAL(yi, r1);
  yi =nt2::nearest(x, y, xi, false);
  std::cout << "extrap " <<  false <<  " extrapval " << "-" << std::endl;
  NT2_DISPLAY(yi);
  NT2_TEST_EQUAL(yi, r1);
  std::cout << "extrap " <<  true <<  " extrapval " << "-" << std::endl;
  yi =nt2::nearest(x, y, xi, true);
  NT2_DISPLAY(yi);
  NT2_TEST_EQUAL(yi, r0);
  T z =  33;
  std::cout << "extrap " <<  "-" <<  " extrapval " << "33" << std::endl;
  yi =nt2::nearest(x, y, xi, z);
  NT2_TEST_EQUAL(yi, r2);
  NT2_DISPLAY(yi);
  std::cout << "extrap " <<  "-" <<  " extrapval " << "33" << std::endl;
  yi =nt2::nearest(x, y, xi, T(33));
  NT2_DISPLAY(yi);
  NT2_TEST_EQUAL(yi, r2);
}

NT2_TEST_CASE_TPL( nearest2, NT2_REAL_TYPES )
{
  using nt2::_;
  nt2::table<T> x =  nt2::linspace(T(1),  T(4), 4);
  nt2::table<T> y =  nt2::linspace(T(2),  T(8), 4);
  nt2::table<T> xi=  nt2::linspace(T(1),  T(4), 11);
  NT2_DISPLAY(x);
  NT2_DISPLAY(y);
  NT2_DISPLAY(xi);
  nt2::table<T> y0 =nt2::nearest(x, y, xi), yi;
  std::cout << "extrap " <<  false <<  " extrapval " << "-" << std::endl;
  NT2_DISPLAY(y0);
  y0 =nt2::nearest(x, y, xi, false);
  std::cout << "extrap " <<  false <<  " extrapval " << "-" << std::endl;
  std::cout << "extrap " <<  true <<  " extrapval " << "-" << std::endl;
  yi =nt2::nearest(x, y, xi, true);
  NT2_TEST_EQUAL(y0, yi);
  NT2_DISPLAY(yi);
  T z =  33;
  std::cout << "extrap " <<  "-" <<  " extrapval " << "33" << std::endl;
  yi =nt2::nearest(x, y, xi, z);
  NT2_DISPLAY(yi);
  NT2_TEST_EQUAL(y0, yi);
  std::cout << "extrap " <<  "-" <<  " extrapval " << "33" << std::endl;
  yi =nt2::nearest(x, y, xi, T(33));
  NT2_DISPLAY(yi);
  NT2_TEST_EQUAL(y0, yi);
}
NT2_TEST_CASE_TPL( nearest3, NT2_REAL_TYPES )
{
  using nt2::_;
  T x = 3;
  T y = 2;
  nt2::table<T> xi=  nt2::linspace(T(1),  T(4), 4);
  NT2_DISPLAY(x);
  NT2_DISPLAY(y);
  NT2_DISPLAY(xi);
  nt2::table<T> y0;
  boost::ignore_unused(y0);
  NT2_TEST_ASSERT(y0=nt2::nearest(x, y, xi));
  NT2_TEST_ASSERT(y0=nt2::nearest(T(3), y, xi));
  NT2_TEST_ASSERT(y0=nt2::nearest(T(3), y, xi, T(32)));
  NT2_TEST_ASSERT(y0=nt2::nearest(T(3), y, xi, true));
  NT2_TEST_ASSERT(T(nt2::nearest(x, y, T(25), true)));
  NT2_TEST_ASSERT(T(nt2::nearest(x, y, T(25), false)));
  NT2_TEST_ASSERT(T(nt2::nearest(x, y, T(25), T(32))));
}
NT2_TEST_CASE_TPL( nearest4, NT2_REAL_TYPES )
{
  using nt2::_;
  nt2::table<T> x =  T(3);
  nt2::table<T> y =  T(2);
  nt2::table<T> xi=  nt2::linspace(T(1),  T(4),4);
  nt2::table<T> y0;
  boost::ignore_unused(y0);
  NT2_TEST_ASSERT(y0=nt2::nearest(x, y, xi));
  NT2_TEST_ASSERT(y0=nt2::nearest(x, y, xi, T(32)));
  NT2_TEST_ASSERT(y0=nt2::nearest(x, y, xi, true));
}
