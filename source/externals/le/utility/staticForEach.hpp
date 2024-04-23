////////////////////////////////////////////////////////////////////////////////
///
/// \file staticForEach.hpp
/// -----------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef staticForEach_hpp__C3553311_E011_474C_B651_74228B0BEDCE
#define staticForEach_hpp__C3553311_E011_474C_B651_74228B0BEDCE
#pragma once
//------------------------------------------------------------------------------
#include "platformSpecifics.hpp"

#include "boost/mpl/begin_end.hpp"
#include "boost/mpl/bool_fwd.hpp"
#include "boost/mpl/deref.hpp"
#include "boost/mpl/is_sequence.hpp"
#include "boost/mpl/next_prior.hpp"
#include "boost/fusion/iterator/value_of.hpp"
#include "boost/fusion/sequence/intrinsic/begin.hpp"
#include "boost/fusion/sequence/intrinsic/end.hpp"

#include <type_traits>
#include <utility> // rvalues
//------------------------------------------------------------------------------
namespace boost { namespace fusion { struct fusion_sequence_tag; } }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// forEach()
// ---------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief A simpler and more efficient version of the mpl::for_each<>()
/// function. @see boost::mpl::for_each.
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    template <typename Iterator, typename LastIterator, typename F>
    void executeMPL( F &&, std::true_type ) {}

    template <typename Iterator, typename LastIterator, typename F>
    void executeMPL( F && f, std::false_type )
    {
        using namespace boost;

        typedef typename mpl::deref<Iterator>::type item;

        f.
        #ifndef _MSC_VER
            BOOST_NESTED_TEMPLATE
        #endif // _MSC_VER
        operator()<item>();

        typedef typename mpl::next<Iterator>::type iter;

        executeMPL<iter, LastIterator, F>( std::forward<F>( f ), std::is_same<iter, LastIterator>() );
    }

    template <typename Sequence, typename F, typename Tag>
    void forEach( F && f, Tag * )
    {
        using namespace boost;

        static_assert( mpl::is_sequence<Sequence>::value, "" );

        typedef typename mpl::begin<Sequence>::type first;
        typedef typename mpl::end  <Sequence>::type last ;

        Detail::executeMPL<first, last, F>( std::forward<F>( f ), std::is_same<first, last>() );
    }


    template <typename Iterator, typename LastIterator, typename F>
    void executeFusion( F &&, std::true_type ) {}

    template <typename Iterator, typename LastIterator, typename F>
    void executeFusion( F && f, std::false_type )
    {
        using namespace boost::fusion;

        typedef typename result_of::value_of<Iterator>::type item;

        f.
        #ifndef _MSC_VER
            BOOST_NESTED_TEMPLATE
        #endif // _MSC_VER
        operator()<item>();

        typedef typename result_of::next<Iterator>::type iter;

        executeFusion<iter, LastIterator, F>( std::forward<F>( f ), std::is_same<iter, LastIterator>() );
    }

    template <typename Sequence, typename F>
    void forEach( F && f, boost::fusion::fusion_sequence_tag * )
    {
        using namespace boost::fusion;

        typedef typename result_of::begin<Sequence>::type first;
        typedef typename result_of::end  <Sequence>::type last ;

        Detail::executeFusion<first, last, F>( std::forward<F>( f ), std::is_same<first, last>() );
    }

    template <typename Iterator, typename LastIterator, typename F, typename P>
    typename F::result_type applyFor( F &&, P &&, std::true_type ) { LE_UNREACHABLE_CODE(); return typename F::result_type(); }

    template <typename Iterator, typename LastIterator, typename F, typename P>
    typename F::result_type applyFor( F && f, P && p, std::false_type )
    {
        using namespace boost;

        typedef typename mpl::deref<Iterator>::type item;

        bool const conditionSatisfied( p. template operator()<item>() );
        if ( conditionSatisfied )
            return f. template operator()<item>();

        typedef typename mpl::next<Iterator>::type iter;

        return applyFor<iter, LastIterator, F, P>( std::forward<F>( f ), std::forward<P>( p ), std::is_same<iter, LastIterator>() );
    }
} // namespace Detail

template <typename Sequence, typename F>
void LE_FASTCALL forEach( F && f )
{
    Detail::forEach<Sequence, F>( std::forward<F>( f ), static_cast<typename Sequence::tag *>( 0 ) );
}


template <typename Sequence, typename Functor, typename Predicate>
typename Functor::result_type LE_FASTCALL applyFor( Functor && f, Predicate && p )
{
    using namespace boost;

    static_assert( mpl::is_sequence<Sequence>::value, "" );

    typedef typename mpl::begin<Sequence>::type first;
    typedef typename mpl::end  <Sequence>::type last ;

    return Detail::applyFor<first, last, Functor, Predicate>
    (
        std::forward<Functor  >( f ),
        std::forward<Predicate>( p ),
        std::is_same<first, last>()
    );
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // staticForEach_hpp
