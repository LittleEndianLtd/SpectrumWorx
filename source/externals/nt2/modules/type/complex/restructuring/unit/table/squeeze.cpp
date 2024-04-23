//==============================================================================
//         Copyright 2003 - 2012 LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012 LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/squeeze.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

//==============================================================================
// Squeezing 2D array is identity
//==============================================================================
NT2_TEST_CASE( squeeze_2D )
{
  typedef std::complex<double> cT;

  NT2_TEST_EQUAL(  nt2::squeeze(1),1);
  nt2::table<cT, nt2::_2D> x( nt2::of_size(1,5) ), sx;
  nt2::table<cT, nt2::_2D> y( nt2::of_size(5,1) ), sy;
  nt2::table<cT, nt2::_2D> z( nt2::of_size(1,1) ), sz;

  for(std::size_t i=1;i<=5;++i) x(i) = i;
  sx = nt2::squeeze(x);

  NT2_TEST_EQUAL( sx.extent(), x.extent( ) );
  for(std::size_t i=1;i<=5;++i) NT2_TEST_EQUAL( x(i) , sx(i) );

  for(std::size_t i=1;i<=5;++i) y(i) = i;
  sy = nt2::squeeze(y);

  NT2_TEST_EQUAL( sy.extent(), y.extent( ) );
  for(std::size_t i=1;i<=5;++i) NT2_TEST_EQUAL( y(i) , sy(i) );

  z(1) = 1;
  sz = nt2::squeeze(z);

  NT2_TEST_EQUAL( sz.extent(), z.extent( ) );
  NT2_TEST_EQUAL( z(1) , sy(1) );
}

template<class S, class Z> void test_squeeze(S const& old_s, Z const& new_s)
{
  typedef std::complex<double> cT;
  nt2::table<cT, S> x(old_s);
  for(std::size_t i=1;i<=nt2::numel(old_s);++i) x(i) = i;

  nt2::table<cT, S> sx = nt2::squeeze(x);

  NT2_TEST_EQUAL( sx.extent(), new_s );

  for(std::size_t i=1;i<=nt2::numel(old_s);++i)
    NT2_TEST_EQUAL( x(i), sx(i) );
}

NT2_TEST_CASE( squeeze_3D )
{
  test_squeeze( nt2::of_size(1,3,3) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(3,1,3) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(3,3,1) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(3,1,1) , nt2::of_size(3,1) );
  test_squeeze( nt2::of_size(1,3,1) , nt2::of_size(3,1) );
  test_squeeze( nt2::of_size(1,1,3) , nt2::of_size(3,1) );
  test_squeeze( nt2::of_size(1,1,1) , nt2::of_size(1,1) );
}

NT2_TEST_CASE( squeeze_4D )
{
  test_squeeze( nt2::of_size(1,3,3,3) , nt2::of_size(3,3,3) );
  test_squeeze( nt2::of_size(3,1,3,3) , nt2::of_size(3,3,3) );
  test_squeeze( nt2::of_size(3,3,1,3) , nt2::of_size(3,3,3) );
  test_squeeze( nt2::of_size(3,3,3,1) , nt2::of_size(3,3,3) );

  test_squeeze( nt2::of_size(1,1,3,3) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(1,3,1,3) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(1,3,3,1) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(3,1,1,3) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(3,1,3,1) , nt2::of_size(3,3) );
  test_squeeze( nt2::of_size(3,3,1,1) , nt2::of_size(3,3) );

  test_squeeze( nt2::of_size(3,1,1,1) , nt2::of_size(3,1) );
  test_squeeze( nt2::of_size(1,3,1,1) , nt2::of_size(3,1) );
  test_squeeze( nt2::of_size(1,1,3,1) , nt2::of_size(3,1) );
  test_squeeze( nt2::of_size(1,1,1,3) , nt2::of_size(3,1) );

  test_squeeze( nt2::of_size(1,1,1,1) , nt2::of_size(1,1) );
}
