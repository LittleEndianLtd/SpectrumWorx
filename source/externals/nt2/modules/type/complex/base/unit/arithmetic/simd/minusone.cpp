//==============================================================================
//         Copyright 2003 - 2013   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/minusone.hpp>

#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <complex>
#include <nt2/sdk/complex/complex.hpp>
#include <nt2/sdk/complex/dry.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/meta/as_integer.hpp>
#include <nt2/include/functions/splat.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/sdk/config.hpp>

#include <nt2/include/constants/mone.hpp>
#include <nt2/include/constants/mtwo.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/zero.hpp>
#include <nt2/include/constants/inf.hpp>
#include <nt2/include/constants/minf.hpp>
#include <nt2/include/constants/nan.hpp>

NT2_TEST_CASE_TPL ( abs_cplx,  (float))
{
  using nt2::minusone;
  using nt2::tag::minusone_;
  typedef typename std::complex<T> cT;
  using boost::simd::native;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<cT,ext_t>                vcT;
  typedef typename nt2::dry<T>             dT;
  typedef native<dT,ext_t>                vdT;

  // specific values tests
    NT2_TEST_ULP_EQUAL(minusone(nt2::Inf<vcT>()), nt2::Inf<vcT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Minf<vcT>()), nt2::Minf<vcT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Nan<vcT>()), nt2::Nan<vcT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Inf<vdT>()), nt2::Inf<vdT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Minf<vdT>()), nt2::Minf<vdT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Nan<vdT>()), nt2::Nan<vdT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::splat<vcT>(cT(T(-1), T(-1.6)))), nt2::splat<vcT>(cT(T(-2), T(-1.6))),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::splat<vcT>(cT(T(1), T(1.6)))),  nt2::splat<vcT>(cT(T(0), T(1.6))),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Mone<vcT>()), nt2::Mtwo<vcT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::One<vcT>()), nt2::Zero<vcT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Zero<vcT>()), nt2::Mone<vcT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::splat<vdT>(dT(T(-1)))), nt2::splat<vdT>(dT(T(-2))),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::splat<vdT>(dT(T(1)))), nt2::splat<vdT>(dT(T(0))),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Mone<vdT>()), nt2::Mtwo<vdT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::One<vdT>()), nt2::Zero<vdT>(),0);
    NT2_TEST_ULP_EQUAL(minusone(nt2::Zero<vdT>()), nt2::Mone<vdT>(),0);
}
