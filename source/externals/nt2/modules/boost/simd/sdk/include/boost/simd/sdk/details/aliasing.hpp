//==============================================================================
//         Copyright 2003 & onward LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 & onward LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef BOOST_SIMD_SDK_DETAILS_ALIASING_HPP_INCLUDED
#define BOOST_SIMD_SDK_DETAILS_ALIASING_HPP_INCLUDED

#include <boost/config.hpp>

/// \note Consider also defaulting to no strict aliasing for Clang (it "wants"
/// to have TBAA but it seems it still isn't enabled or implemented as code that
/// heavily breaks the strict rules compiles w/o warnings and works as expected)
/// and ICC (it has TBAA but defaults to off).
///                                           (02.11.2012.) (Domagoj Saric)
#if defined( BOOST_MSVC ) && !defined( BOOST_SIMD_NO_STRICT_ALIASING )
    #define BOOST_SIMD_NO_STRICT_ALIASING
#endif

#ifdef BOOST_SIMD_NO_STRICT_ALIASING

#define BOOST_SIMD_MAY_ALIAS

namespace boost { namespace simd { namespace meta
{
    template <class T> struct may_alias { typedef T type; };
} } }

#else // BOOST_SIMD_NO_STRICT_ALIASING

#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_class.hpp>

#ifdef __GNUC__
    #define BOOST_SIMD_MAY_ALIAS __attribute__((may_alias))
#else
    #define BOOST_SIMD_MAY_ALIAS
#endif


namespace boost { namespace simd { namespace meta
{
  template<class T, class Enable = void>
  struct may_alias
  {
    typedef T BOOST_SIMD_MAY_ALIAS type;
  };

  template<class T>
  struct may_alias<T, typename boost::enable_if< boost::is_class<T> >::type>
  {
    typedef T type;
  };
} } }

#endif // BOOST_SIMD_NO_STRICT_ALIASING

#endif
