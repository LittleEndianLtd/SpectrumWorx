////////////////////////////////////////////////////////////////////////////////
///
/// \file referenceCounter.hpp
/// --------------------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef referenceCounter_hpp__61C56D58_836D_4ECC_B55D_84DD64DBAD0D
#define referenceCounter_hpp__61C56D58_836D_4ECC_B55D_84DD64DBAD0D
#pragma once
//------------------------------------------------------------------------------
#include "platformSpecifics.hpp"

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/core/ignore_unused.hpp>

#if defined( _MSC_VER ) || defined( _LIBCPP_VERSION ) || !defined( BOOST_NO_CXX11_HDR_ATOMIC )
#include <atomic>
#else
#ifndef BOOST_ATOMIC_NO_LIB
    #define BOOST_ATOMIC_NO_LIB
#endif // BOOST_ATOMIC_NO_LIB
#include <boost/atomic/atomic.hpp>
#endif // BOOST_NO_CXX11_HDR_ATOMIC
#include <cstdint>
#include <limits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

namespace Detail
{
#if defined( _MSC_VER ) || defined( _LIBCPP_VERSION ) || !defined( BOOST_NO_CXX11_HDR_ATOMIC )
    using counter_t = std  ::atomic_uint_fast8_t;
#else
    using counter_t = boost::atomic_uint_fast8_t;
#endif // BOOST_NO_CXX11_HDR_ATOMIC
    static_assert( sizeof( counter_t ) == sizeof( char ), "" );
} // namespace Detail

// https://channel9.msdn.com/Shows/Going+Deep/Cpp-and-Beyond-2012-Herb-Sutter-atomic-Weapons-2-of-2 @ ~1:20:00
struct ReferenceCount : Detail::counter_t
{
    using value_type = std::uint8_t;
    ReferenceCount( std::uint8_t const initialValue = 0 ) : Detail::counter_t( initialValue ) {}
#ifndef _MSC_VER
    using Detail::counter_t::counter_t;
#endif // !_MSC_VER

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4553 ) // '==' : operator has no effect; did you intend '='?
#endif // _MSC_VER
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    /*std::uint8_t*/void LE_FASTCALL operator++()
    {
        LE_ASSUME( (*this) >= 0                                       );
        LE_ASSUME( (*this) < std::numeric_limits<std::uint8_t>::max() );
        /*return*/ fetch_add( 1, std::memory_order_relaxed ) /*+ 1*/;
    }
    std::uint8_t LE_FASTCALL operator--()
    {
        auto const result( fetch_sub( 1, std::memory_order_acq_rel ) - 1 );
        LE_ASSUME     ( result >= 0 );
        BOOST_UNLIKELY( result == 0 );
        return static_cast<std::uint8_t>( result );
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

    void verifyCountEqual( value_type const value )
    {
        BOOST_ASSERT( this->load( std::memory_order_seq_cst ) == value );
        boost::ignore_unused( value );
    }
}; // class ReferenceCount

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // referenceCounter_hpp
