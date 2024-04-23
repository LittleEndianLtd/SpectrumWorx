////////////////////////////////////////////////////////////////////////////////
///
/// \file mapped_view.inl
/// ---------------------
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
#include "../mapped_view.hpp"

#include "../../detail/posix.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

namespace detail
{
    template <>
    struct mapper<char, posix>
    {
        static mapped_view_reference<char, posix> map
        (
            mapping<posix>  const & source_mapping,
            boost::uint64_t         offset        ,
            std  ::size_t           desired_size
        )
        {
            typedef mapped_view_reference<char, posix>::iterator iterator;

            iterator const view_start
            (
                static_cast<iterator>
                (
                    ::mmap
                    (
                        0,
                        desired_size,
                        source_mapping.view_mapping_flags.protection,
                        source_mapping.view_mapping_flags.flags,
                        source_mapping,
                        offset
                    )
                )
            );

            return mapped_view_reference<char>
            (
            /// \note Opt for a bit more complex check in order to better
            /// support user code that uses iterator_range::begin() (!= nullptr)
            /// instead of iterator_range::operator bool() to check for
            /// validity.
            ///                               (23.07.2014.) (Domagoj Saric)
            #if 1
                BOOST_LIKELY( view_start != MAP_FAILED ) ? view_start                : nullptr,
                BOOST_LIKELY( view_start != MAP_FAILED ) ? view_start + desired_size : nullptr
            #else // disabled
                view_start,
                BOOST_LIKELY( view_start != MAP_FAILED )
                    ? view_start + desired_size
                    : view_start
            #endif
            );
        }

        static void unmap( mapped_view_reference<char, posix> const & view )
        {
            BOOST_VERIFY
            (
                ( ::munmap( view.begin(), view.size() ) == 0 ) ||
                view.empty()
            );
        }
    };
} // namespace detail

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------
