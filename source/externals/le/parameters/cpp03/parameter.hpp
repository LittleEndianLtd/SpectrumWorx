////////////////////////////////////////////////////////////////////////////////
///
/// \file parameter.hpp
/// -------------------
///
/// Copyright © 2009 - 2015. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameter_hpp__B49E51E6_E59F_4C49_A702_B6533579846D
#define parameter_hpp__B49E51E6_E59F_4C49_A702_B6533579846D
#pragma once
//------------------------------------------------------------------------------
#include "boost/assert.hpp"
#include "boost/mpl/map/map10.hpp"
#include "boost/mpl/string.hpp"
#include "boost/preprocessor/comparison/greater.hpp"
#include "boost/preprocessor/seq/seq.hpp"
#include "boost/preprocessor/seq/enum.hpp"
#include "boost/preprocessor/seq/transform.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------
namespace Traits
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Parameter "traits" (or "properties" or "options") classes.
// ----------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \defgroup ParameterProperties Parameter properties
///
///   These are actually a form of 'named' parameters for the Parameter class.
/// They define their underlying type and their default value (which is used if
/// the trait/property/option is not explicitly specified in a Parameter
/// declaration.
///
////////////////////////////////////////////////////////////////////////////////
///
/// \struct Minimum
/// \ingroup ParameterProperties
///
////////////////////////////////////////////////////////////////////////////////
///
/// \struct Maximum
/// \ingroup ParameterProperties
///
////////////////////////////////////////////////////////////////////////////////
///
/// \struct Default
/// \ingroup ParameterProperties
///
////////////////////////////////////////////////////////////////////////////////
///
/// \struct ValuesDenominator
/// \ingroup ParameterProperties
///
///   A value with which the default and range values are divided to calculate
/// their real values (used for floating point type parameters, specifying them
/// as rational numbers, e.g.
/// 'real minimum value' = minimumValue / valuesDenominator.
///
////////////////////////////////////////////////////////////////////////////////

// Helper verbosity-reducing macros for Parameter trait declarations.

#define DECLARE_PARAMETER_TRAIT( name, valueType )  \
namespace Tag { struct name; }                      \
template <valueType vvalue>                         \
struct name                                         \
    :                                               \
    boost::mpl::pair                                \
    <                                               \
        Tag::name,                                  \
        boost::mpl::integral_c<valueType, vvalue>   \
    >                                               \
{}

DECLARE_PARAMETER_TRAIT( Minimum          ,          int ); /// \ingroup ParameterProperties
DECLARE_PARAMETER_TRAIT( Maximum          ,          int ); /// \ingroup ParameterProperties
DECLARE_PARAMETER_TRAIT( Default          ,          int ); /// \ingroup ParameterProperties
DECLARE_PARAMETER_TRAIT( ValuesDenominator, unsigned int ); /// \ingroup ParameterProperties

// Helper macro cleanup.
#undef DECLARE_PARAMETER_TRAIT


////////////////////////////////////////////////////////////////////////////////
///
/// \class Unit
/// \ingroup ParameterProperties
/// \brief Specifies the unit type/dimension of a parameter through a string
/// suffix (used for 'printing' of parameter values).
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   The template parameters are simple ints instead of a boost::mpl::string<>
// because it simplifies usage/reduces verbosity and the maximum of eight
// characters incurred with this design seems enough for all current usages.
//                                            (25.09.2009.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

namespace Tag { struct Unit; }

template <unsigned int stringLiteralPart1, unsigned int stringLiteralPart2 = 0>
struct Unit
    :
    boost::mpl::pair
    <
        Tag::Unit,
        boost::mpl::string<stringLiteralPart1, stringLiteralPart2>
    >
{};


////////////////////////////////////////////////////////////////////////////////
///
/// \struct Unit2
/// \ingroup ParameterProperties
/// The comma, required with the Unit trait template for unit suffixes longer
/// than four characters, does not work well with (the LE_DEFINE_PARAMETER(S))
/// macros so this helper/alternative unit specifying trait template, that takes
/// a boost::mpl::string instance, can be used instead for such cases.
///
////////////////////////////////////////////////////////////////////////////////

template <typename MPLString>
struct Unit2
    :
    boost::mpl::pair
    <
        Tag::Unit,
        MPLString
    >
{};

//------------------------------------------------------------------------------
} // namespace Traits

////////////////////////////////////////////////////////////////////////////////
///
/// \class Parameter
///
///   Implements the Parameter concept based on the specified traits.
///
///   None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

template <class ImplTraits>
class Parameter : public ImplTraits
{
public:
    typedef Parameter type;

    typedef typename ImplTraits::value_type value_type ;
    typedef typename ImplTraits::param_type param_type ;
    typedef                      value_type binary_type;

public:
    // Intentional implicit conversion.
    Parameter( param_type const initialValue = ImplTraits::default_() )
    {
        // Traits sanity checks.
        BOOST_ASSERT( this->isValidValue( ImplTraits::minimum () ) );
        BOOST_ASSERT( this->isValidValue( ImplTraits::default_() ) );
        BOOST_ASSERT( this->isValidValue( ImplTraits::maximum () ) );

        BOOST_ASSERT( ImplTraits::maximum() >= ImplTraits::minimum() );

        setValue( initialValue );
    }

    value_type const & getValue(                  ) const { BOOST_ASSERT_MSG( this->isValidValue( value_ ), "Parameter in inconsistent state."                               ); return value_;  }
    void               setValue( param_type value )       { BOOST_ASSERT_MSG( this->isValidValue( value  ), "Specified value is invalid or out of range for this parameter." ); value_ = value; }

