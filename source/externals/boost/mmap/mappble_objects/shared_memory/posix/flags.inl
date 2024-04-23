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
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

BOOST_IMPL_INLINE
shared_memory_flags<posix> shared_memory_flags<posix>::create
(
    flags_t                  const combined_handle_access_flags,
    share_mode  ::value_type const share_mode,
    system_hints::value_type const system_hint
)
{
    file_mapping_flags<posix> flags( file_mapping_flags<posix>::create( combined_handle_access_flags, share_mode ) );
    flags.flags |= system_hint;
    return static_cast<shared_memory_flags<posix> const &>( flags );
}

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------
