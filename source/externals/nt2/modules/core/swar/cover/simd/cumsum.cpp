//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/swar/include/functions/cumsum.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/include/functions/max.hpp>
#include <nt2/include/functions/all.hpp>

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


NT2_TEST_CASE_TPL ( cumsum_real__1_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::cumsum;
  using nt2::tag::cumsum_;
  using nt2::aligned_load;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef typename nt2::meta::upgrade<T>::type   u_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef native<iT,ext_t>                       ivT;
  typedef typename nt2::meta::call<cumsum_(vT)>::type r_t;
  typedef typename nt2::meta::call<cumsum_(T)>::type sr_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;

  // random verifications
  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  {
    NT2_CREATE_BUF(tab_a0,T, NR, T(-1e3), T(1e3));
    for(nt2::uint32_t j = 0; j < NR;j+=cardinal_of<n_t>::value)
      {
        vT a0 = aligned_load<vT>(&tab_a0[0],j);
        r_t v = nt2::cumsum(a0);

        std::vector<T,boost::simd::allocator<T> > z(cardinal_of<n_t>::value);
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++) z[i]=0;

        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++) {
          for( uint32_t k = i; k<cardinal_of<n_t>::value; k++) {
            z[k]+=a0[i];
          }
        }
        vT zz = aligned_load<vT>(&z[0],0);
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++)
         {
            NT2_TEST_ULP_EQUAL(v[i],zz[i], 4);
         }
      }

  }
} // end of test for floating_

NT2_TEST_CASE_TPL ( cumsum_signed_int__1_0,  NT2_SIMD_INTEGRAL_SIGNED_TYPES)
{
  using nt2::cumsum;
  using nt2::tag::cumsum_;
  using nt2::aligned_load;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef typename nt2::meta::upgrade<T>::type   u_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef native<iT,ext_t>                       ivT;
  typedef typename nt2::meta::call<cumsum_(vT)>::type r_t;
  typedef typename nt2::meta::call<cumsum_(T)>::type sr_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;

  // random verifications
  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  {
    NT2_CREATE_BUF(tab_a0,T, NR, nt2::Valmin<T>()/T(cardinal_of<n_t>::value), nt2::Valmax<T>()/T(cardinal_of<n_t>::value));
    for(nt2::uint32_t j = 0; j < NR;j+=cardinal_of<n_t>::value)
      {
        vT a0 = aligned_load<vT>(&tab_a0[0],j);
        r_t v = nt2::cumsum(a0);
        NT2_CREATE_BUF(z,T, cardinal_of<n_t>::value, T(0), T(0));
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++) z[i]=0;
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++) {
          for( uint32_t k = i; k<cardinal_of<n_t>::value; k++) {
            z[k]+=a0[i];
          }
        }
        vT zz = aligned_load<vT>(&z[0],0);
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++)
         {
            NT2_TEST_ULP_EQUAL(v[i],zz[i], 16);
         }
      }

  }
} // end of test for signed_int_

NT2_TEST_CASE_TPL ( cumsum_unsigned_int__1_0,  NT2_SIMD_UNSIGNED_TYPES)
{
  using nt2::cumsum;
  using nt2::tag::cumsum_;
  using nt2::aligned_load;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  typedef NT2_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef typename nt2::meta::upgrade<T>::type   u_t;
  typedef native<T,ext_t>                        n_t;
  typedef n_t                                     vT;
  typedef typename nt2::meta::as_integer<T>::type iT;
  typedef native<iT,ext_t>                       ivT;
  typedef typename nt2::meta::call<cumsum_(vT)>::type r_t;
  typedef typename nt2::meta::call<cumsum_(T)>::type sr_t;
  typedef typename nt2::meta::scalar_of<r_t>::type ssr_t;

  // random verifications
  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  {
    NT2_CREATE_BUF(tab_a0,T, NR, nt2::Valmin<T>(), nt2::Valmax<T>());
    for(nt2::uint32_t j = 0; j < NR;j+=cardinal_of<n_t>::value)
      {
        vT a0 = aligned_load<vT>(&tab_a0[0],j);
        r_t v = nt2::cumsum(a0);
        NT2_CREATE_BUF(z,T, cardinal_of<n_t>::value, T(0), T(0));
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++) z[i]=0;
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++) {
          for( uint32_t k = i; k<cardinal_of<n_t>::value; k++) {
            z[k]+=a0[i];
          }
        }
        vT zz = aligned_load<vT>(&z[0],0);
        for( uint32_t i = 0; i<cardinal_of<n_t>::value; i++)
         {
            NT2_TEST_ULP_EQUAL(v[i],zz[i], 16);
         }
      }

  }
} // end of test for unsigned_int_
