////////////////////////////////////////////////////////////////////////////////
///
/// vocoderImpl.cpp
/// ---------------
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
#include "vocoderImpl.hpp"

#include "le/math/constants.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/processor.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#if LE_UTILITY_MATLAB_INTEROP
#include "le/utility/matlab.hpp"
#endif // LE_UTILITY_MATLAB_INTEROP

#include "boost/range/adaptor/reversed.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include "boost/assert.hpp"
#include "boost/concept_check.hpp"
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
// Vocoder static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Vocoder::title      [] = "Vocoder";
char const Vocoder::description[] = "Classic vocoding.";


////////////////////////////////////////////////////////////////////////////////
//
// Vocoder UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Vocoder    ::EnvelopeBorder, "Envelope border" )
EFFECT_PARAMETER_NAME( Vocoder    ::NoiseIntensity, "Carrier noise"   )
EFFECT_PARAMETER_NAME( VocoderImpl::FilterMethod  , "Filter method"   )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    VocoderImpl, FilterMethod,
    (( CepstrumUdoBrick, "Cepstrum - Brick - Udo" ))
    (( CepstrumBrick   , "Cepstrum - Brick"       ))
    (( CepstrumHamming , "Cepstrum - Hamming"     ))
    (( MovingAverage   , "Moving average"         ))
    (( Envelope        , "Envelope"               ))
    (( MelEnvelope     , "Mel envelope"           ))
    (( Passthrough     , "Passthrough"            ))
)


////////////////////////////////////////////////////////////////////////////////
//
// VocoderImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////
LE_COLD
void VocoderImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    std::uint32_t envelopeBorder( parameters().get<EnvelopeBorder>() );

    filterLength_ = 0;
    switch ( filterMethod() )
    {
        case FilterMethod::MovingAverage:
            using Math::convert;
            filterLength_ =
                std::min<std::uint16_t>
                (
                    workingRange.size() - 1,
                    // http://mathforum.org/kb/thread.jspa?forumID=13&threadID=91719&messageID=455475
                    convert<std::uint16_t>( 0.443f * engineSetup.sampleRate<float>() / convert<float>( envelopeBorder ) )
                );
            break;
        case FilterMethod::CepstrumHamming:
            //...mrmlj...try to achieve a similar envelope to the brickwall cases...
            envelopeBorder = std::min( engineSetup.sampleRate<std::uint32_t>() / 2, envelopeBorder * 2 );
            break;
        default: /*filterLength_ unused*/ break;
    }

    cutoff_ = engineSetup.frequencyInHzToBin( envelopeBorder );
}


