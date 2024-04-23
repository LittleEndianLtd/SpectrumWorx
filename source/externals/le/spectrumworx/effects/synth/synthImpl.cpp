////////////////////////////////////////////////////////////////////////////////
///
/// synthImpl.cpp
/// -------------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
///
/// - tutorials
/// http://zynaddsubfx.sourceforge.net/doc/PADsynth/PADsynth.htm
/// http://www.drkappa.net/index.php/2009/12/23/the-making-of-a-soft-synth-part-i
/// http://lifeofaprogrammergeek.blogspot.com/2009/12/writing-synthesizer.html
/// http://www.codeproject.com/Articles/19618/C-Synth-Toolkit-Part-I
/// http://www.soundonsound.com/search?url=/search&Section=8&Subject=12
///
/// - practical ideas/effects
/// http://www.earslap.com/article/recreating-the-thx-deep-note.html
///
/// - books, documentation, papers, discussions
/// http://basicsynth.com
/// http://www.synthesizer-cookbook.com
/// http://ccrma.stanford.edu/~stilti/papers/blit.pdf
/// http://ccrma.stanford.edu/~jos/sasp/Inverse_FFT_Synthesis.html
/// http://ccrma.stanford.edu/~aguiar/onetool.html Composition with Spectral Modeling Synthesis
/// Real-time Inverse Transform Additive Synthesis for Additive and Pitch Synchronous Noise and Sound Spatialization
///   http://archive.cnmat.berkeley.edu/AES98/Real-time%20inverse/InverseN.html
/// http://en.wikipedia.org/wiki/Additive_synthesis#Inverse_FFT_synthesis
/// http://www.dspguide.com
/// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.43.4818&rep=rep1&type=pdf SPECTRAL ENVELOPES AND INVERSE FFT SYNTHESIS
/// http://www.ee.columbia.edu/~dpwe/papers/RodetD92-envs.pdf
/// http://www.amazon.ca/Computer-Music-Tutorial-Curtis-Roads/dp/0262680823/ref=pd_sim_b_7
/// http://pdf.manual6.com/download.php?id=36191
/// Frequency Shifting with the Phase Vocoder
///   http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.332.1708&rep=rep1&type=pdf
/// Arbitrary Phase Vocoders by means of Warping
///   http://www.univie.ac.at/nuhag-php/bibtex/open_files/14204_EvDoMa13_Musica_Tecnologia.pdf
/// FFT multi-frequency synthesizer
///   http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=196868&url=http%3A%2F%2Fieeexplore.ieee.org%2Fxpls%2Fabs_all.jsp%3Farnumber%3D196868
/// http://mitpress.mit.edu/books/designing-sound
///
/// http://www.kvraudio.com/forum/viewtopic.php?t=364091 FFT synthesis - sine of arbitrary frequency in FFT
// http://stackoverflow.com/questions/4583950/cepstral-analysis-for-pitch-detection/7211695#7211695
/// http://www.embedded.com/design/configurable-systems/4008835/DSP-Tricks-Frequency-Domain-Windowing
/// The comparison of window functions for different subbands in Phase Vocoder
///  http://www.atlantis-press.com/php/download_paper.php?id=10531
/// http://astro.berkeley.edu/~jrg/ngst/fft/leakage.html
/// https://groups.google.com/forum/?fromgroups=#!topic/comp.dsp/3iPm7PMH1Qw (FFT Bin Spectral Leakage)
/// http://fweb.wallawalla.edu/class-wiki/index.php/Exercise:_Sawtooth_Wave_Fourier_Transform
/// http://mathworld.wolfram.com/FourierSeriesSawtoothWave.html
/// http://en.wikipedia.org/wiki/Sawtooth_wave
/// http://en.wikipedia.org/wiki/Sinc_function
/// http://en.wikipedia.org/wiki/Sine_wave
/// Signal modifications using the STFT http://recherche.ircam.fr/anasyn/roebel/amt_audiosignale/VL3.pdf
/// Optimized Sinusoid Synthesis via Inverse Truncated Fourier Transform
///  http://www.cosy.sbg.ac.at/~rkutil/publication/Kutil09a.pdf
/// Multirate additive synthesis http://etheses.dur.ac.uk/5350/1/5350_2788.PDF?UkUDh:CyT
/// Spectral Analysis, Editing, and Resynthesis: Methods and Applications
///  http://www.klingbeil.com/data/Klingbeil_Dissertation_web.pdf
/// Percussion Synthesis https://ccrma.stanford.edu/~sdill/220A-project/drums.html
///
/// - libs, SDKs, code
/// http://ccrma.stanford.edu/software/stk/index.html
/// http://musicdsp.org/archive.php?classid=1
/// http://www.scs.ryerson.ca/~lkolasa/CppWavelets.html
/// http://www.earlevel.com/main/2012/05/25/a-wavetable-oscillator—the-code
/// http://github.com/vinniefalco/DSPFilters
/// http://ldesoras.free.fr
/// http://mobilesynth.googlecode.com/svn/trunk/mobilesynth/Classes/synth
/// http://www.google.co.uk/search?hl=en&q=analog+synthesis+site%3Asourceforge.net
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "synthImpl.hpp"

