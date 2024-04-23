////////////////////////////////////////////////////////////////////////////////
///
/// \file powerOfTwo/conversion.hpp
/// -------------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef conversion_hpp__8CF0596B_4588_4B99_8488_5F77415359B
#define conversion_hpp__8CF0596B_4588_4B99_8488_5F77415359B
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/math/math.hpp"
#include "le/math/conversion.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Detail
{
    template
    <
        typename Source,
                 int sourceRangeOffset,
        unsigned int sourceRangeSize,
        unsigned int sourceRangeScaleFactor,
        class Parameter,
        typename Target
    >
    Target
    convertLinearValueToParameterValue( Source const sourceValue, PowerOfTwoParameterTag )
    {
        return Math::convertLinearRange2PowerOfTwo
        <
            Parameter::unscaledMinimum,
            Parameter::unscaledMaximum,
            Source, sourceRangeOffset, sourceRangeSize, sourceRangeScaleFactor
        >( sourceValue );
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
    Target
    convertParameterValueToLinearValue( Source const sourceValue, PowerOfTwoParameterTag )
    {
        return Math::convertPowerOfTwo2LinearRange
        <
            Target, targetRangeOffset, targetRangeSize, targetRangeScaleFactor,
            Parameter::unscaledMinimum,
            Parameter::unscaledMaximum
        >( sourceValue );
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // conversion_hpp
