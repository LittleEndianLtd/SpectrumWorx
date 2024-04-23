//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifdef _MSC_VER
#pragma warning(disable: 4996) // unsafe std::transform
#endif

#include <algorithm>
#include <boost/array.hpp>
#include <boost/fusion/adapted/array.hpp>
#include <boost/fusion/include/vector_tie.hpp>
#include <boost/fusion/include/make_vector.hpp>
#include <nt2/sdk/memory/buffer.hpp>
#include <nt2/sdk/memory/is_safe.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>
#include "memory_io.hpp"

//==============================================================================
// Test for default buffer ctor
//==============================================================================
NT2_TEST_CASE_TPL( buffer_default_ctor, NT2_TYPES)
{
  using nt2::memory::buffer;

  buffer<T> b;

  NT2_TEST(b.empty());
  NT2_TEST_EQUAL(b.size()     , 0u      );
  NT2_TEST_EQUAL(b.capacity() , 0u      );
  NT2_TEST_EQUAL(b.begin()    , b.end() );
}

//==============================================================================
// Test for size buffer ctor
//==============================================================================
NT2_TEST_CASE_TPL( buffer_size_ctor, NT2_TYPES)
{
  using nt2::memory::buffer;

  buffer<T> b(5);

  NT2_TEST(!b.empty());
  NT2_TEST_EQUAL(b.size()     , 5u    );
  NT2_TEST_EQUAL(b.capacity() , 5u    );
  NT2_TEST_EQUAL(b.data()      , &b[0] );

  for ( std::ptrdiff_t i = 0; i < 5; ++i ) b[i] = T(3+i);
  for ( std::ptrdiff_t i = 0; i < 5; ++i ) NT2_TEST_EQUAL( b[i], T(3+i) );
}

//==============================================================================
// Test for dynamic buffer copy ctor
//==============================================================================
NT2_TEST_CASE_TPL( buffer_data_ctor, NT2_TYPES)
{
  using nt2::memory::buffer;

  buffer<T> b(5);

  for ( std::ptrdiff_t i = 0; i < 5; ++i ) b[i] = T(3+i);

  buffer<T>  x(b);

  NT2_TEST(!x.empty());
  NT2_TEST_EQUAL(x.size()     , 5u    );
  NT2_TEST_EQUAL(x.capacity() , 5u    );
  NT2_TEST_EQUAL(x.data()      , &x[0] );

  NT2_TEST_EQUAL( x, b );
}

//==============================================================================
// Test for buffer resize
//==============================================================================
NT2_TEST_CASE_TPL(buffer_resize, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> b(5);

  for ( std::ptrdiff_t i = 0; i < 5; ++i ) b[i] = T(3+i);

  b.resize(3);
  NT2_TEST(!b.empty());
  NT2_TEST_EQUAL(b.size()       , 3u        );
  NT2_TEST_GREATER_EQUAL(b.capacity() , b.size()  );
  NT2_TEST_EQUAL(b.data()        , &b[0]     );

  for ( std::ptrdiff_t i = 0; i < 3; ++i ) NT2_TEST_EQUAL( b[i], T(3+i) );

  b.resize(5);
  NT2_TEST(!b.empty());
  NT2_TEST_EQUAL(b.size()     , 5u    );
  NT2_TEST_GREATER_EQUAL(b.capacity() , b.size()  );
  NT2_TEST_EQUAL(b.data()      , &b[0] );

  for ( std::ptrdiff_t i = 0; i < 5; ++i ) NT2_TEST_EQUAL( b[i], T(3+i) );

  b.resize(9);
  NT2_TEST(!b.empty());
  NT2_TEST_EQUAL(b.size()     , 9u    );
  NT2_TEST_GREATER_EQUAL(b.capacity() , b.size()  );
  NT2_TEST_EQUAL(b.data()      , &b[0] );

  for ( std::ptrdiff_t i = 0; i < 9; ++i ) b[i] = T(4*(i+1));
  for ( std::ptrdiff_t i = 0; i < 9; ++i ) NT2_TEST_EQUAL( b[i], T(4*(i+1)) );

  b.resize(1);
  NT2_TEST(!b.empty());
  NT2_TEST_EQUAL(b.size()     , 1u    );
  NT2_TEST_GREATER_EQUAL(b.capacity() , b.size()  );
  NT2_TEST_EQUAL(b.data()      , &b[0] );

  for ( std::ptrdiff_t i = 0; i < 1; ++i ) NT2_TEST_EQUAL( b[i], T(4*(i+1)) );
}

//==============================================================================
// Test for buffer assignment
//==============================================================================
NT2_TEST_CASE_TPL(buffer_assignment, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> b(5);

  for ( std::ptrdiff_t i = 0; i < 5; ++i ) b[i] = T(3+i);

  buffer<T> x;
  x = b;

  NT2_TEST(!x.empty());
  NT2_TEST_EQUAL(x.size() , 5u  );
  NT2_TEST_EQUAL(x.data()  , &x[0] );
  NT2_TEST_GREATER_EQUAL(x.capacity(), x.size() );
  NT2_TEST_EQUAL( x, b );
}

