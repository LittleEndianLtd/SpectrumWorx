////////////////////////////////////////////////////////////////////////////////
///
/// \file historyBuffer.hpp
/// -----------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef historyBuffer_hpp__DADB02E4_1F76_4685_BE34_89613A823BAD
#define historyBuffer_hpp__DADB02E4_1F76_4685_BE34_89613A823BAD
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/engine/buffers.hpp"
#include "le/spectrumworx/engine/configuration.hpp"
#include "le/utility/buffers.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/range/iterator_range_core.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
    unsigned int alignIndex( unsigned int index );
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class HistoryBuffer
///
////////////////////////////////////////////////////////////////////////////////

template <typename T, std::uint16_t milliSeconds>
struct HistoryBuffer : public Utility::SharedStorageBuffer<T>
{
    LE_NOTHROWNOALIAS LE_NOINLINE
    static std::uint32_t requiredStorage( Engine::StorageFactors const & factors )
    {
        // Implementation note:
        // N - number of required audio samples per second per channel
        //
        // N = fftSize * numberOfStepsInOneSecond
        //   = frameSize * zeroPadding * numberOfStepsInOneSecond
        //   = frameSize * zeroPadding * ( 1 / stepTime )
        //   = frameSize * zeroPadding * ( 1 / ( stepSize / sampleRate ) )
        //   = frameSize * zeroPadding * ( sampleRate / stepSize )
        //   = frameSize * zeroPadding * ( sampleRate / ( frameSize / overlap ) )
        //   = frameSize * zeroPadding * ( ( sampleRate * overlap ) / frameSize )
        //   = zeroPadding * sampleRate * overlap
        //                                          QeD
        //                                    (20.05.2010.) (Domagoj Saric)

        auto const frameSize        ( factors.fftSize                                               );
        auto const samples          ( ( milliSeconds * factors.samplerate + 999 ) / 1000            );
        auto const overlappedSamples( samples * factors.overlapFactor                               );
        auto const samplesRounded   ( overlappedSamples + frameSize - overlappedSamples % frameSize );

        BOOST_ASSERT( samplesRounded % frameSize == 0 );

        /// \note For each DFT frame we get "2 * ( DFT-size / 2 + 1 ) samples =
        /// DFT-size + 2 samples" (in ReIm or AmPh form). IOW for each frame we
        /// need storage for two additional samples in the frequency domain (for
        /// the DC and Nyquist imaginary zeros). Also each of the two complex
        /// component vectors (whether in ReIm or AmPh form) need to be vector
        /// aligned.
        ///                                   (16.07.2012.) (Domagoj Saric)
        auto const dftRepresentationOverhead( 2                                                                            );
        auto const maximumAlignmentPadding  ( Utility::Constants::vectorAlignment / sizeof( T ) - 1                        );
        auto const numberOfFrames           ( samplesRounded / frameSize                                                   );
        auto const overhead                 ( numberOfFrames * ( dftRepresentationOverhead + 2 * maximumAlignmentPadding ) );

        auto const storageBytes( ( samplesRounded + overhead ) * sizeof( T ) );
        return static_cast<std::uint32_t>( storageBytes );
    }

    void resize( Engine::StorageFactors const & factors, Engine::Storage & storage )
    {
        Utility::SharedStorageBuffer<T>::resize( requiredStorage( factors ), storage );
    }

