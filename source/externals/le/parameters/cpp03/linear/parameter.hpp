////////////////////////////////////////////////////////////////////////////////
///
/// \file linear/parameter.hpp
/// --------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameter_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#define parameter_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/parameter.hpp"

#include <boost/mpl/at.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/map/map10.hpp>

#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

/// \internal
namespace Detail
{
    template <typename T>
    bool isValueInRange( T value, T rangeMinimum, T rangeMaximum );


    ////////////////////////////////////////////////////////////////////////////
    /// \internal
    /// \struct GetTraitDefaulted
    /// Returns the trait specified by TraitTag from Traits or from Defaults if
    /// not present in Traits.
    ////////////////////////////////////////////////////////////////////////////

    template <class TraitTag, class Traits, class Defaults>
    struct GetTraitDefaulted
    {
        typedef typename boost::mpl::at<Traits, TraitTag>::type UserSpecified;
        typedef typename boost::mpl::eval_if
        <
            std::is_same<UserSpecified, boost::mpl::void_>,
            boost::mpl::at<Defaults, TraitTag>,
            UserSpecified
        >::type type;
    };

    template <int minimumValue, int maximumValue, int defaultValue>
    struct LinearParameterTraitsBase
    {
    public: // Types.
        typedef LinearParameterTag Tag;

    public: // Values
        static          int const unscaledMinimum        = minimumValue;
        static          int const unscaledMaximum        = maximumValue;
        static          int const unscaledDefault        = defaultValue;
        static unsigned int const rangeValuesDenominator =            1;

    protected:
        template <typename T> static void increment( T & value ) { ++value; }
        template <typename T> static void decrement( T & value ) { --value; }
    };

    template <int minimumValue, int maximumValue, int defaultValue, unsigned int rangeScaleFactor>
    struct LinearFloatParameterTraitsBase
        :
        LinearParameterTraitsBase<minimumValue, maximumValue, defaultValue>
    {
    public: // Types.
        typedef LinearFloatParameterTag Tag;

        typedef float              value_type;
    #if defined( _MSC_VER ) && !( defined( _M_AMD64 ) || ( _M_IX86_FP >= 2 ) )
        typedef value_type const & param_type;
    #else
        typedef value_type         param_type;
    #endif // _MSC_VER

    public: // Values
        static unsigned int const rangeValuesDenominator = rangeScaleFactor;

        static value_type const & minimum () { static value_type const value( LinearFloatParameterTraitsBase::unscaledMinimum / static_cast<value_type>( rangeScaleFactor ) ); return value; }
        static value_type const & maximum () { static value_type const value( LinearFloatParameterTraitsBase::unscaledMaximum / static_cast<value_type>( rangeScaleFactor ) ); return value; }
        static value_type const & default_() { static value_type const value( LinearFloatParameterTraitsBase::unscaledDefault / static_cast<value_type>( rangeScaleFactor ) ); return value; }

        static unsigned int const discreteValueDistance = 0;

        static bool isValidValue( value_type const value )
        {
            return isValueInRange<param_type>( value, minimum(), maximum() );
        }

    protected:
        static_assert( rangeValuesDenominator != 0, "The rangeValuesDenominator cannot be zero." );
    };

    template <class TraitsParam, class DefaultsParam = boost::mpl::map0<> >
    struct LinearFloatParameterTraits
        :
        LinearFloatParameterTraitsBase
        <
            GetTraitDefaulted<Traits::Tag::Minimum          , TraitsParam, DefaultsParam>::type::value,
            GetTraitDefaulted<Traits::Tag::Maximum          , TraitsParam, DefaultsParam>::type::value,
            GetTraitDefaulted<Traits::Tag::Default          , TraitsParam, DefaultsParam>::type::value,
            GetTraitDefaulted<Traits::Tag::ValuesDenominator, TraitsParam, DefaultsParam>::type::value
        >
    {
    public:
        typedef TraitsParam   Traits  ;
        typedef DefaultsParam Defaults;

        template <class NewTraits>
        struct Modify
        {
            typedef LinearFloatParameterTraits<NewTraits, Traits> type;
        };
    };

    template <typename T, class TraitsParam, class DefaultsParam>
    struct LinearIntegerParameterTraits
        :
        LinearParameterTraitsBase
        <
            GetTraitDefaulted<Traits::Tag::Minimum, TraitsParam, DefaultsParam>::type::value,
            GetTraitDefaulted<Traits::Tag::Maximum, TraitsParam, DefaultsParam>::type::value,
            GetTraitDefaulted<Traits::Tag::Default, TraitsParam, DefaultsParam>::type::value
        >
    {
    public: // Types.
        typedef LinearIntegerParameterTag Tag;

        typedef T          value_type;
        typedef value_type param_type;

        typedef TraitsParam   Traits  ;
        typedef DefaultsParam Defaults;

        template <class NewTraits>
        struct Modify
        {
            typedef LinearIntegerParameterTraits<T, NewTraits, Traits> type;
        };

    public: // Values
        static unsigned int const rangeValuesDenominator = 1;

        static value_type minimum () { return LinearIntegerParameterTraits::unscaledMinimum; }
        static value_type maximum () { return LinearIntegerParameterTraits::unscaledMaximum; }
        static value_type default_() { return LinearIntegerParameterTraits::unscaledDefault; }

        static unsigned int const discreteValueDistance = 1;

        static bool isValidValue( value_type const value )
        {
            return isValueInRange<param_type>( value, minimum(), maximum() );
        }
    };

    template <class Traits, class Defaults>
    struct LinearSignedIntegerParameterTraits   : LinearIntegerParameterTraits<         int, Traits, Defaults> {};

    template <class Traits, class Defaults>
    struct LinearUnsignedIntegerParameterTraits : LinearIntegerParameterTraits<unsigned int, Traits, Defaults> {};
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct LinearSignedInteger
////////////////////////////////////////////////////////////////////////////////

struct LinearSignedInteger
{
    template <class NewTraits>
    struct Modify
    {
        typedef Detail::LinearSignedIntegerParameterTraits
        <
            NewTraits,
            boost::mpl::map1<Traits::Unit<0> >
        > type;
    };
};


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct LinearUnsignedInteger
////////////////////////////////////////////////////////////////////////////////

struct LinearUnsignedInteger
{
    template <class NewTraits>
    struct Modify
    {
        typedef Detail::LinearUnsignedIntegerParameterTraits
        <
            NewTraits,
            boost::mpl::map1<Traits::Unit<0> >
        > type;
    };
};


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct LinearFloat
////////////////////////////////////////////////////////////////////////////////

struct LinearFloat
{
    template <class NewTraits>
    struct Modify
    {
        typedef Detail::LinearFloatParameterTraits
        <
            NewTraits,
            boost::mpl::map2<Traits::ValuesDenominator<1>, Traits::Unit<0> >
        > type;
    };
};

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
