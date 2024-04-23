////////////////////////////////////////////////////////////////////////////////
///
/// \file oldPhaseVocoder.hpp
/// ----------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef oldPhaseVocoder_hpp__51ABC06A_AA69_4C25_8E9C_A672296BB649
#define oldPhaseVocoder_hpp__51ABC06A_AA69_4C25_8E9C_A672296BB649
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
#include "../../common/buffers.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class OldPhaseVocoder
///
/// \ingroup Algorithms 
///
/// \brief Phase vocoder for testing purposes only.
///
///
////////////////////////////////////////////////////////////////////////////////

class OldPhaseVocoder
{
public: // LE::Algorithm interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DEFINE_PARAMETERS
    (
        ( ( Scale                 )( float )( MinimumValue<  1> )( MaximumValue<170> )( DefaultValue<10> )( RangeValuesDenominator<10> ) )
        ( ( SpectrumStartingPoint )( float )( MinimumValue<  0> )( MaximumValue<  1> )( DefaultValue< 0> )                               )
        ( ( SpectrumEndingPoint   )( float )( MinimumValue<  0> )( MaximumValue<  1> )( DefaultValue< 1> )                               )
        ( ( SpectrumOffset        )( int   )( MinimumValue<-20> )( MaximumValue<+20> )( DisplayValueSuffix<' %'> )                        )
    );


    ////////////////////////////////////////////////////////////////////////////
    // ChannelState
    ////////////////////////////////////////////////////////////////////////////

    struct ChannelState
    {
        ////////////////////////////////////////////////////////////////////////
        ///
        /// \brief Resets the phase buffers.
        ///
        /// \throws none
        ///
        ////////////////////////////////////////////////////////////////////////

        void clear();

        Common::SSEAlignedHalfFFTBuffer lastPhase_;
        Common::SSEAlignedHalfFFTBuffer summPhase_;
    };


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( EngineSetup const &, Parameters const & );

    /// \brief Performs analysis, pitch scaling and shifting and synthesis.
    void process( ChannelState &, ChannelData_AmPh & ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: // Descriptions.
    static char const title      [];
    static char const description[];

public:
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Changes only the scaling factor.
    ///
    ///   Required by certain algorithms that use the OldPhaseVocoder and change/
    /// adapt the pitch dynamically.
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \todo Try to improve/generalize the Parameter classes design and the
    /// associated LEL::Algorithms interface to also handle cases like this
    /// (where a wrapping/"higher level" algorithm needs to change the
    /// parameters of its implementing algorithm dynamically, in its process()
    /// member function.
    ///                                       (12.06.2009.) (Domagoj Saric)
    ///
    ////////////////////////////////////////////////////////////////////////////

    void setScalingFactor( float const & newScale );


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Phase vocoder analysis phase.
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////

    void analysis( ChannelState &, float const * phaseIn, float * anaFreqOut ) const;


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Phase vocoder synthesis phase.
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////

    void synthesis( ChannelState &, float const * synthFreqIn, float * phaseOut ) const;


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Performs pitch scaling and shifting.
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \todo Document parameters.
    ///                                       (03.06.2009.) (Domagoj Saric)
    ///
    ////////////////////////////////////////////////////////////////////////////
    
    void pitchShiftAndScale
    (
        float       * amplitudes,
        float const * anaFreqs  ,
        float       * synthFreqs
    ) const;

private:
    void updateActualBounds();

private:
    float freqPerBin_             ;
    float expctRate_              ;
    float invFreqPerBinInvOverlap_;
    float M_PI2inv_               ;
    float scale_                  ;
    int   offset_                 ;
    unsigned int lowerBound_;
    unsigned int upperBound_;
    unsigned int  lowestUserSpecifiedBin_;
    unsigned int highestUserSpecifiedBin_;
    unsigned int fftHalfFrameSize_;
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // oldPhaseVocoder_hpp
