////////////////////////////////////////////////////////////////////////////////
///
/// \file linear/conversion.hpp
/// ---------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef conversion_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#define conversion_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/math/conversion.hpp"

#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Detail
{
    // Implementation note:
    //   Using the 'semi-compile-time' versions of the Math range conversion
    // functions slightly improves build times and actually produces slightly
    // smaller code.
    //                                        (22.03.2011.) (Domagoj Saric)

    template
    <
                 int sourceRangeOffset,
        unsigned int sourceRangeSize,
        unsigned int sourceRangeScaleFactor,
        class Parameter
    >
    struct DirectlyConvertible : std::integral_constant
    <
        bool,
        sourceRangeOffset      ==   Parameter::unscaledMinimum                                &&
        sourceRangeSize        == ( Parameter::unscaledMaximum - Parameter::unscaledMinimum ) &&
        sourceRangeScaleFactor ==   Parameter::rangeValuesDenominator
    >
    {};

    template
    <
        typename Source,
                 int sourceRangeOffset,
        unsigned int sourceRangeSize,
        unsigned int sourceRangeScaleFactor,
        class Parameter,
        typename Target
    >
    typename std::enable_if<DirectlyConvertible<sourceRangeOffset, sourceRangeSize, sourceRangeScaleFactor, Parameter>::value, Target>::type
    convertLinearValueToParameterValue( Source const sourceValue, LinearParameterTag )
    {
         return Math::convert<Target>( sourceValue );
    }

    template
    <
        typename Source,
                 int sourceRangeOffset,
        unsigned int sourceRangeSize,
        unsigned int sourceRangeScaleFactor,
        class Parameter,
        typename Target
    >
    typename std::enable_if<!DirectlyConvertible<sourceRangeOffset, sourceRangeSize, sourceRangeScaleFactor, Parameter>::value, Target>::type
    convertLinearValueToParameterValue( Source const sourceValue, LinearParameterTag )
    {
        
        return Math::convertLinearRange
        <
            Target,
            Source,
            sourceRangeOffset,
            sourceRangeSize,
            sourceRangeScaleFactor
        >( sourceValue, static_cast<Target>( Parameter::minimum() ), static_cast<Target>( Parameter::maximum() ) );
    }


    template
    <
        typename Target,
                 int targetRangeOffset,
        unsigned int targetRangeSize,
        unsigned int targetRangeScaleFactor,
        class Parameter,
        typename Source
    >
    typename std::enable_if<DirectlyConvertible<targetRangeOffset, targetRangeSize, targetRangeScaleFactor, Parameter>::value, Target>::type
    convertParameterValueToLinearValue( Source const sourceValue, LinearParameterTag )
    {
        return Math::convert<Target>( sourceValue );
    }
    template
    <
        typename Target,
                 int targetRangeOffset,
        unsigned int targetRangeSize,
        unsigned int targetRangeScaleFactor,
        class Parameter,
        typename Source
    >
    typename std::enable_if<!DirectlyConvertible<targetRangeOffset, targetRangeSize, targetRangeScaleFactor, Parameter>::value, Target>::type
    convertParameterValueToLinearValue( Source const sourceValue, LinearParameterTag )
    {
        return Math::convertLinearRange
        <
            Target,
            targetRangeOffset,
            targetRangeSize,
            targetRangeScaleFactor,
            Source
        >( sourceValue, static_cast<Source>( Parameter::minimum() ), static_cast<Source>( Parameter::maximum() ) );
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // conversion_hpp
