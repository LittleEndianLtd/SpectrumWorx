//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2014   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2014   NumScale SAS
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/size.hpp>
#include <nt2/include/functions/cat.hpp>
#include <nt2/include/functions/rif.hpp>
#include <nt2/include/functions/cif.hpp>
#include <nt2/include/functions/ones.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>

NT2_TEST_CASE_TPL( cat_scalar, (float)(double) )
{
  {
    nt2::table<T> d = nt2::cat(1, T(3), T(5));
    NT2_TEST_EQUAL(d(1,1),T(3));
    NT2_TEST_EQUAL(d(2,1),T(5));
  }
  {
    nt2::table<T> d = nt2::cat(2, T(3), T(5));
    NT2_TEST_EQUAL(d(1,1),T(3));
    NT2_TEST_EQUAL(d(1,2),T(5));
  }
  {
    nt2::table<T> d = nt2::cat(3, T(3), T(5));
    NT2_TEST_EQUAL(d(1,1,1),T(3));
    NT2_TEST_EQUAL(d(1,1,2),T(5));
  }
  {
    nt2::table<T> d = nt2::cat(4, T(3), T(5));
    NT2_TEST_EQUAL(d(1,1,1,1),T(3));
    NT2_TEST_EQUAL(d(1,1,1,2),T(5));
  }
}

NT2_TEST_CASE_TPL( cat, (float)(double) )
{
  {
    nt2::table<T,nt2::_2D> a = nt2::rif(nt2::of_size(2, 3), nt2::meta::as_<T>());
    nt2::table<T,nt2::_2D> b = nt2::cif(nt2::of_size(4, 3), nt2::meta::as_<T>());
    nt2::table<T,nt2::_2D> d = cat(1, a, b);
    NT2_TEST_EQUAL(a, d(nt2::_(1u, size(a, 1)),nt2::_ ));
    NT2_TEST_EQUAL(b, d(nt2::_(size(a, 1)+1, nt2::end_),nt2::_));
  }

  {
    nt2::table<T> a = nt2::rif(nt2::of_size(3, 2), nt2::meta::as_<T>());
    nt2::table<T> b = nt2::cif(nt2::of_size(3, 4), nt2::meta::as_<T>());
    nt2::table<T> d = cat(2, a, b);
    NT2_TEST_EQUAL(a, d(nt2::_, nt2::_(1u, size(a, 2))));
    NT2_TEST_EQUAL(b, d(nt2::_, nt2::_(size(a, 2)+1, nt2::end_)));
  }

  {
    nt2::table<T> a = nt2::rif(nt2::of_size(4, 3), nt2::meta::as_<T>());
    nt2::table<T> b = nt2::cif(nt2::of_size(4, 3), nt2::meta::as_<T>());
    nt2::table<T> d = cat(3, a, b);
    NT2_TEST_EQUAL(a, d(nt2::_,nt2::_,1));
    NT2_TEST_EQUAL(b, d(nt2::_,nt2::_,2));
  }

  {
    nt2::table<T> a = nt2::rif(nt2::of_size(4, 3), nt2::meta::as_<T>());
    nt2::table<T> b = nt2::cif(nt2::of_size(4, 3), nt2::meta::as_<T>());
    nt2::table<T> d = cat(4, a, b);
    NT2_TEST_EQUAL(a, d(nt2::_,nt2::_,nt2::_,1));
    NT2_TEST_EQUAL(b, d(nt2::_,nt2::_,nt2::_,2));
  }
}

NT2_TEST_CASE( cat_empty )
{
  nt2::table<double> c0;
  nt2::table<double> c1 = nt2::ones(3, 3);

  NT2_TEST_EQUAL(nt2::cat(1,c0, c0), c0);
  NT2_TEST_EQUAL(nt2::cat(1,c0, c1), c1);
  NT2_TEST_EQUAL(nt2::cat(1,c1, c0), c1);
  NT2_TEST_EQUAL(nt2::cat(1,c0, 1.), 1.);
  NT2_TEST_EQUAL(nt2::cat(1,1., c0), 1.);

  NT2_TEST_EQUAL(nt2::cat(2,c0, c0), c0);
  NT2_TEST_EQUAL(nt2::cat(2,c0, c1), c1);
  NT2_TEST_EQUAL(nt2::cat(2,c1, c0), c1);
  NT2_TEST_EQUAL(nt2::cat(2,c0, 1.), 1.);
  NT2_TEST_EQUAL(nt2::cat(2,1., c0), 1.);

  NT2_TEST_EQUAL(nt2::cat(3,c0, c0), c0);
  NT2_TEST_EQUAL(nt2::cat(3,c0, c1), c1);
  NT2_TEST_EQUAL(nt2::cat(3,c1, c0), c1);
  NT2_TEST_EQUAL(nt2::cat(3,c0, 1.), 1.);
  NT2_TEST_EQUAL(nt2::cat(3,1., c0), 1.);

  NT2_TEST_EQUAL(nt2::cat(4,c0, c0), c0);
  NT2_TEST_EQUAL(nt2::cat(4,c0, c1), c1);
  NT2_TEST_EQUAL(nt2::cat(4,c1, c0), c1);
  NT2_TEST_EQUAL(nt2::cat(4,c0, 1.), 1.);
  NT2_TEST_EQUAL(nt2::cat(4,1., c0), 1.);
}
