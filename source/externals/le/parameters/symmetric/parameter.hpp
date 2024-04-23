////////////////////////////////////////////////////////////////////////////////
///
/// \file symmetric/parameter.hpp
/// -----------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( __clang__ ) && ( _MSC_VER < 1800 )
#include "le/parameters/cpp03/symmetric/parameter.hpp"
#endif // old MSVC
//------------------------------------------------------------------------------
#ifndef parameter_hpp__538CC108_800B_442B_882C_AD3800171536
#define parameter_hpp__538CC108_800B_442B_882C_AD3800171536
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/linear/parameter.hpp"

#include <boost/assert.hpp>
#include <boost/mpl/pair.hpp>

#include <cstdint>
#include <type_traits>
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

namespace Tag { struct MaximumOffset; }

template <std::uint16_t value>
using MaximumOffset = boost::mpl::pair<Tag::MaximumOffset, std::integral_constant<std::int16_t, value>>;

//------------------------------------------------------------------------------
} // namespace Traits

namespace Detail ///< \internal
{
    template <class Traits, class... Defaults> struct SymmetricFloatParameterTraits;
    template <class... TraitsParam, class... DefaultsParam>
    struct SymmetricFloatParameterTraits<TraitPack<TraitsParam...>, DefaultsParam...>
        :
        LinearFloatParameterTraitsBase
        <
            0 - static_cast<std::int16_t>( GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitPack<TraitsParam...>, DefaultsParam...>::type::value ),
            0 +                            GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitPack<TraitsParam...>, DefaultsParam...>::type::value  ,
            0,
            GetTraitDefaulted<Traits::Tag::ValuesDenominator, TraitPack<TraitsParam...>, DefaultsParam...>::type::value
        >
    {
    public:
        using Tag = SymmetricFloatParameterTag;

        using Traits   = TraitPack<TraitsParam  ...>;
        using Defaults = TraitPack<DefaultsParam...>;

        template <class... NewTraits>
        struct Modify
        {
            using type = SymmetricFloatParameterTraits<TraitPack<NewTraits...>, TraitsParam...>;
        };
    };

    template <class Traits, class... Defaults> struct SymmetricIntegerParameterTraits;
    template <class... TraitsParam, class... DefaultsParam>
    struct SymmetricIntegerParameterTraits<TraitPack<TraitsParam...>, DefaultsParam...>
        :
        LinearParameterTraitsBase
        <
            0 - static_cast<int>( GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitPack<TraitsParam...>, DefaultsParam...>::type::value ),
            0 +                   GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitPack<TraitsParam...>, DefaultsParam...>::type::value  ,
            0
        >
    {
    public: // Types.
        using Tag = SymmetricIntegerParameterTag;

        using value_type = std::int16_t;
        using param_type = value_type  ;

        using Traits   = TraitPack<TraitsParam  ...>;
        using Defaults = TraitPack<DefaultsParam...>;

        template <class... NewTraits>
        struct Modify
        {
            using type = SymmetricIntegerParameterTraits<TraitPack<NewTraits...>, TraitsParam...>;
        };

    public: // Values
        static unsigned int const rangeValuesDenominator = 1;

        static value_type minimum () { return SymmetricIntegerParameterTraits::unscaledMinimum; }
        static value_type maximum () { return SymmetricIntegerParameterTraits::unscaledMaximum; }
        static value_type default_() { return SymmetricIntegerParameterTraits::unscaledDefault; }

        static unsigned int const discreteValueDistance = 1;

        static bool isValidValue( value_type const value )
        {
            return isValueInRange<param_type>( value, minimum(), maximum() );
        }
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct SymmetricInteger
////////////////////////////////////////////////////////////////////////////////

struct SymmetricInteger
{
    template <class... NewTraits>
    struct Modify
    {
        using type = Detail::SymmetricIntegerParameterTraits
        <
            TraitPack<NewTraits...>, Traits::ValuesDenominator<1>, Traits::Unit<0>
        >;
    };
};


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct SymmetricFloat
////////////////////////////////////////////////////////////////////////////////

struct SymmetricFloat
{
    template <class... NewTraits>
    struct Modify
    {
        using type = Detail::SymmetricFloatParameterTraits
        <
            TraitPack<NewTraits...>, Traits::ValuesDenominator<1>, Traits::Unit<0>
        >;
    };
};

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
