//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 polynom toolbox - db2mag/scalar Mode"

//////////////////////////////////////////////////////////////////////////////
// unit test behavior of polynom components in scalar mode
//////////////////////////////////////////////////////////////////////////////
/// created  by jt the 06/03/2011
///
#include <nt2/signal/include/functions/db2mag.hpp>
#include <nt2/include/functions/exp10.hpp>
#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>

NT2_TEST_CASE_TPL ( db2mag_real__1_0,  NT2_REAL_TYPES)
{
  using nt2::db2mag;
  using nt2::exp10;
  using nt2::tag::db2mag_;

  NT2_TEST_ULP_EQUAL(exp10(T(0.5/20.0)), db2mag(T(0.5)), 0.5);
} // end of test for floating_


