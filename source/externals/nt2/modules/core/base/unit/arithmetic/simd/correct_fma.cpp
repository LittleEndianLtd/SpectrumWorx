//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 arithmetic toolbox - correct_fma/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of arithmetic components in simd mode
//////////////////////////////////////////////////////////////////////////////
/// created by jt the 28/11/2010
///
///
#include <nt2/arithmetic/include/functions/correct_fma.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/include/constants/eps_related.hpp>

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



NT2_TEST_CASE_TPL ( correct_fma_real__3_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::correct_fma;
  using nt2::tag::correct_fma_;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;



  // specific values tests
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::Inf<vT>(), nt2::Inf<vT>(), nt2::Inf<vT>())[0], nt2::Inf<T>(), 0);
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::Minf<vT>(), nt2::Minf<vT>(), nt2::Minf<vT>())[0], nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::Mone<vT>(), nt2::Mone<vT>(), nt2::Mone<vT>())[0], nt2::Zero<T>(), 0);
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::Nan<vT>(), nt2::Nan<vT>(), nt2::Nan<vT>())[0], nt2::Nan<T>(), 0);
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::One<vT>(), nt2::One<vT>(), nt2::One<vT>())[0], nt2::Two<T>(), 0);
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::One<vT>()+nt2::Eps<vT>(), nt2::One<vT>()-nt2::Eps<vT>(),nt2::Mone<vT>())[0], -nt2::Eps<T>()*nt2::Eps<T>(), 0);
  NT2_TEST_ULP_EQUAL(correct_fma(nt2::Zero<vT>(), nt2::Zero<vT>(), nt2::Zero<vT>())[0], nt2::Zero<T>(), 0);
} // end of test for floating_