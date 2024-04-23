////////////////////////////////////////////////////////////////////////////////
///
/// \file printer.hpp
/// -----------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef printer_hpp__616A36CA_66D2_41CD_B907_0BA22A99E3EA
#define printer_hpp__616A36CA_66D2_41CD_B907_0BA22A99E3EA
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/printer_fwd.hpp"
#include "le/utility/lexicalCast.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

/// \todo Yet to be fully and properly implemented.
///                                           (11.03.2011.) (Domagoj Saric)

namespace Detail
{
    template <class Parameter>
    char const * print
    (
        typename Parameter::value_type         const parameterValue,
        SW::Engine::Setup              const &                     ,
        PrintBuffer                    const &       buffer        ,
        DynamicRangeParameterTag
    )
    {
        BOOST_VERIFY( Utility::lexical_cast( parameterValue, buffer.begin() ) < unsigned( buffer.size() ) );
        return buffer.begin();
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // printer_hpp
