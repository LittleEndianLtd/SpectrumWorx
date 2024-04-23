////////////////////////////////////////////////////////////////////////////////
///
/// \file channelBuffers.hpp
/// ------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef channelBuffers_hpp__604836C8_E697_4307_848D_CF2026BF6E99
#define channelBuffers_hpp__604836C8_E697_4307_848D_CF2026BF6E99
#pragma once
//------------------------------------------------------------------------------
#include "channelData.hpp"

#include "le/utility/buffers.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

class ChannelBuffers
{
public:
    std::uint16_t inputDataSize      () const { return inputOLAPosition_; }
    std::uint16_t readyOutputDataSize() const;

    void addNewData
    (
        float const * LE_RESTRICT & pNewMainChannelData,
        float const * LE_RESTRICT & pNewSideChannelData,
        std::uint16_t               sizeToCopy,
        bool                        useSideChannel
    );

    void setCurrentDataToChannelData
    (
        bool                            useSideChannel,
        Math::FFT_float_real_1D const & fft,
        ReadOnlyDataRange       const & window,
        std::uint8_t                    windowSizeFactor
    );

    float * putNewTimeDomainDataToOutput
    (
        Math::FFT_float_real_1D const & fft,
        ReadOnlyDataRange       const & window,
        std::uint8_t                    windowSizeFactor
    );

    void moveForwardByHopSize( std::uint16_t hopSize, bool useSideChannel );

    void extractChunkOfReadyOutputData
    (
        float         * pTargetBuffer,
        std::uint16_t   chunkSize,
        std::uint16_t   incompleteOutputOLASamples
    );

    float * inputBuffer();

    std::uint16_t outputBufferSize() const { return static_cast<std::uint16_t>( outputOLA_.size() ); }

    ChannelData       & channelData()       { return channelData_; }
    ChannelData const & channelData() const { return channelData_; }

    void reset( std::uint16_t initialSilenceSamples );
    void resize( StorageFactors const &, Storage & );
    static LE_CONST_FUNCTION std::uint32_t requiredStorage( StorageFactors const & );

private:
    std::uint16_t inputOLAPosition_ ;
    std::uint16_t outputOLAPosition_;

    ChannelData channelData_;

    using MainOLA = Engine::WindowBuffer<real_t>; MainOLA mainOLA_;
    using SideOLA = Engine::WindowBuffer<real_t>; SideOLA sideOLA_;

    struct OutputOLA : Utility::SharedStorageBuffer<real_t>
    {
        static LE_CONST_FUNCTION std::uint32_t requiredStorage( StorageFactors const & );

        void resize( StorageFactors const & factors, Storage & storage )
        {
            Utility::SharedStorageBuffer<real_t>::resize( requiredStorage( factors ), storage );
        }
    }; // struct OutputOLA
    OutputOLA outputOLA_;
}; // class ChannelBuffers

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // channelBuffers_hpp
