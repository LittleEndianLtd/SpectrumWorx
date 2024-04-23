//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/asum2.hpp>
#include <nt2/include/functions/sum.hpp>
#include <nt2/include/functions/sqr_abs.hpp>
#include <nt2/include/functions/size.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL( asum2_scalar, NT2_REAL_TYPES )
{
  typedef std::complex<T>  cT;
  T x = nt2::asum2(cT(42));
  NT2_TEST_EQUAL( x, nt2::sqr(T(42)) );

  x = nt2::asum2(cT(42),1);
  NT2_TEST_EQUAL( x, nt2::sqr(T(42)) );

  x = nt2::asum2(cT(42),0);
  NT2_TEST_EQUAL( x, nt2::sqr(T(42)) );

}

NT2_TEST_CASE_TPL( asum2, NT2_REAL_TYPES )
{
  typedef std::complex<T>  cT;
  using nt2::_;
  nt2::table<cT> y( nt2::of_size(5,3) );
  nt2::table<T> sy( nt2::of_size(1,3) );
  nt2::table<T> sz( nt2::of_size(1,3) );


  for(int j=1;j<=3;j++)
    for(int i=1;i<=5;i++)
      y(i,j) = i + 10*j;

  for(size_t j=1;j<=size(sy, 2);j++)
    for(size_t i=1;i<=size(sy, 1);i++)
      y(i,j) = i - j;

  sy = nt2::asum2(y);
  sz = nt2::sum(nt2::sqr_abs(y));
  for(int j=1;j<=3;j++)
      NT2_TEST_EQUAL(sz(j), sy(j));
  sy = nt2::asum2(y, 1);
  sz = nt2::sum(nt2::sqr_abs(y), 1);
  for(size_t j=1;j<=size(sy, 2);j++)
      NT2_TEST_EQUAL(sz(j), sy(j));
  sy = nt2::asum2(y, 2);
  sz = nt2::sum(nt2::sqr_abs(y), 2);
    for(size_t i=1;i<=size(sy, 1);i++)
      NT2_TEST_EQUAL(sz(i), sy(i));
  sy = nt2::asum2(y, 3);
  sz = nt2::sum(nt2::sqr_abs(y), 3);
  for(size_t j=1;j<=size(sy, 2);j++)
    for(size_t i=1;i<=size(sy, 1);i++)
      NT2_TEST_EQUAL(sz(i, j), sy(i, j));


  sy = nt2::asum2(y(_));
  sz = nt2::sum(nt2::sqr_abs(y(_)));
  NT2_TEST_EQUAL(sy(1), sz(1));


}

