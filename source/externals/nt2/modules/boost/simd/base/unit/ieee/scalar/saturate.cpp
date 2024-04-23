//==============================================================================
//         Copyright 2003 - 2013   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <boost/simd/ieee/include/functions/saturate.hpp>
#include <boost/simd/sdk/simd/logical.hpp>

#include <boost/dispatch/functor/meta/call.hpp>
#include <nt2/sdk/unit/tests/ulp.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>
#include <nt2/sdk/unit/module.hpp>
#include <boost/simd/sdk/config.hpp>

#include <boost/simd/include/constants/valmax.hpp>
#include <boost/simd/include/constants/valmin.hpp>

NT2_TEST_CASE_TPL ( saturate_unsigned_int16,  (int16_t))
{
  using boost::simd::saturate;
  using boost::simd::tag::saturate_;
  using boost::simd::Valmax;
  using boost::simd::Valmin;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(T, wished_r_t);

  // specific values tests
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ull        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ull        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ull        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ull       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ull      ), 0);
  NT2_TEST_ULP_EQUAL(720,         saturate<T>(720ull      ), 0);
  NT2_TEST_ULP_EQUAL(5040,        saturate<T>(5040ull     ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(40320ull    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ull   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ull  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ull ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ull), 0);
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ll        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ll        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ll        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ll       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ll      ), 0);
  NT2_TEST_ULP_EQUAL(720,         saturate<T>(720ll      ), 0);
  NT2_TEST_ULP_EQUAL(5040,        saturate<T>(5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ll), 0);
  NT2_TEST_ULP_EQUAL(-1,           saturate<T>(-1ll        ), 0);
  NT2_TEST_ULP_EQUAL(-2,           saturate<T>(-2ll        ), 0);
  NT2_TEST_ULP_EQUAL(-6,           saturate<T>(-6ll        ), 0);
  NT2_TEST_ULP_EQUAL(-24,          saturate<T>(-24ll       ), 0);
  NT2_TEST_ULP_EQUAL(-120,         saturate<T>(-120ll      ), 0);
  NT2_TEST_ULP_EQUAL(-720,         saturate<T>(-720ll      ), 0);
  NT2_TEST_ULP_EQUAL(-5040,        saturate<T>(-5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-479001600ll), 0);
}

NT2_TEST_CASE_TPL ( saturate_unsigned_uint16,  (uint16_t))
{
  using boost::simd::saturate;
  using boost::simd::tag::saturate_;
  using boost::simd::Valmax;
  using boost::simd::Valmin;
  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(T, wished_r_t);

  // specific values tests
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ull        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ull        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ull        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ull       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ull      ), 0);
  NT2_TEST_ULP_EQUAL(720,         saturate<T>(720ull      ), 0);
  NT2_TEST_ULP_EQUAL(5040,        saturate<T>(5040ull     ), 0);
  NT2_TEST_ULP_EQUAL(40320,       saturate<T>(40320ull    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ull   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ull  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ull ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ull), 0);
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ll        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ll        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ll        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ll       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ll      ), 0);
  NT2_TEST_ULP_EQUAL(720,         saturate<T>(720ll      ), 0);
  NT2_TEST_ULP_EQUAL(5040,        saturate<T>(5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(40320,       saturate<T>(40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ll), 0);
}

NT2_TEST_CASE_TPL ( saturate_int8,  (int8_t))
{
  using boost::simd::saturate;
  using boost::simd::tag::saturate_;
  using boost::simd::Valmax;
  using boost::simd::Valmin;

  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(T, wished_r_t);

  // specific values tests
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ull        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ull        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ull        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ull       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ull      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(720ull      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(5040ull     ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(40320ull    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ull   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ull  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ull ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ull), 0);
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ll        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ll        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ll        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ll       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ll      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(720ll      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ll), 0);
  NT2_TEST_ULP_EQUAL(-1,           saturate<T>(-1ll        ), 0);
  NT2_TEST_ULP_EQUAL(-2,           saturate<T>(-2ll        ), 0);
  NT2_TEST_ULP_EQUAL(-6,           saturate<T>(-6ll        ), 0);
  NT2_TEST_ULP_EQUAL(-24,          saturate<T>(-24ll       ), 0);
  NT2_TEST_ULP_EQUAL(-120,         saturate<T>(-120ll      ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-720ll      ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(Valmin<T>(),  saturate<T>(-479001600ll), 0);
}

NT2_TEST_CASE_TPL ( saturate_unsigned_uint8,  (uint8_t))
{
  using boost::simd::saturate;
  using boost::simd::tag::saturate_;
  using boost::simd::Valmax;
  using boost::simd::Valmin;

  typedef T wished_r_t;

  // return type conformity test
  NT2_TEST_TYPE_IS(T, wished_r_t);

  // specific values tests
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ull        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ull        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ull        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ull       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ull      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(720ull      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(5040ull     ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(40320ull    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ull   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ull  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ull ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ull), 0);
  NT2_TEST_ULP_EQUAL(1,           saturate<T>(1ll        ), 0);
  NT2_TEST_ULP_EQUAL(2,           saturate<T>(2ll        ), 0);
  NT2_TEST_ULP_EQUAL(6,           saturate<T>(6ll        ), 0);
  NT2_TEST_ULP_EQUAL(24,          saturate<T>(24ll       ), 0);
  NT2_TEST_ULP_EQUAL(120,         saturate<T>(120ll      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(720ll      ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(Valmax<T>(), saturate<T>(479001600ll), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-1ll        ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-2ll        ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-6ll        ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-24ll       ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-120ll      ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-720ll      ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-5040ll     ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-40320ll    ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-362880ll   ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-3628800ll  ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-39916800ll ), 0);
  NT2_TEST_ULP_EQUAL(0, saturate<T>(-479001600ll), 0);
}
