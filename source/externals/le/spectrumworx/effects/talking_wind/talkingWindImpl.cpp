////////////////////////////////////////////////////////////////////////////////
///
/// talkingWindImpl.cpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Implement carrier "whitening" which should be done before
/// cross-synthesis is done.
///                                           (19.01.2010.) (Danijel Domazet)
///
/// \todo According to Oppenheim and Schafer in Discrete-time signal processing,
/// zero-padding can help with time-aliasing issues of the Cepstrum method with
/// shorter frame sizes.
///                                           (22.05.2013.) (Domagoj Saric)
///
/// Additional ideas and resources on vocoding:
/// http://www.keyboardmag.com/article/Dance-Ethereal-Vocoding/2499
/// http://www.electroempire.com/forum/showthread.php?t=2372
/// http://www.soundonsound.com/sos/may99/articles/synthsec.htm
/// http://www.soundonsound.com/sos/apr05/articles/logicnotes.htm
/// http://www.academia.edu/6220929/Analysis_of_Vocoder_Technology_on_Male_Voice
/// http://www.youtube.com/watch?v=4feOFLX6238
/// http://www.youtube.com/watch?v=60vis_aIS_g
/// http://www.audionerdz.com
/// http://www.antarestech.com/products/avox-evo.shtml#throat
/// http://www.samplepacks.ca/how-to-create-a-daft-punk-vocoder-in-ableton-live
/// http://www.kvraudio.com/forum/viewtopic.php?t=202731
/// http://www.kvraudio.com/forum/viewtopic.php?t=355780
/// http://www.kvraudio.com/forum/viewtopic.php?t=357049
/// http://www.warmplace.ru/forum/viewtopic.php?f=11&t=2463
/// Speech emotion modification using a cepstral vocoder
/// http://link.springer.com/chapter/10.1007%2F978-3-642-12397-9_23
///
/// http://freemusicsoftware.org/category/free-vst-effects-2/vocoder
/// https://github.com/borsboom/vocoder/blob/master/vocode.c
///
/// Spectral envelope:
/// http://en.wikipedia.org/wiki/Envelope_detector
/// http://articles.ircam.fr/textes/Schwarz98a/index.pdf
/// http://articles.ircam.fr/textes/Roebel05a/index.pdf
/// http://support.ircam.fr/docs/AudioSculpt/3.0/co/True%20Envelope.html
/// http://recherche.ircam.fr/anasyn/caetano/docs/caetano_ampenv_ICASSP2011.pdf
/// http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.380.6432
/// CheapTrick, a spectral envelope estimator for high-quality speech synthesis
/// http://www.sciencedirect.com/science/article/pii/S0167639314000697
/// http://www.dsprelated.com/dspbooks/sasp/Spectral_Envelope_Cepstral_Windowing.html
/// http://www.dsprelated.com/dspbooks/sasp/Cepstral_Windowing.html
/// http://www.dsprelated.com/dspbooks/sasp/Spectral_Envelope_Linear_Prediction.html
/// https://www.spsc.tugraz.at/sites/default/files/Krebs10_DA.pdf
/// http://www.mathworks.com/help/dsp/examples/envelope-detection.html
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "talkingWindImpl.hpp"

#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/processor.hpp"
#include "le/spectrumworx/engine/setup.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include "boost/assert.hpp"
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
// TalkingWind static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const TalkingWind::title      [] = "Talking Wind";
char const TalkingWind::description[] = "Classic vocoding.";


////////////////////////////////////////////////////////////////////////////////
//
// TalkingWind UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( TalkingWind    ::EnvelopeBorder, "Envelope border" )
EFFECT_PARAMETER_NAME( TalkingWind    ::EnvelopeGain  , "Envelope gain"   )


////////////////////////////////////////////////////////////////////////////////
//
// TalkingWindImpl::setup()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////
LE_COLD
void TalkingWindImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    envgain_ = Math::dB2NormalisedLinear( parameters().get<EnvelopeGain>() );
    cutoff_  = engineSetup.frequencyInHzToBin( parameters().get<EnvelopeBorder>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// TalkingWindImpl::process()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_HOT
void TalkingWindImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & setup ) const
{
    using namespace Math;

    /// \note This effect is known to produce invalid values due to taking the
    /// logarithm of zero or near-zero values.
    ///                                       (03.12.2012.) (Domagoj Saric)
    LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();

    // Implementation note:
    //   The "excitation" or the source signal is the side channel while the
    // main channel provides the spectral envelope.
    //                                        (21.10.2011.) (Domagoj Saric)

    auto const fullNumberOfBins( setup.numberOfBins() );

    // Allocate a work buffer for envelope calculation that can hold a whole
    // time domain frame (i.e. the size of the FFT but allocate it as "two
    // aligned half-FFT size" buffers so that it can be used for both time
    // domain and ReIm intermediate results):
    BOOST_SIMD_ALIGNED_STACK_BUFFER( doubleWorkBuffer, Engine::real_t, ( alignIndex( fullNumberOfBins ) * 2 ) );
    DataRange envelope( &doubleWorkBuffer[ 0 ], &doubleWorkBuffer[ fullNumberOfBins ] );

    std::uint16_t const skippedLeadingBins ( data.beginBin()                  );
    std::uint16_t const skippedTrailingBins( fullNumberOfBins - data.endBin() );

    // Cepstrum-based envelope calculation needs its work buffer to be the
    // whole frame size and to thus alias the envelope buffer (for inplace
    // DFT calculation).
    add( data.full().main().amps(), std::numeric_limits<float>::epsilon(), envelope );
    ln ( envelope );
    lowPassSpectrum_cepstrum( envelope, doubleWorkBuffer, setup );
    envelope.advance_begin( + skippedLeadingBins  );
    envelope.advance_end  ( - skippedTrailingBins );
    exp( envelope );
    multiply( envelope, envgain_ ); // Output gain normalization when using Envelope gain/Carrier attenuation
#ifndef NDEBUG
    // Clear out invalid values to avoid assertion failures.
    for ( auto & amp : envelope ) { if ( !std::isfinite( amp ) ) amp = 0; }
#endif // NDEBUG

    if ( BOOST_LIKELY( skippedLeadingBins == 0 && data.endBin() > 1 ) )
    {
        /// \note Avoid passing the DC through. This primitive approach is far
        /// from ideal as the DC component can leak into the neighbouring bins
        /// (and vice verse) due to windowing. This should be performed before
        /// the DFT.
        ///                                   (05.03.2015.) (Domagoj Saric)
        envelope[ 0 ] /= 10;
        envelope[ 1 ] /=  2;
    }

    multiply
    (
        envelope          .begin(),
        data.side().amps().begin(),
        data.main().amps().begin(),
        data.main().amps().end  ()
    );

    copy( data.side().phases(), data.main().phases() );

    //BOOST_ASSERT( data.main().amps()[ 0 ] == 0 ); //...mrmlj...envelope DC bin is currently not fully zeroed
}


