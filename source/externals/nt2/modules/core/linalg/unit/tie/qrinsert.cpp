//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 linalg toolbox - tied qrinsert function"

#include <nt2/table.hpp>
#include <nt2/include/functions/zeros.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/include/functions/eye.hpp>
#include <nt2/include/functions/qrinsert.hpp>
#include <nt2/include/functions/qr.hpp>
#include <nt2/include/functions/tie.hpp>
#include <nt2/include/functions/mtimes.hpp>
#include <nt2/include/functions/transpose.hpp>
#include <nt2/include/functions/rowvect.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

NT2_TEST_CASE_TPL ( qrinsert, NT2_REAL_TYPES)
{
  typedef nt2::table<T> table_t;
  table_t b = nt2::ones(4, 4, nt2::meta::as_<T>())
                + T(15)*nt2::eye(4, 4, nt2::meta::as_<T>());
  table_t q, r, p;
  q = b;
  r = b;
  p =  b;

  nt2::tie(q, r, p) = nt2::qr(b);
  NT2_DISPLAY(q);
  NT2_DISPLAY(r);
  NT2_DISPLAY(p);

  table_t zz = nt2::mtimes(nt2::trans(p), nt2::mtimes(q, r));
  NT2_TEST_ULP_EQUAL(zz, b, T(20.0));
  table_t q1(nt2::of_size(4, 4)), r1(nt2::of_size(4, 5)), x(nt2::ones(4, 1, nt2::meta::as_<T>()));
  nt2::tie(q1, r1) = nt2::qrinsert(q, r, 2, x);
  NT2_DISPLAY(q1);
  NT2_DISPLAY(r1);
  NT2_DISPLAY( nt2::mtimes(q1, r1));
  NT2_TEST_ULP_EQUAL(nt2::mtimes(q1, r1),  nt2::cath(nt2::cath(b(nt2::_, 1),x), b(nt2::_, nt2::_(2, 4))), T(20.0));

  nt2::tie(q1, r1) = nt2::qrinsert(q, r, 2, nt2::rowvect(x), 'r');
  NT2_DISPLAY(q1);
  NT2_DISPLAY(r1);
  NT2_DISPLAY( nt2::mtimes(q1, r1));
  NT2_TEST_ULP_EQUAL(nt2::mtimes(q1, r1),  nt2::catv(nt2::catv(b(1, nt2::_),nt2::rowvect(x)), b(nt2::_(2, 4), nt2::_)), T(20.0));


}

