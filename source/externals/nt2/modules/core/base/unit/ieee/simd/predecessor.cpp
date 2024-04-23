//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 ieee toolbox - predecessor/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of ieee components in simd mode
//////////////////////////////////////////////////////////////////////////////
/// created by jt the 04/12/2010
///
#include <nt2/ieee/include/functions/predecessor.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/constant/constant.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/meta/scalar_of.hpp>
#include <boost/type_traits/is_same.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL ( predecessor_real__1_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::predecessor;
  using nt2::tag::predecessor_;
  using boost::simd::native;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::call<predecessor_(T)>::type sr_t;

  // specific values tests
  NT2_TEST_EQUAL(predecessor(nt2::Inf<vT>())[0], nt2::Valmax<sr_t>());
  NT2_TEST_EQUAL(predecessor(nt2::Minf<vT>())[0], nt2::Nan<sr_t>());
  NT2_TEST_EQUAL(predecessor(nt2::Mone<vT>())[0], nt2::Mone<sr_t>()-nt2::Eps<sr_t>());
  NT2_TEST_EQUAL(predecessor(nt2::Nan<vT>())[0], nt2::Nan<sr_t>());
  NT2_TEST_EQUAL(predecessor(nt2::One<vT>())[0], nt2::One<sr_t>()-nt2::Eps<sr_t>()/2);
  NT2_TEST_EQUAL(predecessor(nt2::Valmin<vT>())[0], nt2::Minf<sr_t>());
  NT2_TEST_EQUAL(predecessor(nt2::Zero<vT>())[0], -nt2::Bitincrement<T>());
} // end of test for floating_