////////////////////////////////////////////////////////////////////////////////
//
// VocoderImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace
{
    class RC
    {
    protected:
        void reset() { envelope_ = 0; }

        static float LE_FASTCALL tau( std::uint16_t const cutoffBin, Engine::Setup const & setup, float const correction = 1 )
        {
            // http://en.wikipedia.org/wiki/Rise_time
            // https://en.wikipedia.org/wiki/Exponential_smoothing#Time_Constant
            // tr(1%:99%) ~ 4.59 tau
            float const spectrumSampleRate( setup.frequencyRangePerBin<float>()  );
            float const cutoffFrequency   ( spectrumSampleRate * cutoffBin       );
            float const tau( 0.35 / cutoffFrequency / 2.2 );
            return 1 - Math::exp( -1 / ( tau * setup.sampleRate<float>() * correction ) );
        }

        float LE_FASTCALL process( float const sample, float const release ) const
        {
            LE_ASSUME( sample    >= 0 );
            LE_ASSUME( envelope_ >= 0 );
            auto const delta      ( sample - envelope_ );
            auto const newEnvelope( ( delta < 0 ) ? envelope_ + release * delta : sample );
            LE_ASSUME( newEnvelope >= sample );
            envelope_ = newEnvelope;
            return newEnvelope;
        }

    private:
        mutable float envelope_;
    }; // class RC

    class ConstantRC : private RC
    {
    public:
        void setup( std::uint16_t const cutoffBin, Engine::Setup const & setup ) { release_ = tau( cutoffBin, setup ); }
        using RC::reset;
        float process( float const sample ) const { return RC::process( sample, release_ ); }

    private:
        float release_;
    }; // class ConstantRC

    class LogRC : private RC
    {
    public:
        void setup( std::uint16_t const cutoffBin, Engine::Setup const & setup )
        {
            /// \note A prototype implementation that tries to emulate the "Mel
            /// scale" of frequencies by using a progressively longer release
            /// time for higher frequencies (expecting that formants are further
            /// apart higher up the spectrum).
            /// http://en.wikipedia.org/wiki/Mel_scale
            ///                               (06.03.2015.) (Domagoj Saric)
            auto const endOfSpectrumReleaseCorrection( 250 );
            auto const beginRelease( tau( cutoffBin, setup                                 ) );
            auto const endRelease  ( tau( cutoffBin, setup, endOfSpectrumReleaseCorrection ) );

            release_        = beginRelease;
            currentRelease_ = beginRelease;

            releaseDelta_ = ( endRelease - beginRelease ) / setup.numberOfBins();
        }
        void reset() { RC::reset(); /*...mrmlj...currentRelease_ = release_;*/ }

        float LE_FASTCALL processForward( float const sample ) const
        {
            auto const result( RC::process( sample, currentRelease_ ) );
            currentRelease_ += releaseDelta_;
            return result;
        }
        float LE_FASTCALL processBackward( float const sample ) const
        {
            auto const result( RC::process( sample, currentRelease_ ) );
            currentRelease_ -= releaseDelta_;
            return result;
        }

        void skipSamplesForward ( std::uint16_t samplesToSkip ) const { while ( BOOST_UNLIKELY( samplesToSkip-- ) ) processForward ( 0 ); }
        void skipSamplesBackward( std::uint16_t samplesToSkip ) const { while ( BOOST_UNLIKELY( samplesToSkip-- ) ) processBackward( 0 ); }

    private:
                float release_     ;
                float releaseDelta_;

        mutable float currentRelease_;
    }; // class LogRC
} // anonymous namespace

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4702 ) // Unreachable code.
#endif // _MSC_VER
LE_HOT
void VocoderImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & setup ) const
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

    DataRange envelope( data.main().amps() ); // most 'filter methods' create the envelope in-place
    std::uint16_t const skippedLeadingBins ( data.beginBin()                  );
    std::uint16_t const skippedTrailingBins( fullNumberOfBins - data.endBin() );

#if LE_UTILITY_MATLAB_INTEROP
    auto & filter( parameters().get<FilterMethod>() ); filter; // make it easier to change from the debugger
    namespace Matlab = Utility::Matlab;

    Matlab::Engine::singleton().setVariable( "amps", envelope );