//==============================================================================
// Test for buffer swap
//==============================================================================
NT2_TEST_CASE_TPL(buffer_swap, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> b(5);
  buffer<T> x(7);

  for ( std::ptrdiff_t i = 0; i < 5; ++i ) b[i] = T(3+i);
  for ( std::ptrdiff_t i = 0; i < 7; ++i ) x[i] = T(3*(i+1));

  swap(x,b);

  NT2_TEST(!x.empty());
  NT2_TEST_EQUAL(x.size()     , 5u    );
  NT2_TEST_EQUAL(x.capacity() , 5u    );
  NT2_TEST_EQUAL(x.data()      , &x[0] );

  NT2_TEST(!b.empty());
  NT2_TEST_EQUAL(b.size()     , 7u    );
  NT2_TEST_EQUAL(b.capacity() , 7u    );
  NT2_TEST_EQUAL(b.data()      , &b[0] );

  for ( std::ptrdiff_t i = 0; i < 7; ++i ) NT2_TEST_EQUAL( b[i], T(3*(i+1)) );
  for ( std::ptrdiff_t i = 0; i < 5; ++i ) NT2_TEST_EQUAL( x[i], T(3+i) );
}

//==============================================================================
// buffer Range interface
//==============================================================================
struct f_
{
  template<class T> T operator()(T const& e) const { return T(10*e); }
};

NT2_TEST_CASE_TPL(buffer_iterator, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> x(5);
  for ( std::ptrdiff_t i = 0; i < 5; ++i ) x[i] = T(3+i);

  f_ f;

  typename buffer<T>::iterator b = x.begin();
  typename buffer<T>::iterator e = x.end();

  std::transform(b,e,b,f);

  for ( std::ptrdiff_t i = 0; i < 5; ++i )  NT2_TEST_EQUAL( x[i], f(T(3+i)) );
}

//==============================================================================
// buffer push_back single value
//==============================================================================
NT2_TEST_CASE_TPL(buffer_push_back, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> x(5);
  for ( std::ptrdiff_t i = 0; i < 5; ++i ) x[i] = T(3+i);
  for ( std::ptrdiff_t i = 0; i < 7; ++i ) x.push_back(11+i);

  NT2_TEST_EQUAL( x.size(), 12u );
  std::ptrdiff_t i = 0;
  for ( ; i < 5;  ++i ) NT2_TEST_EQUAL( x[i], T(3+i) );
  for ( ; i < 12; ++i ) NT2_TEST_EQUAL( x[i], T(11+i-5) );

  std::cout << "capacity = " << x.capacity() << std::endl;
}

//==============================================================================
// buffer push_back range of values
//==============================================================================
NT2_TEST_CASE_TPL(buffer_append, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> x(5);
  buffer<T> y(7);
  for ( std::ptrdiff_t i = 0; i < 5; ++i ) x[i] = T(3+i);
  for ( std::ptrdiff_t i = 0; i < 7; ++i ) y[i] = T(2*i);

  x.append(y.begin(),y.end());

  NT2_TEST_EQUAL( x.size(), 12u );

  std::ptrdiff_t i = 0;
  for ( ; i < 5;  ++i ) NT2_TEST_EQUAL( x[i], T(3+i) );
  for ( ; i < 12; ++i ) NT2_TEST_EQUAL( x[i], T(2*(i-5)) );

  x.append(y.begin(),y.end());

  NT2_TEST_EQUAL( x.size(), 19u );
  for ( ; i < 19; ++i ) NT2_TEST_EQUAL( x[i], T(2*(i-12)) );

  std::cout << "capacity = " << x.capacity() << std::endl;
}

//==============================================================================
// buffer push_back values in empty buffer
//==============================================================================
NT2_TEST_CASE_TPL(buffer_append_def, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> x;
  for ( std::ptrdiff_t i = 0; i < 7; ++i ) x.push_back(11+i);

  NT2_TEST_EQUAL( x.size(), (std::size_t)7 );
  std::ptrdiff_t i = 0;
  for ( ; i < 7; ++i ) NT2_TEST_EQUAL( x[i], T(11+i) );

  std::cout << "capacity = " << x.capacity() << std::endl;
}

//==============================================================================
// buffer push_back range of values
//==============================================================================
NT2_TEST_CASE_TPL(buffer_push_backs_def, NT2_TYPES )
{
  using nt2::memory::buffer;

  buffer<T> x;
  buffer<T> y(7);
  for ( std::ptrdiff_t i = 0; i < 7; ++i ) y[i] = T(2*i);

  x.append(y.begin(),y.end());

  NT2_TEST_EQUAL( x.size(), 7u );

  std::ptrdiff_t i = 0;
  for ( ; i < 7; ++i ) NT2_TEST_EQUAL( x[i], T(2*i) );

  std::cout << "capacity = " << x.capacity() << std::endl;
}
