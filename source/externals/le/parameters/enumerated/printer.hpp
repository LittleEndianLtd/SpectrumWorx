////////////////////////////////////////////////////////////////////////////////
///
/// \file printer.hpp
/// -----------------
///
/// Copyright (c) 2012. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef printer_hpp__5820E6B3_7684_4DF4_BC99_B0A5CCB0F3E9
#define printer_hpp__5820E6B3_7684_4DF4_BC99_B0A5CCB0F3E9
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/math/conversion.hpp"

#include "le/parameters/printer_fwd.hpp"
#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Detail
{
    template <class Parameter>
    static char const * print
    (
        typename Parameter::param_type const   parameterValue,
        SW::Engine::Setup              const &,
        PrintBuffer                    const &               ,
        EnumeratedParameterTag
    )
    {
        return DiscreteValues<Parameter>::strings[ parameterValue ];
    }

    template <class Parameter, typename Source>
    static char const * print
    (
        Source            const & parameterValue,
        SW::Engine::Setup const & engineSetup   ,
        PrintBuffer       const & buffer        ,
        EnumeratedParameterTag
    )
    {
        return print<Parameter>( Math::convert<typename Parameter::binary_type>( parameterValue ), engineSetup, buffer, EnumeratedParameterTag() );
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // printer_hpp
