////////////////////////////////////////////////////////////////////////////////
///
/// \file factory.hpp
/// -----------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleFactory_hpp__C444656C_70DA_479E_8BB5_C889A9B1EFA5
#define moduleFactory_hpp__C444656C_70DA_479E_8BB5_C889A9B1EFA5
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/cstdint.hpp"

#include "boost/smart_ptr/intrusive_ptr.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

struct ModuleFactory
{
    template <class ModuleInterface>
    static LE_NOTHROW boost::intrusive_ptr<ModuleInterface> LE_FASTCALL create( std::int8_t effectIndex );
}; // struct ModuleFactory

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleFactory_hpp
