//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 operator toolbox - unary_minus/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of operator components in simd mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 18/02/2011
///
#include <nt2/operator/include/functions/unary_minus.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/type_traits/is_same.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/meta/as_integer.hpp>
#include <nt2/sdk/meta/as_floating.hpp>
#include <nt2/sdk/meta/as_signed.hpp>
#include <nt2/sdk/meta/upgrade.hpp>
#include <nt2/sdk/meta/downgrade.hpp>
#include <nt2/sdk/meta/scalar_of.hpp>
#include <boost/dispatch/meta/as_floating.hpp>
#include <boost/type_traits/common_type.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/constant/constant.hpp>
#include <nt2/sdk/meta/cardinal_of.hpp>
#include <nt2/include/functions/splat.hpp>


NT2_TEST_CASE_TPL ( unary_minus_real__1_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::unary_minus;
  using nt2::tag::unary_minus_;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::call<unary_minus_(T)>::type sr_t;



  // specific values tests
  NT2_TEST_EQUAL(unary_minus(nt2::Inf<vT>())[0], nt2::Minf<sr_t>());
  NT2_TEST_EQUAL(unary_minus(nt2::Minf<vT>())[0], nt2::Inf<sr_t>());
  NT2_TEST_EQUAL(unary_minus(nt2::Nan<vT>())[0], nt2::Nan<sr_t>());
  NT2_TEST_EQUAL(unary_minus(nt2::One<vT>())[0], nt2::Mone<sr_t>());
  NT2_TEST_EQUAL(unary_minus(nt2::Zero<vT>())[0], nt2::Mzero<sr_t>());
} // end of test for floating_

NT2_TEST_CASE_TPL ( unary_minus_signed_int__1_0,  NT2_SIMD_INTEGRAL_SIGNED_TYPES)
{
  using nt2::unary_minus;
  using nt2::tag::unary_minus_;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::call<unary_minus_(T)>::type sr_t;



  // specific values tests
  NT2_TEST_EQUAL(unary_minus(nt2::Mone<vT>())[0], nt2::One<sr_t>());
  NT2_TEST_EQUAL(unary_minus(nt2::One<vT>())[0], nt2::Mone<sr_t>());
  NT2_TEST_EQUAL(unary_minus(nt2::Two<vT>())[0], sr_t(-nt2::Two<sr_t>()));
  NT2_TEST_EQUAL(unary_minus(nt2::Zero<vT>())[0], nt2::Zero<sr_t>());
} // end of test for signed_int_