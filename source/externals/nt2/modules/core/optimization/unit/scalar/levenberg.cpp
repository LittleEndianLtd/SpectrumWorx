//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 optimize toolbox - levenberg"

#include <nt2/include/functions/levenberg.hpp>

#include <nt2/sdk/unit/tests.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/include/functions/sqr.hpp>
#include <nt2/include/functions/exp.hpp>
#include <nt2/include/functions/norm.hpp>
#include <nt2/include/functions/sqrt.hpp>
#include <nt2/include/functions/zeros.hpp>
#include <nt2/include/functions/globalsum.hpp>
#include <nt2/include/functions/globalmax.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/include/constants/half.hpp>
#include <nt2/include/constants/eps.hpp>
#include <nt2/include/constants/sqrteps.hpp>
#include <nt2/include/constants/four.hpp>
#include <nt2/table.hpp>

template < class Tabout >
struct fpp
{
  template < class Tabin> inline
  Tabout operator()(const Tabin & x ) const
  {
    typedef typename Tabin::value_type value_type;
    Tabout r = (nt2::sqr((x-nt2::_(value_type(1), value_type(numel(x))))));
    return r;
  }
};

template<class Tabout, class Tabin >  Tabout f1(const Tabin & x )
{
    typedef typename Tabin::value_type value_type;
    Tabout r = (nt2::sqr(x)-value_type(3));
    return r;
}

NT2_TEST_CASE_TPL( levenberg_function_ptr, NT2_REAL_TYPES )
{
  using nt2::levenberg;
  using nt2::optimization::output;
  typedef nt2::table<T> tab_t;
  typedef typename nt2::meta::as_logical<T>::type lT;
  typedef nt2::table<lT> ltab_t;
  tab_t x0 = nt2::ones(nt2::of_size(1, 3), nt2::meta::as_<T>());
  ltab_t h = nt2::is_nez(nt2::ones (nt2::of_size(1, 3), nt2::meta::as_<T>())*nt2::Half<T>());
  tab_t r = nt2::sqrt(T(3))*nt2::ones (nt2::of_size(1, 3), nt2::meta::as_<T>());
  output<tab_t,T> res = levenberg(&f1<tab_t, tab_t>, x0, h);
  NT2_TEST(res.successful);
  NT2_TEST_LESSER_EQUAL(nt2::globalmax(nt2::abs(res.minimum()-r)), nt2::Sqrteps<T>());
  NT2_TEST_LESSER_EQUAL(nt2::norm(res.covar), T(12.1));
}

NT2_TEST_CASE_TPL( levenberg_functor, NT2_REAL_TYPES )
{
  using nt2::levenberg;
  using nt2::options;
  using nt2::optimization::output;
  typedef nt2::table<T> tab_t;
  typedef typename nt2::meta::as_logical<T>::type lT;
  typedef nt2::table<lT> ltab_t;
  tab_t x0 = nt2::zeros(nt2::of_size(1, 3), nt2::meta::as_<T>());
  ltab_t h = nt2::is_nez(nt2::ones (nt2::of_size(1, 3), nt2::meta::as_<T>())*nt2::Half<T>());
  tab_t r = nt2::_(T(1), T(3));
  output<tab_t,T> res = levenberg(fpp<tab_t>(), x0, h,
                                  options [ nt2::iterations_ = 100,
                                            nt2::tolerance::absolute_ = nt2::Eps<T>()
                                    ]);
  NT2_TEST(res.successful);
  NT2_TEST_LESSER_EQUAL(nt2::globalmax(nt2::abs(res.minimum()-r)), nt2::Four<T>()*nt2::Sqrteps<T>());
  NT2_TEST_LESSER_EQUAL(nt2::norm(res.covar), T(10*nt2::numel(res.covar))*nt2::Eps<T>());
}

