//==============================================================================
//         Copyright 2003 - 2013   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/trigonometric/include/functions/cot.hpp>

#include <nt2/sdk/functor/meta/call.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/include/functions/splat.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/sdk/config.hpp>
#include <boost/simd/sdk/simd/io.hpp>

#include <nt2/include/constants/mone.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/pi.hpp>
#include <nt2/include/constants/zero.hpp>
#include <nt2/include/constants/inf.hpp>
#include <nt2/include/constants/minf.hpp>
#include <nt2/include/constants/nan.hpp>

NT2_TEST_CASE_TPL ( cot_real_1,  NT2_SIMD_REAL_TYPES)
{
  using nt2::cot;
  using nt2::tag::cot_;
  using boost::simd::native;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                  vT;

  typedef typename nt2::meta::call<cot_(vT)>::type r_t;
  typedef vT wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(r_t, wished_r_t);


  // specific values tests
#ifndef BOOST_SIMD_NO_INVALIDS
  NT2_TEST_ULP_EQUAL(cot(-nt2::Zero<vT>()), -nt2::Inf<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(nt2::Inf<vT>()), nt2::Nan<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(nt2::Minf<vT>()), nt2::Nan<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(nt2::Nan<vT>()), nt2::Nan<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(nt2::Zero<vT>()), nt2::Inf<r_t>(), 0.5);
#endif
  NT2_TEST_ULP_EQUAL(cot(-nt2::Pi<vT>()/nt2::splat<vT>(2)), nt2::Zero<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(-nt2::Pi<vT>()/nt2::splat<vT>(4)), nt2::Mone<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(nt2::Pi<vT>()/nt2::splat<vT>(2)), nt2::Zero<r_t>(), 0.5);
  NT2_TEST_ULP_EQUAL(cot(nt2::Pi<vT>()/nt2::splat<vT>(4)), nt2::One<r_t>(), 0.5);
}
