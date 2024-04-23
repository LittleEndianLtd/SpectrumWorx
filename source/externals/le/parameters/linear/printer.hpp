////////////////////////////////////////////////////////////////////////////////
///
/// \file printer.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef printer_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#define printer_hpp__AC776212_E8E5_4781_922F_DC16639D2364
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/printer_fwd.hpp"
#include "le/utility/lexicalCast.hpp"

#include "boost/mpl/string.hpp"

#include <cstdio>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Detail
{
    template <class TraitTag, class Traits, class... DefaultTraits>
    struct GetTraitDefaulted;

    template <typename Source>
    LE_NOTHROWNOALIAS char const * LE_FASTCALL
    printLinear
    (
        char         * const buffer        ,
        Source const &       parameterValue,
        LinearFloatParameterTag const &
    )
    {
        Utility::lexical_cast( parameterValue, 1, buffer );
        return buffer;
    }

    template <typename Source>
    typename boost::enable_if<std::is_integral<Source>, char const *>::type
    LE_FORCEINLINE
    printLinear
    (
        char   * const buffer        ,
        Source   const parameterValue,
        LinearIntegerParameterTag const &
    )
    {
        Utility::lexical_cast( parameterValue, buffer );
        return buffer;
    }

    template <typename Source>
    typename boost::enable_if<std::is_floating_point<Source>, char const *>::type
    LE_FASTCALL
    printLinear
    (
        char         * const buffer        ,
        Source const &       parameterValue,
        LinearIntegerParameterTag const &
    )
    {
        Utility::lexical_cast( parameterValue, 0, buffer );
        return buffer;
    }

    template <class Parameter, typename Source>
    char const * LE_FASTCALL print
    (
        Source            const & parameterValue,
        SW::Engine::Setup const & engineSetup   ,
        PrintBuffer       const & buffer        ,
        LinearParameterTag const &
    )
    {
        typedef DisplayValueTransformer<Parameter> ValueTransformer;

        return printLinear
        (
            buffer.begin(),
            ValueTransformer::transform( parameterValue, engineSetup ),
            typename Parameter::Tag()
        );
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // printer_hpp
