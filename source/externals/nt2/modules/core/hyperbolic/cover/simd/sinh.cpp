//==============================================================================
//         Copyright 2003 - 2013   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/hyperbolic/include/functions/sinh.hpp>

#include <nt2/sdk/unit/tests/cover.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/sdk/meta/cardinal_of.hpp>
#include <nt2/include/functions/aligned_load.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <iostream>


NT2_TEST_CASE_TPL ( sinh,  NT2_SIMD_REAL_TYPES)
{
  using nt2::sinh;
  using nt2::tag::sinh_;
  using boost::simd::native;
  using nt2::meta::cardinal_of;
  using nt2::aligned_load;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                  vT;

  static const nt2::uint32_t NR = NT2_NB_RANDOM_TEST*100;
  {
    NT2_CREATE_BUF(tab_a0,T, NR, T(-10), T(10));
    for(nt2::uint32_t j = 0; j < NR;j+=cardinal_of<vT>::value)
      {
        vT a0 = aligned_load<vT>(&tab_a0[0],j);
        vT v = sinh(a0);
        for(nt2::uint32_t i = 0; i< cardinal_of<vT>::value; i++)
        {
          std::cout << "input = " << a0[i] << std::endl;
          NT2_TEST_ULP_EQUAL( v[i],T(nt2::sinh (a0[i])), 4);
        }
      }
  }
} // end of test for floating_
