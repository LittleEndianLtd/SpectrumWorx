////////////////////////////////////////////////////////////////////////////////
///
/// \file printer.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef printer_hpp__D017E02E_44C0_40FE_BC6B_4493C53BF028
#define printer_hpp__D017E02E_44C0_40FE_BC6B_4493C53BF028
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/printer_fwd.hpp"
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
        bool                      const parameterValue,
        SW::Engine::Setup const &                     ,
        PrintBuffer       const &                     ,
        BooleanParameterTag
    )
    {
        return parameterValue ? "yes" : "no";
    }

    template <class Parameter>
    char const * print
    (
        float             const & parameterValue,
        SW::Engine::Setup const &,
        PrintBuffer       const &               ,
        BooleanParameterTag
    )
    {
        return Math::round( parameterValue ) ? "yes" : "no";
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // printer_hpp
