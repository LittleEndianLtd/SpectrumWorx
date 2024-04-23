////////////////////////////////////////////////////////////////////////////////
///
/// \file linear/parameter.hpp
/// --------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( __clang__ ) && ( _MSC_VER < 1800 )
#include "le/parameters/cpp03/linear/parameter.hpp"
#endif // old MSVC
//------------------------------------------------------------------------------
#ifndef parameter_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#define parameter_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/parameter.hpp"

#include <cstdint>
#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

template <typename... Traits> struct TraitPack;

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

    template <typename TraitTag, typename... TraitsSequence> struct GetTraitImpl;

    template <typename TraitTag>
    struct GetTraitImpl<TraitTag> { using type = boost::mpl::void_; };

    template <typename TraitTag, typename FirstTrait, typename... TraitsSequence>
    struct GetTraitImpl<TraitTag, FirstTrait, TraitsSequence...> : GetTraitImpl<TraitTag, TraitsSequence...> {};

    template <typename TraitTag, typename Value, typename... TraitsSequence>
    struct GetTraitImpl<TraitTag, boost::mpl::pair<TraitTag, Value>, TraitsSequence...> { using type = Value ; };

    template <typename TraitTag, typename... TraitsSequence>
    //...mrmlj...if an alias is used, MSVC14 (u0,1&2) chokes a few lines below with an C3520 "'TraitsSequence': parameter pack must be expanded in this context."
#if defined( _MSC_VER ) && ( _MSC_VER == 1900 )
    struct GetTrait : GetTraitImpl<TraitTag, typename TraitsSequence::type...> {};
#else
    using  GetTrait = GetTraitImpl<TraitTag, typename TraitsSequence::type...>;
