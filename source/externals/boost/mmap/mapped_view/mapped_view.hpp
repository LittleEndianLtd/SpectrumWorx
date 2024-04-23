////////////////////////////////////////////////////////////////////////////////
///
/// \file mapped_view.hpp
/// ---------------------
///
/// Copyright (c) Domagoj Saric 2010 - 2016.
///
///  Use, modification and distribution is subject to the
///  Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
/// (obsolete version, latest @ https://github.com/psiha/mmap
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef mapped_view_hpp__D9C84FF5_E506_4ECB_9778_61E036048D28
#define mapped_view_hpp__D9C84FF5_E506_4ECB_9778_61E036048D28
#pragma once
//------------------------------------------------------------------------------
#include "../detail/impl_selection.hpp"
#include "../handles/handle.hpp"
#include "../mapping/mapping.hpp"

#include "boost/assert.hpp"
#include "boost/cstdint.hpp"
#include "boost/noncopyable.hpp"
#include "boost/range/iterator_range_core.hpp"

#include <utility>
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

typedef iterator_range<char       *> basic_memory_range_t;
typedef iterator_range<char const *> basic_read_only_memory_range_t;

template <typename Element, typename Impl = BOOST_MMAP_IMPL()>
class mapped_view_reference;

typedef mapped_view_reference<char      > basic_mapped_view_ref;
typedef mapped_view_reference<char const> basic_mapped_read_only_view_ref;

namespace detail
{
    template <typename Element, typename Impl>
    struct mapper
    {
    public:
        static mapped_view_reference<Element, Impl> map
        (
            mapping<Impl>   const & source_mapping,
            boost::uint64_t         offset        ,
            std  ::size_t           desired_size
        )
        {
            return make_typed_view( mapper<char, Impl>::map( source_mapping, offset, desired_size ) );
        }

        static void unmap( mapped_view_reference<Element, Impl> const & view )
        {
            mapper<char, Impl>::unmap( make_basic_view( view ) );
        }

    private:
        static mapped_view_reference<char, Impl>
        #ifdef BOOST_MSVC
            const &
        #endif
        make_basic_view( mapped_view_reference<Element, Impl> const & );

        static mapped_view_reference<Element, Impl>
        #ifdef BOOST_MSVC
            const &
        #endif
        make_typed_view( mapped_view_reference<char, Impl> const & );
    };

    template <typename Impl>
    struct mapper<char, Impl>
    {
        static mapped_view_reference<char, Impl> map
        (
            mapping<Impl>   const & source_mapping,
            boost::uint64_t         offset        ,
            std  ::size_t           desired_size
        );

        static void unmap( mapped_view_reference<char, Impl> const & );
    };
} // namespace detail

template <typename Impl> struct mapping;

template <typename Element, typename Impl>
class mapped_view_reference : public iterator_range<Element *>
{
public:
    typedef iterator_range<Element *> memory_range_t;

public: // Factory methods.
    static mapped_view_reference map
    (
        mapping<Impl>   const & source_mapping,
        boost::uint64_t         offset       = 0,
        std  ::size_t           desired_size = 0
    )
    {
        BOOST_ASSERT_MSG
        (
            !boost::is_const<Element>::value || source_mapping.is_read_only(),
            "Use const element mapped view for read only mappings."
        );
        return detail::mapper<Element, Impl>::map( source_mapping, offset, desired_size );
    }

    static void unmap( mapped_view_reference const & view )
    {
        detail::mapper<Element, Impl>::unmap( view );
    }

private: template <typename Element_, typename Impl_> friend struct detail::mapper;
    mapped_view_reference( iterator_range<Element *> const & mapped_range ) : iterator_range<Element *>( mapped_range   ) {}
    mapped_view_reference( Element * const p_begin, Element * const p_end ) : iterator_range<Element *>( p_begin, p_end ) {}

private: // Hide mutable members
    void advance_begin();
    void advance_end  ();

