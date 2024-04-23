//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 linalg toolbox - expm"

#include <nt2/table.hpp>
#include <nt2/include/functions/expm.hpp>
#include <nt2/include/functions/eye.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/ten.hpp>
#include <nt2/include/constants/exp_1.hpp>

#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

#include <nt2/sdk/complex/meta/as_complex.hpp>
#include <nt2/include/constants/i.hpp>

 NT2_TEST_CASE_TPL(expm, NT2_REAL_TYPES)
{
  typedef typename nt2::meta::as_complex<T>::type  cT;
  using nt2::expm;
  using nt2::tag::expm_;
  std::cout << std::setprecision(20);
  nt2::table<cT> n(nt2::of_size(2, 2));
  n(1, 1) = n(1, 2) = n(2, 2) = cT(1);
  n(2, 1) = cT(0);
  nt2::table<cT> r = n*nt2::Exp_1<T>();
  nt2::table<cT> expmn = nt2::expm(n);
  NT2_TEST_ULP_EQUAL(expmn, r, 3);
}