#endif // LE_UTILITY_MATLAB_INTEROP

    if ( filterMethod() == FilterMethod::MovingAverage )
    {
        //...mrmlj...probably still broken 'reduced range' operation...
        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( workBuffer, Engine::real_t, fullNumberOfBins );
        lowPassSpectrum_movingAverage( envelope, workBuffer, setup );
    #ifndef NDEBUG //...mrmlj...?...clear out negative values to avoid assertion failures.
        for ( auto & amp : envelope ) { amp = std::max( 0.0f, amp ); }
    #endif // NDEBUG
    }
    else
    if ( filterMethod() == FilterMethod::Envelope )
    {
        ConstantRC envelopeCalculator;
        envelopeCalculator.setup( cutoff_, setup );
        envelopeCalculator.reset();
        for ( float & bin :                           envelope   ) { bin = envelopeCalculator.process( bin ); }
        envelopeCalculator.reset();
        for ( float & bin : boost::adaptors::reverse( envelope ) ) { bin = envelopeCalculator.process( bin ); }
    }
    else
    if ( filterMethod() == FilterMethod::MelEnvelope )
    {
        LogRC envelopeCalculator;
        envelopeCalculator.setup( cutoff_, setup );
        envelopeCalculator.reset();
        envelopeCalculator.skipSamplesForward ( skippedLeadingBins  );
        for ( float & bin :                           envelope   ) { bin = envelopeCalculator.processForward ( bin ); }
        envelopeCalculator.reset();
        envelopeCalculator.skipSamplesBackward( skippedTrailingBins );
        for ( float & bin : boost::adaptors::reverse( envelope ) ) { bin = envelopeCalculator.processBackward( bin ); }
    }
    else
    if ( filterMethod() == FilterMethod::Passthrough )
    {
    }
    else
    {
        // Allocate a work buffer for envelope calculation that can hold a whole
        // time domain frame (i.e. the size of the FFT but allocate it as "two
        // aligned half-FFT size" buffers so that it can be used for both time
        // domain and ReIm intermediate results):
        BOOST_SIMD_ALIGNED_STACK_BUFFER( doubleWorkBuffer, Engine::real_t, ( alignIndex( fullNumberOfBins ) * 2 ) );
        envelope = DataRange( &doubleWorkBuffer[ 0 ], &doubleWorkBuffer[ fullNumberOfBins ] );

        // Cepstrum-based envelope calculation needs its work buffer to be the
        // whole frame size and to thus alias the envelope buffer (for inplace
        // DFT calculation).
        add( data.full().main().amps(), std::numeric_limits<float>::epsilon(), envelope );
        ln ( envelope );
        lowPassSpectrum_cepstrum( envelope, doubleWorkBuffer, setup );
        envelope.advance_begin( + skippedLeadingBins  );
        envelope.advance_end  ( - skippedTrailingBins );
        exp( envelope );
    #ifndef NDEBUG
        // Clear out invalid values to avoid assertion failures.
        for ( auto & amp : envelope ) { if ( !std::isfinite( amp ) ) amp = 0; }
    #endif // NDEBUG
    }

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

#if LE_UTILITY_MATLAB_INTEROP
    Matlab::Engine::singleton().setVariable( "envs", envelope );
#endif // LE_UTILITY_MATLAB_INTEROP

    if ( BOOST_UNLIKELY( parameters().get<NoiseIntensity>() ) )
    {
        /// \note Modifying the carrier directly is easier and faster but does
        /// not play nice with downstream effects that also use the side chain.
        ///                                   (23.10.2015.) (Domagoj Saric)
        // http://www.dsprelated.com/showthread/comp.dsp/102908-1.php
        auto & carrierData( const_cast<Engine::ChannelData_AmPh &>( data.side() ) ); //...mrmlj...

        float const noiseLevel( std::sqrt( Math::percentage2NormalisedLinear( parameters().get<NoiseIntensity>() ) ) );

        float const ampNoiseRange( noiseLevel * setup.maximumAmplitude() / 8 );
        for ( auto & amp : carrierData.amps() ) { amp += Math::rangedRand( ampNoiseRange ); }

        // float const phaseNoiseRange( noiseLevel * Math::Constants::twoPi );
        // for ( auto & phase : carrierData.phases() ) { phase += Math::rangedRand( phaseNoiseRange ); }
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

#if LE_UTILITY_MATLAB_INTEROP
    static bool freqsSet( false );
    if ( !freqsSet )
    {
        Matlab::Array freqs;
        freqs.resize( envelope.size() );
        auto * pFreqs( freqs.get() );
        for ( std::uint16_t freq( 0 ); freq < setup.numberOfBins(); ++freq )
            *pFreqs++ = freq * setup.frequencyRangePerBin<float>();
        Matlab::Engine::singleton().setVariable( "freqs", freqs );
        Matlab::Engine::singleton().execute
        (
          //"close all;"
          //"figure( 'units', 'normalized', 'outerposition', [0 0 1 1] );"
            "set( gca, 'position', [.02,.02,.98,.97] );"
        );
        freqsSet = true;
    }
    Matlab::Engine::singleton().setVariable( "carrier", data.side().amps() );
    Matlab::Engine::singleton().setVariable( "voco"   , data.main().amps() );
    Matlab::Engine::singleton().execute
    (
        "hold off;"
        "semilogy( freqs(1:200), amps   (1:200), 'b--.' );"
        "hold on;"
        "semilogy( freqs(1:200), envs   (1:200), 'r-o'  );"
        "semilogy( freqs(1:200), carrier(1:200), 'g-^'  );"
        "semilogy( freqs(1:200), voco   (1:200), 'k-*'  );"
    );
#endif // LE_UTILITY_MATLAB_INTEROP
}

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