    //...mrmlj...required for automatic reset() member function generation by
    //...mrmlj...the LE_DYNAMIC_CHANNEL_STATE macro...
    void reset() { this->clear(); }
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class ReversedHistoryBufferState
///
///   Utility ('step counter') class for implementing (time) reversed history
/// buffers with 'history emulation'. When there is not enough data in a history
/// buffer the history corresponding to a requested history step/frame will be
/// 'emulated' with existing history data (i.e. uninitialised or garbage history
/// data from previous uses will never be output, instead the available data
/// will be repeatedly traversed until enough history has been acquired).
///
////////////////////////////////////////////////////////////////////////////////
//
// Implementation note:
//   This class makes it possible to use a single buffer both for 'ready' chunks
// (for playback) and for (reversed) 'history' chunks. This is accomplished by
// traversing the buffer (i.e. incrementing  the counter) not in a circular mode
// but in a 'ping-pong' (left-to-right-to-left-to-right...) mode. When the
// direction of traversal changes we, in effect, start reading the signal
// history backwards. Note that this technique only reverses the order of
// individual chunks/steps/frames and not the data of/in the individual chunks.
// Because different ways of performing the time-reversal of individual frames
// will be better suited for different effects (usually some form of negating
// the imaginary or phase data) this part of the reversing process was left to
// be implemented elsewhere/in the actual effects.
//                                            (09.02.2011.) (Domagoj Saric)
//
////////////////////////////////////////////////////////////////////////////////

//...mrmlj...cleanup these duplicated typedefs (also in effects.hpp, channelDataReIm.hpp and fft.hpp)...
using         DataRange = boost::iterator_range<float       * LE_RESTRICT>;
using ReadOnlyDataRange = boost::iterator_range<float const * LE_RESTRICT>;

class ReversedHistoryBufferState
{
public:
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \struct HistoryData
    ///
    ///   Used for returning pointers to history data. Because of history
    /// emulation two pairs of pointers are returned. The 'target history' pair
    /// points to the location where the new history data should be stored. The
    /// 'source history' pair points to the location where the history for the
    /// requested step is located. If there is enough history data in the given
    /// buffer both pointer pairs point to the same location (i.e. you should
    /// read the requested history data from that location and then save the new
    /// history data to the same location, overwriting the consumed history
    /// step). If, on the other hand, history had to be 'emulated' the two
    /// pointer pairs will point to different locations and you must use the
    /// 'source history' pair as a source of history data (because the 'target
    /// history' pair points to uninitialised/garbage history data).
    ///   The data member names reflect the fact that history buffers can be
    /// used for both AmPh and ReIm data.
    ///
    ////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4512 ) // Assignment operator could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
#endif // _MSC_VER

    struct HistoryData
    {
        struct TargetHistory
        {
            float * const pAmplitudesOrReals;
            float * const pPhasesOrImags    ;
        } targetHistory;

        struct SourceHistory
        {
            float const * const pAmplitudesOrReals;
            float const * const pPhasesOrImags    ;
        } sourceHistory;

        bool isEmulated() const;
    }; // struct HistoryData

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

public:
    HistoryData LE_FASTCALL getCurrentStepData
    (
        std::uint16_t historyLengthInSteps,
        std::uint16_t numberOfBins,
        DataRange     historyData
    );

    void reset();

private:
    std::uint16_t step_     ;
    std:: int8_t  increment_;

    // State required for history emulation.
    std::uint16_t actualHistoryLengthInSteps_;
    std::uint16_t emulatedHistoryStepOffset_ ;
}; // class ReversedHistoryBufferState


////////////////////////////////////////////////////////////////////////////////
///
/// \class ReversedHistoryChannelState
///
/// \brief Wraps and maintains a history buffer with a corresponding reversed
/// history buffer state while implementing the SW::Effects ChannelState
/// interface.
///
////////////////////////////////////////////////////////////////////////////////

template <unsigned int historyLengthInMilliseconds>
class ReversedHistoryChannelState : private HistoryBuffer<float, historyLengthInMilliseconds>
{
private:
    using BaseBuffer = HistoryBuffer<float, historyLengthInMilliseconds>;

public:
    ReversedHistoryBufferState::HistoryData LE_FASTCALL getCurrentStepData
    (
        std::uint16_t historyLengthInSteps,
        std::uint16_t numberOfBins
    )
    {
        return bufferState_.getCurrentStepData( historyLengthInSteps, numberOfBins, static_cast<BaseBuffer &>( *this ) );
    }

    void reset()
    {
        bufferState_ .reset();
        BaseBuffer  ::clear();
    }

    using BaseBuffer::requiredStorage;
    using BaseBuffer::resize;

private:
    ReversedHistoryBufferState bufferState_;
}; // class ReversedHistoryChannelState

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // historyBuffer_hpp
