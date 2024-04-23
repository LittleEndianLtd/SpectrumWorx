//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::globalvar function"

#include <nt2/table.hpp>
#include <nt2/include/functions/globalvar.hpp>
#include <nt2/include/functions/var.hpp>
#include <nt2/include/functions/reshape.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>

NT2_TEST_CASE_TPL( globalvar, NT2_REAL_TYPES )
{
  using nt2::_;
  using nt2::var;
  nt2::table<T> a = nt2::reshape(nt2::_(T(1), T(9)), 3, 3);
  NT2_TEST_ULP_EQUAL( nt2::globalvar(a), var(a(_))(1), 0.5 );
}
