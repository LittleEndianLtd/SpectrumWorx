//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2014   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2011 - 2014   MetaScale SAS
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/arecrosscompatible.hpp>
#include <nt2/include/functions/ones.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>

NT2_TEST_CASE( container_arecrosscompatible )
{
  NT2_TEST( !nt2::arecrosscompatible( nt2::ones(0),      nt2::ones(0)       , 1 ));
  NT2_TEST( !nt2::arecrosscompatible( nt2::ones(1,0),    nt2::ones(1,0)     , 1 ));
  NT2_TEST( !nt2::arecrosscompatible( nt2::ones(2,0,2),  nt2::ones(2,0,2)   , 1 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(3,3,0,1),nt2::ones(3,3,0,1) , 1));
  NT2_TEST( !nt2::arecrosscompatible( nt2::ones(3,3,0,1),nt2::ones(3,3,0,1) , 3));

  NT2_TEST( !nt2::arecrosscompatible( nt2::ones(1),      nt2::ones(1)       , 1 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(1,3),    nt2::ones(1,3)     , 2 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(1,3,1),  nt2::ones(1,3,1)   , 2 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(1,3,1,1),nt2::ones(1,3,1,1) , 2 ));

  NT2_TEST( nt2::arecrosscompatible( nt2::ones(3),     nt2::ones(3)       , 1 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(2,3),   nt2::ones(2,3)     , 2 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(1,3,2), nt2::ones(1,3,2)   , 2 ));
  NT2_TEST( nt2::arecrosscompatible( nt2::ones(1,3,1,9), nt2::ones(1,3,1,9), 2 ));
  NT2_TEST( !nt2::arecrosscompatible( nt2::ones(1,3,1,9), nt2::ones(1,3,2,9), 2 ));
}
