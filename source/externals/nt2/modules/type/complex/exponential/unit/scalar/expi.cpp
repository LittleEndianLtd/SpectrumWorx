//==============================================================================
//         Copyright 2003 - 2013   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/expi.hpp>

#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <complex>
#include <nt2/sdk/complex/complex.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/sdk/config.hpp>

#include <nt2/include/constants/mone.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/pi.hpp>
#include <nt2/include/constants/zero.hpp>
#include <nt2/include/constants/inf.hpp>
#include <nt2/include/constants/minf.hpp>
#include <nt2/include/constants/nan.hpp>
#include <nt2/include/constants/i.hpp>
#include <nt2/include/functions/mul_i.hpp>
#include <nt2/include/functions/exp.hpp>

NT2_TEST_CASE_TPL ( expi_real,  NT2_REAL_TYPES)
{
  using nt2::expi;
  using nt2::tag::expi_;
  typedef typename std::complex<T> cT;
  NT2_TEST_TYPE_IS( typename nt2::meta::call<expi_(cT)>::type, cT );

  // specific values tests
#ifndef BOOST_SIMD_NO_INVALIDS
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::Inf<T>()))  , nt2::exp(nt2::mul_i(cT(nt2::Inf<T>())))  , 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::Minf<T>())) , nt2::exp(nt2::mul_i(cT(nt2::Minf<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::Nan<T>()))  , nt2::exp(nt2::mul_i(cT(nt2::Nan<T>()))  ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::Inf<T>()))  , nt2::exp(nt2::mul_i(cT(nt2::Inf<T>())))  , 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::Minf<T>())) , nt2::exp(nt2::mul_i(cT(nt2::Minf<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::Nan<T>()))  , nt2::exp(nt2::mul_i(cT(nt2::Nan<T>()))  ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::Inf<T>()))  , nt2::exp(nt2::mul_i(cT(0, nt2::Inf<T>())))  , 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::Minf<T>())) , nt2::exp(nt2::mul_i(cT(0, nt2::Minf<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::Nan<T>()))  , nt2::exp(nt2::mul_i(cT(0, nt2::Nan<T>()))  ), 1);
#endif
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::Mone<T>())) , nt2::exp(nt2::mul_i(cT(nt2::Mone<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::One<T>()))  , nt2::exp(nt2::mul_i(cT(nt2::One<T>()))  ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::Zero<T>())) , nt2::exp(nt2::mul_i(cT(nt2::Zero<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(nt2::Pi<T>())), nt2::exp(nt2::mul_i(cT(nt2::Pi<T>()))), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::Mone<T>())) , nt2::exp(nt2::mul_i(cT(nt2::Mone<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::One<T>()))  , nt2::exp(nt2::mul_i(cT(nt2::One<T>()))  ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::Zero<T>())) , nt2::exp(nt2::mul_i(cT(nt2::Zero<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(T(nt2::Pi<T>()))   , nt2::exp(nt2::mul_i(cT(nt2::Pi<T>()))), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::Mone<T>())) , nt2::exp(nt2::mul_i(cT(0, nt2::Mone<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::One<T>()))  , nt2::exp(nt2::mul_i(cT(0, nt2::One<T>()))  ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::Zero<T>())) , nt2::exp(nt2::mul_i(cT(0, nt2::Zero<T>())) ), 1);
  NT2_TEST_ULP_EQUAL(nt2::expi(cT(0, nt2::Pi<T>()))   , nt2::exp(nt2::mul_i(cT(0, nt2::Pi<T>()))), 1);
}
