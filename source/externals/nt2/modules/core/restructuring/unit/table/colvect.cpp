//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::colvect function"

#include <nt2/table.hpp>
#include <nt2/include/functions/colvect.hpp>
#include <nt2/include/functions/size.hpp>
#include <nt2/include/functions/numel.hpp>


#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>

NT2_TEST_CASE_TPL( colvect_scalar, NT2_TYPES )
{
  NT2_TEST_EQUAL(nt2::colvect(T(1)), T(1));
}


NT2_TEST_CASE_TPL( colvect_ofsize,NT2_TYPES )
{
  nt2::table<T> r;
  nt2::table<T, nt2::_2D> y( nt2::of_size(4,4) );

  for(int j=1;j<=4;j++)
    for(int i=1;i<=4;i++)
      y(i,j) = T(i + 10*j);

  r = nt2::colvect(y);

  int k = 1;
  for(int j=1;j<=4;j++)
    for(int i=1;i<=4;i++, k++)
      NT2_TEST_EQUAL( T(r(k)) , T(y(i, j)) );
  NT2_TEST_EQUAL( nt2::ndims(r)  , 2u );
  NT2_TEST_EQUAL( nt2::size(r,1) , numel(y) );
  NT2_TEST_EQUAL( nt2::size(r,2) , 1u );
  NT2_TEST_EQUAL( nt2::size(r,3) , 1u );

}

NT2_TEST_CASE_TPL( colvect, NT2_TYPES)
{
  nt2::table<T> r;
  nt2::table<T, nt2::_2D> y( nt2::of_size(4,4) );
  for(int j=1;j<=4;j++)
    for(int i=1;i<=4;i++)
      y(i,j) = T(i + 10*j);
  r = nt2::colvect(y);

  NT2_TEST_EQUAL(r, y(nt2::_));
  NT2_TEST_EQUAL(nt2::colvect(y), y(nt2::_));
}
