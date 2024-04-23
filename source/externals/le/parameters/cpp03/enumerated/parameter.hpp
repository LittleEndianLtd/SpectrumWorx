////////////////////////////////////////////////////////////////////////////////
///
/// \file enumerated/parameter.hpp
/// ------------------------------
///
/// Copyright (c) 2011 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameter_hpp__5820E6B3_7684_4DF4_BC99_B0A5CCB0F3E9
#define parameter_hpp__5820E6B3_7684_4DF4_BC99_B0A5CCB0F3E9
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/linear/parameter.hpp"

#include "boost/mpl/map/map0.hpp"

#include "stdint.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Detail ///< \internal
{
    template </*std::*/uint_fast8_t numberOfValues>
    struct EnumeratedParameterTraits
        :
        LinearParameterTraitsBase<0, numberOfValues - 1, 0>
    {
    public: // Types.
        typedef EnumeratedParameterTag Tag;

        typedef /*std::*/uint_fast8_t value_type;
        typedef value_type            param_type;

        typedef boost::mpl::map1<Traits::Unit<0, 0>> Defaults;
        typedef boost::mpl::map0<>                   Traits  ; //...mrmlj...FMOD param info...

    public: // Values
        static unsigned int const rangeValuesDenominator = 1;

        static value_type minimum () { return                  0; }
        static value_type maximum () { return numberOfValues - 1; }
        static value_type default_() { return                  0; }

        static /*std::*/uint_fast8_t const discreteValueDistance = 1;

        static /*std::*/uint_fast8_t const numberOfDiscreteValues = numberOfValues;

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

template <unsigned int numberOfValues>
class EnumeratedParameter
    :
    public Parameter<Detail::EnumeratedParameterTraits<numberOfValues> >
{
protected:
    typedef EnumeratedParameter EnumeratedBase;

public:
    typedef /*std::*/uint_fast8_t binary_type;

    explicit EnumeratedParameter( binary_type const initialValue = EnumeratedParameter::default_() ) : EnumeratedParameter::type( initialValue ) {}
}; // class EnumeratedParameter


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
        typedef EnumeratedBase Base;                                                        \
    public:                                                                                 \
        parameterName                                                                       \
            ( type::param_type const initialValue = Base::default_() )                      \
            : Base( initialValue ) {}                                                       \
        LE_ENUMERATED_PARAMETER_WARNING_BEGIN()                                             \
        enum value_type : /*std::*/uint_fast8_t { BOOST_PP_SEQ_ENUM( valueSequence ) };     \
        LE_ENUMERATED_PARAMETER_WARNING_END()                                               \
        operator value_type() const { return static_cast<value_type>( Base::getValue() ); } \
    }

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
