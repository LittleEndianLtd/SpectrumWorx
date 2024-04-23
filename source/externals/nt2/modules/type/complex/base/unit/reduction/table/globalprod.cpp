//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2::globalprod function"

#include <nt2/table.hpp>
#include <nt2/include/functions/globalprod.hpp>
#include <nt2/include/functions/complexify.hpp>

#include <nt2/include/functions/zeros.hpp>
#include <nt2/include/constants/true.hpp>
#include <nt2/include/constants/false.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>
#include <nt2/table.hpp>

NT2_TEST_CASE_TPL( globalprod, NT2_REAL_TYPES )
{
  typedef std::complex<T> cT;
  nt2::table<cT> a = nt2::reshape(nt2::complexify(nt2::_(T(1), T(4))), 2, 2);
  NT2_TEST_EQUAL( nt2::globalprod(a), cT(24));
  NT2_TEST_EQUAL( nt2::globalprod(cT(1)), cT(1));
  a(2, 2) = cT(0);
  NT2_TEST_EQUAL( nt2::globalprod(a), cT(0));
  NT2_TEST_EQUAL( nt2::globalprod(cT(0)), cT(0));
}