void LE_HOT TalkingWindImpl::lowPassSpectrum_cepstrum
(
    DataRange     const & spectrum,
    DataRange     const & workBuffer,
    Engine::Setup const & engineSetup
) const
{
    using namespace Math;

    /// \note Cepstrum should be the inverse DFT of the log spectrum. Log
    /// spectrum is a real sequence. This whole Cepstrum and (I)DFT idea is
    /// still not fully clear. Why must the IDFT (instead of the DFT) be used to
    /// get the Cepstrum? Aren't the forward and inverse DFTs supposed to be
    /// interchangeable (in fact, Udo uses the "inverted" DFT definitions where
    /// the forward transform has the positive exponent twiddle factors)? What
    /// about the redundancies in the real Cepstrum calculations (imaginaries
    /// are zeros and/or are discarded), is this really how it is supposed to be
    /// done and/or can this be avoided/exploited (e.g. by using DFT transforms
    /// of half the size)?
    /// Udo Zolzer: DAFx, chapter 9 (9.2.3 and 9.3.1).
    /// Oppenheim, Schafer: Discrete-time signal processing, chapter 12.
    /// http://en.wikipedia.org/wiki/Cepstrum
    /// http://music.columbia.edu/pipermail/music-dsp/2010-January/068353.html
    /// http://ccrma.stanford.edu/~jos/SpecEnv/SpecEnv.pdf
    /// http://ccrma.stanford.edu/~jos/SpecEnv/Cepstral_Smoothing.html
    /// http://ccrma.stanford.edu/~jos/sasp/Spectral_Envelope_Cepstral_Windowing.html
    /// http://ccrma.stanford.edu/~jos/SpecEnv/LPC_Envelope_Example_Speech.html
    /// http://www.kvraudio.com/forum/viewtopic.php?t=342931
    ///
    /// http://en.wikipedia.org/wiki/Mel-frequency_cepstrum
    ///                                       (13.06.2012.) (Domagoj Saric)

    // spectrum must alias (and be at the beginning of) the workBuffer...
    BOOST_ASSERT( spectrum.begin() == workBuffer.begin() );
    BOOST_ASSERT( spectrum.end  () <  workBuffer.end  () );

    float * LE_RESTRICT const pReals( spectrum.begin()                                );
    float * LE_RESTRICT const pImags( static_cast<float *>( align( spectrum.end() ) ) );
    // Dummy/null imaginary components (the spectrum is real):
    clear( pImags, engineSetup.numberOfBins() );

    // Transform into the quefrency domain (the cepstrum):
    FFT_float_real_1D const & fft( Engine::Processor::fromEngineSetup( engineSetup ).fft() );
    fft.inverseTransform( pReals, pImags, fft.size() );

    // Inplace FFT: pReals now points to/contains the real cepstrum:
    float * const pCepstrum( pReals );

    // "Low pass" the spectrum:
    float const scale( envgain_ );

    auto const cutoff( cutoff_ );
    /// \note According to DAFX equation 9.24 the cepstrum should be
    /// multiplied by 2 but this gives a too quiet output and is
    /// currently skipped as a quick-fix.
    ///                               (07.11.2013.) (Domagoj Saric)
    float const udoGain( scale /** 2*/ );
    if ( cutoff > 2 )
    {
        multiply( &pCepstrum[ 1 ], udoGain, cutoff - 2 );
        // an attempt to make the LP less of a brick...
        pCepstrum[ 0          ] *= 0.50f * udoGain;
        pCepstrum[ cutoff - 1 ] *= 0.75f * udoGain;
    }
        pCepstrum[ cutoff     ] *= 0.50f * udoGain;
        pCepstrum[ cutoff + 1 ] *= 0.25f * udoGain;
    clear( &pCepstrum[ cutoff + 2 ], workBuffer.end() );

    // Transform back into the frequency domain (low passed spectrum/envelope)
    // and discard the imaginary components:
    fft.transform( pCepstrum, pImags, fft.size() );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
