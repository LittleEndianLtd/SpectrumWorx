////////////////////////////////////////////////////////////////////////////////
///
/// \file factoryMacro.hpp
/// ----------------------
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef factoryMacro_hpp__8F075C97_FE9A_47F0_A01B_A3632AA17AC9
#define factoryMacro_hpp__8F075C97_FE9A_47F0_A01B_A3632AA17AC9
#pragma once
//------------------------------------------------------------------------------
#include "parameter.hpp"

#include <boost/fusion/support/category_of.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/control/iif.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/size.hpp>

#include <stdint.h>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// Helper verbosity reducing macros for parameter specifications.
/// --------------------------------------------------------------
///
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
/// \internal
/// \def LE_SKIP_ALREADY_DEFINED_PARAMETER
///
////////////////////////////////////////////////////////////////////////////////

#define LE_SKIP_ALREADY_DEFINED_PARAMETER( parameterSequence )


////////////////////////////////////////////////////////////////////////////////
///
/// \internal
/// \def LE_DEFINE_INDIVIDUAL_PARAMETER
/// \brief Calls LE_DEFINE_PARAMETER if the parameter is not already defined (if
/// its sequence has more than one element).
///
////////////////////////////////////////////////////////////////////////////////

#define LE_DEFINE_INDIVIDUAL_PARAMETER( r, dummy, parameterSequence )  \
    BOOST_PP_IIF                                                       \
    (                                                                  \
        BOOST_PP_GREATER( BOOST_PP_SEQ_SIZE( parameterSequence ), 1 ), \
        LE_DEFINE_PARAMETER,                                           \
        LE_SKIP_ALREADY_DEFINED_PARAMETER                              \
    )( parameterSequence )


////////////////////////////////////////////////////////////////////////////////
///
/// \internal
/// \def LE_ADD_INDIVIDUAL_PARAMETER
/// \brief Adds a parameter to the Parameters container struct.
///
////////////////////////////////////////////////////////////////////////////////

#ifndef LE_PARAMETERS_NO_CLASSIC_ACCESSORS
    #define LE_ADD_INDIVIDUAL_PARAMETER_CLASSIC_WORKER( parameter )                                                            \
        parameter       & get##parameter (                                   )       { return BOOST_PP_CAT( parameter, _ ) ; } \
        parameter const & get##parameter (                                   ) const { return BOOST_PP_CAT( parameter, _ ) ; } \
        void              set##parameter ( parameter::param_type const value )       { BOOST_PP_CAT( parameter, _ ) = value; }
#else
    #define LE_ADD_INDIVIDUAL_PARAMETER_CLASSIC_WORKER( parameter )
#endif

#define LE_ADD_INDIVIDUAL_PARAMETER_WORKER( index, parameter )                                                      \
    template <int dummy> struct ParameterAt<index    , dummy> { typedef parameter type; };                          \
    template <int dummy> struct IndexOf    <parameter, dummy> : boost::mpl::integral_c</*std::*/uint8_t, index> {}; \
    parameter & get( parameter * ) { return BOOST_PP_CAT( parameter, _ ); }                                         \
    LE_ADD_INDIVIDUAL_PARAMETER_CLASSIC_WORKER( parameter )                                                         \
    parameter BOOST_PP_CAT( parameter, _ );

#define LE_ADD_INDIVIDUAL_PARAMETER( r, dummy, index, parameterSequence ) \
    LE_ADD_INDIVIDUAL_PARAMETER_WORKER( index, BOOST_PP_SEQ_HEAD( parameterSequence ) )


////////////////////////////////////////////////////////////////////////////////
///
/// \internal
/// \def LE_DEFINE_MY_PARAMETERS
/// \brief Declares all the parameters from the parameter sequence.
///
////////////////////////////////////////////////////////////////////////////////

#define LE_DEFINE_MY_PARAMETERS( parameterSequence ) \
    BOOST_PP_SEQ_FOR_EACH( LE_DEFINE_INDIVIDUAL_PARAMETER, 0, parameterSequence )


