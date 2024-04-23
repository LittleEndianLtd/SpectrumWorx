//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/expand.hpp>
#include <nt2/include/functions/size.hpp>
#include <nt2/include/functions/zeros.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL( colvect_scalar, NT2_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> ref = zeros( nt2::of_size(3,3), nt2::meta::as_<cT>());
  ref(1, 1) = cT(1);

  NT2_TEST_EQUAL(nt2::expand(cT(1), nt2::of_size(3,3)), ref);

}

NT2_TEST_CASE_TPL( of_size, NT2_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> in( nt2::of_size(3,3) ), ref, out;

  for(int j=1;j<=9;j++) in(j) = cT(j);

  out = nt2::expand(in,nt2::of_size(0,0));
  NT2_TEST_EQUAL( nt2::numel(out), 0u);

  for(int j=1;j<5;++j)
  {
    for(int i=1;i<5;++i)
    {
      ref = nt2::zeros(i,j,nt2::meta::as_<cT>());

      for(int vj=1;vj<=std::min(j,3);++vj)
        for(int vi=1;vi<=std::min(i,3);++vi)
          ref(vi,vj) = in(vi,vj);

      out = nt2::expand(in,nt2::of_size(i,j));
      NT2_TEST_EQUAL( out,ref );
    }
  }
}

NT2_TEST_CASE_TPL( size, NT2_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> in( nt2::of_size(3,3) ), ref, out;

  for(int j=1;j<=9;j++) in(j) = cT(j);

  out = nt2::expand(in,nt2::of_size(0,0));
  NT2_TEST_EQUAL( nt2::numel(out), 0u);

  for(int j=1;j<5;++j)
  {
    for(int i=1;i<5;++i)
    {
      ref = nt2::zeros(i,j,nt2::meta::as_<cT>());

      for(int vj=1;vj<=std::min(j,3);++vj)
        for(int vi=1;vi<=std::min(i,3);++vi)
          ref(vi,vj) = in(vi,vj);

      out = nt2::expand(in,nt2::size(ref));
      NT2_TEST_EQUAL( out,ref );
    }
  }
}

NT2_TEST_CASE_TPL( scalar, NT2_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> in( nt2::of_size(3,3) ), ref, out;

  for(int j=1;j<=9;j++) in(j) = cT(j);

  out = nt2::expand(in,0,0);
  NT2_TEST_EQUAL( nt2::numel(out), 0u);

  for(int j=1;j<5;++j)
  {
    for(int i=1;i<5;++i)
    {
      ref = nt2::zeros(i,j,nt2::meta::as_<cT>());

      for(int vj=1;vj<=std::min(j,3);++vj)
        for(int vi=1;vi<=std::min(i,3);++vi)
          ref(vi,vj) = in(vi,vj);

      out = nt2::expand(in,i,j);
      NT2_TEST_EQUAL( out,ref );
    }
  }
}
