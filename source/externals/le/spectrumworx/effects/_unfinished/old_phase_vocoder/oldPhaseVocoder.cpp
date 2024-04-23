#include "oldPhaseVocoder.hpp"

#include "../../parameters/uiElements.hpp"
#include "../../math/math.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const OldPhaseVocoder::title      [] = "Old Phase Vocoder";
char const OldPhaseVocoder::description[] = "Basic phase vocoder";

char const UIElements<OldPhaseVocoder::Scale                >::name_[] = "Scale";
char const UIElements<OldPhaseVocoder::SpectrumStartingPoint>::name_[] = "Spectrum starting point";
char const UIElements<OldPhaseVocoder::SpectrumEndingPoint  >::name_[] = "Spectrum ending point";
char const UIElements<OldPhaseVocoder::SpectrumOffset       >::name_[] = "Spectrum offset";


////////////////////////////////////////////////////////////////////////////////
//
// Private implementation details for this module.
// -----------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \todo This section contains duplicated code/items from the fastMath module
/// (from the SpectrumWorx source tree) only so that this module would not
/// depend on it. Remove this section once the generic math functionality is
/// extracted from the SpectrumWorx source tree.
///                                           (01.06.2009.) (Domagoj Saric)
///
////////////////////////////////////////////////////////////////////////////////

#define PI  3.1415926535897932384626433832795f     // PI
#define iPI 0.31830988618379067153776752674503f    // 1 / PI



