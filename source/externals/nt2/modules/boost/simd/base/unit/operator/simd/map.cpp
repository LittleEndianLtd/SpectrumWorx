//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/operator/include/functions/map.hpp>
#include <boost/simd/include/functions/bitwise_cast.hpp>
#include <boost/simd/include/functions/unary_plus.hpp>
#include <boost/simd/include/functions/plus.hpp>
#include <boost/simd/include/functions/if_else.hpp>
#include <boost/simd/include/constants/true.hpp>
#include <boost/simd/include/constants/false.hpp>
#include <boost/simd/include/constants/zero.hpp>
#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/two.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <boost/simd/sdk/simd/logical.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <boost/dispatch/meta/strip.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

NT2_TEST_CASE_TPL ( map_integer,  BOOST_SIMD_SIMD_TYPES)
{
  using boost::simd::map;
  using boost::simd::native;
  using boost::simd::tag::map_;
  using boost::simd::tag::unary_plus_;
  using boost::simd::tag::plus_;
  using boost::simd::tag::if_else_;
  typedef boost::dispatch::functor<unary_plus_> uni_f;
  typedef boost::dispatch::functor<plus_> bin_f;
  typedef boost::dispatch::functor<if_else_> tri_f;

  typedef native<T,BOOST_SIMD_DEFAULT_EXTENSION> nT;
  typedef typename boost::simd::meta::as_logical<nT>::type nlT;

  typedef typename boost::dispatch::meta::call<map_(uni_f,nT)>::type ur_t;
  typedef typename boost::dispatch::meta::call<map_(bin_f,nT,nT)>::type br_t;
  typedef typename boost::dispatch::meta::call<map_(tri_f,nT,nT,nT)>::type tr_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(ur_t, nT);
  NT2_TEST_TYPE_IS(br_t, nT);
  NT2_TEST_TYPE_IS(tr_t, nT);

  // specific values tests
  NT2_TEST_EQUAL(map(uni_f(), boost::simd::One<nT>()), boost::simd::One<ur_t>());
  NT2_TEST_EQUAL(map(uni_f(), boost::simd::One<nT>()), boost::simd::One<ur_t>());
  NT2_TEST_EQUAL(map(uni_f(), boost::simd::One<nT>()), boost::simd::One<ur_t>());

  NT2_TEST_EQUAL(map(bin_f(), boost::simd::One<nT>(), boost::simd::One<nT>()), boost::simd::Two<br_t>());
  NT2_TEST_EQUAL(map(bin_f(), boost::simd::One<nT>(), boost::simd::One<nT>()), boost::simd::Two<br_t>());
  NT2_TEST_EQUAL(map(bin_f(), boost::simd::One<nT>(), boost::simd::One<nT>()), boost::simd::Two<br_t>());

  NT2_TEST_EQUAL(map(tri_f(), boost::simd::False<nlT>(), boost::simd::One<nT>(), boost::simd::Two<nT>()), boost::simd::Two<br_t>());
  NT2_TEST_EQUAL(map(tri_f(), boost::simd::True<nlT>(),  boost::simd::One<nT>(), boost::simd::Two<nT>()), boost::simd::One<br_t>());
  NT2_TEST_EQUAL(map(tri_f(), boost::simd::True<nlT>(),  boost::simd::One<nT>(), boost::simd::Two<nT>()), boost::simd::One<br_t>());
}

struct logical_f
{
  template<class Sig>
  struct result;

  template<class This, class A0>
  struct result<This(A0)>
  {
    typedef typename boost::dispatch::meta::strip<A0>::type A0_;
    typedef typename boost::simd::meta::as_logical<A0_>::type type;
  };

  template<class A0>
  typename result<logical_f(A0 const&)>::type operator()(A0 const&) const
  {
    return boost::simd::logical<A0>(true);
  }

  template<class A0, class X>
  typename result<logical_f(boost::simd::native<A0, X> const&)>::type operator()(boost::simd::native<A0, X> const& a0) const
  {
    return boost::simd::map(*this, a0);
  }
};

NT2_TEST_CASE_TPL ( map_logical, (float)(boost::simd::int32_t)(boost::simd::uint32_t) )
{
  using boost::simd::logical;
  using boost::simd::native;
  using boost::simd::map;
  using boost::simd::True;
  using boost::mpl::_;
  typedef native<T, BOOST_SIMD_DEFAULT_EXTENSION> in_type;
  typedef native<logical<T>, BOOST_SIMD_DEFAULT_EXTENSION> out_type;

  in_type a;
  NT2_TEST_EXPR_TYPE( map(logical_f(), a), _, out_type );
  NT2_TEST_EQUAL( map(logical_f(), a), True<out_type>() );
}
