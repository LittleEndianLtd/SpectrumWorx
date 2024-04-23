//==============================================================================
//         Copyright 2003 - 2014   LASMEA UMR 6602 CNRS/UBP
//         Copyright 2009 - 2014   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
// cover for functor signgam in scalar mode
#include <nt2/euler/include/functions/signgam.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <cmath>
#include <iostream>
#include <nt2/sdk/meta/as_integer.hpp>
#include <nt2/sdk/unit/args.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/cover.hpp>
#include <vector>
extern "C" {long double cephes_lgaml(long double);}
#include <boost/math/special_functions/gamma.hpp>

extern int sgngam;

NT2_TEST_CASE_TPL(signgam_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::unit::args;
  const std::size_t NR = args("samples", NT2_NB_RANDOM_TEST);
  const double ulpd = args("ulpd", 0.5);

  typedef typename nt2::meta::as_integer<T>::type iT;
  const T min = args("min", T(-30));
  const T max = args("max", T(30));
  std::cout << "Argument samples #0 chosen in range: [" << min << ",  " << max << "]" << std::endl;
  NT2_CREATE_BUF(a0,T, NR, min, max);

  std::vector<T> ref(NR);
  for(std::size_t i=0; i!=NR; ++i)
  {
    int s;
    boost::math::lgamma(a0[i], &s);
    ref[i] = s;
  }


  NT2_COVER_ULP_EQUAL(nt2::tag::signgam_, ((T, a0)), ref, ulpd);

}
