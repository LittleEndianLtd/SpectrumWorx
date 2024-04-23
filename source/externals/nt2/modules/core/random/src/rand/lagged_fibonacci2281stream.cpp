//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/core/utility/config.hpp>
#include <nt2/core/utility/details/rands.hpp>
#include <nt2/core/utility/lagged_fibonacci2281stream.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/normal_distribution.hpp>

namespace nt2
{
  NT2_IMPLEMENT_RAND( lagged_fibonacci2281stream_ )

  NT2_CORE_RANDOM_DECL nt2::randstream_* lagged_fibonacci2281stream()
  {
    static nt2::lagged_fibonacci2281stream_ instance;
    return &instance;
  }
}
