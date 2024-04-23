////////////////////////////////////////////////////////////////////////////////
///
/// \file parametersUtilities.hpp
/// -----------------------------
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parametersUtilities_hpp__EB1C0F5A_FC45_4407_A713_9197376BC784
#define parametersUtilities_hpp__EB1C0F5A_FC45_4407_A713_9197376BC784
#pragma once
//------------------------------------------------------------------------------
#include "fusionAdaptors.hpp"

#include "le/utility/cstdint.hpp"
#ifdef __GNUC__
    #include "le/utility/staticForEach.hpp"
#endif // __GNUC__
#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
#include "le/utility/switch.hpp"
#pragma warning( pop )

#include <boost/fusion/sequence/intrinsic/at.hpp>
#include <boost/fusion/sequence/intrinsic/value_at.hpp>
#include <boost/mpl/range_c.hpp>

#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// Private implementation details for this module.
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class Invoker
    /// \internal
    /// \brief Helper functor for invoking the parameter Functor with a proper
    /// reference to a parameter object along with the parameter index.
    ///
    ////////////////////////////////////////////////////////////////////////////

    template <class Parameters, class Functor>
    class Invoker : private Functor
    {
    public:
        Invoker( Parameters & parameters, Functor && functor )
            : Functor( functor ), pParameters_( &parameters ) {}

        typedef typename Functor::result_type result_type;

        template <class TypeIndex>
        result_type operator()( TypeIndex const & ) const
        {
            return Functor::operator()
            (
                boost::fusion::at<TypeIndex>( *pParameters_ )
            );
        }

    private:
        void operator=( Invoker const & );
        Parameters * LE_RESTRICT const pParameters_;
    }; // class Invoker

    template <class Parameters, class Functor>
    class StaticInvoker : public Functor
    {
    public:
        typedef typename Functor::result_type result_type;

        template <class TypeIndex>
        result_type operator()( TypeIndex const & )
        {
            typedef typename boost::fusion::result_of:: template value_at<Parameters, TypeIndex>::type Parameter;
            return Functor:: LE_GNU_SPECIFIC( template ) operator()<Parameter>();
        }

        template <class TypeIndex>
        result_type operator()( TypeIndex const & typeIndex ) const { return const_cast<StaticInvoker &>( *this )( typeIndex ); }

    private:
        StaticInvoker();
       ~StaticInvoker();
        StaticInvoker ( StaticInvoker const & );
        void operator=( StaticInvoker const & );
    }; // class StaticInvoker


    template <class WrappingFunctor, class SourceFunctor> WrappingFunctor       && forwardDownCast( SourceFunctor       & sourceFunctor ) { return std::forward<WrappingFunctor      >( static_cast<WrappingFunctor       &>( sourceFunctor ) ); }
    template <class WrappingFunctor, class SourceFunctor> WrappingFunctor const && forwardDownCast( SourceFunctor const & sourceFunctor ) { return std::forward<WrappingFunctor const>( static_cast<WrappingFunctor const &>( sourceFunctor ) ); }


    template <class Parameters, class Functor>
    typename Functor::result_type LE_FASTCALL invokeFunctorOnIndexedParameter( std::uint8_t const parameterIndex, Functor && functor )
    {
        LE_ASSUME( parameterIndex < Parameters::static_size );
        using namespace boost;

        typedef mpl::range_c<std::uint8_t, 0, Parameters::static_size> ValidIndices;
        return switch_<ValidIndices>
               (
                   parameterIndex,
                   std::forward<Functor>( functor ),
                   assert_no_default_case<typename Functor::result_type>()
               );
    }
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
//
// forEachIndexed()
// ----------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief An extension to boost::fusion::for_each() that also passes the index
/// of the parameter as a template parameter to the passed functor.
///
/// \throws <anything_the_passed_functor's_operator()_may_throw>
///
////////////////////////////////////////////////////////////////////////////////

template <class Parameters, class Functor>
void LE_FASTCALL forEachIndexed( Parameters & parameters, Functor && functor )
{
    using namespace boost;

    using ValidIndices   = mpl::range_c<std::uint8_t, 0, Parameters::static_size>;
    using WrappedFunctor = Detail::Invoker<Parameters, Functor>;
    WrappedFunctor wrappedFunctor( parameters, functor );
    Utility::forEach<ValidIndices>( wrappedFunctor );
}

template <class Parameters, class Functor>
void LE_FASTCALL forEachIndexed( Functor && functor )
{
    using namespace boost;

    using ValidIndices = mpl::range_c<std::uint8_t, 0, Parameters::static_size>;
    using Invoker      = Detail::StaticInvoker<Parameters, Functor>;
    Utility::forEach<ValidIndices>( Detail::forwardDownCast<Invoker>( functor ) );
}


////////////////////////////////////////////////////////////////////////////
//
// invokeFunctorOnIndexedParameter()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////
///
/// \brief Enables runtime selection of a Parameter object based on its
/// index.
///
/// \param parameterIndex - index of the desired parameter
/// \param functor        - functor you wish to be invoked with the indexed
///                         parameter object
/// \return - returns the passed functor's return value
///
/// \throws <anything_the_passed_functor's_operator()_may_throw>
///
////////////////////////////////////////////////////////////////////////////
//
// Implementation note:
//   Because Parameter objects contained in a Parameters instance are all
// of a different type, bind and lambda expressions cannot be used to
// 'manipulate' them (contained Parameter objects) and invoke their member
// functions (as the passed functor's operator() has to be a template
// function). For this reason, with the current implementation, you must
// write manual functors to do the desired task.
//                                        (11.05.2009.) (Domagoj Saric)
//
////////////////////////////////////////////////////////////////////////////
// (todo)
//   Document requirements for passed functor objects.
//                                        (13.05.2009.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////

template <class Parameters, class Functor>
typename Functor::result_type LE_FASTCALL invokeFunctorOnIndexedParameter( Parameters & parameters, std::uint8_t const parameterIndex, Functor && functor )
{
    return Detail::invokeFunctorOnIndexedParameter<Parameters>
    (
        parameterIndex,
        Detail::Invoker<Parameters, Functor>( parameters, std::forward<Functor>( functor ) )
    );
}

template <class Parameters, class Functor>
typename Functor::result_type LE_FASTCALL invokeFunctorOnIndexedParameter( std::uint8_t const parameterIndex, Functor const & functor )
{
    typedef Detail::StaticInvoker<Parameters, Functor> Invoker;
    return Detail::invokeFunctorOnIndexedParameter<Parameters>
    (
        parameterIndex,
        Detail::forwardDownCast<Invoker>( functor )
    );
}


////////////////////////////////////////////////////////////////////////////////
//
// IndexOf
// -------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates the index (position or order) of a Parameter.
///
////////////////////////////////////////////////////////////////////////////////

template <class Parameters, class Parameter>
using IndexOf = typename Parameters:: template IndexOf<Parameter>;

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parametersUtilities_hpp
