//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================

#include <nt2/table.hpp>
#include <nt2/include/functions/isfloating.hpp>
#include <nt2/include/functions/ones.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>

NT2_TEST_CASE( fundamental_isfloating )
{
  NT2_TEST( !nt2::isfloating('e') );
  NT2_TEST( !nt2::isfloating(1)   );
  NT2_TEST( nt2::isfloating(1.)  );
  NT2_TEST( nt2::isfloating(1.f) );
}

NT2_TEST_CASE( container_isfloating )
{
  NT2_TEST( !nt2::isfloating( nt2::ones(4, nt2::meta::as_<char > ()))       );
  NT2_TEST( !nt2::isfloating( nt2::ones(4,1, nt2::meta::as_<int > ()))     );
  NT2_TEST( nt2::isfloating( nt2::ones(4,1,1, nt2::meta::as_<float > ()))   );

}
