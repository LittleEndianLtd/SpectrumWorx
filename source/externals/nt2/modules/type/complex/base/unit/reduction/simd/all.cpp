//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/reduction/include/functions/all.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/simd/logical.hpp>
#include <nt2/sdk/functor/meta/call.hpp>

#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/meta/cardinal_of.hpp>
#include <nt2/sdk/complex/complex.hpp>
#include <nt2/include/constants/inf.hpp>
#include <nt2/include/constants/minf.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/mone.hpp>
#include <nt2/include/constants/zero.hpp>
#include <nt2/include/constants/nan.hpp>
#include <nt2/include/constants/true.hpp>
#include <nt2/include/constants/false.hpp>

NT2_TEST_CASE_TPL ( all_real,  NT2_SIMD_REAL_TYPES)
{
  using nt2::all;
  using nt2::tag::all_;
  using boost::simd::native;
  using nt2::meta::cardinal_of;

  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<std::complex<T> ,ext_t>            vcT;
  typedef typename nt2::meta::call<all_(vcT)>::type r_t;

  // specific values tests
  NT2_TEST_EQUAL(nt2::all(nt2::Inf<vcT>()), nt2::True<r_t>());
  NT2_TEST_EQUAL(nt2::all(nt2::Minf<vcT>()), nt2::True<r_t>());
  NT2_TEST_EQUAL(nt2::all(nt2::Mone<vcT>()), nt2::True<r_t>());
  NT2_TEST_EQUAL(nt2::all(nt2::Nan<vcT>()), nt2::True<r_t>());
  NT2_TEST_EQUAL(nt2::all(nt2::One<vcT>()), nt2::True<r_t>());
  NT2_TEST_EQUAL(nt2::all(nt2::Zero<vcT>()), nt2::False<r_t>());
} // end of test for floating_
