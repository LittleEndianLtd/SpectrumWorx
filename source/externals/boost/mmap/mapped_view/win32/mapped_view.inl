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
#include "../mapped_view.hpp"

#include "../../detail/windows.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <>
struct detail::mapper<char, win32>
{
    static mapped_view_reference<char, win32> map
    (
        mapping<win32>  const & source_mapping,
        boost::uint64_t         offset        ,
        std  ::size_t           desired_size
    )
    {
        // Implementation note:
        // Mapped views hold internal references to the following handles so we do
        // not need to hold/store them ourselves:
        // http://msdn.microsoft.com/en-us/library/aa366537(VS.85).aspx
        //                                        (26.03.2010.) (Domagoj Saric)

        typedef mapped_view_reference<char, win32>::iterator iterator;

        ULARGE_INTEGER large_integer;
        large_integer.QuadPart = offset;

        iterator const view_start
        (
            static_cast<iterator>
            (
                ::MapViewOfFile
                (
                    source_mapping.get(),
                    source_mapping.view_mapping_flags,
                    large_integer.HighPart,
                    large_integer.LowPart,
                    desired_size
                )
            )
        );

        return mapped_view_reference<char>
        (
            view_start,
            view_start
                ? view_start + desired_size
                : view_start
        );
    }

    static void unmap( mapped_view_reference<char, win32> const & view )
    {
        BOOST_VERIFY( ::UnmapViewOfFile( view.begin() ) || view.empty() );
    }
};

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------
