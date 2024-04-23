////////////////////////////////////////////////////////////////////////////////
///
/// \file peakDetector.hpp
/// ----------------------
///
/// Copyright (c) 2010 - 2016. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef peakDetector_hpp__23C36B8D_DC64_4B55_B3D5_75201C713C96
#define peakDetector_hpp__23C36B8D_DC64_4B55_B3D5_75201C713C96
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/engine/buffers.hpp"

#include <array>
#include <cstdint>
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( LE )
//------------------------------------------------------------------------------
//namespace Analysis
//{
//------------------------------------------------------------------------------

/// \todo Thresholds should be negative. Think of a better way to do this.
///                                           (27.01.2011.) (Danijel Domazet)

#define PD_DEFAULT_STRENGTH          10
#define PD_MIN_STRENGTH               0
#define PD_MAX_STRENGTH              90

#define PD_MIN_LOCAL_THRESHOLD      120
#define PD_MAX_LOCAL_THRESHOLD        0
#define PD_DEFAULT_LOCAL_THRESHOLD   10

#define PD_MIN_GLOBAL_THRESHOLD     120 
#define PD_MAX_GLOBAL_THRESHOLD      10
#define PD_DEFAULT_GLOBAL_THRESHOLD  30  

#define MAX_NUM_PEAKS               100     // ...LE_CONFIGURATION_MAX_FFT_SIZE/2!


////////////////////////////////////////////////////////////////////////////////
///
/// \class Peak
///
/// \brief Holds peak info.
///
////////////////////////////////////////////////////////////////////////////////

struct Peak
{
    float         freq     ; /// Peak frequency calculated by parabola fit algorithm. 
    float         amplitude; /// True amplitude.   
    float         strength ; /// Peak strength when compared to neighbouring bins.         
    std::uint16_t startPos ; /// Peak starting position (DFT bin).
    std::uint16_t maxPos   ; /// Peak location (DFT bin).
    std::uint16_t stopPos  ; /// Peak stop position (DFT bin).    
    bool          valid    ; /// This peak is valid or not.
}; // struct Peak


////////////////////////////////////////////////////////////////////////////////
///
/// \class PeakDetector
///
/// \brief Finds peaks in the spectrum. Sorts peaks according to their strength. 
/// 
////////////////////////////////////////////////////////////////////////////////

class PeakDetector
{
public:
    PeakDetector();

    ////////////////////////////////////////////////////////////////////////////////
    //
    // setStrengthThreshold()
    // ----------------------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Sets strength threshold. Peak is compared to the neighbouring 
    /// bins, if below the threshold - discarded.
    ///
    /// \param threshold - Decibel value for minimum peak strength. 
    /// \return True if successful, false otherwise.
    ///
    /// \throws None.
    ///
    ////////////////////////////////////////////////////////////////////////////////

    void setStrengthThreshold( float threshold );

    
    ////////////////////////////////////////////////////////////////////////////////
    //
    // setLocalThreshold()
    // -------------------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Sets local threshold. Peaks more then "threshold" dB below the 
    /// local maximum will be discarded. 
    ///
    /// \param threshold - Decibel value for local threshold.
    ///
    /// \throws None.
    ///
    ////////////////////////////////////////////////////////////////////////////////

    void setLocalThreshold( float threshold );


    ////////////////////////////////////////////////////////////////////////////////
    //
    // setGlobalThreshold()
    // --------------------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Sets global threshold. Peaks more then "threshold" dB below the 
    /// global maximum will be discarded.
    ///
    /// \param threshold - Decibel value for global threshold.
    ///
    /// \throws None. 
    ///
    ////////////////////////////////////////////////////////////////////////////////

    void setGlobalThreshold( float threshold );


    ////////////////////////////////////////////////////////////////////////////
    //
    // findPeaks()
    // -----------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Finds peaks in the spectrum. 
    ///
    /// \param amplitudes   - Input amplitudes.
    /// \param numberOfBins - Number of bins.
    /// \param fs           - Sampling frequency. If zero then no frequency is 
    ///                       estimated for any peak.
    /// \return None. 
    ///
    /// \throws None. 
    ///
    ////////////////////////////////////////////////////////////////////////////
    
    void LE_FASTCALL findPeaks                    ( float const * amplitudes, std::uint16_t numberOfBins                   );    
    void LE_FASTCALL findPeaksAndStrengthSort     ( float const * amplitudes, std::uint16_t numberOfBins                   );
    void LE_FASTCALL findPeaksAndEstimateFrequency( float const * amplitudes, std::uint16_t numberOfBins, std::uint32_t fs );
    

    ////////////////////////////////////////////////////////////////////////////////
    //
    // getNumPeaks()
    // -------------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Returns the number of peaks found in last findPeaks call.
    ///
    /// \return Number of peaks found.
    ///
    /// \throws None. 
    ///
    ////////////////////////////////////////////////////////////////////////////////

    std::uint8_t getNumPeaks() const { return numberOfPeaks_; }


    ////////////////////////////////////////////////////////////////////////////////
    //
    // getPeak()
    // ---------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Gets a peak. 
    ///
    /// \param position - Desired peaks' location. 
    /// \return Pointer to Peak struct that holds valid peak data. NULL if no 
    /// valid peak is available. 
    ///
    /// \throws None. 
    ///
    ////////////////////////////////////////////////////////////////////////////////
    
    Peak const * LE_FASTCALL getPeak              ( std::uint8_t position  ) const;
    Peak const * LE_FASTCALL getPeakAboveThreshold( float        threshold ) const;
    
    ////////////////////////////////////////////////////////////////////////////////
    //
    // attenuatePeaks()
    // ----------------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Attenuates peaks.
    ///
    /// \param amplitudes    - target data
    /// \param startBin      - start bin
    /// \param stopInclusive - stop bin inclusive
    /// \param factor        - attenuation intensity in deciBell ( >0 ) 
    /// \return nothing
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////////    

    void LE_FASTCALL attenuatePeaks   ( float * amplitudes, std::uint16_t startBin, std::uint16_t stopInclusive, float factor );    
    void LE_FASTCALL attenuateNonPeaks( float * amplitudes, std::uint16_t startBin, std::uint16_t stopInclusive, float factor );


    ////////////////////////////////////////////////////////////////////////////////
    //
    // setZeroDecibelValue()
    // ---------------------
    //
    ////////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Sets maximum possible input amplitude value. Used as 0 dB
    /// threshold. This should always be set, otherwise all thresholds will be
    /// less valid.
    ///
    /// \param zeroDecibel - input zero dB value
    /// \return nothing
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////////    
    
    void LE_FASTCALL setZeroDecibelValue( float zeroDecibel );
    
private:
    void LE_FASTCALL restart();
    void LE_FASTCALL findPeaksImpl( float const * amplitudes, std::uint16_t numberOfBins, std::uint32_t fs );

private:
    float localThreshold_   ;
    float globalThreshold_  ;
    float strengthThreshold_;

    float maxGlobal_;

    std::uint8_t numberOfPeaks_;

    std::array<Peak, MAX_NUM_PEAKS                               > peaks_ ;
    std::array<bool, SW::Engine::StaticHalfFFTBuffer::static_size> isPeak_;
}; // class PeakDetector

//------------------------------------------------------------------------------
//} // namespace Analysis
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( LE )
//------------------------------------------------------------------------------
#endif // peakDetector_hpp
