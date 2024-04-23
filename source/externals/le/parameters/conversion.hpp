////////////////////////////////////////////////////////////////////////////////
///
/// \file conversion.hpp
/// --------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef conversion_hpp__C3D317BC_661C_4BCF_B057_8F2BB9D89B75
#define conversion_hpp__C3D317BC_661C_4BCF_B057_8F2BB9D89B75
#pragma once
//------------------------------------------------------------------------------
#include "boolean/conversion.hpp"
#include "dynamic/conversion.hpp"
#include "enumerated/conversion.hpp"
#include "linear/conversion.hpp"
#include "powerOfTwo/conversion.hpp"
#include "symmetric/conversion.hpp"
#include "trigger/conversion.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

#ifdef __GNUC__
namespace Detail
{
    struct DummyTag;

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
    convertLinearValueToParameterValue( Source const sourceValue, DummyTag );

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
    convertParameterValueToLinearValue( Source const sourceValue, DummyTag );
} // namespace Detail
#endif // __GNUC__


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
convertLinearValueToParameterValue( Source const sourceValue ) ///< \throws nothing
{
    return Detail::convertLinearValueToParameterValue<Source, sourceRangeOffset, sourceRangeSize, sourceRangeScaleFactor, Parameter, Target>
    (
        sourceValue,
        typename Parameter::Tag()
    );
}


template
<
    typename Source,
             int sourceRangeOffset,
    unsigned int sourceRangeSize,
    unsigned int sourceRangeScaleFactor,
    class Parameter
>
typename Parameter::value_type
convertLinearValueToParameterValue( Source const sourceValue ) ///< \throws nothing
{
    return Detail::convertLinearValueToParameterValue<Source, sourceRangeOffset, sourceRangeSize, sourceRangeScaleFactor, Parameter, typename Parameter::value_type>
    (
        sourceValue,
        typename Parameter::Tag()
    );
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
convertParameterValueToLinearValue( Source const sourceValue ) ///< \throws nothing
{
    return Detail::convertParameterValueToLinearValue<Target, targetRangeOffset, targetRangeSize, targetRangeScaleFactor, Parameter>
    (
        sourceValue,
        typename Parameter::Tag()
    );
}


template
<
    typename Target,
             int targetRangeOffset,
    unsigned int targetRangeSize,
    unsigned int targetRangeScaleFactor,
    class Parameter
>
Target
convertParameterValueToLinearValue( Parameter const & source ) ///< \throws nothing
{
    return Detail::convertParameterValueToLinearValue<Target, targetRangeOffset, targetRangeSize, targetRangeScaleFactor, Parameter>
    (
        source.getValue(),
        typename Parameter::Tag()
    );
}

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // conversion_hpp
