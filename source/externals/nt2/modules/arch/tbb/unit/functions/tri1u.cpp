//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::tri1u function"

#include <nt2/table.hpp>
#include <nt2/include/functions/tri1u.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL( tri1u_scalar, NT2_TYPES )
{
  T x = nt2::tri1u(T(42));
  NT2_TEST_EQUAL( x, T(1) );
  x = nt2::tri1u(T(42),1);
  NT2_TEST_EQUAL( x, T(0) );
   x = nt2::tri1u(T(42),0);
  NT2_TEST_EQUAL( x, T(1) );
  x = nt2::tri1u(T(42),-1);
  NT2_TEST_EQUAL( x, T(42) );
}

NT2_TEST_CASE_TPL( tri1u_scalar_table, NT2_TYPES )
{
  nt2::table<T> tx,ty( nt2::of_size(1, 1) );
  ty(1) = T(42);
  tx = nt2::tri1u(ty);
  NT2_TEST_EQUAL( T(tx(1)), T(1) );

  tx = nt2::tri1u(ty, 1);
  NT2_TEST_EQUAL( T(tx(1)), T(0) );

  tx = nt2::tri1u(ty, 0);
  NT2_TEST_EQUAL( T(tx(1)), T(1) );

  tx = nt2::tri1u(ty, -1);
  NT2_TEST_EQUAL( T(tx(1)), T(42) );
}

NT2_TEST_CASE_TPL( tri1u, NT2_TYPES )
{
  nt2::table<T> x,y( nt2::of_size(4,5) );

  for(int j=1;j<=5;j++)
    for(int i=1;i<=4;i++)
      y(i,j) = T(i + 10*j);
  for(int i=1;i<=4;i++)
    {
      for(int j=1;j<=5;j++)
        std::cout << y(i,j) << "\t";
      std::cout << std::endl;
    }
  std::cout << std::endl;

  x = nt2::tri1u(y);

  for(int j=1;j<=5;j++)
    for(int i=1;i<=4;i++)
      NT2_TEST_EQUAL( T(x(i,j)), (i == j) ? T(1) : (i<=j) ? T(y(i,j)) : T(0));

  for(int i=1;i<=4;i++)
    {
      for(int j=1;j<=5;j++)
        std::cout << x(i,j) << "\t";
      std::cout << std::endl;
    }
  std::cout << std::endl;
}

NT2_TEST_CASE_TPL( offset_tri1u, NT2_TYPES )
{
  nt2::table<T> x,y( nt2::of_size(4,5) );

  for(int j=1;j<=5;j++)
    for(int i=1;i<=4;i++)
      y(i,j) = T(i + 10*j);

  x = nt2::tri1u(y,1);

  for(int j=1;j<=5;j++)
    for(int i=1;i<=4;i++)
      NT2_TEST_EQUAL( T(x(i,j)), (i+1 == j) ? T(1) : (i+1<j) ? T(y(i,j)) : T(0));

  x = nt2::tri1u(y,-1);

  for(int j=1;j<=5;j++)
    for(int i=1;i<=4;i++)
      NT2_TEST_EQUAL( T(x(i,j)), (i-1 == j) ? T(1) : (i-1<j) ? T(y(i,j)) : T(0));
}
