////////////////////////////////////////////////////////////////////////////////
///
/// \file uiElements.hpp
/// --------------------
///
///   Defines "placeholders" for data and functional user interface elements for
/// a particular Parameter class.
///
///  "Placeholders", that is declarations without default implementations, are
/// used to ensure that the user/programmer provides a proper implementation for
/// his/hers parameter class (so the error is caught at link time with a missing
/// symbol error instead of at runtime through wrong behaviour caused by the
/// usage of a default implementation).
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef uiElements_hpp__E78E35E8_D163_442F_84C0_19427B8844BA
#define uiElements_hpp__E78E35E8_D163_442F_84C0_19427B8844BA
#pragma once
//------------------------------------------------------------------------------
#ifdef __GNUC__
    #include "linear/parameter.hpp"
#endif // __GNUC__

#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tchar.hpp"

#include "boost/preprocessor/seq/for_each_i.hpp"
#include "boost/preprocessor/tuple/elem.hpp"
#include "boost/utility/string_ref.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
LE_IMPL_NAMESPACE_BEGIN( Engine )
    class Setup;
LE_IMPL_NAMESPACE_END( Engine )
} // namespace SW
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Name
///
/// \brief Placeholder for the name of the parameter (non-optional, must be
/// defined for each parameter).
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Outside code accesses a particular parameter name through the name() free
// function template while a parameter's name is defined by specializing the
// string_ static data member of the Name class template. This approach
// minimizes verbosity of both name-fetching and name-defining code.
//                                            (22.02.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

template <class Parameter>
struct Name
{
    static char const string_[];
};

template <class Parameter>
BOOST_CONSTEXPR boost::string_ref name()
{
    return Name<Parameter>::string_;
}


////////////////////////////////////////////////////////////////////////////////
///
/// \struct DiscreteValues
///
/// \brief Placeholder for individual value strings for discrete-valued
/// parameters.
///
////////////////////////////////////////////////////////////////////////////////

template <class Parameter>
struct DiscreteValues
{
    static char const * LE_RESTRICT const strings[ Parameter::numberOfDiscreteValues ];
};


////////////////////////////////////////////////////////////////////////////////
///
/// 'Transforms' the value of a parameter into a value that should be used for
/// displaying on a user interface.
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail { template <class TraitTag, class TraitsPack, class... DefaultTraits> struct GetTraitDefaulted; }
namespace Traits
{
    namespace Tag { struct Unit; }
    template <int stringLiteralPart1, int stringLiteralPart2> struct Unit;
}

template <class Parameter>
struct DisplayValueTransformer
{
    template <typename Source>
    static Source const & transform( Source const & value, SW::Engine::Setup const & ) { return value; }

    using Suffix = typename Detail::GetTraitDefaulted
    <
        Traits::Tag::Unit,
        typename Parameter::Traits,
        typename Parameter::Defaults
    >::type;
}; // struct DisplayValueTransformer


/// \defgroup UIElementMacros UIElement verbosity reducing macros.
/// \ingroup UIElementMacros
/// \{

////////////////////////////////////////////////////////////////////////////////
///
/// \def ENUMERATED_PARAMETER_STRINGS
///
/// \brief Writes the declaration part of a parameter discrete value's string_
/// definition.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Because value strings for enumerated parameters are defined separately (in
// a .cpp file) from the enumerated parameter we need a mechanism that would
// prevent one to unnoticeably change the order of values at the parameter
// definition site and forget to also change the order of value strings thus
// getting an incorrect mapping between values and their string representations.
//   A first solution (up to revision 4632) was to use a separate
// DiscreteValue<>::string instantiation for each value instead of a single
// std::array<char const *, <numberOfDiscreteValues>>. This required the
// developer to explicitly state for which parameter value a particular string
// is for thereby eliminating the above issue. However this required more
// verbose discrete value name definitions and it added compile time and runtime
// overhead (it required boost::switch_ to fetch strings for values at runtime).
// For this reason the current solution does use a plain array of strings but
// the macro for defining the array also adds static assertions that verify that
// the strings are defined in the proper order.
//                                            (15.07.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

#define ADD_INDIVIDUAL_VALUE_STRING( r, parameter, valueIndex, valueStringPair ) \
    BOOST_PP_TUPLE_ELEM( 2, 1, valueStringPair ),

#define ADD_INDIVIDUAL_VALUE_STRING_ASSERT( r, parameter, valueIndex, valueStringPair ) \
    static_assert( ( parameter::BOOST_PP_TUPLE_ELEM( 2, 0, valueStringPair ) == valueIndex ), "Incorrect order of enumerated parameter value-string pairs" );

#define ENUMERATED_PARAMETER_STRINGS( parentNameSpaceOrClass, parameter, discreteValueStringPairs ) \
    template<> char const * LE_RESTRICT const DiscreteValues<parentNameSpaceOrClass::parameter>::strings[ parentNameSpaceOrClass::parameter::numberOfDiscreteValues ] = { \
    BOOST_PP_SEQ_FOR_EACH_I( ADD_INDIVIDUAL_VALUE_STRING       , parentNameSpaceOrClass::parameter, discreteValueStringPairs ) }; \
    BOOST_PP_SEQ_FOR_EACH_I( ADD_INDIVIDUAL_VALUE_STRING_ASSERT, parentNameSpaceOrClass::parameter, discreteValueStringPairs )

#define EFFECT_ENUMERATED_PARAMETER_STRINGS( parentClass, parameter, discreteValueStringPairs )   \
    } } namespace Parameters {                                                                    \
    ENUMERATED_PARAMETER_STRINGS( SW::Effects::parentClass, parameter, discreteValueStringPairs ) \
    } namespace SW { namespace Effects {


////////////////////////////////////////////////////////////////////////////////
///
/// \def UI_NAME
///
/// \brief Writes the declaration part of a parameter's name definition.
///
////////////////////////////////////////////////////////////////////////////////

#define UI_NAME( parameter ) template<> char const Name<parameter>::string_[]

#define EFFECT_PARAMETER_NAME( parameter, name ) \
    } } namespace Parameters { UI_NAME( SW::Effects::parameter ) = name; } namespace SW { namespace Effects {


/// \} // UIElementMacros

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

namespace boost
{
    template <std::size_t N>
    char const * LE_RESTRICT (*addressof( char const * LE_RESTRICT (&strings)[ N ] ))[ N ] { return &strings; }
} // namespace boost
#endif // uiElements_hpp
