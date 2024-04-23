//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/U.B.P.
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ. Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef BOOST_SIMD_ARITHMETIC_FUNCTIONS_SQRS_HPP_INCLUDED
#define BOOST_SIMD_ARITHMETIC_FUNCTIONS_SQRS_HPP_INCLUDED
#include <boost/simd/include/functor.hpp>
#include <boost/dispatch/include/functor.hpp>

namespace boost { namespace simd { namespace tag
  {
   /*!
      @brief  sqrs generic tag

      Represents the sqrs function in generic contexts.

      @par Models:
      Hierarchy
    **/
    struct sqrs_ : ext::elementwise_<sqrs_>
    {
      /// @brief Parent hierarchy
      typedef ext::elementwise_<sqrs_> parent;
      template<class... Args>
      static BOOST_FORCEINLINE BOOST_AUTO_DECLTYPE dispatch(Args&&... args)
      BOOST_AUTO_DECLTYPE_BODY( dispatching_sqrs_( ext::adl_helper(), static_cast<Args&&>(args)... ) )
    };
  }
  namespace ext
  {
   template<class Site>
   BOOST_FORCEINLINE generic_dispatcher<tag::sqrs_, Site> dispatching_sqrs_(adl_helper, boost::dispatch::meta::unknown_<Site>, ...)
   {
     return generic_dispatcher<tag::sqrs_, Site>();
   }
   template<class... Args>
   struct impl_sqrs_;
  }
  /*!
    Computes the saturated square of its parameter.

    @par semantic:
    For any given value @c x of type @c T:

    @code
    T r = sqrs(x);
    @endcode

    is similar to:

    @code
    T r = x*x > Valmax ? Valmax : x*x;
    @endcode

    @par Alias

    @c sqrs, saturated_sqr

    @param  a0

    @return      a value of the same type as the input.

  **/
  BOOST_DISPATCH_FUNCTION_IMPLEMENTATION(tag::sqrs_, sqrs, 1)
  BOOST_DISPATCH_FUNCTION_IMPLEMENTATION(tag::sqrs_, saturated_sqr, 1)
} }

#endif


