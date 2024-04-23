//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/combinatorial/include/functions/factorial.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <vector>
#include <nt2/include/functions/gammaln.hpp>
#include <nt2/include/functions/exp.hpp>
#include <nt2/include/functions/round.hpp>
#include <nt2/boost_math/include/functions/factorial.hpp>
#include <nt2/include/functions/toint.hpp>
#include <nt2/include/functions/abs.hpp>
#include <nt2/include/functions/min.hpp>

#include <nt2/sdk/unit/args.hpp>
#include <nt2/sdk/unit/tests/cover.hpp>
#include <nt2/sdk/unit/module.hpp>

NT2_TEST_CASE_TPL(factorial,  NT2_REAL_TYPES)
{
  using nt2::unit::args;
  const std::size_t NR = args("samples", NT2_NB_RANDOM_TEST);
  const double ulpd = args("ulpd", 0);

  const T min = args("min", T(0));
  const T max = args("max", T(10));
  std::cout << "Argument samples a0 chosen in range: [" << min << ",  " << max << "]" << std::endl;
  std::cout << "Argument samples a1 chosen in range: [ a0,  a0+" << max << "]" << std::endl;
  NT2_CREATE_BUF(a0,T, NR, min, max);
  NT2_CREATE_BUF(a1,T, NR, min, max);

  std::vector<T> ref(NR);
  for(std::size_t i=0; i!=NR; ++i)
  {
    a0[i] = nt2::round(a0[i]);
    ref[i] = nt2::boost_math::factorial<double>(nt2::abs(nt2::toint(a0[i])));
  }
  NT2_COVER_ULP_EQUAL(nt2::tag::factorial_, ((T, a0)), ref, ulpd);
}

NT2_TEST_CASE_TPL(factorial_int,  NT2_INTEGRAL_TYPES)
{
  using nt2::unit::args;
  const std::size_t NR = args("samples", NT2_NB_RANDOM_TEST);
  const double ulpd = args("ulpd", 0);

  const T min = args("min", T(0));
  const T max = args("max", T(10));
  std::cout << "Argument samples a0 chosen in range: [" << int(min) << ",  " << int(max) << "]" << std::endl;
  std::cout << "Argument samples a1 chosen in range: [ a0,  a0+" << max << "]" << std::endl;
  NT2_CREATE_BUF(a0,T, NR, min, max);
  NT2_CREATE_BUF(a1,T, NR, min, max);

  std::vector<T> ref(NR);
  for(std::size_t i=0; i!=NR; ++i)
  {
    a0[i]  = nt2::round(a0[i]);
    ref[i] = nt2::min((uint64_t)nt2::min(double(nt2::Valmax<T>()),
                                         nt2::boost_math::factorial<double>(nt2::abs(a0[i]))),
                      (uint64_t)nt2::Valmax<T>());
  }
  NT2_COVER_ULP_EQUAL(nt2::tag::factorial_, ((T, a0)), ref, ulpd);

}
