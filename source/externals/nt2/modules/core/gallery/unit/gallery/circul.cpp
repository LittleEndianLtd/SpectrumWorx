//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 gallery toolbox - circul function"

#include <nt2/table.hpp>
#include <nt2/include/functions/circul.hpp>
#include <nt2/include/functions/transpose.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/include/functions/cons.hpp>
#include <nt2/include/functions/cast.hpp>
#include <nt2/include/constants/i.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>

NT2_TEST_CASE_TPL ( circul, NT2_REAL_TYPES)
{
  nt2::table<T> a = nt2::_(T(1), T(3));
  nt2::table<T> t3 =nt2::trans(nt2::reshape(nt2::cons(
                                              T(1), T(2), T(3),
                                              T(3), T(1), T(2),
                                              T(2), T(3), T(1)),
                                            nt2::of_size(3, 3)));
  nt2::table<T> v =  nt2::circul(3, nt2::meta::as_<T>());
  nt2::table<T> v1=  nt2::circul(a);
  nt2::table<int> v2 = nt2::circul(3);
  nt2::table<T>   v3 = nt2::circul<T>(3);
  NT2_TEST_EQUAL(v, t3);
  NT2_TEST_EQUAL(v1, t3);
  NT2_TEST_EQUAL(v2, nt2::cast<int>(t3));
  NT2_TEST_EQUAL(v3, t3);
}



