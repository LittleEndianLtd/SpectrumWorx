////////////////////////////////////////////////////////////////////////////////
///
/// \file powerOfTwo/parameter.hpp
/// ------------------------------
///
/// Copyright (c) 2011 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameter_hpp__8CF0596B_4588_4B99_8488_5F77415359B
#define parameter_hpp__8CF0596B_4588_4B99_8488_5F77415359B
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/parameters/parameter.hpp"

#include "boost/mpl/at.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Detail ///< \internal
{
    template <class ArgumentPack>
    struct PowerOfTwoParameterTraits
    {
    public:
        typedef PowerOfTwoParameterTag Tag;

        typedef unsigned int value_type;
        typedef value_type   param_type;

        static value_type const unscaledMinimum = boost::mpl::at<ArgumentPack, Traits::Tag::Minimum>::type::value;
        static value_type const unscaledMaximum = boost::mpl::at<ArgumentPack, Traits::Tag::Maximum>::type::value;
        static value_type const unscaledDefault = boost::mpl::at<ArgumentPack, Traits::Tag::Default>::type::value;
        static value_type const rangeValuesDenominator = 1;

        static value_type minimum () { return unscaledMinimum; }
        static value_type maximum () { return unscaledMaximum; }
        static value_type default_() { return unscaledDefault; }

        static value_type const discreteValueDistance = 0;

        static bool isValidValue( param_type const value )
        {
            return
                Math::isValueInRange( value, minimum(), maximum() ) &&
                Math::isPowerOfTwo  ( value                       );
        }

        typedef boost::mpl::map1<Traits::Unit<0, 0>> Defaults;
        typedef ArgumentPack                         Traits  ; //...mrmlj...FMOD param info...

    protected:
        static void increment( value_type & value ) { value *= 2; }
        static void decrement( value_type & value ) { value /= 2; }

    private:
        static_assert( Math::IsPowerOfTwo<unscaledMinimum>::value, "Internal inconsistency" );
        static_assert( Math::IsPowerOfTwo<unscaledMaximum>::value, "Internal inconsistency" );
        static_assert( Math::IsPowerOfTwo<unscaledDefault>::value, "Internal inconsistency" );

        static_assert( unscaledMinimum <= unscaledMaximum, "Internal inconsistency" );
        static_assert( unscaledDefault <= unscaledMaximum, "Internal inconsistency" );
        static_assert( unscaledMinimum <= unscaledDefault, "Internal inconsistency" );
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class PowerOfTwoParameter
////////////////////////////////////////////////////////////////////////////////

template
<
    typename Trait0,
    typename Trait1,
    typename Trait2
>
class PowerOfTwoParameter
    :
    public Parameter<Detail::PowerOfTwoParameterTraits<boost::mpl::map3<Trait0, Trait1, Trait2> > >
{
public:
    explicit PowerOfTwoParameter( unsigned int const initialValue = PowerOfTwoParameter::default_() )
        : PowerOfTwoParameter::type( initialValue ) {}
};

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
