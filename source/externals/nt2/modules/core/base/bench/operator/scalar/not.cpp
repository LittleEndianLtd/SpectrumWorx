//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/include/functor.hpp>
#include <nt2/include/functions/logical_not.hpp>
#include <nt2/sdk/bench/benchmark.hpp>

////////////////////////////////////////////////////////////////////////////////
// Runtime benchmark for functor<logical_not_>
////////////////////////////////////////////////////////////////////////////////
using nt2::tag::logical_not_;
using namespace nt2;

////////////////////////////////////////////////////////////////////////////////
// Symmetric range
////////////////////////////////////////////////////////////////////////////////
#define RS(T,V) ((T, -V , V))

////////////////////////////////////////////////////////////////////////////////
// Scalar benchmark
////////////////////////////////////////////////////////////////////////////////
NT2_TIMING( logical_not_ , RS(double,2000000))
NT2_TIMING( logical_not_ , RS(float ,2000000))
NT2_TIMING( logical_not_ , RS(nt2::int64_t,2000000))
NT2_TIMING( logical_not_ , RS(nt2::int32_t,2000000))
NT2_TIMING( logical_not_ , RS(nt2::int16_t,32768))
NT2_TIMING( logical_not_ , RS(nt2::int8_t,127))
NT2_TIMING( logical_not_ , ((nt2::uint64_t, 0, ~0ULL)) )
NT2_TIMING( logical_not_ , ((nt2::uint32_t, 0, ~0U)) )
NT2_TIMING( logical_not_ , ((nt2::uint16_t, 0, 65535)) )
NT2_TIMING( logical_not_ , ((nt2::uint8_t , 0, 255)) )

