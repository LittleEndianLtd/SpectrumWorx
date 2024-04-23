//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::globalall function"

#include <nt2/table.hpp>
#include <nt2/include/functions/globalall.hpp>
#include <nt2/include/functions/is_gtz.hpp>
#include <nt2/include/constants/true.hpp>
#include <nt2/include/constants/false.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>
#include <nt2/table.hpp>

NT2_TEST_CASE_TPL( globalall, NT2_TYPES )
{
  nt2::table<T> a = nt2::reshape(nt2::_(T(1), T(9)), 3, 3);
  NT2_TEST_EQUAL( nt2::globalall(a), true);
  NT2_TEST_EQUAL( nt2::globalall(T(1)), true);
  a(3, 3) = T(0);
  NT2_TEST_EQUAL( nt2::globalall(a), false);
  NT2_TEST_EQUAL( nt2::globalall(T(0)), false);
  NT2_TEST_EQUAL( nt2::globalall(nt2::is_gtz(T(0))), false);
}

