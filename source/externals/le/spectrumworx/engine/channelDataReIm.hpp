////////////////////////////////////////////////////////////////////////////////
///
/// \file channelDataReIm.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef channelDataReIm_hpp__E139EF92_51AE_4817_A166_E1B3399EDAE1
#define channelDataReIm_hpp__E139EF92_51AE_4817_A166_E1B3399EDAE1
#pragma once
//------------------------------------------------------------------------------
#include "buffers.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class FullChannelData_ReIm
///
/// \brief Holds real and imaginary components of the Fourier spectrum for 
///  frequency domain effects.
///
////////////////////////////////////////////////////////////////////////////////

class FullChannelData_ReIm : public SharedStorageHalfFFTBufferPair
{
public:
    DataRange         const & reals()       { return first (); }
    DataRange         const & imags()       { return second(); }

    ReadOnlyDataRange const & reals() const { return first (); }
    ReadOnlyDataRange const & imags() const { return second(); }
};


#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

class ChannelData_ReIm : public SubRange<FullChannelData_ReIm, DataRange>
{
public:
    ChannelData_ReIm( FullChannelData_ReIm & data, IndexRange const & workingRange );

    DataRange         const & reals()       { return first (); }
    DataRange         const & imags()       { return second(); }

    ReadOnlyDataRange const & reals() const { return first (); }
    ReadOnlyDataRange const & imags() const { return second(); }
};

#pragma warning( pop )

using FullMainSideChannelData_ReIm = MainSide<FullChannelData_ReIm>;
using     MainSideChannelData_ReIm = MainSide<SubRange<FullMainSideChannelData_ReIm, ChannelData_ReIm>>;

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // channelDataReIm_hpp
