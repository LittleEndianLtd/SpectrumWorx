//==============================================================================
//         Copyright 2003 - 2014   LASMEA UMR 6602 CNRS/UBP
//         Copyright 2009 - 2014   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
// cover for functor y0 in simd mode
#include <nt2/bessel/include/functions/y0.hpp>

#include <boost/simd/sdk/simd/io.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <cmath>
#include <iostream>
#include <nt2/sdk/meta/as_integer.hpp>
#include <nt2/sdk/unit/args.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/cover.hpp>
#include <vector>
extern "C" {long double cephes_y0l(long double);}

NT2_TEST_CASE_TPL(y0_0,  NT2_SIMD_REAL_TYPES)
{
  using boost::simd::native;
  typedef BOOST_SIMD_DEFAULT_EXTENSION  ext_t;
  typedef native<T,ext_t>                  vT;

  using nt2::unit::args;
  const std::size_t NR = args("samples", NT2_NB_RANDOM_TEST);
  const double ulpd = args("ulpd",  30);

  typedef typename nt2::meta::as_integer<vT>::type ivT;
  const T min = args("min", T(0));
  const T max = args("max", T(10));
  std::cout << "Argument samples randomly chosen in [" << min << ",  " << max << "]" << std::endl;
  NT2_CREATE_BUF(a0,T, NR,min, max);

  std::vector<T> ref(NR);
  for(std::size_t i=0; i!=NR; ++i)
    ref[i] = nt2::y0(a0[i]);

  NT2_COVER_ULP_EQUAL(nt2::tag::y0_, ((vT, a0)), ref, ulpd);

}
