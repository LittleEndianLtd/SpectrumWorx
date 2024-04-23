////////////////////////////////////////////////////////////////////////////////
///
/// \file channelStateStatic.hpp
/// ----------------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef channelStateStatic_hpp__00473652_1B0C_4BDC_A10A_D2AA913A036A
#define channelStateStatic_hpp__00473652_1B0C_4BDC_A10A_D2AA913A036A
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace boost { template <class IteratorT> class iterator_range; }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

namespace Engine
{
    struct StorageFactors;
    typedef boost::iterator_range<char * LE_RESTRICT> Storage;
} // namespace Engine

//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

struct StaticChannelState
{
    static unsigned char requiredStorage( Engine::StorageFactors const &                          ) { return 0; }
    static void          resize         ( Engine::StorageFactors const &, Engine::Storage const & ) {}
}; // struct StaticChannelState

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // channelStateStatic_hpp