////////////////////////////////////////////////////////////////////////////////
///
/// \internal
/// \def LE_ENUMERATE_PARAMETERS
///
////////////////////////////////////////////////////////////////////////////////

#define LE_ENUMERATE_PARAMETERS( parameters ) \
    BOOST_PP_SEQ_FOR_EACH_I( LE_ADD_INDIVIDUAL_PARAMETER, 0, parameters )


////////////////////////////////////////////////////////////////////////////////
///
/// \def LE_DEFINE_PARAMETERS
///
/// \brief Helper macro used to simplify the definition of an effect's
/// parameters, both the individual parameters and the Parameters collection/
/// container.
///
///   It accepts a "two dimensional" Boost preprocessor sequence
/// (http://www.boost.org/doc/libs/release/libs/preprocessor/doc/data/sequences.html),
/// in other words, a sequence whose each element represents one parameter and
/// is in fact also a sequence (of properties that define the particular
/// parameter).
///   The latter (the sequence that defines a parameter, a "parameter sequence"
/// from now on) can have one or more elements. If it has only one element it
/// is interpreted as the name of an already defined parameter which is then
/// simply 'inserted' into the parameters being defined. If it has more than
/// one element it, the sequence, is passed on to the LE_DEFINE_PARAMETER macro.
///
///   The macro automatically prepends any required namespace names.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Because full specialization is allowed only at namespace scope the dummy
// partial specialization workaround is used for metafunctions and the typed
// pointer dispatch for functions.
//                                            (28.06.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

#define LE_DEFINE_PARAMETERS( parameters )                                                                     \
    LE_DEFINE_MY_PARAMETERS(  parameters )                                                                     \
    struct Parameters                                                                                          \
    {                                                                                                          \
        static /*std::*/uint8_t const static_size = BOOST_PP_SEQ_SIZE( parameters );                           \
        struct category : boost::fusion::forward_traversal_tag, boost::fusion::associative_tag {};             \
        typedef ::LE::Parameters::Tag                               fusion_tag;                                \
        typedef boost::fusion::fusion_sequence_tag                  tag       ;                                \
        typedef boost::mpl::false_                                  is_view   ;                                \
        typedef boost::mpl::integral_c</*std::*/uint8_t, static_size> size    ;                                \
        template <unsigned int   , int dummy = 0> struct ParameterAt;                                          \
        template <class Parameter, int dummy = 0> struct IndexOf : size {};                                    \
        template <class Parameter>                                                                             \
        Parameter       & get()       { return get( static_cast<Parameter *>( 0 ) ); }                         \
        template <class Parameter>                                                                             \
        Parameter const & get() const { return const_cast<Parameters &>( *this ).template get<Parameter>(); }  \
        template <class Parameter>                                                                             \
        void set( typename Parameter::param_type const value ) { get<Parameter>().setValue( value ); }         \
        operator boost::fusion::detail::from_sequence_convertible_type() const;                                \
        LE_ENUMERATE_PARAMETERS( parameters )                                                                  \
    };

struct Tag;

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

// Implementation note:
//   Forward declarations for Boost.Fusion adaptation code.
//                                            (28.06.2011.) (Domagoj Saric)
namespace boost
{
namespace fusion
{
    struct fusion_sequence_tag;

    namespace detail
    {
        struct from_sequence_convertible_type;
    }

    namespace extension
    {
        template <typename> struct begin_impl;
        template <>         struct begin_impl<LE::Parameters::Tag>;

        template <typename> struct end_impl;
        template <>         struct end_impl<LE::Parameters::Tag>;

        template <typename Tag> struct value_at_key_impl;
        template <>             struct value_at_key_impl<LE::Parameters::Tag>;

        template <typename Tag> struct value_at_impl;
        template <>             struct value_at_impl<LE::Parameters::Tag>;

        template <typename Tag> struct at_impl;
        template <>             struct at_impl<LE::Parameters::Tag>;

        template <typename Tag> struct has_key_impl;
        template <>             struct has_key_impl<LE::Parameters::Tag>;
    } // namespace extension
} // namespace fusion
} // namespace boost

//------------------------------------------------------------------------------
#endif // factoryMacro_hpp
