////////////////////////////////////////////////////////////////////////////////
///
/// \file enumerated/parameter.hpp
/// ------------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( __clang__ ) && ( _MSC_VER < 1800 )
#include "le/parameters/cpp03/enumerated/parameter.hpp"
#endif // old MSVC
//------------------------------------------------------------------------------ 
#ifndef parameter_hpp__5820E6B3_7684_4DF4_BC99_B0A5CCB0F3E9
#define parameter_hpp__5820E6B3_7684_4DF4_BC99_B0A5CCB0F3E9
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/linear/parameter.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

template <typename... Traits> struct TraitPack;

namespace Detail ///< \internal
{
    template <std::uint8_t numberOfValues>
    struct EnumeratedParameterTraits
        :
        LinearParameterTraitsBase<0, numberOfValues - 1, 0>
    {
    public: // Types.
        using Tag = EnumeratedParameterTag;

        using value_type  = std::uint8_t;
        using param_type  = value_type  ;
        using binary_type = value_type  ;

        using Defaults = TraitPack<Traits::Unit<0, 0>>;
        using Traits   = TraitPack<>                  ;

    public: // Values
        static unsigned int const rangeValuesDenominator = 1;

        static value_type minimum () { return                  0; }
        static value_type maximum () { return numberOfValues - 1; }
        static value_type default_() { return                  0; }

        static value_type const discreteValueDistance = 1;

        static value_type const numberOfDiscreteValues = numberOfValues;

        static bool isValidValue( value_type const value )
        {
            return isValueInRange<param_type>( value, minimum(), maximum() );
        }

    protected:
        static void increment( value_type & value ) { ++value; }
        static void decrement( value_type & value ) { --value; }
    }; // struct EnumeratedParameterTraits
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class EnumeratedParameter
////////////////////////////////////////////////////////////////////////////////

template <std::uint8_t numberOfValues>
using EnumeratedParameter = Parameter<Detail::EnumeratedParameterTraits<numberOfValues>>;


////////////////////////////////////////////////////////////////////////////////
///
/// \def LE_ENUMERATED_PARAMETER
///
/// \brief Helps to define a parameter that has a discrete set of allowed
/// values.
///
///   It will assign automatically generated values to all the named values
/// specified in the valueSequence and will create a member enum with the enum
/// constants/"members" named just as specified in the valueSequence parameter.
///
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#define LE_ENUMERATED_PARAMETER_WARNING_BEGIN() \
    __pragma( warning( push )           )
    __pragma( warning( disable : 4480 ) ) /* Nonstandard extension*/

#define LE_ENUMERATED_PARAMETER_WARNING_END() \
    __pragma( warning( pop ) )
#else
#define LE_ENUMERATED_PARAMETER_WARNING_BEGIN()
#define LE_ENUMERATED_PARAMETER_WARNING_END()
#endif // _MSC_VER

#define LE_ENUMERATED_PARAMETER( parameterName, valueSequence )                             \
    class parameterName                                                                     \
        : public LE::Parameters::EnumeratedParameter<BOOST_PP_SEQ_SIZE( valueSequence )>    \
    {                                                                                       \
    private:                                                                                \
        using Base = type;                                                                  \
    public:                                                                                 \
        parameterName                                                                       \
            ( type::param_type const initialValue = Base::default_() )                      \
            : Base( initialValue ) {}                                                       \
        LE_ENUMERATED_PARAMETER_WARNING_BEGIN()                                             \
        enum value_type : /*std::*/uint8_t { BOOST_PP_SEQ_ENUM( valueSequence ) };          \
        LE_ENUMERATED_PARAMETER_WARNING_END()                                               \
        operator value_type() const { return static_cast<value_type>( Base::getValue() ); } \
    }

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