    void pop_front();
    void pop_back ();
}; // class mapped_view_reference


namespace detail
{
    // Implementation note:
    //   These have to be defined after mapped_view_reference for eager
    // compilers (e.g. GCC and Clang).
    //                                        (14.07.2011.) (Domagoj Saric)

    template <typename Element, typename Impl>
    mapped_view_reference<char, Impl>
    #ifdef BOOST_MSVC
        const &
    #endif
    mapper<Element, Impl>::make_basic_view( mapped_view_reference<Element, Impl> const & range )
    {
        return
        #ifdef BOOST_MSVC
            reinterpret_cast<mapped_view_reference<char, Impl> const &>( range );
        #else // compiler might care about strict aliasing rules
            mapped_view_reference<char, Impl>
            (
                static_cast<char *>( const_cast<void *>( static_cast<void const *>( range.begin() ) ) ),
                static_cast<char *>( const_cast<void *>( static_cast<void const *>( range.end  () ) ) )
            );
        #endif // compiler
    }


    template <typename Element, typename Impl>
    mapped_view_reference<Element, Impl>
    #ifdef BOOST_MSVC
        const &
    #endif
    mapper<Element, Impl>::make_typed_view( mapped_view_reference<char, Impl> const & range )
    {
        //...zzz...add proper error handling...
        BOOST_ASSERT( reinterpret_cast<std::size_t>( range.begin() ) % sizeof( Element ) == 0 );
        BOOST_ASSERT( reinterpret_cast<std::size_t>( range.end  () ) % sizeof( Element ) == 0 );
        BOOST_ASSERT(                                range.size ()   % sizeof( Element ) == 0 );
        return
        #ifdef BOOST_MSVC
            reinterpret_cast<mapped_view_reference<Element, Impl> const &>( range );
        #else // compiler might care about strict aliasing rules
            mapped_view_reference<Element, Impl>
            (
                static_cast<Element *>( static_cast<void *>( range.begin() ) ),
                static_cast<Element *>( static_cast<void *>( range.end  () ) )
            );
        #endif // compiler
    }
} // namespace detail


template <typename Handle>
struct is_mappable : mpl::false_ {};

template <> struct is_mappable<char                                 *> : mpl::true_ {};
template <> struct is_mappable<char                           const *> : mpl::true_ {};
template <> struct is_mappable<FILE                                 *> : mpl::true_ {};
template <> struct is_mappable<handle<posix>::native_handle_t        > : mpl::true_ {};
#ifdef _WIN32
template <> struct is_mappable<wchar_t                              *> : mpl::true_ {};
template <> struct is_mappable<wchar_t                        const *> : mpl::true_ {};
template <> struct is_mappable<handle<win32>::native_handle_t        > : mpl::true_ {};
#endif // _WIN32


template <typename Element, typename Impl = BOOST_MMAP_IMPL()>
class mapped_view
    :
    public  mapped_view_reference<Element, Impl>,
    private noncopyable
{
public:
    typedef typename mapped_view_reference<Element, Impl>::memory_range_t memory_range_t;

    explicit mapped_view( mapped_view_reference<Element, Impl> const range ) : mapped_view_reference<Element, Impl>( range ) {}
    ~mapped_view() { mapped_view_reference<Element, Impl>::unmap( *this ); }

    mapped_view( mapped_view && source ) : mapped_view_reference<Element, Impl>( source ) { static_cast<memory_range_t &>( source ) = memory_range_t(); }

    void swap( mapped_view & other )
    {
        std::swap( static_cast<mapped_view_reference<Element, Impl> &>( *this ), static_cast<mapped_view_reference<Element, Impl> &>( other ) );
    }
}; // class mapped_view

typedef mapped_view<char      > basic_mapped_view;
typedef mapped_view<char const> basic_read_only_mapped_view;

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "mapped_view.inl"
#endif

#endif // mapped_view_hpp