#endif // _MSC_VER
    
    template <typename TraitTag, typename TraitsPack, typename... DefaultTraits>
    struct GetTraitDefaulted;

    template <typename TraitTag, typename... TraitsParam, typename... DefaultsParam>
    struct GetTraitDefaulted<TraitTag, TraitPack<TraitsParam...>, DefaultsParam...>
        : GetTrait<TraitTag, TraitsParam..., DefaultsParam...> {};

    template <typename TraitTag, typename... TraitsParam, typename... DefaultsParam>
    struct GetTraitDefaulted<TraitTag, TraitPack<TraitsParam...>, TraitPack<DefaultsParam...>>
        : GetTraitDefaulted<TraitTag, TraitPack<TraitsParam...>, DefaultsParam...> {};

    template <int minimumValue, int maximumValue, int defaultValue>
    struct LinearParameterTraitsBase
    {
    public: // Types.
        using Tag = LinearParameterTag;

    public: // Values
        static std:: int32_t BOOST_CONSTEXPR_OR_CONST unscaledMinimum        = minimumValue;
        static std:: int32_t BOOST_CONSTEXPR_OR_CONST unscaledMaximum        = maximumValue;
        static std:: int32_t BOOST_CONSTEXPR_OR_CONST unscaledDefault        = defaultValue;
        static std::uint32_t BOOST_CONSTEXPR_OR_CONST rangeValuesDenominator =            1;

    protected:
        template <typename T> static void increment( T & value ) { ++value; }
        template <typename T> static void decrement( T & value ) { --value; }
    }; // struct LinearParameterTraitsBase

    template <int minimumValue, int maximumValue, int defaultValue, unsigned int rangeScaleFactor>
    struct LinearFloatParameterTraitsBase
        :
        LinearParameterTraitsBase<minimumValue, maximumValue, defaultValue>
    {
    public: // Types.
        using Tag = LinearFloatParameterTag;

        using value_type = float;
    #if defined( _MSC_VER ) && !( defined( _M_AMD64 ) || ( _M_IX86_FP >= 2 ) )
        using param_type = value_type const &;
    #else
        using param_type = value_type;
    #endif // _MSC_VER

    public: // Values
        static unsigned int BOOST_CONSTEXPR_OR_CONST rangeValuesDenominator = rangeScaleFactor;

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
    }; // struct LinearFloatParameterTraitsBase

    template <class Traits, class... DefaultsParam> struct LinearFloatParameterTraits;
    template <class... TraitsParam, class... DefaultsParam>
    struct LinearFloatParameterTraits<TraitPack<TraitsParam...>, DefaultsParam...>
        :
        LinearFloatParameterTraitsBase
        <
            GetTraitDefaulted<Traits::Tag::Minimum          , TraitPack<TraitsParam...>, DefaultsParam...>::type::value,
            GetTraitDefaulted<Traits::Tag::Maximum          , TraitPack<TraitsParam...>, DefaultsParam...>::type::value,
            GetTraitDefaulted<Traits::Tag::Default          , TraitPack<TraitsParam...>, DefaultsParam...>::type::value,
            GetTraitDefaulted<Traits::Tag::ValuesDenominator, TraitPack<TraitsParam...>, DefaultsParam...>::type::value
        >
    {
    public:
        using Traits   = TraitPack<TraitsParam  ...>;
        using Defaults = TraitPack<DefaultsParam...>;

        template <class... NewTraits>
        struct Modify
        {
            using type = LinearFloatParameterTraits<TraitPack<NewTraits...>, TraitsParam...>;
        };
    }; // struct LinearFloatParameterTraits

    template <typename T, class Traits, class... DefaultsParam> struct LinearIntegerParameterTraits;
    template <typename T, class... TraitsParam, class... DefaultsParam>
    struct LinearIntegerParameterTraits<T, TraitPack<TraitsParam...>, DefaultsParam...>
        :
        LinearParameterTraitsBase
        <
            GetTraitDefaulted<Traits::Tag::Minimum, TraitPack<TraitsParam...>, DefaultsParam...>::type::value,
            GetTraitDefaulted<Traits::Tag::Maximum, TraitPack<TraitsParam...>, DefaultsParam...>::type::value,
            GetTraitDefaulted<Traits::Tag::Default, TraitPack<TraitsParam...>, DefaultsParam...>::type::value
        >
    {
    public: // Types.
        using Tag = LinearIntegerParameterTag;

        using value_type = T         ;
        using param_type = value_type;

        using Traits   = TraitPack<TraitsParam  ...>;
        using Defaults = TraitPack<DefaultsParam...>;

        template <class... NewTraits>
        struct Modify
        {
            using type = LinearIntegerParameterTraits<T, TraitPack<NewTraits...>, TraitsParam...>;
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
    }; // LinearIntegerParameterTraits

    template <class Traits, class... Defaults>
    using LinearSignedIntegerParameterTraits   = LinearIntegerParameterTraits<std:: int16_t, Traits, Defaults...>;

    template <class Traits, class... Defaults>
    using LinearUnsignedIntegerParameterTraits = LinearIntegerParameterTraits<std::uint16_t, Traits, Defaults...>;
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct LinearSignedInteger
////////////////////////////////////////////////////////////////////////////////

struct LinearSignedInteger
{
    template <class... NewTraits>
    struct Modify
    {
        using type = Detail::LinearSignedIntegerParameterTraits
        <
            TraitPack<NewTraits...>, Traits::Unit<0>
        >;
    };
}; // struct LinearSignedInteger


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct LinearUnsignedInteger
////////////////////////////////////////////////////////////////////////////////

struct LinearUnsignedInteger
{
    template <class... NewTraits>
    struct Modify
    {
        using type = Detail::LinearUnsignedIntegerParameterTraits
        <
            TraitPack<NewTraits...>, Traits::Unit<0>
        >;
    };
}; // struct LinearUnsignedInteger


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct LinearFloat
////////////////////////////////////////////////////////////////////////////////

struct LinearFloat
{
    template <class... NewTraits>
    struct Modify
    {
        using type = Detail::LinearFloatParameterTraits
        <
            TraitPack<NewTraits...>, Traits::ValuesDenominator<1>, Traits::Unit<0>
        >;
    };
}; // struct LinearFloat

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
