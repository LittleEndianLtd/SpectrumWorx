//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/reduction/include/functions/is_included.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/constant/constant.hpp>

NT2_TEST_CASE_TPL ( is_included_integer__2_0,  BOOST_SIMD_SIMD_INTEGRAL_TYPES)
{
  using boost::simd::is_included;
  using boost::simd::tag::is_included_;
  using boost::simd::native;
  using boost::simd::meta::cardinal_of;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename boost::dispatch::meta::call<is_included_(vT,vT)>::type r_t;
  typedef typename boost::simd::meta::scalar_of<r_t>::type sr_t;

  // specific values tests
  NT2_TEST_EQUAL(is_included(boost::simd::Mone<vT>(),boost::simd::Zero<vT>()), boost::simd::False<sr_t>());
  NT2_TEST_EQUAL(is_included(boost::simd::One<vT>(), boost::simd::One<vT>()), boost::simd::True<sr_t>());
  NT2_TEST_EQUAL(is_included(boost::simd::One<vT>(),boost::simd::Mone<vT>()), boost::simd::True<sr_t>());
  NT2_TEST_EQUAL(is_included(boost::simd::One<vT>(),boost::simd::Three<vT>()), boost::simd::True<sr_t>());
  NT2_TEST_EQUAL(is_included(boost::simd::One<vT>(),boost::simd::Two<vT>()), boost::simd::False<sr_t>());
  NT2_TEST_EQUAL(is_included(boost::simd::One<vT>(),boost::simd::Zero<vT>()), boost::simd::False<sr_t>());
  NT2_TEST_EQUAL(is_included(boost::simd::Zero<vT>(), boost::simd::Zero<vT>()), boost::simd::True<sr_t>());
} // end of test for integer_
