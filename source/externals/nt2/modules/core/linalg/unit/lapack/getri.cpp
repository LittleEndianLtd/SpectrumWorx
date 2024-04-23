//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functions/getri.hpp>
#include <nt2/include/functions/getrf.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/include/functions/eye.hpp>
#include <nt2/include/functions/triu.hpp>
#include <nt2/include/functions/tri1l.hpp>
#include <nt2/include/functions/colon.hpp>
#include <nt2/include/functions/width.hpp>
#include <nt2/include/functions/height.hpp>
#include <nt2/include/functions/mtimes.hpp>
#include <nt2/include/functions/cons.hpp>

#include <nt2/table.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/exceptions.hpp>

NT2_TEST_CASE_TPL(tri, NT2_REAL_TYPES )
{
  using nt2::_;

  typedef nt2::table<T>           t_t;
  typedef nt2::table<nt2_la_int>  i_t;

  t_t in  = nt2::ones(4, 4, nt2::meta::as_<T>())
          + T(10)*nt2::eye(4, 4, nt2::meta::as_<T>());
  t_t ref(in);
  i_t pv;

  nt2::getrf(boost::proto::value(in),boost::proto::value(pv));

  nt2_la_int p = nt2::getri(boost::proto::value(in),boost::proto::value(pv));

  NT2_TEST_ULP_EQUAL( (nt2::mtimes(in, ref))
                    , nt2::eye  (4, 4, nt2::meta::as_<T>())
                    , 0.75
                    );

  NT2_TEST_EQUAL(p, 0);
}

NT2_TEST_CASE_TPL(tric, NT2_REAL_TYPES )
{
  using nt2::_;

  typedef std::complex<T> cT;
  typedef nt2::table<cT>           t_t;
  typedef nt2::table<nt2_la_int>  i_t;

  t_t in  = nt2::ones(4, 4, nt2::meta::as_<cT>())
          + T(10)*nt2::eye(4, 4, nt2::meta::as_<cT>());

  t_t test = nt2::zeros(4, 4, nt2::meta::as_<cT>())
          +nt2::eye(4, 4, nt2::meta::as_<cT>());

  t_t ref(in);
  i_t pv;

  nt2::getrf(boost::proto::value(in),boost::proto::value(pv));

  nt2_la_int p = nt2::getri(boost::proto::value(in),boost::proto::value(pv));


  NT2_TEST_ULP_EQUAL( (nt2::mtimes(in, ref))
                    , test
                    , 0.75
                    );

  NT2_TEST_EQUAL(p, 0);
}
