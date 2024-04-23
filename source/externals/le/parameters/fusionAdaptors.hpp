////////////////////////////////////////////////////////////////////////////////
///
/// \file fusionAdaptors.hpp
/// ------------------------
///
/// Adapts parameters created with the LE_DEFINE_PARAMETERS macro into
/// associative and random access Boost Fusion containers.
///
/// Copyright � 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Better compile times are achieved by creating our own Parameters structs
// (with the LE_DEFINE_PARAMETERS macro) and then adapting them to Fusion
// containers than by using available Fusion containers.
// http://www.boost.org/doc/libs/release/libs/fusion/doc/html/fusion/extension/ext_full.html
//                                            (28.06.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef fusionAdaptors_hpp__9E966537_27FB_41D5_9EB1_7BBFDF8999A9
#define fusionAdaptors_hpp__9E966537_27FB_41D5_9EB1_7BBFDF8999A9
#pragma once
//------------------------------------------------------------------------------
#include <boost/fusion/support/category_of.hpp>
#include <boost/fusion/support/iterator_base.hpp>
#include <boost/mpl/if.hpp>

#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

struct Tag;

struct IteratorTag;

template <typename Parameters, unsigned int Index>
struct Iterator : boost::fusion::iterator_base<Iterator<Parameters, Index> >
{
    static unsigned int const parameterIndex = Index;

    typedef IteratorTag                                fusion_tag;
    typedef boost::fusion::random_access_traversal_tag category;

    Iterator( Parameters & parameters ) : parameters_( parameters ) {}
    Parameters & parameters_;
};

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

namespace boost
{
namespace fusion
{
namespace extension
{
    template <typename Tag> struct value_of_impl;
    template <>
    struct value_of_impl<LE::Parameters::IteratorTag>
    {
        template <typename Iterator>
        struct apply;

        template <typename Parameters, unsigned int Index>
        struct apply<LE::Parameters::Iterator<Parameters, Index> > : Parameters:: template ParameterAt<Index> {};
    };


    template <typename Tag> struct deref_impl;
    template <>
    struct deref_impl<LE::Parameters::IteratorTag>
    {
        template <typename Iterator>
        struct apply;

        template <typename Parameters, unsigned int Index>
        struct apply<LE::Parameters::Iterator<Parameters, Index> >
        {
            typedef typename Parameters:: template ParameterAt<Index>::type value_type;
            typedef typename mpl::if_
            <
                std::is_const<Parameters>,
                value_type const &,
                value_type       &
            >::type type;

            static type call( ::LE::Parameters::Iterator<Parameters, Index> const it )
            {
                return it.parameters_. template get<value_type>();
            }
        };
    };


    template <typename Tag> struct next_impl;
    template <>
    struct next_impl<LE::Parameters::IteratorTag>
    {
        template <typename Iterator>
        struct apply;

        template <typename Parameters, unsigned int Index>
        struct apply<LE::Parameters::Iterator<Parameters, Index> >
        {
            typedef ::LE::Parameters::Iterator<Parameters, Index + 1> type;

            static type call( ::LE::Parameters::Iterator<Parameters, Index> const it ) { return type( it.parameters_ ); }
        };
    };


    template <typename Tag> struct prior_impl;
    template <>
    struct prior_impl<LE::Parameters::IteratorTag>
    {
        template <typename Iterator>
        struct apply;

        template <typename Parameters, unsigned int Index>
        struct apply<LE::Parameters::Iterator<Parameters, Index> >
        {
            typedef ::LE::Parameters::Iterator<Parameters, Index - 1> type;

            static type call( ::LE::Parameters::Iterator<Parameters, Index> const it ) { return type( it.parameters_ ); }
        };
    };


    template<typename Tag> struct distance_impl;
    template <>
    struct distance_impl<LE::Parameters::IteratorTag>
    {
        template <typename First, typename Last>
        struct apply
        {
            typedef mpl::int_<Last::parameterIndex - First::parameterIndex> type;

            static type call( First, Last ) { return type(); }
        };
    };


    template <typename> struct begin_impl;
    template <>
    struct begin_impl<LE::Parameters::Tag>
    {
        template <typename Parameters>
        struct apply
        {
            typedef ::LE::Parameters::Iterator<Parameters, 0> type;
            static type call( Parameters & parameters ) { return type( parameters ); }
        };
    };


    template <typename> struct end_impl;
    template <>
    struct end_impl<LE::Parameters::Tag>
    {
        template <typename Parameters>
        struct apply
        {
            typedef ::LE::Parameters::Iterator<Parameters, Parameters::static_size> type;
            static type call( Parameters & parameters ) { return type( parameters ); }
        };
    };


    template <typename Tag> struct value_at_key_impl;
    template <>
    struct value_at_key_impl<LE::Parameters::Tag>
    {
        template <typename Parameters, typename Key>
        struct apply { typedef Key type; };
    };


    template <typename Tag> struct value_at_impl;
    template <>
    struct value_at_impl<LE::Parameters::Tag>
    {
        template <typename Parameters, typename N>
        struct apply : Parameters:: template ParameterAt<N::value> {};
    };


    template <typename Tag> struct at_impl;
    template <>
    struct at_impl<LE::Parameters::Tag>
    {
        template <typename Parameters, typename N>
        struct apply
        {
            typedef typename Parameters:: template ParameterAt<N::value>::type value_type;
            typedef typename mpl::if_
            <
                std::is_const<Parameters>,
                value_type const &,
                value_type       &
            >::type type;

            static type call( Parameters & parameters ) { return parameters. template get<value_type>(); };
        };
    };


    template <typename Tag> struct has_key_impl;
    template <>
    struct has_key_impl<LE::Parameters::Tag>
    {
        template <typename Parameters, typename Key>
        struct apply
            : boost::mpl::bool_<Parameters:: template IndexOf<Key>::value != Parameters::static_size> {};
    };
} // namespace extension
} // namespace fusion
} // namespace boost

//------------------------------------------------------------------------------
#endif // fusionAdaptors_hpp
