////////////////////////////////////////////////////////////////////////////////
///
/// \file domainConversion.hpp
/// --------------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef domainConversion_hpp__8429E54E_2E4F_4552_8B77_DE27CCF355F2
#define domainConversion_hpp__8429E54E_2E4F_4552_8B77_DE27CCF355F2
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

void LE_FASTCALL reim2AmPh
(
    float const * reals     , float const * imags ,  // input
    float       * amplitudes, float       * phases,  // output
    std::uint16_t numberOfSamples
);

void LE_FASTCALL amph2ReIm
(
    float const * amplitudes, float const * phases,  // input
    float       * reals     , float       * imags ,  // output
    std::uint16_t numberOfSamples
);

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // domainConversion_hpp