#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/dft/domainConversion.hpp"
#include "le/math/dft/fft.hpp"
#include "le/math/vector.hpp"
#include "le/math/windows.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/buffers.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/processor.hpp"
#include "le/spectrumworx/engine/setup.hpp"

//#define LE_UTILITY_MATLAB_INTEROP 1
#if LE_UTILITY_MATLAB_INTEROP
#include "le/utility/matlab.hpp"
#endif // LE_UTILITY_MATLAB_INTEROP

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include <cmath>
#include <limits>
#include <numeric>
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
// Synth static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Synth::title      [] = "Synth";
char const Synth::description[] = "Spectrum colour transfer.";


////////////////////////////////////////////////////////////////////////////////
//
// Synth UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Synth::Frequency      , "Frequency"     )
EFFECT_PARAMETER_NAME( Synth::HarmonicSlope  , "Slope"         )
EFFECT_PARAMETER_NAME( Synth::FlangeIntensity, "Flange amount" )
EFFECT_PARAMETER_NAME( Synth::FlangeOffset   , "Flange offset" )


////////////////////////////////////////////////////////////////////////////////
//
// SynthImpl::setup()
// ------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SIZE_BEGIN()

namespace
{
    // http://en.wikipedia.org/wiki/Cent_%28music%29
    //...mrmlj...0 and 7 - approximation of the old carrier.wav
    std::uint8_t BOOST_CONSTEXPR_OR_CONST toneSemitones      [ SynthImpl::numberOfTones ] = { 0, 7                                                  };
    float        BOOST_CONSTEXPR_OR_CONST toneGain           [ SynthImpl::numberOfTones ] = { 1, 1.0f / 6                                           }; // attenuate non fundamental frequencies...
    float        const                    toneFrequencyScales[ SynthImpl::numberOfTones ] = { 1, Math::semitone2Interval12TET( toneSemitones[ 1 ] ) };

    /// \note 'Original carrier.wav' emulation: it appears that the '7th
    /// semitone overtone' does not have 'proper' harmonics itself but rather
    /// each main harmonic has an 'overtone' offset in frequency by the same
    /// amount (in Hz) as the 7th semitone for the main fundamental frequency.
    ///                                       (28.10.2015.) (Domagoj Saric)
    bool const standardHarmonicFormula( false );
} // anonymous namespace

