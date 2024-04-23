//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/moler.hpp>
#include <nt2/include/functions/cons.hpp>
#include <nt2/include/functions/transpose.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

NT2_TEST_CASE_TPL ( moler, NT2_REAL_TYPES)
{
  nt2::table<T> m3 =nt2::trans(nt2::cons(nt2::of_size(3, 3),
                                         T( 1),     T(-1),     T(-1),
                                         T(-1),     T( 2),     T( 0),
                                         T(-1),     T( 0),     T( 3)));

  NT2_TEST_ULP_EQUAL(nt2::moler(3,nt2::meta::as_<T>()), m3, 0.5);
  NT2_TEST_ULP_EQUAL(nt2::moler(3,T(-1)), m3, 0.5);
}

NT2_TEST_CASE( moler_default )
{
  typedef double T;

  nt2::table<T> m3 =nt2::trans(nt2::cons(nt2::of_size(3, 3),
                                         T( 1),     T(-1),     T(-1),
                                         T(-1),     T( 2),     T( 0),
                                         T(-1),     T( 0),     T( 3)));
  NT2_TEST_ULP_EQUAL(nt2::moler(3), m3, 0.5);
}




