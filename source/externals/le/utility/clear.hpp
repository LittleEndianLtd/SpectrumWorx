////////////////////////////////////////////////////////////////////////////////
///
/// \file clear.hpp
/// ---------------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef clear_hpp__834DF3B1_52B1_4110_899E_A926DA81EF95
#define clear_hpp__834DF3B1_52B1_4110_899E_A926DA81EF95
#pragma once
//------------------------------------------------------------------------------
#include "platformSpecifics.hpp"

#include <cstring>
#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

namespace Detail
{
    template <unsigned int size>
    void LE_FASTCALL clear( void * LE_RESTRICT const pPODObject ) { std::memset( pPODObject, 0, size ); }
} // namespace Detail

template <typename POD>
void LE_FASTCALL clear( POD & pod )
{
    // https://svn.boost.org/trac/boost/ticket/5635
    // http://boost.2283326.n4.nabble.com/TypeTraits-A-patch-for-clang-s-intrinsics-was-type-traits-is-enum-on-scoped-enums-doesn-t-works-as-e-td3781550.html
    static_assert( std::is_pod<POD>::value || /*...OSX...*//*__is_pod*/__has_trivial_assign( POD ), "Will not memset a non POD." );
    Detail::clear<sizeof( pod )>( &pod );
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // clear_hpp
