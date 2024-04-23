//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 linalg toolbox - vandermonde function"

#include <nt2/table.hpp>
#include <nt2/include/functions/vandermonde.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>
#include <nt2/include/functions/isulpequal.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>


NT2_TEST_CASE_TPL ( vandermonde_2, NT2_REAL_TYPES)
{
  typedef std::complex<T> cT;
  nt2::table<cT> a0(nt2::of_size(1, 3));
  for(int i=1; i <= 3; i++)
  {
    a0(i) = cT(i);
  }
  NT2_DISPLAY(a0);
  nt2::table<cT> v  =  nt2::vandermonde(a0);
  nt2::display("vandermonde(a0)", v);
  nt2::display("vandermonde(a0)", nt2::vandermonde(a0));
  T bc[9] =  {
    1, 1, 1,
    4, 2, 1,
    9, 3, 1
  };
  int k = 0;
  nt2::table<cT> a(nt2::of_size(3, 3));
  for(int i=1; i <= 3; i++)
  {
    for(int j=1; j <= 3; j++)
    {
      a(i, j) = cT(bc[k++]);
    }

  }
  NT2_DISPLAY(a);
 NT2_TEST(nt2::isulpequal(a, v, 2.0));
}

