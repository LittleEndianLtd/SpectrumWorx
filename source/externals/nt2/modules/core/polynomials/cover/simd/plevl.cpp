//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 polynomials toolbox - plevl/simd Mode"

//////////////////////////////////////////////////////////////////////////////
// cover test behavior of polynomials components in simd mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 06/03/2011
///
#include <nt2/polynomials/include/functions/plevl.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/include/functions/max.hpp>
#include <vector>

#include <boost/type_traits/is_same.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/include/constants/real.hpp>
#include <nt2/include/constants/infinites.hpp>
#include <nt2/include/functions/aligned_load.hpp>
//COMMENTED

NT2_TEST_CASE_TPL ( plevl_real__2_0,  NT2_SIMD_REAL_TYPES)
{
//   using nt2::plevl;
//   using nt2::tag::plevl_;
//   using nt2::aligned_load;
//   using nt2::simd::native;
//   using nt2::meta::cardinal_of;
//   typedef std::vector<T> A_t;
//   typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
//   typedef typename nt2::meta::upgrade<T>::type   u_t;
//   typedef native<T,ext_t>                        n_t;
//   typedef n_t                                     vT;
//   typedef typename nt2::meta::as_integer<T>::type iT;
//   typedef native<iT,ext_t>                       ivT;
//   typedef typename nt2::meta::call<plevl_(vT,A_t)>::type r_t;
//   typedef typename nt2::meta::call<plevl_(T,A_t)>::type sr_t;
//   typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;
//   double ulpd;
//   ulpd=0.0;

//   // random verifications
//   static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
//   {
//     typedef std::vector<T> A_t;
//     NT2_CREATE_BUF(tab_a0,T, NR, T(-10), T(10));
//     NT2_CREATE_BUF(tab_a1,A_t, NR, T(-10), T(10));
//     double ulp0, ulpd ; ulpd=ulp0=0.0;
//     for(nt2::uint32_t j = 0; j < NR;j+=cardinal_of<n_t>::value)
//       {
//         vT a0 = aligned_load<vT>(&tab_a0[0],j);
//         A_t a1 = aligned_load<A_t>(&tab_a1[0],j);
//         r_t v = plevl(a0,a1);
//         for(int i = 0; i< cardinal_of<n_t>::value; i++)
//         {
//           int k = i+j*cardinal_of<n_t>::value;
//           NT2_TEST_ULP_EQUAL( v[i],ssr_t(nt2::plevl (a0[i],a1[i])), 0.5);
//           ulp0 = nt2::max(ulpd,ulp0);
//         }
//       }
//     std::cout << "max ulp found is: " << ulp0 << std::endl;
//   }
} // end of test for floating_
