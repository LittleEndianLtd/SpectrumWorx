////////////////////////////////////////////////////////////////////////////////
///
/// \file symmetric/parameter.hpp
/// -----------------------------
///
/// Copyright (c) 2011 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameter_hpp__538CC108_800B_442B_882C_AD3800171536
#define parameter_hpp__538CC108_800B_442B_882C_AD3800171536
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/linear/parameter.hpp"

#include "boost/assert.hpp"
#include "boost/mpl/integral_c.hpp"
#include "boost/mpl/map/map10.hpp"
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

template <unsigned int value>
struct MaximumOffset
    : boost::mpl::pair<Tag::MaximumOffset, boost::mpl::integral_c<unsigned int, value> >
{};

//------------------------------------------------------------------------------
} // namespace Traits

namespace Detail ///< \internal
{
    template <class TraitsParam, class DefaultsParam = boost::mpl::map0<> >
    struct SymmetricFloatParameterTraits
        :
        LinearFloatParameterTraitsBase
        <
            0 - static_cast<int>( GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitsParam, DefaultsParam>::type::value ),
            0 +                   GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitsParam, DefaultsParam>::type::value  ,
            0,
            GetTraitDefaulted<Traits::Tag::ValuesDenominator, TraitsParam, DefaultsParam>::type::value
        >
    {
    public:
        typedef SymmetricFloatParameterTag Tag;

        typedef TraitsParam   Traits  ;
        typedef DefaultsParam Defaults;

        template <class NewTraits>
        struct Modify
        {
            typedef SymmetricFloatParameterTraits<NewTraits, Traits> type;
        };
    };

    template <class TraitsParam, class DefaultsParam = boost::mpl::map0<> >
    struct SymmetricIntegerParameterTraits
        :
        LinearParameterTraitsBase
        <
            0 - static_cast<int>( GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitsParam, DefaultsParam>::type::value ),
            0 +                   GetTraitDefaulted<Traits::Tag::MaximumOffset, TraitsParam, DefaultsParam>::type::value  ,
            0
        >
    {
    public: // Types.
        typedef SymmetricIntegerParameterTag Tag;

        typedef std::int16_t value_type;
        typedef value_type   param_type;

        typedef TraitsParam   Traits  ;
        typedef DefaultsParam Defaults;

        template <class NewTraits>
        struct Modify
        {
            typedef SymmetricIntegerParameterTraits<NewTraits, Traits> type;
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
    template <class NewTraits>
    struct Modify
    {
        typedef Detail::SymmetricIntegerParameterTraits
        <
            NewTraits,
            boost::mpl::map2<Traits::ValuesDenominator<1>, Traits::Unit<0> >
        > type;
    };
};


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct SymmetricFloat
////////////////////////////////////////////////////////////////////////////////

struct SymmetricFloat
{
    template <class NewTraits>
    struct Modify
    {
        typedef Detail::SymmetricFloatParameterTraits
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
