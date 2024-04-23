//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/ieee/include/functions/signnz.hpp>
#include <vector>

#include <nt2/sdk/unit/tests/cover.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/include/constants/valmax.hpp>
#include <nt2/include/constants/valmin.hpp>
#include <nt2/include/constants/half.hpp>
#include <nt2/include/functions/frexp.hpp>
#include <nt2/include/functions/abs.hpp>

NT2_TEST_CASE_TPL ( signnz,  NT2_TYPES)
{

  using nt2::signnz;
  using nt2::tag::signnz_;
  typedef typename nt2::meta::as_integer<T>::type iT;
  nt2::uint32_t NR = NT2_NB_RANDOM_TEST;
  std::vector<T> in1(NR);
  T Mi = nt2::Valmin<T>()/2;
  T Ma = nt2::Valmax<T>()/2;
  nt2::roll(in1, T(Mi), T(Ma));
  std::vector<iT> ref(NR);
  for(nt2::uint32_t i=0; i < NR ; ++i)
  {
    ref[i] = ( (in1[i] > 0) ? 1 : (in1[i] ==  0) ? 1 : -1); //oddity to avoid warnings if the type is unsigned
  }
  NT2_COVER_ULP_EQUAL(signnz_, ((T, in1)), ref, 0);
}

