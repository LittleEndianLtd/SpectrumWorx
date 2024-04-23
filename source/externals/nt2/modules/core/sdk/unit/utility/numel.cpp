//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::numel function"

#include <nt2/table.hpp>
#include <nt2/include/functions/numel.hpp>
#include <nt2/include/functions/of_size.hpp>
#include <boost/mpl/vector.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

////////////////////////////////////////////////////////////////////////////////
// numel of arithmetic types
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE( fundamental_numel )
{
  using nt2::numel;
  using boost::mpl::_;

  NT2_TEST_EQUAL( numel('4'), 1U  );
  NT2_TEST_EXPR_TYPE( numel('4'), _, (boost::mpl::size_t<1ul>) );

  NT2_TEST_EQUAL( numel(4)  , 1U  );
  NT2_TEST_EXPR_TYPE( numel(4), _, (boost::mpl::size_t<1ul>) );

  NT2_TEST_EQUAL( numel(4.) , 1U  );
  NT2_TEST_EXPR_TYPE( numel(4.), _, (boost::mpl::size_t<1ul>) );

  NT2_TEST_EQUAL( numel(4.f), 1U  );
  NT2_TEST_EXPR_TYPE( numel(4.f), _, (boost::mpl::size_t<1ul>) );

  // Numel of fusion vector preserves types
  boost::fusion::vector<nt2::uint16_t,nt2::uint16_t> fv(2,3);
  NT2_TEST_EQUAL( numel(fv), nt2::uint16_t(6) );
  NT2_TEST_EXPR_TYPE( numel(fv), _, (nt2::uint16_t) );
}

////////////////////////////////////////////////////////////////////////////////
// numel of container
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE( container_numel )
{
  using nt2::numel;
  using nt2::tag::table_;
  using nt2::of_size;

  typedef nt2::memory::container<table_,float,nt2::settings()> container_t;

  container_t t0;
  container_t t1( of_size(2,1,1,1) );
  container_t t2( of_size(2,2,1,1) );
  container_t t3( of_size(2,2,2,1) );
  container_t t4( of_size(2,2,2,2) );

  NT2_TEST_EQUAL( numel(t0), 0U   );
  NT2_TEST_EQUAL( numel(t1), 2U   );
  NT2_TEST_EQUAL( numel(t2), 4U   );
  NT2_TEST_EQUAL( numel(t3), 8U   );
  NT2_TEST_EQUAL( numel(t4), 16U  );
}

////////////////////////////////////////////////////////////////////////////////
// numel of table
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE( table_numel )
{
  using nt2::numel;
  using nt2::of_size;
  using nt2::table;

  table<float> t0;
  table<float> t1( of_size(2) );
  table<float> t2( of_size(2,2) );
  table<float> t3( of_size(2,2,2) );
  table<float> t4( of_size(2,2,2,2) );

  NT2_TEST_EQUAL( numel(t0), 0U   );
  NT2_TEST_EQUAL( numel(t1), 2U   );
  NT2_TEST_EQUAL( numel(t2), 4U   );
  NT2_TEST_EQUAL( numel(t3), 8U   );
  NT2_TEST_EQUAL( numel(t4), 16U  );
}

////////////////////////////////////////////////////////////////////////////////
// numel of table expression
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE( expression_numel )
{
  using nt2::numel;
  using nt2::of_size;
  using nt2::table;

  table<float> t0;
  table<float> t1( of_size(2) );
  table<float> t2( of_size(2,2) );
  table<float> t3( of_size(2,2,2) );
  table<float> t4( of_size(2,2,2,2) );

  NT2_TEST_EQUAL( numel(-t0), 0U   );
  NT2_TEST_EQUAL( numel(t1*t1), 2U   );
  NT2_TEST_EQUAL( numel(t2-t2*t2), 4U   );
  NT2_TEST_EQUAL( numel(t3/t3+t3), 8U   );
  NT2_TEST_EQUAL( numel(t4 * -t4), 16U  );
}

NT2_TEST_CASE( static_numel )
{
  using nt2::numel;
  using nt2::of_size_;
  using nt2::table;
  using boost::mpl::_;

  float a;
  of_size_<2, 3> b;
  table<float, of_size_<2, 3> > c;
  boost::mpl::vector_c<nt2::int16_t,2,3,4> mv;

  NT2_TEST_EXPR_TYPE( numel(a)
                    , _
                    , boost::mpl::size_t<1>
                    );

  NT2_TEST_EXPR_TYPE( numel(b)
                    , _
                    , ( boost::mpl::integral_c<std::size_t, 6> )
                    );

  NT2_TEST_EXPR_TYPE( numel(c)
                    , _
                    , ( boost::mpl::integral_c<std::size_t, 6> )
                    );

  NT2_TEST_EXPR_TYPE( numel(mv)
                    , _
                    , ( boost::mpl::integral_c<nt2::int16_t, 24> )
                    );
}