void LE_HOT VocoderImpl::lowPassSpectrum_cepstrum
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
    auto const cutoff      ( cutoff_                 );
    auto const cutoffMirror( fft.size() - cutoff + 1 );

    switch ( filterMethod() )
    {
        case FilterMethod::CepstrumUdoBrick:
        {
            /// \note According to DAFX equation 9.24 the cepstrum should be
            /// multiplied by 2 but this gives a too quiet output and is
            /// currently skipped as a quick-fix.
            ///                               (07.11.2013.) (Domagoj Saric)
            float const udoGain( 1 /*2*/ );
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
            break;
        }

        case FilterMethod::CepstrumBrick:
            clear( &pCepstrum[ cutoff + 2 ], &pCepstrum[ cutoffMirror - 2 ] );
            // an attempt to make the LP less of a brick...
            pCepstrum[ cutoff - 1 ] = pCepstrum[ cutoffMirror + 1 ] = pCepstrum[ cutoff - 1 ] * 0.75f;
            pCepstrum[ cutoff     ] = pCepstrum[ cutoffMirror     ] = pCepstrum[ cutoff     ] * 0.50f;
            pCepstrum[ cutoff + 1 ] = pCepstrum[ cutoffMirror - 1 ] = pCepstrum[ cutoff + 1 ] * 0.25f;
            break;

        case FilterMethod::CepstrumHamming:
        {
            float const dw( Math::Constants::pi / Math::convert<float>( cutoff ) );
            float        w( cutoff * dw );
            for ( std::uint16_t bin( 1 ); bin < cutoff; ++bin )
            {
                float const window( static_cast<float>( 0.54 - 0.46 * std::cos( w ) ) );
                pCepstrum[ bin              ] *= window;
                pCepstrum[ fft.size() - bin ] *= window;
                w += dw;
            }
            clear( &pCepstrum[ cutoff ], &pCepstrum[ cutoffMirror ] );
            break;
        }

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    // Transform back into the frequency domain (low passed spectrum/envelope)
    // and discard the imaginary components:
    fft.transform( pCepstrum, pImags, fft.size() );

    //...mrmlj...testing what to do about imaginary components...
    //BOOST_ASSERT_MSG( Math::max( pImags, engineSetup.numberOfBins() ) < std::numeric_limits<float>::epsilon() * 10, "Power spectrum not real." );
    //for ( std::uint16_t bin( 0 ); bin < engineSetup.numberOfBins(); ++bin )
    //{
    //    float const real( pCepstrum[ bin ] );
    //    float const imag( pImags   [ bin ] );
    //    pCepstrum[ bin ] = Math::copySign( std::sqrt( real * real + imag * imag ), real );
    //}
}


void VocoderImpl::lowPassSpectrum_movingAverage
(
    DataRange     const & spectrum,
    DataRange     const & workBuffer,
    Engine::Setup const &
) const
{
    if ( !filterLength_ )
        return;

    // Moving Average Filters
    // http://www.analog.com/media/en/technical-documentation/dsp-book/dsp_book_Ch15.pdf
    BOOST_ASSERT_MSG( spectrum.size() == workBuffer.size(), "Buffer sizes mismatch." );
    DataRange const smoothedSpectrum( workBuffer );
    Math::symmetricMovingAverage( spectrum        , smoothedSpectrum, filterLength_ );
    Math::copy                  ( smoothedSpectrum, spectrum                        );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
