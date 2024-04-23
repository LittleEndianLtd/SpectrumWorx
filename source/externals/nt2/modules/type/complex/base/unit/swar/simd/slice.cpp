//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/slice.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <nt2/include/functions/splat.hpp>
#include <nt2/include/functions/enumerate.hpp>
#include <nt2/include/constants/valmin.hpp>
#include <nt2/include/constants/valmax.hpp>
#include <nt2/sdk/complex/complex.hpp>

#include <boost/dispatch/functor/meta/call.hpp>
#include <boost/fusion/include/vector_tie.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

template<typename T>
struct big_enough : boost::mpl::bool_< (BOOST_SIMD_BYTES/sizeof(T) > 2) >
{};

NT2_TEST_CASE_TPL_MPL ( slice
                      , NT2_TEST_SEQ_MPL_FILTER ( NT2_SIMD_REAL_TYPES
                                                , big_enough<_>
                                                )
                      )
{
  using boost::simd::slice;
  using boost::simd::tag::slice_;
  using boost::simd::native;
  using boost::simd::splat;
  using boost::simd::enumerate;
  using boost::simd::meta::vector_of;

  typedef BOOST_SIMD_DEFAULT_EXTENSION                    ext_t;
  typedef std::complex<T>                                 cT;
  typedef native<cT,ext_t>                                wT;
  typedef native<T,ext_t>                                 uT;
  typedef typename vector_of<cT, wT::static_size/2>::type vT;

  NT2_TEST_TYPE_IS( (typename boost::dispatch::meta::call<slice_(wT)>::type)
                  , (std::pair<vT,vT>)
                  );

  vT ref0, ref1;
  std::size_t i=0;

  for(; i!=vT::static_size; ++i)
    ref0[i] = cT(i,i);

  for(; i!=vT::static_size*2; ++i)
    ref1[i-vT::static_size] = cT(i,i);

  wT base(enumerate<uT>(0),enumerate<uT>(0));

  {
    vT out0, out1;
    slice(base, out0, out1);
    NT2_TEST_EQUAL(out0, ref0);
    NT2_TEST_EQUAL(out1, ref1);
  }

  {
    vT out0, out1;
    out0 = slice(base, out1);
    NT2_TEST_EQUAL(out0, ref0);
    NT2_TEST_EQUAL(out1, ref1);
  }

  {
    vT out0, out1;
    boost::fusion::vector_tie(out0,out1) = slice(base);
    NT2_TEST_EQUAL(out0, ref0);
    NT2_TEST_EQUAL(out1, ref1);
  }

  {
    std::pair<vT,vT> p;
    p = slice(base);
    NT2_TEST_EQUAL(p.first, ref0);
    NT2_TEST_EQUAL(p.second, ref1);
  }

  {
    vT f,s;

    slice(boost::simd::Valmax<wT>(), f, s);
    NT2_TEST_EQUAL(f, boost::simd::Valmax<vT>() );
    NT2_TEST_EQUAL(s, boost::simd::Valmax<vT>() );

    slice(boost::simd::Valmin<wT>(), f, s);
    NT2_TEST_EQUAL(f, boost::simd::Valmin<vT>() );
    NT2_TEST_EQUAL(s, boost::simd::Valmin<vT>() );
  }
}
