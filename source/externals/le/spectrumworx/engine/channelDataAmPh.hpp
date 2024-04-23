////////////////////////////////////////////////////////////////////////////////
///
/// \file channelDataAmPh.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef channelDataAmPh_hpp__F6AD46F5_F0FA_487F_8EF9_0889B8FD9BCB
#define channelDataAmPh_hpp__F6AD46F5_F0FA_487F_8EF9_0889B8FD9BCB
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/engine/buffers.hpp"

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
namespace PhaseVocoderShared
{
    struct AnalysisChannelState ;
    struct SynthesisChannelState;
} // namespace PhaseVocoderShared
} // namespace Effects

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class FullChannelData_AmPh
///
/// \brief Holds amplitude and phase of the Fourier spectrum for frequency 
///  domain effects.
///
////////////////////////////////////////////////////////////////////////////////

class FullChannelData_AmPh : public Engine::SharedStorageHalfFFTBufferPair
{
public:
    DataRange         const & amps  ()       { return first (); }
    DataRange         const & phases()       { return second(); }

    ReadOnlyDataRange const & amps  () const { return first (); }
    ReadOnlyDataRange const & phases() const { return second(); }
}; // class FullChannelData_AmPh


////////////////////////////////////////////////////////////////////////////////
///
/// \class ChannelData_AmPh
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

class ChannelData_AmPh : public SubRange<FullChannelData_AmPh, DataRange>
{
public:
    ChannelData_AmPh( FullChannelData_AmPh & data, IndexRange const & workingRange );

    DataRange         const & amps  ()       { return first (); }
    DataRange         const & phases()       { return second(); }

    ReadOnlyDataRange const & amps  () const { return first (); }
    ReadOnlyDataRange const & phases() const { return second(); }

#ifdef LE_PV_USE_TSS
    //...mrmlj...quick temporary workaround to enable PVD effects to work with
    //...mrmlj...with TSS enabled...
    Effects::PhaseVocoderShared::AnalysisChannelState  const * LE_RESTRICT pAnalysisState ;
    Effects::PhaseVocoderShared::SynthesisChannelState       * LE_RESTRICT pSynthesisState;
#endif // LE_PV_USE_TSS
}; // class ChannelData_AmPh

#pragma warning( pop )


////////////////////////////////////////////////////////////////////////////////
///
/// \class FullMainSideChannelData_AmPh
///
////////////////////////////////////////////////////////////////////////////////

using FullMainSideChannelData_AmPh = MainSide<FullChannelData_AmPh                                    >;
using MainSideChannelData_AmPh     = MainSide<SubRange<FullMainSideChannelData_AmPh, ChannelData_AmPh>>;


#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

//...mrmlj...temporary workaround until engine refactoring is finished...
//...mrmlj...http://lists.cs.uiuc.edu/pipermail/cfe-dev/2010-December/012702.html
class ChannelData_AmPhStorage
    :
    private FullChannelData_AmPh,
    public  ChannelData_AmPh
{
private:
    FullChannelData_AmPh & constructFull( Engine::StorageFactors const & storageFactors, Storage & storage )
    {
        full().resize( storageFactors, storage );
        return full();
    }

public:
    ChannelData_AmPhStorage
    (
        std::uint16_t fftSize,
        std::uint16_t beginBin,
        std::uint16_t endBin,
        Storage       storage
    );

    using ChannelData_AmPh::amps        ;
    using ChannelData_AmPh::phases      ;
    using ChannelData_AmPh::numberOfBins;

    FullChannelData_AmPh & full() { return *this; }

    static std::uint32_t requiredStorage( std::uint16_t fftSize );
};

#pragma warning( pop )


DataRange subRange( DataRange const &, std::uint16_t beginIndex, std::uint16_t endIndex );

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // channelDataAmPh_hpp
