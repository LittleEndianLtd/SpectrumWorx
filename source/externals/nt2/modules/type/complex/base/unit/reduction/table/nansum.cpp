//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::nansum function"

#include <nt2/table.hpp>
#include <nt2/include/functions/nansum.hpp>
#include <nt2/include/functions/sum.hpp>
#include <nt2/include/functions/size.hpp>
#include <nt2/include/functions/firstnonsingleton.hpp>
#include <nt2/include/functions/is_nan.hpp>
#include <nt2/include/functions/if_zero_else.hpp>
#include <nt2/include/constants/nan.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>

NT2_TEST_CASE_TPL( nansum_scalar, NT2_REAL_TYPES )
{
  typedef std::complex<T> cT;
  cT x = nt2::nansum(cT(42));
  NT2_TEST_EQUAL( x, cT(42) );

  x = nt2::nansum(cT(42),1);
  NT2_TEST_EQUAL( x, cT(42) );

  x = nt2::nansum(cT(42),0);
  NT2_TEST_EQUAL( x, cT(42) );

  x = nt2::nansum(cT(nt2::Nan<T>()),0);
  NT2_TEST_EQUAL( x, nt2::Zero<cT>() );
}

NT2_TEST_CASE_TPL( nansum, NT2_REAL_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> y( nt2::of_size(5,3) );
  nt2::table<cT> sy;
  nt2::table<cT> sy2;


  for(int j=1;j<=3;j++)
    for(int i=1;i<=5;i++)
      y(i,j) = i + 10*j;
  y(2, 3) = nt2::Nan<T>();
  sy = nt2::sum(nt2::if_zero_else(nt2::is_nan(y), y));
  sy2 = nt2::nansum(y);
  for(size_t j=1;j<=size(sy, 2);j++)
    for(size_t i=1;i<=size(sy, 1);i++)
      NT2_TEST_EQUAL(sy(i,j), sy2(i, j));

  sy = nt2::sum(nt2::if_zero_else(nt2::is_nan(y), y), 1);
  sy2 = nt2::nansum(y, 1);
  for(size_t j=1;j<=size(sy, 2);j++)
    for(size_t i=1;i<=size(sy, 1);i++)
      NT2_TEST_EQUAL(sy(i,j), sy2(i, j));

  sy = nt2::sum(nt2::if_zero_else(nt2::is_nan(y), y), 2);
  sy2 = nt2::nansum(y, 2);
  for(size_t j=1;j<=size(sy, 2);j++)
    for(size_t i=1;i<=size(sy, 1);i++)
      NT2_TEST_EQUAL(sy(i,j), sy2(i, j));

  sy = nt2::sum(nt2::if_zero_else(nt2::is_nan(y), y), 3);
  sy2 = nt2::nansum(y, 3);
  for(size_t j=1;j<=size(sy, 2);j++)
    for(size_t i=1;i<=size(sy, 1);i++)
      NT2_TEST_EQUAL(sy(i,j), sy2(i, j));


}

NT2_TEST_CASE_TPL( nansum_2, NT2_REAL_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> y( nt2::of_size(5,3) );
  nt2::table<cT> sy;
  nt2::table<cT> sy2;


  for(int j=1;j<=3;j++)
    for(int i=1;i<=5;i++)
      y(i,j) = i + 10*j;
  y(2, 3) = nt2::Nan<T>();
  sy = nt2::sum(nt2::if_zero_else(nt2::is_nan(y), y));
  sy2 = nt2::nansum(y);
  NT2_TEST_EQUAL(sy2, sy);
  NT2_TEST_EQUAL(sy2, nt2::nansum(y));
  sy2 = nt2::nansum(y, 1);
  NT2_TEST_EQUAL(sy2, nt2::nansum(y, 1));
  sy2 = nt2::nansum(y, 2);
  NT2_TEST_EQUAL(sy2, nt2::nansum(y, 2));
  sy2 = nt2::nansum(y, 3);
  NT2_TEST_EQUAL(sy2, nt2::nansum(y, 3));
  sy2 = nt2::nansum(y, 4);
  NT2_TEST_EQUAL(sy2, nt2::nansum(y, 4));


}
