//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/simd/fast_rec.hpp>
#include <nt2/include/functions/simd/splat.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/io.hpp>

#include <boost/dispatch/functor/meta/call.hpp>
#include <boost/simd/constant/constant.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

NT2_TEST_CASE_TPL ( fast_rec,  BOOST_SIMD_SIMD_REAL_TYPES)
{
  using nt2::fast_rec;
  using nt2::splat;
  using nt2::tag::fast_rec_;
  using boost::simd::native;

  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>               vT;

  NT2_TEST_TYPE_IS( typename boost::dispatch::meta::call<fast_rec_(vT)>::type
                  , (native<T,ext_t>)
                  );

  // 1/+-1 = +-1
  NT2_TEST_ULP_EQUAL(fast_rec(boost::simd::Mone<vT>()), boost::simd::Mone<vT>(), 16);
  NT2_TEST_ULP_EQUAL(fast_rec(boost::simd::One<vT>()), boost::simd::One<vT>(), 16);

  NT2_TEST_ULP_EQUAL(fast_rec( splat<vT>(2) ), splat<vT>(0.5)  , 16);
  NT2_TEST_ULP_EQUAL(fast_rec( splat<vT>(10)), splat<vT>(0.1)  , 16);
  NT2_TEST_ULP_EQUAL(fast_rec( splat<vT>(3) ), splat<vT>(1./3.), 16);

  // 1/(1/x) = x
  NT2_TEST_ULP_EQUAL(fast_rec(fast_rec(boost::simd::Ten<vT>())), boost::simd::Ten<vT>(), 16);
}
