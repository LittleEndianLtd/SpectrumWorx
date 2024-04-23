//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2013   Domagoj Saric, Little Endian Ltd.
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef INTERLEAVED_DATA_TRANSFORMATION_HPP_INCLUDED
#define INTERLEAVED_DATA_TRANSFORMATION_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <nt2/signal/details/missing_functionality.hpp> //...mrmlj...to be moved elsewhere...

#include <boost/simd/include/functions/simd/deinterleave_first.hpp>
#include <boost/simd/include/functions/simd/deinterleave_second.hpp>
#include <boost/simd/include/functions/simd/interleave_first.hpp>
#include <boost/simd/include/functions/simd/interleave_second.hpp>

#include <boost/dispatch/attributes.hpp>

#include <boost/assert.hpp>

#include <cstddef>
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace simd
{
//------------------------------------------------------------------------------
namespace details
{
//------------------------------------------------------------------------------

template <typename Vector>
static void interleave_two_channels
(
    Vector const * BOOST_DISPATCH_RESTRICT p_channel0,
    Vector const * BOOST_DISPATCH_RESTRICT p_channel1,
    Vector       * BOOST_DISPATCH_RESTRICT p_interleaved,
    std::size_t                            size
)
{
    BOOST_ASSERT_MSG( size % 2 == 0, "Two way interleaved data must have an even number of elements." );
    size /= 2;
    while ( size-- )
    {
        Vector const channel0( *p_channel0++ );
        Vector const channel1( *p_channel1++ );

        Vector * BOOST_DISPATCH_RESTRICT const p_data0( p_interleaved++ );
        Vector * BOOST_DISPATCH_RESTRICT const p_data1( p_interleaved++ );

        typename Vector::native_type const pair0( boost::simd::interleave_first ( channel0, channel1 ) );
        typename Vector::native_type const pair1( boost::simd::interleave_second( channel0, channel1 ) );

        *p_data0 = pair0;
        *p_data1 = pair1;
    }
}

template <typename Vector>
static void deinterleave_two_channels
(
    Vector const * BOOST_DISPATCH_RESTRICT p_interleaved,
    Vector       * BOOST_DISPATCH_RESTRICT p_channel0,
    Vector       * BOOST_DISPATCH_RESTRICT p_channel1,
    std::size_t                            size
)
{
    BOOST_ASSERT_MSG( size % 2 == 0, "Two way interleaved data must have an even number of elements." );
    size /= 2;
    while ( size-- )
    {
        Vector const pair0( *p_interleaved++ );
        Vector const pair1( *p_interleaved++ );

        *p_channel0++ = boost::simd::deinterleave_first ( pair0, pair1 );
        *p_channel1++ = boost::simd::deinterleave_second( pair0, pair1 );
    }
}

//------------------------------------------------------------------------------
} // namespace details
//------------------------------------------------------------------------------
} // namespace simd
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#endif // interleaved_data_transformation_hpp
