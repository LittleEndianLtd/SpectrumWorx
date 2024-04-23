////////////////////////////////////////////////////////////////////////////////
///
/// \file flags.inl
/// ---------------
///
/// Copyright (c) Domagoj Saric 2010 - 2014.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "flags.hpp"

#include "../../../detail/windows.hpp"

#include "boost/static_assert.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_STATIC_ASSERT( shared_memory_flags<win32>::system_hints::default_                   == SEC_COMMIT  );
BOOST_STATIC_ASSERT( shared_memory_flags<win32>::system_hints::only_reserve_address_space == SEC_RESERVE );


BOOST_IMPL_INLINE
shared_memory_flags<win32> shared_memory_flags<win32>::create
(
    flags_t                  const combined_handle_access_flags,
    share_mode  ::value_type const share_mode,
    system_hints::value_type const system_hint
)
{
    file_mapping_flags<win32> flags( file_mapping_flags<win32>::create( combined_handle_access_flags, share_mode ) );
    flags.create_mapping_flags |= system_hint;
    return static_cast<shared_memory_flags<win32> const &>( flags );
}

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------
