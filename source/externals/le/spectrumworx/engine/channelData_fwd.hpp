////////////////////////////////////////////////////////////////////////////////
///
/// \file channelData_fwd.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef channelData_fwd_hpp__A3D62820_9F64_4D13_AA59_70401537C88E
#define channelData_fwd_hpp__A3D62820_9F64_4D13_AA59_70401537C88E
#pragma once
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{
    template <class Data                               > class MainSide;
    template <class FullRangeData, class SubRangeHolder> class SubRange;
} // namespace Engine
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

class ChannelData_AmPh;
class ChannelData_ReIm;

class FullChannelData_AmPh;
class FullChannelData_ReIm;

using FullMainSideChannelData_AmPh = MainSide<FullChannelData_AmPh>;
using FullMainSideChannelData_ReIm = MainSide<FullChannelData_ReIm>;

using     MainSideChannelData_AmPh = MainSide<SubRange<FullMainSideChannelData_AmPh, ChannelData_AmPh>>;
using     MainSideChannelData_ReIm = MainSide<SubRange<FullMainSideChannelData_ReIm, ChannelData_ReIm>>;

struct ChannelData_AmPh2ReIm;
struct ChannelData_ReIm2AmPh;

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // channelData_fwd_hpp
