//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::var function"

#include <nt2/table.hpp>
#include <nt2/include/functions/var.hpp>
#include <nt2/include/functions/sum.hpp>
#include <nt2/include/functions/center.hpp>
#include <nt2/include/functions/sqr_abs.hpp>
#include <nt2/include/functions/size.hpp>
#include <nt2/include/functions/firstnonsingleton.hpp>
#include <nt2/include/functions/zeros.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL( var_scalar, NT2_REAL_TYPES )
{
  typedef std::complex<T>  cT;
  cT x = nt2::var(cT(42));
  NT2_TEST_EQUAL( x, cT(0) );

  x = nt2::var(cT(42),1);
  NT2_TEST_EQUAL( x, cT(0) );

  x = nt2::var(T(42),0);
  NT2_TEST_EQUAL( x, cT(0) );

}

NT2_TEST_CASE_TPL( var, NT2_REAL_TYPES )
{
  typedef std::complex<T>  cT;
  nt2::table<cT> y( nt2::of_size(5,3) );
  nt2::table<cT> cy, cy2;
  nt2::table<T> sy;
  nt2::table<T> sy2;


  for(int j=1;j<=3;j++)
    for(int i=1;i<=5;i++)
      y(i,j) = T(i + 10*j);
  cy =  center(y, 2);
  sy =  asum2(cy, 2)/T(nt2::size(y, 2)-1);
  sy2 = nt2::var(y, 0, 2);
  NT2_TEST_EQUAL(sy, sy2);
  NT2_TEST_EQUAL(sy,  nt2::var(y, 0, 2));

  cy =  center(y, 1);
  sy =  asum2(cy, 1)/T(nt2::size(y, 1)-1);
  sy2 = nt2::var(y, 0, 1);
  NT2_TEST_EQUAL(sy, sy2);
  NT2_TEST_EQUAL(sy,  nt2::var(y, 0, 1));


  sy2 = nt2::var(y, 0, 3);
  NT2_TEST_EQUAL(zeros(size(sy2), nt2::meta::as_<T>()), sy2);
  NT2_TEST_EQUAL(zeros(size(sy2), nt2::meta::as_<T>()), nt2::var(y, 0, 3));


}


