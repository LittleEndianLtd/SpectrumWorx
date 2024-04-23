//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/arithmetic/include/functions/two_prod.hpp>
#include <boost/simd/sdk/simd/native.hpp>
#include <boost/simd/sdk/simd/io.hpp>
#include <boost/simd/include/constants/inf.hpp>
#include <boost/simd/include/constants/eps.hpp>
#include <boost/simd/include/constants/one.hpp>
#include <boost/simd/include/constants/zero.hpp>
#include <boost/simd/include/functions/unary_minus.hpp>
#include <boost/simd/include/functions/plus.hpp>
#include <boost/simd/include/functions/minus.hpp>
#include <boost/simd/include/functions/multiplies.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <boost/fusion/include/vector_tie.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

NT2_TEST_CASE_TPL( two_prod, BOOST_SIMD_SIMD_REAL_TYPES)
{
  using boost::simd::two_prod;
  using boost::simd::tag::two_prod_;
  using boost::simd::native;

  typedef native<T,BOOST_SIMD_DEFAULT_EXTENSION>   vT;

  NT2_TEST_TYPE_IS( (typename boost::dispatch::meta::call<two_prod_(vT,vT)>::type)
                  , (std::pair<vT,vT>)
                  );

  vT inf_    = boost::simd::Inf<vT>();
  vT zero_   = boost::simd::Zero<vT>();
  vT one_    = boost::simd::One<vT>();
  vT eps_    = boost::simd::Eps<vT>();
  vT meps2_   = -eps_*eps_;

  {
    vT s,r;

    two_prod(inf_,one_, s, r);
    NT2_TEST_EQUAL(s, inf_);
    NT2_TEST_EQUAL(r, zero_);

    two_prod(one_, inf_, s, r);
    NT2_TEST_EQUAL(s, inf_);
    NT2_TEST_EQUAL(r, zero_);

    two_prod(one_ + eps_, one_ - eps_, s, r);
    NT2_TEST_EQUAL(s, one_);
    NT2_TEST_EQUAL(r, meps2_);

    two_prod(one_ - eps_,one_ + eps_, s, r);
    NT2_TEST_EQUAL(s, one_);
    NT2_TEST_EQUAL(r, meps2_);
  }

  {
    vT s,r;

    s = two_prod(inf_,one_, r);
    NT2_TEST_EQUAL(s, inf_);
    NT2_TEST_EQUAL(r, zero_);

    s = two_prod(one_, inf_, r);
    NT2_TEST_EQUAL(s, inf_);
    NT2_TEST_EQUAL(r, zero_);

    s = two_prod(one_ + eps_, one_ - eps_, r);
    NT2_TEST_EQUAL(s, one_);
    NT2_TEST_EQUAL(r, meps2_);

    s = two_prod(one_ - eps_,one_ + eps_, r);
    NT2_TEST_EQUAL(s, one_);
    NT2_TEST_EQUAL(r, meps2_);
  }

  {
    vT s,r;

    boost::fusion::vector_tie(s,r) = two_prod(inf_,one_);
    NT2_TEST_EQUAL(s, inf_);
    NT2_TEST_EQUAL(r, zero_);

    boost::fusion::vector_tie(s,r) = two_prod(one_, inf_);
    NT2_TEST_EQUAL(s, inf_);
    NT2_TEST_EQUAL(r, zero_);

    boost::fusion::vector_tie(s,r) = two_prod(one_ + eps_, one_ - eps_);
    NT2_TEST_EQUAL(s, one_);
    NT2_TEST_EQUAL(r, meps2_);

    boost::fusion::vector_tie(s,r) = two_prod(one_ - eps_,one_ + eps_);
    NT2_TEST_EQUAL(s, one_);
    NT2_TEST_EQUAL(r, meps2_);
  }

  {
    std::pair<vT,vT> p;

    p = two_prod(inf_,one_);
    NT2_TEST_EQUAL(p.first, inf_);
    NT2_TEST_EQUAL(p.second, zero_);

    p = two_prod(one_, inf_);
    NT2_TEST_EQUAL(p.first, inf_);
    NT2_TEST_EQUAL(p.second, zero_);

    p = two_prod(one_ + eps_, one_ - eps_);
    NT2_TEST_EQUAL(p.first, one_);
    NT2_TEST_EQUAL(p.second, meps2_);

    p = two_prod(one_ - eps_,one_ + eps_);
    NT2_TEST_EQUAL(p.first, one_);
    NT2_TEST_EQUAL(p.second, meps2_);
  }
}
