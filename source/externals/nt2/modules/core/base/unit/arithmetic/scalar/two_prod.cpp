//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/arithmetic/include/functions/two_prod.hpp>
#include <nt2/include/constants/inf.hpp>
#include <nt2/include/constants/eps.hpp>
#include <nt2/include/constants/one.hpp>
#include <nt2/include/constants/zero.hpp>
#include <nt2/include/functions/unary_minus.hpp>
#include <boost/dispatch/functor/meta/call.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <boost/fusion/include/vector_tie.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

NT2_TEST_CASE_TPL( two_prod, NT2_REAL_TYPES)
{
  using nt2::two_prod;
  using nt2::tag::two_prod_;

  NT2_TEST_TYPE_IS( (typename boost::dispatch::meta::call<two_prod_(T,T)>::type)
                  , (std::pair<T,T>)
                  );

  T inf_    = nt2::Inf<T>();
  T zero_   = nt2::Zero<T>();
  T one_    = nt2::One<T>();
  T eps_    = nt2::Eps<T>();
  T meps2_   = -eps_*eps_;

  {
    T s,r;

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
    T s,r;

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
    T s,r;

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
    std::pair<T,T> p;

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
