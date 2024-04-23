//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/memory/include/functions/make.hpp>
#include <boost/simd/sdk/simd/native.hpp>

#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/module.hpp>

template<class T> T fibo(std::size_t n)
{
  if(n == 0) return 0;
  if(n == 1) return 1;
  return fibo<T>(n-1) + fibo<T>(n-2);
}

#define M1(z, n, t) fibo<T>(n)
#define M0(z, n, t)                                                            \
template<class T>                                                              \
struct make_fibo<T, n>                                                         \
{                                                                              \
  typedef boost::simd::native<T, BOOST_SIMD_DEFAULT_EXTENSION> type;           \
  static type call()                                                           \
  {                                                                            \
    return boost::simd::make<type>(BOOST_PP_ENUM(n, M1, ~));                   \
  }                                                                            \
};                                                                             \
/**/
template<class T, std::size_t N = boost::simd::native<T, BOOST_SIMD_DEFAULT_EXTENSION>::static_size>
struct make_fibo;
BOOST_SIMD_PP_REPEAT_POWER_OF_2(M0, ~)
#undef M0
#undef M1

NT2_TEST_CASE_TPL ( make_fibonnaci,  BOOST_SIMD_SIMD_TYPES)
{
  typedef boost::simd::native<T, BOOST_SIMD_DEFAULT_EXTENSION> vT;
  vT x = make_fibo<T>::call();
  for(std::size_t i=0; i!=boost::simd::meta::cardinal_of<vT>::value; ++i)
    NT2_TEST_EQUAL(x[i], fibo<T>(i));
}
