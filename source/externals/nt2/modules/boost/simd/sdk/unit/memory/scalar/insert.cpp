//==============================================================================
//         Copyright 2003 - 2012 LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012 LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/include/functions/insert.hpp>
#include <boost/dispatch/functor/meta/call.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include "../common/foo.hpp"


NT2_TEST_CASE_TPL( insert, BOOST_SIMD_TYPES)
{
  using boost::simd::insert;
  using boost::simd::tag::insert_;

  typedef typename boost::dispatch::meta::call<insert_(T, T&, int)>::type rT;

  NT2_TEST_TYPE_IS( rT, void );

  T data;
  T value = T(42);

  insert(value, data, 0);

  NT2_TEST_EQUAL( data, value );
}

NT2_TEST_CASE( insert_foo )
{
  using boost::simd::insert;
  using boost::simd::tag::insert_;

  typedef boost::dispatch::meta::call<insert_(foo, foo&, int)>::type rT;

  NT2_TEST_TYPE_IS( rT, void );

  foo data;
  foo value;
  value.d = 1;
  value.f = 2.f;
  value.c = 3;

  insert(value, data, 0);

  NT2_TEST_EQUAL( data, value );
}

NT2_TEST_CASE_TPL( insert_logical, BOOST_SIMD_TYPES)
{
  using boost::simd::logical;
  using boost::simd::insert;
  using boost::simd::tag::insert_;

  typedef typename boost::dispatch::meta
                        ::call<insert_(bool, logical<T>&, int)>::type rT;

  NT2_TEST_TYPE_IS( rT, void );

  logical<T> data;
  logical<T> value(true);

  insert(value,data, 0);

  NT2_TEST_EQUAL( data, value );
}
