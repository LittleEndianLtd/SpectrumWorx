////////////////////////////////////////////////////////////////////////////////
///
/// \file printer.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef printer_hpp__8CF0596B_4588_4B99_8488_5F77415359B
#define printer_hpp__8CF0596B_4588_4B99_8488_5F77415359B
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/linear/printer.hpp"

#include "le/utility/cstdint.hpp"
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
	char const * print
    (
        std::uint16_t             const parameterValue,
        SW::Engine::Setup const &                     ,
        PrintBuffer       const &       buffer        ,
        PowerOfTwoParameterTag
    )
    {
        return printLinear( buffer.begin(), parameterValue, LinearIntegerParameterTag() );
    }

    template <class Parameter>
	char const * print
    (
        float                     const parameterValue,
        SW::Engine::Setup const &                     ,
        PrintBuffer       const &       buffer        ,
        PowerOfTwoParameterTag
    )
    {
        return printLinear( buffer.begin(), parameterValue, LinearIntegerParameterTag() );
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // printer_hpp
