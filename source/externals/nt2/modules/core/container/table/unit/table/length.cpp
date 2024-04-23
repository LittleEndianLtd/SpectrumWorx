//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/length.hpp>
#include <nt2/include/functions/of_size.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE( table_length )
{
  using nt2::length;
  using nt2::of_size;
  using nt2::table;

  table<float> t0;
  table<float,nt2::of_size_<2,3,0,5> > s0;

  table<float> t1( of_size(2) );
  table<float> t2( of_size(4,2) );
  table<float> t3( of_size(4,6,2) );
  table<float> t4( of_size(4,6,8,2) );

  NT2_TEST_EQUAL( length(t0), 0U  );
  NT2_TEST_EQUAL( length(s0), 0U  );
  NT2_TEST_EQUAL( length(t1), 2U  );
  NT2_TEST_EQUAL( length(t2), 4U  );
  NT2_TEST_EQUAL( length(t3), 6U  );
  NT2_TEST_EQUAL( length(t4), 8U  );
}
