//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/sdk/memory/composite_reference.hpp>
#include <nt2/sdk/meta/hierarchy_of.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

#include <complex>
#include <nt2/sdk/complex/complex.hpp>

//==============================================================================
// Test for composite_reference hierarchy
//==============================================================================
NT2_TEST_CASE( composite_hierarchy )
{
  using boost::dispatch::meta::proxy_;
  using boost::dispatch::meta::unspecified_;
  using nt2::meta::hierarchy_of;
  using nt2::container::composite_reference;

  NT2_TEST_TYPE_IS
  ( hierarchy_of< composite_reference< std::complex<double> > >::type
  , proxy_< composite_reference<std::complex<double> > >
  );

  NT2_TEST_TYPE_IS
  ( hierarchy_of< composite_reference< std::complex<double> > >::type::parent
  , unspecified_< composite_reference<std::complex<double> > >
  );
}

//==============================================================================
// Test for const composite_reference
//==============================================================================
NT2_TEST_CASE( composite_const_reference )
{
  using nt2::container::composite_reference;
  typedef std::complex<float> complex;
  typedef boost::fusion::vector2<float, float> complex_rw;

  complex_rw f(4,7.3f);
  composite_reference<complex const> cref_f(f);

  NT2_TEST_EQUAL( boost::fusion::at_c<0>(cref_f), boost::fusion::at_c<0>(f) );
  NT2_TEST_EQUAL( boost::fusion::at_c<1>(cref_f), boost::fusion::at_c<1>(f) );

  std::cout << cref_f << "\n";

  boost::fusion::at_c<0>(f) = -6.87f;
  boost::fusion::at_c<1>(f) = 0.98f;

  NT2_TEST_EQUAL( -6.87f, boost::fusion::at_c<0>(f) );
  NT2_TEST_EQUAL( 0.98f, boost::fusion::at_c<1>(f) );

  std::cout << cref_f << "\n";
}

//==============================================================================
// Test for non-const composite_reference
//==============================================================================
NT2_TEST_CASE( composite_reference )
{
  using nt2::container::composite_reference;
  typedef std::complex<float> complex;
  typedef boost::fusion::vector2<float, float> complex_rw;

  complex_rw f(4,7.3f);
  composite_reference<complex> ref_f(f);

  NT2_TEST_EQUAL( boost::fusion::at_c<0>(ref_f), boost::fusion::at_c<0>(f) );
  NT2_TEST_EQUAL( boost::fusion::at_c<1>(ref_f), boost::fusion::at_c<1>(f) );

  std::cout << ref_f << "\n";

  boost::fusion::at_c<0>(ref_f) = 9.87f;
  boost::fusion::at_c<1>(ref_f) = 1.234f;

  NT2_TEST_EQUAL( 9.87f, boost::fusion::at_c<0>(f) );
  NT2_TEST_EQUAL( 1.234f, boost::fusion::at_c<1>(f) );

  std::cout << ref_f << "\n";
}
