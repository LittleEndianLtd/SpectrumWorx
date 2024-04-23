//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::meanad function"

#include <nt2/table.hpp>
#include <nt2/include/functions/meanad.hpp>
#include <nt2/include/functions/center.hpp>
#include <nt2/include/functions/mean.hpp>
#include <nt2/include/functions/abs.hpp>
#include <nt2/include/functions/size.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

NT2_TEST_CASE_TPL( meanad_scalar, NT2_REAL_TYPES )
{
  typedef std::complex<T> cT;
  cT x = nt2::meanad(cT(42));
  NT2_TEST_EQUAL( x, T(0) );

  x = nt2::meanad(cT(42),1);
  NT2_TEST_EQUAL( x, T(0) );

  x = nt2::meanad(cT(42),0);
  NT2_TEST_EQUAL( x, T(0) );

}

NT2_TEST_CASE_TPL( meanad, NT2_REAL_TYPES )
{
  typedef std::complex<T> cT;
  using nt2::_;
  nt2::table<cT> y( nt2::of_size(5,3) ), y0( nt2::of_size(5,3) );
  nt2::table<T> sy( nt2::of_size(1,3) );
  nt2::table<T> sz( nt2::of_size(1,3) );


  for(size_t j=1;j<=size(y, 2);j++)
    for(size_t i=1;i<=size(y, 1);i++)
      y0(i,j) = cT(T(i) - T(j), T(i) - T(j));
  NT2_DISPLAY(y0);

  sy = nt2::meanad(y0);
  y =  center(y0);
  NT2_DISPLAY(y);
  NT2_DISPLAY(nt2::abs(y));
  sz = nt2::mean(nt2::abs(y));
  NT2_TEST_EQUAL(sz, sy);
  y =  center(y0, 1);
  sy = nt2::meanad(y0, 1);
  sz = nt2::mean(nt2::abs(y), 1);
  NT2_TEST_EQUAL(sz, sy);
  y =  center(y0, 2);
  sy = nt2::meanad(y0, 2);
  sz = nt2::mean(nt2::abs(y), 2);
  NT2_TEST_EQUAL(sz, sy);
  y =  center(y0, 3);
  sy = nt2::meanad(y0, 3);
  sz = nt2::mean(nt2::abs(y), 3);
  NT2_TEST_EQUAL(sz, sy);
  y =  center(y0(_));
  sy = nt2::meanad(y0(_));
  sz = nt2::mean(nt2::abs(y(_)));
  NT2_TEST_ULP_EQUAL(sy(1), sz(1), 1.0);
}
