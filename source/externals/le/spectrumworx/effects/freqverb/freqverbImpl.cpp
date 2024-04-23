////////////////////////////////////////////////////////////////////////////////
///
/// freqverbImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// http://christianfloisand.wordpress.com/tag/plug-in
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "freqverbImpl.hpp"

#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/math/dft/domainConversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/channelDataReIm.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include "boost/assert.hpp"

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
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Freqverb static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Freqverb::title      [] = "Freqverb";
char const Freqverb::description[] = "Frequency domain reverberation with pitch.";


////////////////////////////////////////////////////////////////////////////////
//
// Freqverb UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Freqverb::Time60dB    , "Life time" )
EFFECT_PARAMETER_NAME( Freqverb::RoomSize    , "Room size" )
EFFECT_PARAMETER_NAME( Freqverb::ReverbPitch , "Pitch"     )
EFFECT_PARAMETER_NAME( Freqverb::HFAbsorption, "HF absorb" )

////////////////////////////////////////////////////////////////////////////////
//
// FreqverbImpl::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////
LE_COLD
void FreqverbImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    roomLevel_ = Math::dB2NormalisedLinear( parameters().get<RoomSize>() );

    // Border!
    noEchoBin_ = engineSetup.frequencyInHzToBin( 7000 );

    // Init pitch shifter:
    ps_.setup( engineSetup );

    float const semitonesPerSecond( parameters().get<ReverbPitch>()                   );
    float const pitchScale        ( semitonesPerSecond / engineSetup.stepsPerSecond() );
    ps_.setPitchScaleFromSemitones( pitchScale, engineSetup.numberOfBins() );
}


////////////////////////////////////////////////////////////////////////////////
//
// FreqverbImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////
LE_HOT
void FreqverbImpl::process
(
    ChannelState                   & cs,
    Engine::ChannelData_ReIm         data,
    Engine::Setup            const & engineSetup
) const
{
    // Based on:
    // "Frequency Domain Artificial Reverberation using Spectral Magnitude
    //  Decay",  AES Convention Paper 6926, Earl Vickers.


    // Send last feedback to output:
        // - attenuate "old" feedback with Room Level
        // - add it to Input if DryMix is on
        // - send to output

    // Prepare new feedback-sum:
        // - convert to AmPh domain and randomize phase
        // - sum feedback with new input
        // - save as new feedback for next frame

    using namespace Math;

    {
        /// \note As with all signal-history based effects, we always have to
        /// process the entire range while modifying only the working range.
        ///                                   (19.10.2015.) (Domagoj Saric)
        float * LE_RESTRICT pSignalReal  ( data.full().reals().begin() );
        float * LE_RESTRICT pSignalImag  ( data.full().imags().begin() );
        float * LE_RESTRICT pFeedbackReal( cs.feedbackSumReals.begin() );
        float * LE_RESTRICT pFeedbackImag( cs.feedbackSumImags.begin() );

        float const * const pEnd( &data.full().reals()[ noEchoBin_ ] );

        float const roomLevel               ( roomLevel_                                                             );
        float const secondsPerStep          ( engineSetup.stepTime()                                                 );
        float const time60dB                ( parameters().get<Time60dB>()                                           );
        float const hfExtraAttenuationPerBin( parameters().get<HFAbsorption>() / float( engineSetup.numberOfBins() ) );

        float hfExtraAttenuation( 1 );
        while ( pSignalReal != pEnd )
        {
            float & signalReal     ( *pSignalReal  ++ );
            float & signalImag     ( *pSignalImag  ++ );
            float & newFeedbackReal( *pFeedbackReal++ );
            float & newFeedbackImag( *pFeedbackImag++ );

            float const feedbackReal( newFeedbackReal );
            float const feedbackImag( newFeedbackImag );

            // attenuate with 60dB/Time gain, and add to current input

			// kill high frequencies faster,
			// increase attenuation (decrease attenuation time) linearly from
			// low to high frequencies:
            float const time60dBThisBin   ( time60dB / hfExtraAttenuation );
			float const attenuationThisBin( Math::dB2NormalisedLinear( -60 * secondsPerStep / time60dBThisBin ) );
            hfExtraAttenuation += hfExtraAttenuationPerBin;

			newFeedbackReal = (feedbackReal + signalReal) * attenuationThisBin;
			newFeedbackImag = (feedbackImag + signalImag) * attenuationThisBin;

            bool const binWithinWorkingRange
            (
                ( &signalReal >= data.reals().begin() ) &&
                ( &signalReal <  data.reals().end  () )
            );
            if ( binWithinWorkingRange )
            {
                // send feedback to output with room level attenuation:
                signalReal = roomLevel * feedbackReal;
                signalImag = roomLevel * feedbackImag;
            }
        }

        BOOST_ASSERT( pSignalReal == pEnd );
        if ( pSignalReal < data.reals().end() )
        {
            Math::clear( pSignalReal, data.reals().end() );
            Math::clear( pSignalImag, data.imags().end() );
        }
    }

    {
        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( data2Storage, char, Engine::ChannelData_AmPhStorage::requiredStorage( engineSetup.fftSize<std::uint16_t>() ) );
        Engine::ChannelData_AmPhStorage data2( engineSetup.fftSize<std::uint16_t>(), 0, noEchoBin_, data2Storage );
        BOOST_ASSERT( data2.numberOfBins() == noEchoBin_ );

        reim2AmPh
        (
            cs.feedbackSumReals  .begin(),
            cs.feedbackSumImags  .begin(),
            data2.full().amps  ().begin(),
            data2.full().phases().begin(),
            noEchoBin_
        );

        // Pitch shift echoed signal:
        ps_.process( cs.ps, std::forward<Engine::ChannelData_AmPh>( data2 ) );

        // Randomize phase (skip DC bin):
        //...mrmlj...could be done with reim...http://en.wikipedia.org/wiki/Rotation_(mathematics)#Complex_numbers
        for ( auto & phase : DataRange( data2.phases() ).advance_begin( 1 ) )
        {
            phase = rangedRand( Math::Constants::twoPi );
        }

        amph2ReIm
        (
            data2.full().amps  ().begin(),
            data2.full().phases().begin(),
            cs.feedbackSumReals  .begin(),
            cs.feedbackSumImags  .begin(),
            noEchoBin_
        );
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