    void reset() { setValue( ImplTraits::default_() ); }

public:
    operator value_type const &() const { return getValue(); }

    Parameter & operator++()
    {
        BOOST_ASSERT_MSG( getValue() != ImplTraits::maximum(), "Tried to increment a parameter past its maximum value." );
        ImplTraits::increment( value_ );
        BOOST_ASSERT_MSG( ImplTraits::isValidValue( value_ ),  "Parameter in inconsistent state after increment."       );
        return *this;
    }

    Parameter & operator--()
    {
        BOOST_ASSERT_MSG( getValue() != ImplTraits::minimum(), "Tried to decrement a parameter below its minimum value." );
        ImplTraits::decrement( value_ );
        BOOST_ASSERT_MSG( ImplTraits::isValidValue( value_ ),  "Parameter in inconsistent state after decrement."        );
        return *this;
    }

    bool operator!=( param_type other ) const { return this->getValue() != other; }

protected:
    value_type value_;

private:
    static_assert( ImplTraits::unscaledMaximum >= ImplTraits::unscaledMinimum, "Invalid range." );

    static_assert
    (
        ( ImplTraits::unscaledDefault >= ImplTraits::unscaledMinimum ) &&
        ( ImplTraits::unscaledDefault <= ImplTraits::unscaledMaximum ),
        "Default value out of range."
    );
};


template <class OriginalParameter, class NewTraits>
struct Modify
{
    typedef typename OriginalParameter:: template Modify<NewTraits>::type ModifiedTraits;
    typedef Parameter<ModifiedTraits> type;
};


////////////////////////////////////////////////////////////////////////////////
///
/// Helper verbosity reducing macros for parameter specifications.
/// --------------------------------------------------------------
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   In order to create unique types (for individual parameters), structs that
// publicly derive from a specialized templates are used instead of typedefs.
// This also gives far more readable compiler and linker errors and also seems
// to slightly improve build times.
//                                            (25.09.2009.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
/// \def LE_ADD_TRAIT_PREFIX
/// \internal
///
////////////////////////////////////////////////////////////////////////////////

#define LE_ADD_TRAIT_PREFIX( r, dummy, trait ) LE::Parameters::Traits::trait


////////////////////////////////////////////////////////////////////////////////
///
/// \def LE_ENUMERATE_PARAMETER_TRAITS
/// \internal
///
////////////////////////////////////////////////////////////////////////////////

#define LE_ENUMERATE_PARAMETER_TRAITS( traitsSequence )                  \
    BOOST_PP_CAT( boost::mpl::map, BOOST_PP_SEQ_SIZE( traitsSequence ) ) \
    <                                                                    \
        BOOST_PP_SEQ_ENUM                                                \
        (                                                                \
            BOOST_PP_SEQ_TRANSFORM                                       \
            (                                                            \
                LE_ADD_TRAIT_PREFIX,                                     \
                0,                                                       \
                traitsSequence                                           \
            )                                                            \
        )                                                                \
    >


////////////////////////////////////////////////////////////////////////////////
///
/// \def LE_DEFINE_PARAMETER
/// \brief Creates the struct representing the individual parameter and adds the
/// forwarding constructor (to minimize the difference between a typedef
/// parameter specification and a struct that publicly derives from Parameter<>.
///
///   It accepts a Boost PP sequence where the first element is the desired name
/// for the parameter, the second is the base/existing parameter from which to
/// create the new parameter (by modifying it) and the following elements are
/// the desired parameter traits. If the sequence only has two elements the
/// macro only 'renames' the specified base parameter (i.e. it creates a new
/// parameter type that is the same/has the same traits as the base one).
///
////////////////////////////////////////////////////////////////////////////////

#define LE_DEFINE_PARAMETER_FULL( parameterSequence )                                                    \
    class BOOST_PP_SEQ_HEAD( parameterSequence )                                                         \
        :                                                                                                \
        public LE::Parameters::Modify                                                                    \
        <                                                                                                \
            BOOST_PP_SEQ_HEAD( BOOST_PP_SEQ_TAIL( parameterSequence ) ),                                 \
            LE_ENUMERATE_PARAMETER_TRAITS( BOOST_PP_SEQ_TAIL( BOOST_PP_SEQ_TAIL( parameterSequence ) ) ) \
        >::type                                                                                          \
    {                                                                                                    \
    public:                                                                                              \
        BOOST_PP_SEQ_HEAD( parameterSequence )                                                           \
            ( type::value_type const initialValue = type::default_() )                                   \
        { setValue( initialValue ); }                                                                    \
    };


#define LE_DEFINE_PARAMETER_RENAME_ONLY( parameterSequence )                             \
    class BOOST_PP_SEQ_HEAD( parameterSequence )                                         \
    : public BOOST_PP_SEQ_HEAD( BOOST_PP_SEQ_TAIL( parameterSequence ) )                 \
    {                                                                                    \
    public:                                                                              \
        BOOST_PP_SEQ_HEAD( parameterSequence )                                           \
        ( type::value_type const initialValue = type::default_() )                       \
        : BOOST_PP_SEQ_HEAD( BOOST_PP_SEQ_TAIL( parameterSequence ) )( initialValue ) {} \
    };


#define LE_DEFINE_PARAMETER( parameterSequence )                       \
    BOOST_PP_IIF                                                       \
    (                                                                  \
        BOOST_PP_GREATER( BOOST_PP_SEQ_SIZE( parameterSequence ), 2 ), \
        LE_DEFINE_PARAMETER_FULL,                                      \
        LE_DEFINE_PARAMETER_RENAME_ONLY                                \
    )( parameterSequence )

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
