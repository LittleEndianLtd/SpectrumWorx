//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/transpose.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL( trans, NT2_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> z, x( nt2::of_size(5,4) ),y( nt2::of_size(4,5) );

  for(int j=1;j<=5;j++)
    for(int i=1;i<=4;i++)
      x(j, i) = y(i,j) = cT(i + 10*j);

  z = nt2::trans(y);

  for(std::size_t i=1;i<=nt2::numel(x);i++)
    NT2_TEST_EQUAL( x(i),z(i) );
}

NT2_TEST_CASE_TPL( trans_scalar, NT2_TYPES )
{
  typedef std::complex<T> cT;
  NT2_TEST_EQUAL(nt2::trans(cT(1)), cT(1));
}
