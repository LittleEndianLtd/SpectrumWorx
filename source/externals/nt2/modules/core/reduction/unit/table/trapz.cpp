//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::trapz function"

#include <nt2/table.hpp>
#include <nt2/include/functions/trapz.hpp>
#include <nt2/include/functions/colvect.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

NT2_TEST_CASE_TPL( trapz_scalar, (float)(double))//NT2_TYPES )
{
  T x = nt2::trapz(T(42), T(2));
  NT2_TEST_EQUAL( x, nt2::trapz(T(42), T(0)) );

  x = nt2::trapz(T(42),T(2), 1);
  NT2_TEST_EQUAL( x, nt2::trapz(T(42), T(0)) );

  x = nt2::trapz(T(42),T(2), 2);
  NT2_TEST_EQUAL( x, nt2::trapz(T(42), T(0)) );

}

NT2_TEST_CASE_TPL( trapz, NT2_REAL_TYPES )
{
  using nt2::_;
  nt2::table<T> y( nt2::of_size(5,3) );
  nt2::table<T> x =  nt2::_(T(1), T(5));
  nt2::table<T> x1=  nt2::_(T(1), T(3));
  nt2::table<T> sy, sy1, sy2, sy3;
  nt2::table<T> r1 = nt2::_(T(12), T(20), T(52));
  nt2::table<T> r2 = nt2::colvect(nt2::_(T(12), T(2), T(20)));

  y =  nt2::reshape(nt2::_(T(1), T(15)), 5, 3);
  NT2_DISPLAY(y);
  sy = nt2::trapz(y(_));
  NT2_DISPLAY(sy);
  NT2_TEST_EQUAL(sy, T(112));

  sy1 = nt2::trapz(y);
  NT2_DISPLAY(sy1);
  NT2_TEST_EQUAL(sy1, r1);

  sy2 = nt2::trapz(x, y, 1);
  NT2_DISPLAY(sy2);
  NT2_TEST_EQUAL(sy1, r1);

  sy3 = nt2::trapz(x1, y, 2);
  NT2_DISPLAY(sy3);
  NT2_TEST_EQUAL(sy3, r2);




}

