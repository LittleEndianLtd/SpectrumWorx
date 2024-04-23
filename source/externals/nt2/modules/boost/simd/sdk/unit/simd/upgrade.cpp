//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "boost::simd::meta::upgrade SIMD"

#include <boost/simd/sdk/simd/logical.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/pack.hpp>
#include <boost/simd/sdk/simd/meta/is_vectorizable.hpp>
#include <boost/dispatch/meta/upgrade.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/type_traits/is_same.hpp>

#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/module.hpp>

////////////////////////////////////////////////////////////////////////////////
// Test that upgrade is correct for SIMD types
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE_TPL(upgrade_native, BOOST_SIMD_SIMD_TYPES)
{
  using boost::simd::native;
  using boost::dispatch::meta::upgrade;
  using boost::mpl::_;

  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>             native_t;
  typedef typename upgrade<T>::type   base_t;
  typedef typename upgrade<T,unsigned>::type   ubase_t;

  typedef typename
  boost::mpl::if_ < boost::simd::meta::is_vectorizable<base_t,ext_t>
                  , ext_t
                  , boost::simd::tag::simd_emulation_<BOOST_SIMD_BYTES>
                  >::type extu_t;

  typedef typename
  boost::mpl::if_ < boost::simd::meta::is_vectorizable<ubase_t,ext_t>
                  , ext_t
                  , boost::simd::tag::simd_emulation_<BOOST_SIMD_BYTES>
                  >::type uextu_t;

  native_t a0;
  NT2_TEST_EXPR_TYPE( a0, upgrade<_>,            (native<base_t, extu_t>) );
  NT2_TEST_EXPR_TYPE( a0, (upgrade<_,unsigned>), (native<ubase_t, uextu_t>) );
}

////////////////////////////////////////////////////////////////////////////////
// Test that upgrade is correct for SIMD logical types
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE_TPL(upgrade_logical_native, BOOST_SIMD_SIMD_TYPES)
{
  using boost::simd::logical;
  using boost::simd::native;
  using boost::dispatch::meta::upgrade;
  using boost::mpl::_;

  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<logical<T>,ext_t>      native_t;
  typedef typename upgrade<T>::type     base_t;
  typedef typename upgrade<T,unsigned>::type   ubase_t;

  typedef typename
  boost::mpl::if_ < boost::simd::meta::is_vectorizable<base_t,ext_t>
                  , ext_t
                  , boost::simd::tag::simd_emulation_<BOOST_SIMD_BYTES>
                  >::type extu_t;

  typedef typename
  boost::mpl::if_ < boost::simd::meta::is_vectorizable<ubase_t,ext_t>
                  , ext_t
                  , boost::simd::tag::simd_emulation_<BOOST_SIMD_BYTES>
                  >::type uextu_t;

  native_t a0;
  NT2_TEST_EXPR_TYPE( a0, upgrade<_>,            (native<logical<base_t>,extu_t>) );
  NT2_TEST_EXPR_TYPE( a0, (upgrade<_,unsigned>), (native<logical<ubase_t>,uextu_t>) );
}

////////////////////////////////////////////////////////////////////////////////
// Test that upgrade is correct for pack
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE_TPL(upgrade_pack, BOOST_SIMD_SIMD_TYPES)
{
  using boost::simd::pack;
  using boost::dispatch::meta::upgrade;
  using boost::mpl::_;

  typedef pack<T>                       pack_t;
  typedef typename upgrade<T>::type     base_t;

  pack_t a0;
  NT2_TEST_EXPR_TYPE( a0, upgrade<_>, pack<base_t> );
}

////////////////////////////////////////////////////////////////////////////////
// Test that upgrade is correct for pack logical types
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE_TPL(upgrade_logical_pack, BOOST_SIMD_SIMD_TYPES)
{
  using boost::simd::logical;
  using boost::simd::pack;
  using boost::dispatch::meta::upgrade;
  using boost::mpl::_;

  typedef pack< logical<T> >            pack_t;
  typedef typename upgrade<T>::type     base_t;

  pack_t a0;
  NT2_TEST_EXPR_TYPE( a0, upgrade<_>, pack< logical<base_t> > );
}

////////////////////////////////////////////////////////////////////////////////
// Test that upgrade is correct for complex fusion cases
////////////////////////////////////////////////////////////////////////////////
template<int N>
struct upgrade_at
{
  template<class X>
  struct apply
  {
    typedef typename boost::dispatch::meta::upgrade<X>::type upgraded;
    typedef typename boost::fusion::result_of::value_at_c<upgraded, N>::type type;
  };
};

NT2_TEST_CASE(upgrade_fusion)
{
  using boost::simd::native;
  using boost::dispatch::meta::upgrade;
  using boost::mpl::_;

  typedef std::pair<nt2::int32_t, float> T;
  typedef native<T, BOOST_SIMD_DEFAULT_EXTENSION> vT;
  typedef native<nt2::int32_t, BOOST_SIMD_DEFAULT_EXTENSION> vT0;
  typedef native<float, BOOST_SIMD_DEFAULT_EXTENSION> vT1;

  vT va0;
  //NT2_TEST_EXPR_TYPE( va0,  upgrade<_>, int ); // for debug purposes
  NT2_TEST_EXPR_TYPE( va0, upgrade_at<0>, upgrade<vT0>::type );
  NT2_TEST_EXPR_TYPE( va0, upgrade_at<1>, upgrade<vT1>::type );
}
