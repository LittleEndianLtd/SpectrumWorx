//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 linalg toolbox - null space"

#include <nt2/table.hpp>
#include <nt2/include/functions/null.hpp>
#include <nt2/include/functions/eye.hpp>
#include <nt2/include/functions/zeros.hpp>
#include <nt2/include/functions/norm.hpp>
#include <nt2/include/functions/mtimes.hpp>
#include <nt2/include/constants/eps.hpp>
#include <nt2/include/constants/i.hpp>
#include <nt2/sdk/complex/meta/as_complex.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL(null, NT2_REAL_TYPES)
{
  typedef typename nt2::meta::as_complex<T>::type  cT;
  using nt2::null;
  using nt2::tag::null_;
  nt2::table<cT> n = nt2::eye(10, 10, nt2::meta::as_<T>());
  n(3, 5) = cT(2);
  n(4, 4) = cT(0);
  n(1, 1) = 5*nt2::Eps<T>();
  nt2::table<cT> nulln = nt2::null(n);
  nt2::table<cT> nulln1 = nt2::null(n,  T(0.1)*nt2::Eps<T>());

  nt2::table<cT> rn1 = nt2::zeros(10, 1, nt2::meta::as_<T>());
  rn1(4) = T(-1);
  nt2::table<cT> rn = nt2::zeros(10, 2, nt2::meta::as_<T>());
  rn(4, 1) = cT(-1);
  rn(1, 2) = cT(1);
  NT2_TEST_ULP_EQUAL(rn, nulln, 3.5);
  NT2_TEST_ULP_EQUAL(rn1, nulln1, 3.5);
}

NT2_TEST_CASE_TPL(null2, NT2_REAL_TYPES)
{
  typedef typename nt2::meta::as_complex<T>::type  cT;
  using nt2::null;
  using nt2::tag::null_;

  nt2::table<cT> n = nt2::reshape(nt2::_(T(1), T(16)), 4, 4)+
  nt2::I<cT>()*nt2::reshape(nt2::_(T(1), T(16)), 4, 4);
  nt2::table<cT> nulln = nt2::null(n);
  NT2_TEST_LESSER_EQUAL(nt2::norm(nt2::mtimes(nt2::trans(nt2::conj(nulln)), nulln)-nt2::eye(2, nt2::meta::as_<cT>())), T(10)*nt2::Eps<T>());
}
