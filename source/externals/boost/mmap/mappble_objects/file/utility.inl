////////////////////////////////////////////////////////////////////////////////
///
/// \file mapped_view.inl
/// ---------------------
///
/// Copyright (c) Domagoj Saric 2010.-2013.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "utility.hpp"

#include "file.hpp"

#include "../../detail/impl_inline.hpp"
#include "../../detail/impl_selection.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

namespace detail
{
    typedef file_open_flags<BOOST_MMAP_IMPL()> open_flags;
    typedef file_handle    <BOOST_MMAP_IMPL()> default_file_handle; 

    BOOST_IMPL_INLINE
    open_flags create_rw_file_flags()
    {
        return open_flags::create
               (
                   open_flags::handle_access_rights  ::read | open_flags::handle_access_rights::write,
                   open_flags::open_policy           ::open_or_create,
                   open_flags::system_hints          ::sequential_access,
                   open_flags::on_construction_rights::read | open_flags::on_construction_rights::write
               );
    }

    BOOST_IMPL_INLINE
    open_flags create_r_file_flags()
    {
        return open_flags::create_for_opening_existing_files
               (
                   open_flags::handle_access_rights::read,
                   false,
                   open_flags::system_hints        ::sequential_access
               );
    }

    BOOST_IMPL_INLINE
    basic_mapped_view_ref map_file( default_file_handle::reference const file_handle, std::size_t desired_size )
    {
        if ( desired_size )
            set_size( file_handle, desired_size );
        else
            desired_size = get_size( file_handle );

        typedef file_mapping_flags<BOOST_MMAP_IMPL()> mapping_flags;
        return basic_mapped_view_ref::map
        (
            create_mapping
            (
                file_handle,
                mapping_flags::create
                (
                    mapping_flags::handle_access_rights::read | mapping_flags::handle_access_rights::write,
                    mapping_flags::share_mode          ::shared
                )
            ),
            0,
            desired_size
        );
    }


    BOOST_IMPL_INLINE
    basic_mapped_read_only_view_ref map_read_only_file( default_file_handle::reference const file_handle )
    {
        typedef file_mapping_flags<BOOST_MMAP_IMPL()> mapping_flags;
        return basic_mapped_read_only_view_ref::map
        (
            create_mapping
            (
                file_handle,
                mapping_flags::create
                (
                    mapping_flags::handle_access_rights::read,
                    mapping_flags::share_mode          ::shared
                )
            ),
            0,
            get_size( file_handle )
        );
    }
} // namespace detail

BOOST_IMPL_INLINE
basic_mapped_view_ref map_file( char const * const file_name, std::size_t const desired_size )
{
    return detail::map_file( create_file( file_name, detail::create_rw_file_flags() ), desired_size );
}

BOOST_IMPL_INLINE
basic_mapped_read_only_view_ref map_read_only_file( char const * const file_name )
{
    return detail::map_read_only_file( create_file( file_name, detail::create_r_file_flags() ) );
}

#ifdef _WIN32
    BOOST_IMPL_INLINE
    basic_mapped_view_ref map_file( wchar_t const * const file_name, std::size_t const desired_size )
    {
        return detail::map_file( create_file( file_name, detail::create_rw_file_flags() ), desired_size );
    }

    BOOST_IMPL_INLINE
    basic_mapped_read_only_view_ref map_read_only_file( wchar_t const * const file_name )
    {
        return detail::map_read_only_file( create_file( file_name, detail::create_r_file_flags() ) );
    }
#endif // _WIN32

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------

#ifndef BOOST_MMAP_HEADER_ONLY
    #include "file.inl"
#endif // BOOST_MMAP_HEADER_ONLY
