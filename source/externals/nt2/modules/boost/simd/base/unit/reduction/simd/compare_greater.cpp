//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/reduction/include/functions/compare_greater.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/include/functions/all.hpp>

#include <boost/type_traits/is_same.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/constant/constant.hpp>

NT2_TEST_CASE_TPL ( compare_greater_real__2_0,  BOOST_SIMD_SIMD_REAL_TYPES)
{
 //  using boost::simd::compare_greater;
  using boost::simd::tag::compare_greater_;
  using boost::simd::native;
  using boost::simd::meta::cardinal_of;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename boost::dispatch::meta::call<compare_greater_(vT,vT)>::type r_t;

  // specific values tests
  NT2_TEST_EQUAL(compare_greater(boost::simd::Inf<vT>(), boost::simd::Inf<vT>()), r_t(false));
  NT2_TEST_EQUAL(compare_greater(boost::simd::Minf<vT>(), boost::simd::Minf<vT>()), r_t(false));
  NT2_TEST_EQUAL(compare_greater(boost::simd::Nan<vT>(), boost::simd::Nan<vT>()), r_t(false));
  NT2_TEST_EQUAL(compare_greater(boost::simd::One<vT>(),boost::simd::Two<vT>()), r_t(false));
  NT2_TEST_EQUAL(compare_greater(boost::simd::One<vT>(),boost::simd::Zero<vT>()), r_t(true));
  NT2_TEST_EQUAL(compare_greater(boost::simd::Zero<vT>(), boost::simd::Zero<vT>()), r_t(false));
} // end of test for floating_

NT2_TEST_CASE_TPL ( compare_greater_integer__2_0,  BOOST_SIMD_SIMD_INTEGRAL_TYPES)
{
  using boost::simd::compare_greater;
  using boost::simd::tag::compare_greater_;
  using boost::simd::native;
  using boost::simd::meta::cardinal_of;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename boost::dispatch::meta::call<compare_greater_(vT,vT)>::type r_t;

  // specific values tests
  NT2_TEST_EQUAL(compare_greater(boost::simd::One<vT>(), boost::simd::One<vT>()), r_t(false));
  NT2_TEST_EQUAL(compare_greater(boost::simd::One<vT>(),boost::simd::Zero<vT>()), r_t(true));
  NT2_TEST_EQUAL(compare_greater(boost::simd::Zero<vT>(), boost::simd::Zero<vT>()), r_t(false));
} // end of test for integer_
