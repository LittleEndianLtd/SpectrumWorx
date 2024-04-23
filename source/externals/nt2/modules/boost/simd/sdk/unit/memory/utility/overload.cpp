//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/preprocessor/new.hpp>
#include <boost/simd/memory/is_aligned.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

class foo
{
  public:
  BOOST_SIMD_MEMORY_OVERLOAD_NEW_DELETE(16)
  int  member;
};

////////////////////////////////////////////////////////////////////////////////
// Test foo allocation using the aligned new/delete
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE(overload_new_delete)
{
  using boost::simd::is_aligned;

  foo* ptr = new foo;

  NT2_TEST( is_aligned(ptr,16) );
  ptr->member = 42;
  NT2_TEST_EQUAL( ptr->member, 42 );
  delete ptr;
}

////////////////////////////////////////////////////////////////////////////////
// Test foo allocation using the aligned new/delete
////////////////////////////////////////////////////////////////////////////////
NT2_TEST_CASE(overload_new_delete_array)
{
  using boost::simd::is_aligned;

  foo* ptr = new foo[5];

  NT2_TEST( is_aligned(ptr,16) );

  for(int i=0;i<5;++i) ptr[i].member = 10*i;
  for(int i=0;i<5;++i) NT2_TEST_EQUAL( ptr[i].member, 10*i );

  delete[] ptr;
}

