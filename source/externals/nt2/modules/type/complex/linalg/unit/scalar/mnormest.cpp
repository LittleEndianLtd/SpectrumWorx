
//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/mnormest.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/include/functions/sqrt.hpp>
#include <nt2/include/functions/pow.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/ten.hpp>
#include <nt2/include/constants/inf.hpp>
#include <nt2/include/constants/minf.hpp>
#include <nt2/include/constants/sqrt_2.hpp>

#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/sdk/complex/meta/as_complex.hpp>

NT2_TEST_CASE_TPL(mnormest, NT2_REAL_TYPES)
{
  using nt2::mnormest;
  using nt2::tag::mnormest_;
  typedef typename nt2::meta::as_complex<T>::type  cT;

  nt2::table<cT> n = cT(1, 1)*nt2::ones(10, 10, nt2::meta::as_<T>());
  NT2_TEST_ULP_EQUAL(mnormest(n, T(1.0e-6)), nt2::Ten<T>()*nt2::Sqrt_2<T>(), 1.5);
}
