//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/include/functions/deinterleave_first.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <boost/simd/sdk/meta/cardinal_of.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL(deinterleave_first, BOOST_SIMD_SIMD_TYPES)
{
  using boost::simd::native;
  using boost::simd::meta::cardinal_of;

  typedef BOOST_SIMD_DEFAULT_EXTENSION      ext_t;
  typedef native<T,ext_t>                      vT;

  const std::size_t card = cardinal_of<vT>::value;
  vT a,b,c,ref;

  for(std::size_t i=1; i<=card; ++i)
  {
    a[i-1]=T(i);
    b[i-1]=T(i*10);
  }

  for(std::size_t i=0; i<card; ++i)
    ref[i] = i<(card/2) ? a[i*2] : b[(i-card/2)*2];

  c = boost::simd::deinterleave_first(a,b);

  NT2_TEST_EQUAL(c,ref);
}
