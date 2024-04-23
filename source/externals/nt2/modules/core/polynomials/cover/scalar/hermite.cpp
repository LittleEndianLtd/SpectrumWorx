//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 polynomials toolbox - hermite/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// cover test behavior of polynomials components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 06/03/2011
///
#include <nt2/polynomials/include/functions/hermite.hpp>
#include <nt2/include/functions/max.hpp>
#include <nt2/boost_math/include/functions/hermite.hpp>

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


NT2_TEST_CASE_TPL ( hermite_real__2_0,  NT2_REAL_TYPES)
{

  using nt2::hermite;
  using nt2::tag::hermite_;
    typedef typename nt2::meta::as_integer<T>::type iscalar;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<hermite_(iT,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef typename boost::dispatch::meta::as_floating<T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;

  // random verifications
  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  {
      typedef typename nt2::meta::as_integer<T>::type iscalar;
    NT2_CREATE_BUF(tab_a0,iT, NR, iT(0), iT(10));
    NT2_CREATE_BUF(tab_a1,T, NR, T(-10), T(10));
    iT a0;
    T a1;
    for(nt2::uint32_t j =0; j < NR; ++j )
      {
        std::cout << "for params "
                  << "  a0 = "<< u_t(a0 = tab_a0[j])
                  << ", a1 = "<< u_t(a1 = tab_a1[j])
                  << std::endl;
        NT2_TEST_ULP_EQUAL( nt2::hermite(a0,a1),nt2::boost_math::hermite(a0,a1),50);
     }
   }
} // end of test for floating_

NT2_TEST_CASE_TPL ( hermite_unsigned_int__2_0,  NT2_UNSIGNED_TYPES)
{

  using nt2::hermite;
  using nt2::tag::hermite_;
    typedef typename nt2::meta::as_integer<T>::type iscalar;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<hermite_(iT,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef typename boost::dispatch::meta::as_floating<T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;
  double ulpd;
  ulpd=0.0;

} // end of test for unsigned_int_

NT2_TEST_CASE_TPL ( hermite_signed_int__2_0,  NT2_INTEGRAL_SIGNED_TYPES)
{

  using nt2::hermite;
  using nt2::tag::hermite_;
    typedef typename nt2::meta::as_integer<T>::type iscalar;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef typename nt2::meta::call<hermite_(iT,T)>::type r_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
  typedef typename nt2::meta::upgrade<T>::type u_t;
  typedef typename boost::dispatch::meta::as_floating<T>::type wished_r_t;


  // return type conformity test
  NT2_TEST( (boost::is_same < r_t, wished_r_t >::value) );
  std::cout << std::endl;
  double ulpd;
  ulpd=0.0;

} // end of test for signed_int_
