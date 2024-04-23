//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/arithmetic/include/functions/subs.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/include/functions/simd/plus.hpp>
#include <boost/simd/include/constants/mtwo.hpp>
#include <boost/simd/include/constants/two.hpp>
#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/mone.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/minf.hpp>
#include <boost/simd/include/constants/nan.hpp>
#include <boost/simd/sdk/config.hpp>
#include <boost/simd/sdk/simd/io.hpp>

NT2_TEST_CASE_TPL ( subs_signed_int,  BOOST_SIMD_SIMD_INTEGRAL_SIGNED_TYPES)
{
  using boost::simd::subs;
  using boost::simd::tag::subs_;
  using boost::simd::native;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                  vT;
  typedef typename boost::dispatch::meta::call<subs_(vT,vT)>::type r_t;

  // specific values tests
  NT2_TEST_EQUAL(subs(boost::simd::splat<vT>(-2), boost::simd::Valmin<vT>()), boost::simd::Valmax<r_t>()-boost::simd::One<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Mone<vT>(), boost::simd::Mone<vT>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Mone<vT>(), boost::simd::Valmax<vT>()), boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Mone<vT>(), boost::simd::Valmin<vT>()), boost::simd::Valmax<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::One<vT>(),boost::simd::Mone<vT>()), boost::simd::Two<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Valmin<vT>(),boost::simd::Mone<vT>()), boost::simd::Valmin<r_t>()+boost::simd::One<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Valmin<vT>(),boost::simd::One<vT>()), boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Zero<vT>(), boost::simd::Zero<vT>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Zero<vT>(), boost::simd::Valmin<vT>()), boost::simd::Valmax<r_t>());
} // end of test for signed_int_

NT2_TEST_CASE_TPL ( subs_unsigned_int,  BOOST_SIMD_SIMD_UNSIGNED_TYPES)
{
  using boost::simd::subs;
  using boost::simd::tag::subs_;
  using boost::simd::native;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                  vT;
  typedef typename boost::dispatch::meta::call<subs_(vT,vT)>::type r_t;

  // specific values tests
  NT2_TEST_EQUAL(subs(boost::simd::One<vT>(), boost::simd::One<vT>()), boost::simd::Zero<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::One<vT>(),boost::simd::Valmax<vT>()), boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Valmin<vT>(),boost::simd::One<vT>()), boost::simd::Valmin<r_t>());
  NT2_TEST_EQUAL(subs(boost::simd::Zero<vT>(), boost::simd::Zero<vT>()), boost::simd::Zero<r_t>());
} // end of test for unsigned_int_
