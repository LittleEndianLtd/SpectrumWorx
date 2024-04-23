//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/table.hpp>
#include <nt2/include/functions/ishermitian.hpp>
#include <nt2/include/functions/ones.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <complex>

NT2_TEST_CASE( fundamental_ishermitian )
{
  NT2_TEST( nt2::ishermitian('e') );
  NT2_TEST( nt2::ishermitian(1)   );
  NT2_TEST( nt2::ishermitian(1.)  );
  NT2_TEST( nt2::ishermitian(1.f) );
  NT2_TEST( nt2::ishermitian(std::complex<float>(1, 0)));
  NT2_TEST( !nt2::ishermitian(std::complex<float>(1, 1)));
}


NT2_TEST_CASE( table_ishermitian )
{
  typedef std::complex<float>  type;
  nt2::table<type> a(nt2::of_size(3, 3));
  for(std::ptrdiff_t i=1; i <= 3; i++)
   {
     for(std::ptrdiff_t j=1; j <= 3; j++)
       {
         a(i, j) = (i < j) ? type(i, j) : ((i > j) ? type(j, -i) : type(i, 0));
       }
   }

  NT2_TEST( nt2::ishermitian(a)     );
  a(1, 2) = type(25.0f);
  NT2_TEST( !nt2::ishermitian(a)     );

}