LE_COLD
void SynthImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    synthesisParameters_.setup( engineSetup );

    flangeDelayPhase_ = parameters().get<FlangeOffset>() / FlangeOffset::maximum() * Math::Constants::pi;
    flangeGain_       = std::sqrt( Math::percentage2NormalisedLinear( parameters().get<FlangeIntensity>() ) );

    double const fundamentalFrequency( parameters().get<Frequency>() );
    auto   const freqPerBin          ( engineSetup.frequencyRangePerBin<double>() );

    /// \note Harmonics should have 'alternating'/'opposite' phases.
    /// http://hep.physics.indiana.edu/~rickv/Making_complex_waves.html
    ///                                       (06.10.2015.) (Domagoj Saric)
    {
        auto const halfNumberOfCoefficients  ( static_cast<std::uint8_t>( ( coefficients_.size() - 1 ) / 2 )          );
        auto const nyquist                   ( engineSetup.sampleRate<std::uint32_t>() / 2                            );
        auto const maxBaseBin                ( std::max( 0, workingRange.end() - 1 - halfNumberOfCoefficients )       );
        auto const maxFrequency              ( nyquist * maxBaseBin / engineSetup.numberOfBins()                      );
        auto const maxToneFrequencyMultiplier( toneFrequencyScales[ numberOfTones - 1 ]                               );
        auto const maxToneFrequencyDelta     ( fundamentalFrequency * toneFrequencyScales[ 1 ] - fundamentalFrequency );
        auto const maxHarmonic               (
                                                 standardHarmonicFormula
                                                     ? (   maxFrequency                           / ( fundamentalFrequency * maxToneFrequencyMultiplier ) )
                                                     : ( ( maxFrequency - maxToneFrequencyDelta ) /   fundamentalFrequency                                )
                                             );
        auto const minFrequency              ( ( workingRange.begin() + halfNumberOfCoefficients ) * freqPerBin );
        auto const minHarmonic               ( std::ceil( minFrequency / fundamentalFrequency ) );
        auto const maxHarmonics              ( std::max<float>( 0, maxHarmonic - minHarmonic + 1 ) );

        LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();
        harmonicSlope_ = std::pow( Math::percentage2NormalisedLinear( parameters().get<HarmonicSlope>() ), 4 ); // 'delinearise', slope = slope**4
        auto const maxAudibleHarmonic( 99 / harmonicSlope_ ); // based on the formula for harmonicGain (in process()) and a maximum 40dB harmonic attenuation
        harmonics_     = static_cast<std::uint8_t>( std::min<float>( maxAudibleHarmonic, maxHarmonics ) );
        startHarmonic_ = static_cast<std::uint8_t>( minHarmonic );
    }

    if
    (
        ( lastFreq_    != fundamentalFrequency                 ) ||
        ( lastFFTSize_ != engineSetup.fftSize<std::uint16_t>() ) ||
        ( lastWindow_  != engineSetup.windowFunction        () )
    )
    {
        lastFreq_    = fundamentalFrequency;
        lastFFTSize_ = engineSetup.fftSize<std::uint16_t>();
        lastWindow_  = engineSetup.windowFunction        ();

        auto const freqBinFractional( fundamentalFrequency / freqPerBin );
        auto const freqBin          ( static_cast<std::uint16_t>( Math::round( freqBinFractional ) ) );

        auto const omega( Math::Constants::twoPi_d * fundamentalFrequency );

        auto const sr          ( engineSetup.sampleRate<double>() );
        auto const numberOfBins( engineSetup.numberOfBins()       );

        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( freqCoefficients, Engine::real_t, lastFFTSize_ + 2 + 16 /*for alignment padding between real and imag*/ );
        LE_DISABLE_LOOP_UNROLLING()
        for ( std::uint16_t bin( 0 ); bin < lastFFTSize_; ++bin )
            freqCoefficients[ bin ] = std::sin( omega * bin / sr );

        auto const & processor( Engine::Processor::fromEngineSetup( engineSetup ) );

        // https://ccrma.stanford.edu/~jos/parshl/Analysis_Window_Step_1.html
        Math::multiply( processor.analysisWindow().begin(), freqCoefficients.begin(), lastFFTSize_ );
      //Math::copy    ( processor.analysisWindow().begin(), freqCoefficients.begin(), lastFFTSize_ );

        auto const & fft( processor.fft() );

        auto const pTimeDomain( freqCoefficients.begin() );
        auto const pReals     ( freqCoefficients.begin() );
        auto const pImags     ( static_cast<float *>( Math::align( &freqCoefficients[ numberOfBins ] ) ) );
        fft.transform( pTimeDomain, DataRange( pImags, pImags + numberOfBins ), true );

        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( amps  , Engine::real_t, numberOfBins );
        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( phases, Engine::real_t, numberOfBins );
        auto const pAmps  ( amps  .begin() );
        auto const pPhases( phases.begin() );
        Math::reim2AmPh( pReals, pImags, pAmps, pPhases, numberOfBins );
        coefficients_.fill( 0 );
        auto const coefficientsMiddle      ( static_cast<std::uint8_t>( ( coefficients_.size() - 1 ) / 2 ) );
        auto const halfNumberOfCoefficients( std::min<std::uint16_t>( coefficientsMiddle, freqBin - 1 )    );
        auto const startCoefficient        ( coefficientsMiddle - halfNumberOfCoefficients                 );
        std::copy( &pAmps[ freqBin - halfNumberOfCoefficients ], &pAmps[ freqBin + 1 + halfNumberOfCoefficients ], &coefficients_[ startCoefficient ] );

        resetState_ = true;
    }
    else
        resetState_ = false;
}

