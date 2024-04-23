//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/memory/direct_output_iterator.hpp>
#include <boost/simd/include/functions/splat.hpp>
#include <boost/simd/memory/allocator.hpp>
#include <boost/simd/sdk/simd/pack.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <vector>

NT2_TEST_CASE_TPL(types, BOOST_SIMD_TYPES)
{
  using std::iterator_traits;
  using boost::simd::direct_output_iterator;
  using boost::simd::pack;

  NT2_TEST_TYPE_IS( typename iterator_traits< direct_output_iterator<T*> >::value_type
                  , (pack<T>)
                  );
}

NT2_TEST_CASE_TPL(distance, BOOST_SIMD_TYPES)
{
  using std::iterator_traits;
  using boost::simd::direct_output_iterator;
  using boost::simd::direct_output_begin;
  using boost::simd::direct_output_end;
  using boost::simd::pack;
  using boost::simd::allocator;

  std::vector<T,allocator<T> > data(pack<T>::static_size*3);
  typedef typename std::vector<T,allocator<T> >::iterator it_t;

  direct_output_iterator<it_t> b = direct_output_begin(data.begin());
  direct_output_iterator<it_t> e = direct_output_end(data.end());

  NT2_TEST_EQUAL( std::distance(b,e), 3);
}

NT2_TEST_CASE_TPL(iteration, BOOST_SIMD_TYPES)
{
  using std::iterator_traits;
  using boost::simd::direct_output_iterator;
  using boost::simd::direct_output_begin;
  using boost::simd::direct_output_end;
  using boost::simd::pack;
  using boost::simd::allocator;
  using boost::simd::splat;

  std::vector<T,allocator<T> > ref(pack<T>::static_size*3);
  std::vector<T,allocator<T> > data(pack<T>::static_size*3);

  typedef typename std::vector<T,allocator<T> >::iterator it_t;

  for(std::size_t i=0;i<ref.size();i++)
    ref[i] = (i/pack<T>::static_size)+1;

  direct_output_iterator<it_t> b = direct_output_begin(data.begin());
  direct_output_iterator<it_t> e = direct_output_end(data.end());

  int i=0;
  while(b != e)
    *b++ = splat< pack<T> >(++i);

  NT2_TEST_EQUAL( data, ref );
}
