//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 polynom toolbox - db/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of polynom components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 06/03/2011
///
#include <nt2/signal/include/functions/db.hpp>
#include <nt2/include/functions/ulpdist.hpp>
#include <nt2/include/functions/isequal.hpp>
#include <boost/type_traits/is_same.hpp>
#include <nt2/sdk/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>

#include <nt2/include/constants/real.hpp>


NT2_TEST_CASE_TPL ( db_real__1_0,  NT2_REAL_TYPES)
{
  using nt2::db;
  using nt2::tag::db_;

  NT2_TEST_ULP_EQUAL(T(10)*log10(T(0.5)), db(T(1), T(2)), 0.5);
  NT2_TEST_ULP_EQUAL(T(10)*log10(T(0.5)), db(T(1), nt2::voltage_, T(2)), 0.5);
  NT2_TEST_ULP_EQUAL(T(10)*log10(T(0.25)), db(T(0.5)), 0.5);
  NT2_TEST_ULP_EQUAL(T(10)*log10(T(0.25)), db(T(0.5), nt2::voltage_), 0.5);
  NT2_TEST_ULP_EQUAL(T(10)*log10(T(0.5)), db(T(0.5), nt2::power_), 0.5);
}