////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::analysis()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::analysis( ChannelState & channelState, float const * const pIn, float * const pOut ) const
{
    for ( unsigned int k( 0 ); k < fftHalfFrameSize_; ++k )
    {
        float const phase( pIn[ k ] );
 
        float tmp( phase - channelState.lastPhase_[ k ] ); // phase difference
        channelState.lastPhase_[ k ] = phase; // update last phase

        tmp -= k * expctRate_;
        int qdrnt( Math::round( tmp * iPI ) );  // quadrant
        if ( qdrnt >= 0 )
            qdrnt += qdrnt & 1;
        else
            qdrnt -= qdrnt & 1; //TODO: cache stall
        tmp -= PI * qdrnt; // mapping phase
        tmp *= M_PI2inv_; 
        tmp = k * freqPerBin_ + tmp * freqPerBin_; 

        pOut[ k ] = tmp;
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::pitchShiftAndScale()
// ----------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::pitchShiftAndScale
(
    float       * const amplitudes,
    float const * const anaFreqs  ,
    float       * const synthFreqs
) const
{
    Common::SSEAlignedHalfFFTBuffer synthdMgn;
    Common::SSEAlignedHalfFFTBuffer synthdFrq;

    float const scaleInverse( 1 / scale_ );

    for ( unsigned int outputIndex( lowerBound_ ); outputIndex < upperBound_; ++outputIndex )
    {
        unsigned int const inputIndex( Math::convert<unsigned int>( ( outputIndex * scaleInverse ) + offset_ ) );
        assert( inputIndex < fftHalfFrameSize_ );
        {
            //  Here we (temporarily) assert our assumptions (even if obvious)
            // to aid in better code/algorithm understanding and bug tracking.
            assert( synthdMgn[ outputIndex ] == 0 );
            assert( synthdFrq[ outputIndex ] == 0 );
            assert( amplitudes[ inputIndex ] >= synthdMgn[ outputIndex ] );
            
            synthdMgn[ outputIndex ] = amplitudes[ inputIndex ];
            synthdFrq[ outputIndex ] = anaFreqs  [ inputIndex ] * scale_;

            if ( ( synthdFrq[ outputIndex ] == 0 ) && ( outputIndex > 0 ) ) // fill empty with nearest neighbour
            {
                synthdMgn[ outputIndex ] = synthdMgn[ outputIndex - 1 ];
                synthdFrq[ outputIndex ] = synthdFrq[ outputIndex - 1 ];
            }
        }
    }

    std::memcpy( amplitudes, synthdMgn.begin(), fftHalfFrameSize_ * sizeof( float ) );
    std::memcpy( synthFreqs, synthdFrq.begin(), fftHalfFrameSize_ * sizeof( float ) );
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::process
(
    ChannelState     & channelState,
    ChannelData_AmPh & data
) const
{
    /// \todo Now that algorithm process() functions have been made to be
    /// in-place see if these temporary buffers (and possibly redundant copying)
    /// in OldPhaseVocoder can be removed.
    ///                                       (29.09.2009.) (Domagoj Saric)

    LE::Common::SSEAlignedHalfFFTBuffer anaFreq  ;
    LE::Common::SSEAlignedHalfFFTBuffer synthFreq;
    
    analysis          ( channelState           , data.phases.begin(), anaFreq    .begin() );
    pitchShiftAndScale( data.amplitudes.begin(), anaFreq    .begin(), synthFreq  .begin() );
    synthesis         ( channelState           , synthFreq  .begin(), data.phases.begin() );
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::setScalingFactor()
// --------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::setScalingFactor( float const & newScale )
{
    assert( newScale > 0 );
    scale_ = newScale;
    updateActualBounds();
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{
    freqPerBin_              = engineSetup.sampleRate<float>() / engineSetup.fftSize<float>();
    invFreqPerBinInvOverlap_ = ( 1 / freqPerBin_ ) * ( 2 * PI ) * ( 1 / engineSetup.windowOverlappingFactor<float>() );
    expctRate_               = ( 2 * PI ) * engineSetup.stepSize<float>() / engineSetup.fftSize<float>();
    M_PI2inv_                = engineSetup.windowOverlappingFactor<float>() / ( 2 * PI );
    fftHalfFrameSize_        = engineSetup.fftSize<unsigned int>() / 2;

    /// \todo If it becomes necessary, fix this function to work with
    /// non-symmetric spectrum offset parameter ranges (e.g. -30% - +50%).
    ///                                       (08.06.2009.) (Domagoj Saric)
    assert( ( - SpectrumOffset::Traits::minimumValue == SpectrumOffset::Traits::maximumValue ) && "Symmetric offset range condition breached." );
    // Offset[ -FFTSize/16, +FFTSize/16 ] = Offset[ % ] / 100 * FFTSize / 16
    offset_ = -myParameters.get<SpectrumOffset>() * static_cast<int>( engineSetup.fftSize<unsigned int>() / 16 ) / 100;

    // Calculate the user specified bounds (lowest and highest bin).
     lowestUserSpecifiedBin_ = Math::round( myParameters.get<SpectrumStartingPoint>() * fftHalfFrameSize_ );
    highestUserSpecifiedBin_ = Math::round( myParameters.get<SpectrumEndingPoint  >() * fftHalfFrameSize_ );

    setScalingFactor( myParameters.get<Scale>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::synthesis()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::synthesis( ChannelState & channelState, float const * const pIn, float * const pOut ) const
{
    for ( unsigned int k( 0 ); k < fftHalfFrameSize_; ++k )
    {
        float tmp( pIn[ k ] );

        tmp -= k * freqPerBin_;
        tmp *= invFreqPerBinInvOverlap_; 
        tmp += k * expctRate_;      

        channelState.summPhase_[ k ] += tmp;

        pOut[ k ] = channelState.summPhase_[ k ];
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::updateActualBounds()
// ----------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Calculates the actual bounds (lowest and highest bin) to be used for
/// processing (taking the offset and scale parameters into consideration).
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::updateActualBounds()
{
    // Implementation note:
    //   This calculation cannot (currently) be merged into the setup function
    // because OldPhaseVocoder publicly exposes the setScalingFactor() interface
    // (required by algorithms like AutoTune) that enables dynamic changing of
    // the scaling factor (required for this calculation) even after the setup()
    // call.
    //                                        (22.10.2009.) (Domagoj Saric)

    // Implementation note:
    //  If an offset was specified the (previously calculated) range has to be
    // adjusted to prevent negative and greater-than-fftHalfFrameSize values
    // when calculating the inputIndex for the input arrays.
    //  The inputIndex for the input arrays is calculated with the following
    // formula: InputIndex = ( OutputIndex * ( 1 / PitchScale ) ) + Offset.
    // Therefor the lowest allowable inputIndex value is that which satisfies
    // the following inequality:
    // ( OutputIndex * ( 1 / PitchScale ) ) + Offset >= 0, from which follows
    // that LowestOutputIndex = - ( Offset / ( 1 / PitchScale ) ). The highest
    // allowable inputIndex value is that which satisfies the following
    // inequality:
    // ( OutputIndex * ( 1 / PitchScale ) ) + Offset <= FFTHalfFrameSize, from
    // which follows that
    // HighestOutputIndex = ( FFTHalfFrameSize - Offset ) / ( 1 / PitchScale ).
    //  The lower inputIndex limit has to be modified only for negative offsets
    // while the upper inputIndex limit has to be modified only for positive
    // offsets. This check (as implemented bellow) could also be implemented
    // using std::min()/std::max() calls but this produced slightly smaller code
    // with MSVC++ 8.0.
    //                                        (09.06.2009.) (Domagoj Saric)
    unsigned int const lowerIndexLimit( ( offset_ < 0 ) ? Math::convert<unsigned int>( std::ceil (                     - offset_   * scale_ ) ) : 0                 );
    unsigned int const upperIndexLimit( ( offset_ > 0 ) ? Math::convert<unsigned int>( std::floor( ( fftHalfFrameSize_ - offset_ ) * scale_ ) ) : fftHalfFrameSize_ );

    lowerBound_ = std::max(  lowestUserSpecifiedBin_, lowerIndexLimit );
    upperBound_ = std::min( highestUserSpecifiedBin_, upperIndexLimit );
    // Implementation note:
    //   If the scaling factor is less than 1 (i.e. the pitch is being
    // downscaled) the inputIndex value will grow faster than outputIndex so a
    // final adjustment must be made to "final" upperLimit to prevent the
    // inputIndex growing out of range.
    //                                        (12.06.2009.) (Domagoj Saric)
    if ( scale_ < 1 )
        upperBound_ = Math::convert<unsigned int>( upperBound_ * scale_ );
}


////////////////////////////////////////////////////////////////////////////////
//
// OldPhaseVocoder::ChannelState::clear()
// -----------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void OldPhaseVocoder::ChannelState::clear()
{
    lastPhase_.clear();
    summPhase_.clear();
}

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
