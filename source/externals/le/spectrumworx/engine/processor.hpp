////////////////////////////////////////////////////////////////////////////////
///
/// \file processor.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// https://github.com/jamoma/JamomaDSP
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef processor_hpp__F3D546FF_92C1_4B24_8CCD_EDD88713742D
#define processor_hpp__F3D546FF_92C1_4B24_8CCD_EDD88713742D
#pragma once
//------------------------------------------------------------------------------
#include "buffers.hpp"
#include "channelBuffers.hpp"
#include "setup.hpp"

#include "le/math/dft/fft.hpp"
#include "le/parameters/lfoImpl.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace boost
{
    template <class T> class intrusive_ptr ;
    template <class T> class iterator_range;
} // namespace boost
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
/// \class Processor
///
////////////////////////////////////////////////////////////////////////////////

class ModuleChainImpl;

class Processor
{
public:
    using InputData             = float const * LE_RESTRICT const * LE_RESTRICT;
    using OutputData            = float       * LE_RESTRICT const * LE_RESTRICT;
    using InterleavedInputData  = float const * LE_RESTRICT                    ;
    using InterleavedOutputData = float       * LE_RESTRICT                    ;

    LE_NOTHROWNOALIAS
    void LE_FASTCALL process // separated channel data
    (
        InputData     mainInputs,
        InputData     sideChannels,
        OutputData    outputs,
        std::uint32_t samples,
        float         outputGain,
        float         mixAmount
    );
    LE_NOTHROWNOALIAS
    void LE_FASTCALL process // interleaved channel data
    (
        InterleavedInputData  mainInputs,
        InterleavedInputData  sideChannels,
        InterleavedOutputData outputs,
        std::uint32_t         samples,
        float                 outputGain,
        float                 mixAmount
    );

    void reset() { lfoTimer().reset(); }

    void setNumberOfChannels( std::uint8_t numberOfMainChannels, std::uint8_t numberOfSideChannels );

    void changeWOLAParameters( StorageFactors const &, Setup::Window, Storage );

    void changeWindowFunction( Setup::Window );

public:
    void clearSideChannelData();
    void resetChannelBuffers ();

    Setup                   const & engineSetup    () const { return engineSetup_    ; }
    Math::FFT_float_real_1D const & fft            () const { return fft_            ; }
    ReadOnlyDataRange       const & analysisWindow () const { return analysisWindow_ ; }
    ReadOnlyDataRange       const & synthesisWindow() const { return synthesisWindow_; }

    FullChannelData_AmPh const & currentAmPhData( std::uint8_t const channel ) const { return static_cast<ChannelData const &>( channels_[ channel ].channelData() ).currentAmPhData(); }
    FullChannelData_ReIm const & currentReImData( std::uint8_t const channel ) const { return static_cast<ChannelData const &>( channels_[ channel ].channelData() ).currentReImData(); }

    static Processor       & fromEngineSetup( Setup       & );
    static Processor const & fromEngineSetup( Setup const & );

public: //...mrmlj...SDK and plugin shared functionality (cleanup and extract)...
    bool LE_NOTHROW LE_FASTCALL setSampleRate( float const sampleRate, StorageFactors & currentStorageFactors );
    bool LE_FASTCALL resize
    (
        StorageFactors                  & currentStorageFactors,
        StorageFactors            const & newStorageFactors,
        Setup::Window                     window,
        Engine::HeapSharedStorage       & sharedStorage
    );

    static StorageFactors makeFactors
    (
        std::uint16_t fftSize         ,
    #if LE_SW_ENGINE_WINDOW_PRESUM
        std::uint8_t  windowSizeFactor,
    #endif // LE_SW_ENGINE_WINDOW_PRESUM
        std::uint8_t  overlapFactor   ,
        std::uint8_t  numberOfChannels,
        std::uint32_t sampleRate
    );

protected: // LFO & timing
#ifdef LE_NO_LFOs
    private: using lfoTimer = Parameters::LFOImpl::Timer; public:
    void updatePosition( std::uint32_t ) {}
#else
    using LFO = Parameters::LFOImpl;

    void setPosition   ( std::uint32_t absolutePositionInSamples );
    void updatePosition( std::uint32_t deltaSamples              );

    LFO::Timer::TimingInformationChange LE_FASTCALL updatePositionAndTimingInformation( float positionInBars, float barDuration, std::uint8_t measureNumerator );
    LFO::Timer::TimingInformationChange LE_FASTCALL updatePositionAndTimingInformation( std::uint32_t deltaNumberOfSamples );

    void handleTimingInformationChange( LFO::Timer::TimingInformationChange );

    LFO::Timer       & lfoTimer()       { return lfoTimer_; }
    LFO::Timer const & lfoTimer() const { return lfoTimer_; }

private:
    void updateModuleLFOs( LFO::Timer::TimingInformationChange );
#endif // LE_NO_LFOs
private:
    class ProcessParameters;

    Setup & engineSetup() { return engineSetup_; }

    void LE_FASTCALL processSingleChannel( ProcessParameters const & );
    void LE_FASTCALL preProcess          ();

    ModuleChainImpl       & modules()      ;
    ModuleChainImpl const & modules() const;

    void calculateWindowAndWOLAGain();

private:
    using FFTWindow = WindowBuffer<>;

    struct Channels : Utility::SharedStorageBuffer<ChannelBuffers>
    {
        static LE_CONST_FUNCTION std::uint32_t requiredStorage( StorageFactors const & );

        void resize( StorageFactors const & factors, Storage & storage );
    }; // struct Channels

private:
    Setup                   engineSetup_    ;
#ifndef LE_NO_LFOs
    LFO::Timer              lfoTimer_       ;
#endif // LE_NO_LFOs
    Math::FFT_float_real_1D fft_            ;
    FFTWindow               analysisWindow_ ;
    FFTWindow               synthesisWindow_;
    Channels                channels_       ;

    /// \note See the related note in the calculateWindowAndWOLAGain() member
    /// function.
    ///                                       (04.03.2015.) (Domagoj Saric)
    FFTWindow synthesisWindowBackup_;

public:
    static LE_CONST_FUNCTION std::uint32_t LE_FASTCALL requiredStorage( StorageFactors const & );

    void LE_FASTCALL resize( StorageFactors const &, Storage & );
}; // class Processor

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // processor_hpp
