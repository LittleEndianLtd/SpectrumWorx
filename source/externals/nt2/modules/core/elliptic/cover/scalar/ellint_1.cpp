//==============================================================================
//         Copyright 2003 - 2014   LASMEA UMR 6602 CNRS/UBP
//         Copyright 2009 - 2014   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
// cover for functor ellint_1 in scalar mode
#include <nt2/elliptic/include/functions/ellint_1.hpp>
#include <boost/math/special_functions/ellint_1.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <cmath>
#include <iostream>
#include <nt2/sdk/meta/as_integer.hpp>
#include <nt2/sdk/unit/args.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/cover.hpp>
#include <vector>

NT2_TEST_CASE_TPL(ellint_1_0,  NT2_SIMD_REAL_TYPES)
{
  using nt2::unit::args;
  const std::size_t NR = args("samples", NT2_NB_RANDOM_TEST);
  const double ulpd = args("ulpd", 4);

  typedef typename nt2::meta::as_integer<T>::type iT;
  const T min = args("min", T(-1));
  const T max = args("max", T(1));
  std::cout << "Argument samples #0 chosen in range: [" << min << ",  " << max << "]" << std::endl;
  NT2_CREATE_BUF(a0,T, NR, min, max);

  std::vector<T> ref(NR);
  for(std::size_t i=0; i!=NR; ++i)
    ref[i] = boost::math::ellint_1(a0[i]);

  NT2_COVER_ULP_EQUAL(nt2::tag::ellint_1_, ((T, a0)), ref, ulpd);

}
