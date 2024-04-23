////////////////////////////////////////////////////////////////////////////////
///
/// \file effectNames.hpp
/// ---------------------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef effectTypeNames_hpp__86C2F997_AEC7_485D_B9C3_9EE34628192D
#define effectTypeNames_hpp__86C2F997_AEC7_485D_B9C3_9EE34628192D
#pragma once
//------------------------------------------------------------------------------
#include "boost/utility/string_ref_fwd.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

char const * LE_FASTCALL effectIndex2TypeName( std::uint8_t      effectIndex    );
std::uint8_t LE_FASTCALL effectTypeName2Index( boost::string_ref effectTypeName );

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // effectTypeNames_hpp