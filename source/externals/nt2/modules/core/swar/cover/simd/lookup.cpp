//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 swar toolbox - lookup/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// cover test behavior of swar components in simd mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 24/02/2011
///
#include <nt2/swar/include/functions/lookup.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <nt2/include/functions/max.hpp>
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
#include <nt2/include/functions/aligned_load.hpp>
#include <nt2/constant/constant.hpp>


NT2_TEST_CASE_TPL ( lookup_real__2_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::lookup;
  using nt2::tag::lookup_;
  using nt2::aligned_load;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef typename nt2::meta::upgrade<T>::type   u_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef native<iT,ext_t>                       ivT;
  typedef typename nt2::meta::call<lookup_(vT,ivT)>::type r_t;
  typedef typename nt2::meta::call<lookup_(T,iT)>::type sr_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;


  // random verifications
  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  {
    NT2_CREATE_BUF(tab_a0,T, NR, T(-1e30), T(1e30));
    NT2_CREATE_BUF(tab_a1,iT, NR, 0, nt2::meta::cardinal_of<T>::value-1);
    double ulp0, ulpd ; ulpd=ulp0=0.0;
    for(nt2::uint32_t j = 0; j < NR;j+=cardinal_of<n_t>::value)
      {
        vT a0 = aligned_load<vT>(&tab_a0[0],j);
        ivT a1 = aligned_load<ivT>(&tab_a1[0],j);
        r_t v = nt2::lookup(a0,a1);
        for(uint32_t i=0; i<cardinal_of<n_t>::value; i++)
        {
           NT2_TEST_EQUAL(v[i],a0[a1[i]]);
        }
      }

  }
} // end of test for floating_

NT2_TEST_CASE_TPL ( lookup_integer__2_0,  NT2_SIMD_INTEGRAL_TYPES)
{
  using nt2::lookup;
  using nt2::tag::lookup_;
  using nt2::aligned_load;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef typename nt2::meta::upgrade<T>::type   u_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef native<iT,ext_t>                       ivT;
  typedef typename nt2::meta::call<lookup_(vT,ivT)>::type r_t;
  typedef typename nt2::meta::call<lookup_(T,iT)>::type sr_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;


  // random verifications
  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  {
    NT2_CREATE_BUF(tab_a0,T, NR, T(0), T(+10000));
    NT2_CREATE_BUF(tab_a1,iT, NR, 0u, nt2::meta::cardinal_of<vT>::value-1);
    double ulp0, ulpd ; ulpd=ulp0=0.0;
    for(nt2::uint32_t j = 0; j < NR; j+= cardinal_of<n_t>::value)
      {
        vT a0 = aligned_load<vT>(&tab_a0[0],j);
        ivT a1 = aligned_load<ivT>(&tab_a1[0],j);
        std::cout << "a0 " << a0 << std::endl;
        std::cout << "a1 " << a1 << std::endl;
        r_t v = nt2::lookup(a0,a1);
        for(uint32_t i=0; i<cardinal_of<n_t>::value; i++)
        {
           NT2_TEST_EQUAL(v[i],a0[a1[i]]);
        }
      }

  }
} // end of test for integer_