LE_COLD
void SynthImpl::ChannelState::reset()
{
    /// \note Randomize initial phases for a more 'spacey' effect. To be further
    /// investigated...
    ///                                       (22.10.2015.) (Domagoj Saric)
#if 0
    LE_DISABLE_LOOP_UNROLLING()
    for ( auto & oscillatorPhases : phases )
        LE_DISABLE_LOOP_UNROLLING()
        for ( auto & phase : oscillatorPhases ) { phase = Math::rangedRand( Math::Constants::pi ); }
#else
    Math::clear( phases.front().begin(), phases.back().end() );
#endif // initial phases generation
}

LE_COLD
void SynthImpl::ChannelState::resize( Engine::StorageFactors const & factors, Engine::Storage & storage )
{
    LE_DISABLE_LOOP_UNROLLING()
    for ( auto & oscillatorPhases : phases )
        oscillatorPhases.resize( factors, storage );
}

LE_COLD
std::uint32_t SynthImpl::ChannelState::requiredStorage( Engine::StorageFactors const & factors )
{
    return Utility::align( static_cast<std::uint32_t>( OscillatorPhases::requiredStorage( factors ) ) ) * static_cast<std::uint8_t>( Phases().size() );
}

LE_OPTIMIZE_FOR_SIZE_END()

LE_OPTIMIZE_FOR_SPEED_BEGIN()

////////////////////////////////////////////////////////////////////////////////
//
// SynthImpl::process()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace //...mrmlj...copy-pasted from phase vocoder sources...
{
    LE_FORCEINLINE LE_HOT
    float LE_FASTCALL reconstructPhase
    (
        float const estimatedFrequency,
        float const binFrequency,
        float const invDeviationFactor,
        float const expectedPhaseDifference,
        float const currentPhaseSum
    )
    {
        float const phase
        (
            ( estimatedFrequency - binFrequency ) * invDeviationFactor
                +
            expectedPhaseDifference
        );

        float const phaseSum( currentPhaseSum + phase );
        return phaseSum;
    }

    LE_FORCEINLINE LE_HOT
    float LE_FASTCALL mapTo2PiInterval( float const phase )
    {
        float const reducedPhase( Math::PositiveFloats::modulo( phase, Math::Constants::twoPi ) );
        return reducedPhase;
    }
} // anonymous namespace

