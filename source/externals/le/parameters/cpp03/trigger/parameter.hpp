////////////////////////////////////////////////////////////////////////////////
///
/// \file trigger/parameter.hpp
/// ---------------------------
///
/// Copyright (c) 2011 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameter_hpp__52B1FD32_BCD7_416A_8B25_1A640BF6E636
#define parameter_hpp__52B1FD32_BCD7_416A_8B25_1A640BF6E636
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/boolean/parameter.hpp"
#include "le/parameters/parameter.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class TriggerParameter
////////////////////////////////////////////////////////////////////////////////

class TriggerParameter : public Boolean
{
public:
    typedef TriggerParameterTag Tag;

public:
    TriggerParameter( param_type const initialValue = false )
        : Boolean( initialValue ) {}

    LE_NOTHROWNOALIAS value_type consumeValue(            ) const;
    LE_NOTHROWNOALIAS void       setValue    ( param_type );

private:
    operator value_type const &() const;
    void operator++();
    void operator--();
}; // class TriggerParameter

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
