//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/predicates/include/functions/is_negative.hpp>
#include <vector>
#include <nt2/include/constants/valmin.hpp>
#include <nt2/include/constants/valmax.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/functions/bitofsign.hpp>
#include <nt2/include/functions/bitwise_or.hpp>

#include <nt2/sdk/unit/tests/cover.hpp>
#include <nt2/sdk/unit/module.hpp>

NT2_TEST_CASE_TPL ( is_negative,  NT2_REAL_TYPES)
{

  using nt2::is_negative;
  using nt2::tag::is_negative_;
  typedef typename nt2::meta::call<is_negative_(T)>::type r_t;

  nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  std::vector<T> in1(NR), in2(NR);
  nt2::roll(in1, nt2::Valmin<T>()/2, nt2::Valmax<T>()/2);
  std::vector<r_t>  ref(NR);
  for(nt2::uint32_t i=0; i < NR ; ++i)
  {
    ref[i] = nt2::bitwise_or(nt2::bitofsign(in1[i]), nt2::One<T>()) < 0;
  }

  NT2_COVER_ULP_EQUAL(is_negative_, ((T, in1)), ref, 0);
}
NT2_TEST_CASE_TPL ( is_negative2,  NT2_INTEGRAL_TYPES)
{

  using nt2::is_negative;
  using nt2::tag::is_negative_;
  typedef typename nt2::meta::call<is_negative_(T)>::type r_t;

  nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  std::vector<T> in1(NR), in2(NR);
  nt2::roll(in1, nt2::Valmin<T>()/2, nt2::Valmax<T>()/2);
  std::vector<r_t>  ref(NR);
  for(nt2::uint32_t i=0; i < NR ; ++i)
  {
    ref[i] = in1[i] < 0;
  }

  NT2_COVER_ULP_EQUAL(is_negative_, ((T, in1)), ref, 0);
}