LE_HOT
void SynthImpl::process( SynthImpl::ChannelState & cs, Engine::MainSideChannelData_AmPh data, Engine::Setup const & engineSetup ) const
{
    if ( BOOST_UNLIKELY( resetState_ ) )
        cs.reset();

#if 1
    auto & targetData( const_cast<Engine::ChannelData_AmPh &>( data.side() ) );
#else
    auto & targetData(                                         data.main()   );
#endif

    float const freqPerBin             ( engineSetup.frequencyRangePerBin<float>() );
    float const expectedPhaseDifference( synthesisParameters_.expctRate         () );
    float const invDeviationFactor     ( synthesisParameters_.invDeviationFactor() );
    float const fundamentalFrequency   ( parameters().get<Frequency>()             );

    float const toneFrequencyDeltas[ numberOfTones ] = { 0, fundamentalFrequency * toneFrequencyScales[ 1 ] - fundamentalFrequency };

    // http://www.uaudio.com/blog/flangers-and-phasers
    // http://www.soundonsound.com/sos/mar06/articles/qa0306_1.htm What's the difference between phasing and flanging?
    // http://www.harmonycentral.com/forum/forum/guitar/acapella-29/1850979-
    // https://www.reddit.com/r/DSP/comments/34w3d5/fft_to_measure_relative_phase_shift_of_two_signals
    // https://www.cs.sfu.ca/~tamaras/delayEffects/delayEffects.pdf

    /// \note This could be slightly optimised by incorporating the clearing of
    /// (only the actually unused) bins into the loop below.
    ///                                       (27.10.2015.) (Domagoj Saric)
    //Math::clear( targetData.amps  () );
    //Math::clear( targetData.phases() );
    //...mrmlj...clear the entire side chain as quick-fix for correct 'reduced working range' operation...
    Math::clear( targetData.full().jointView() );

    /// \note Our basBin calculation below gives 'full spectrum' bin numbers so
    /// we use full range data while obeying the working range by limiting the
    /// harmonic range.
    ///                                       (27.10.2015.) (Domagoj Saric)
    auto const amps  ( targetData.full().amps  () );
    auto const phases( targetData.full().phases() );

    float const flangeDelayPhase( flangeDelayPhase_ );
    float const flangeGain      ( flangeGain_       );

    auto  const harmonicSlope   ( harmonicSlope_                                            );
    float const harmonicGainBase( 2 / Math::Constants::pi * ( 0.1f + 0.9f * harmonicSlope ) ); // "rotate" rather then just "slope" the harmonics to preserve loudness
    auto        harmonics       ( harmonics_                                                );
    bool        evenHarmonic    ( true                                                      );

    //...mrmlj...think of a better name...
    auto const halfNumberOfCoefficients( static_cast<std::uint8_t>( ( coefficients_.size() - 1 ) / 2 ) );

    /// \note Math::addPolar() calls atan2() which can in turn do a division by
    /// zero internally.
    ///                                       (22.10.2015.) (Domagoj Saric)
    LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();

    for ( std::uint8_t harmonic( startHarmonic_ ); harmonics; ++harmonic, --harmonics )
    {
        evenHarmonic = !evenHarmonic;

        float const harmonicGain( harmonicGainBase / ( harmonicSlope * harmonic + 1 ) );
        //attenuation *= std::sqrt( attenuation );
        //attenuation *= Math::dB2NormalisedLinear( harmonic * 0.1f );

        /// \note Use a phase shift instead of a negative amplitude
        /// in order to obey the 'no negative amps' rule.
        ///                                   (06.10.2015.) (Domagoj Saric)
        float const phaseInversion( evenHarmonic ? 0 : Math::Constants::pi );

        auto const harmonicFrequency( fundamentalFrequency * harmonic );

        for ( std::uint8_t tone( 0 ); tone < numberOfTones; ++tone )
        {
            float const frequency
            (
                standardHarmonicFormula
                    ? ( harmonicFrequency * toneFrequencyScales[ tone ] )
                    : ( harmonicFrequency + toneFrequencyDeltas[ tone ] )
            );
            float const gain( toneGain[ tone ] * harmonicGain );

            auto          const baseBin ( static_cast<std::uint16_t>( Math::round( frequency / freqPerBin ) ) );
            std::uint16_t const beginBin( baseBin     - halfNumberOfCoefficients );
            std::uint16_t const endBin  ( baseBin + 1 + halfNumberOfCoefficients );
            BOOST_ASSERT( endBin <= targetData.endBin() );

            float const * LE_RESTRICT pAmp     ( &coefficients_        [        0 ] );
            float       * LE_RESTRICT pPhaseSum( &cs.phases    [ tone ][ beginBin ] );
            //Math::clear( std::min<float *>( pCSPhase, pPhaseSum ), pPhaseSum ); //...mrmlj...always clear all 'unused' phase bins...
            for ( auto bin( beginBin ); bin != endBin; ++bin )
            {
                float const phaseDelta         ( bin * expectedPhaseDifference );
                float const currentBinFrequency( bin * freqPerBin              );
                float const phaseSum( reconstructPhase( frequency, currentBinFrequency, invDeviationFactor, phaseDelta, *pPhaseSum ) );
                float const amp     ( *pAmp++ * gain );
                *pPhaseSum++ = mapTo2PiInterval( phaseSum );
                float const phase( phaseSum + phaseInversion );
                /// \note The addPolar() function is the bottlneck here so
                /// try to avoid it if possible.
                ///                           (05.10.2015.) (Domagoj Saric)
                if ( amps[ bin ] )
                    Math::addPolar( amp, phase, amps[ bin ], phases[ bin ] );
                else
                {
                    amps  [ bin ] = amp  ;
                    phases[ bin ] = phase;
                }

                if ( flangeGain )
                {
                    Math::addPolar
                    (
                        amp * flangeGain,
                        phase + ( harmonic * flangeDelayPhase ), // https://en.wikipedia.org/wiki/Linear_phase
                        amps  [ bin ],
                        phases[ bin ]
                    );
                }
            }
            //pCSPhase = pPhaseSum;
        }
    }

    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow | Math::Negative, amps  , "synth amplitudes" );
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow                 , phases, "synth phases"     );
}

LE_OPTIMIZE_FOR_SPEED_END()

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
