////////////////////////////////////////////////////////////////////////////////
///
/// processor.cpp
/// -------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "processor.hpp"

#include "module.hpp"
#include "moduleChainImpl.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/constants.hpp"
#include "le/math/vector.hpp"
#include "le/math/windows.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/utility/parentFromMember.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include <boost/assert.hpp>

#include <algorithm>
#include <cfloat>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

LE_NOTHROW
void Processor::preProcess()
{
    modules().preProcessAll( lfoTimer(), engineSetup() );
}

////////////////////////////////////////////////////////////////////////////////
///
/// \class Processor::ProcessParameters
///
////////////////////////////////////////////////////////////////////////////////

class Processor::ProcessParameters
{
public:
    ProcessParameters
    (
        InputData  inputs,
        InputData  sideChannel,
        OutputData outputs,
        Channels const & channels,
        std::uint32_t numberOfSamples,
        float         outputGain,
        float         mixAmount
    );
    ProcessParameters( ProcessParameters const & ) = delete;

    std::uint8_t  currentChannel () const { return currentChannel_ ; }
    std::uint32_t numberOfSamples() const { return numberOfSamples_; }

    float const & mixPercentage() const { return mixPercentage_; }
    float const & outputScaling() const { return outputScaling_; } ///< Combined postAmp and mixPercentage

    bool doMix() const { return doMix_; }

    float const * mainChannel   () const { LE_ASSUME( *ppMainChannels_ ); return *ppMainChannels_; }
    float const * sideChannel   () const {                                return *ppSideChannels_; }
#ifdef LE_SW_PURE_ANALYSIS
    float       * output        () const { return nullptr; }
#else
    float       * output        () const { BOOST_ASSERT( pOutput_ ); return *pOutput_; }
#endif // LE_SW_PURE_ANALYSIS
    ChannelBuffers & channelBuffers() const { return channelBuffers_.front(); }

    bool haveSideChannel() const { return sideChannel() != nullptr; }

    bool setNextChannel( std::uint8_t const numberOfChannels )
    {
        ++currentChannel_;
        ++ppMainChannels_;
        ++pOutput_       ;
        channelBuffers_.advance_begin( 1 );
        if ( haveSideChannel() )
            ++ppSideChannels_;

        if ( currentChannel() < numberOfChannels ) return true ;
        else                                       return false;
    }

private:
    InputData  ppMainChannels_;
    InputData  ppSideChannels_;
    OutputData pOutput_       ;

    boost::iterator_range<ChannelBuffers * LE_RESTRICT> channelBuffers_;

    std::uint8_t currentChannel_;

    std::uint32_t const numberOfSamples_;

    float const mixPercentage_;
    float const outputScaling_;

    bool const doMix_;
}; // class Processor::ProcessParameters


////////////////////////////////////////////////////////////////////////////////
//
// Processor::process()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS
void Processor::process /// \throws nothing
(
    InputData     const mainInputs,
    InputData     const sideInputs,
    OutputData    const outputs   ,
    std::uint32_t const samples   ,
    float         const outputGain,
    float         const mixAmount
)
{
    BOOST_ASSERT_MSG
    (
        engineSetup().fftSize                <std::uint16_t>() &&
        engineSetup().windowOverlappingFactor<std::uint8_t >(),
        "WOLA parameters not setup."
    );

#ifdef LE_SW_SDK_BUILD
    Math::FPUDisableDenormalsGuard const disableDenormals;
#endif // LE_SW_SDK_BUILD

    preProcess();

    ProcessParameters processParameters
    (
        mainInputs,
        sideInputs,
        outputs   ,
        channels_ ,
        samples   ,
        outputGain,
        mixAmount
    );

    do
    {
        processSingleChannel( processParameters );
    }
    while ( processParameters.setNextChannel( engineSetup().numberOfChannels() ) );
}


namespace
{
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
    float * * LE_FASTCALL makeDeinterLeaveBuffers
    (
        boost::iterator_range<float *  > const deinterLeavedDataStorage ,
        boost::iterator_range<float * *> const deinterLeavedDataPointers,
        std::uint16_t const size,
        std::uint8_t  const numberOfChannels
    )
    {
        LE_ASSUME( deinterLeavedDataStorage .begin() );
        LE_ASSUME( deinterLeavedDataPointers.begin() );
        LE_DISABLE_LOOP_VECTORIZATION()
        for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
        {
        #ifndef NDEBUG
            if ( size == 0 )
                deinterLeavedDataPointers[ channel ] = deinterLeavedDataStorage.begin();
            else
        #endif // NDEBUG
            deinterLeavedDataPointers[ channel ] = &deinterLeavedDataStorage[ channel * size ];
            LE_ASSUME( deinterLeavedDataPointers[ channel ] );
        }
        auto const resultPointer( deinterLeavedDataPointers.begin() );
        LE_ASSUME( resultPointer );
        return resultPointer;
    }
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

    /// \note Clang's alloca returns unaligned pointers and the Boost.SIMD
    /// macros used to crash with Apple's Clang (Xcode 5, 6) so always look
    /// here if strange things happen with Clang builds and non-mono interleaved
    /// input (because of Clang's unaligned alloca we have to use the aligned
    /// version of the macro even for the pointer array).
    /// https://bugs.chromium.org/p/nativeclient/issues/detail?id=3795
    /// https://llvm.org/bugs/show_bug.cgi?id=22728
    ///                                       (24.05.2016.) (Domagoj Saric)
    #define LE_MAKE_DEINTERLEAVE_BUFFERS( resultPointer, size, numberOfChannels )                                                                            \
        BOOST_SIMD_ALIGNED_STACK_BUFFER( resultPointer##DeinterLeavedDataStorage , float  , size * numberOfChannels );                                       \
        BOOST_SIMD_ALIGNED_STACK_BUFFER( resultPointer##DeinterLeavedDataPointers, float *,        numberOfChannels );                                       \
        resultPointer = makeDeinterLeaveBuffers( resultPointer##DeinterLeavedDataStorage, resultPointer##DeinterLeavedDataPointers, size, numberOfChannels )
} // anonymous namespace

LE_NOTHROWNOALIAS
void Processor::process /// \throws nothing
(
    InterleavedInputData        interleavedMainInputs,
    InterleavedInputData        interleavedSideInputs,
    InterleavedOutputData       interleavedOutputs,
    std::uint32_t               samples,
    float                 const outputGain,
    float                 const mixAmount
)
{
    auto const numberOfChannels( engineSetup().numberOfChannels() );

#ifdef LE_SW_PURE_ANALYSIS
#if !( defined( __clang__ ) && ( defined( __arm__ ) || defined( __aarch64__ ) ) )
    LE_ASSUME( interleavedSideInputs == nullptr );
#endif // crashy clang 3.8 (ndk11c)..!?
    LE_ASSUME( interleavedOutputs    == nullptr );
#endif // LE_SW_PURE_ANALYSIS

                                 LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, boost::make_iterator_range_n( interleavedMainInputs, samples * numberOfChannels ), "main input" );
    if ( interleavedSideInputs ) LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, boost::make_iterator_range_n( interleavedSideInputs, samples * numberOfChannels ), "side input" );


    BOOST_ASSERT_MSG
    (
        engineSetup().fftSize                <std::uint16_t>() &&
        engineSetup().windowOverlappingFactor<std::uint8_t >(),
        "WOLA parameters not setup."
    );

#ifdef LE_SW_SDK_BUILD
    Math::FPUDisableDenormalsGuard const disableDenormals;
#endif // LE_SW_SDK_BUILD

    preProcess();

    float const * LE_RESTRICT const * LE_RESTRICT mainInputs;
    float const * LE_RESTRICT const * LE_RESTRICT sideInputs;
    float       * LE_RESTRICT const * LE_RESTRICT outputs   ;

    std::uint32_t processBlockSize;

#ifdef LE_MELODIFY_SDK_BUILD
    LE_ASSUME( interleavedSideInputs );
#endif // LE_MELODIFY_SDK_BUILD

    if ( numberOfChannels == 1 )
    {
        mainInputs =                         &interleavedMainInputs          ;
        sideInputs = interleavedSideInputs ? &interleavedSideInputs : nullptr;
        outputs    =                         &interleavedOutputs             ;

        processBlockSize = samples;
    }
    else
    {
        processBlockSize = std::min<std::uint32_t>( samples, engineSetup().fftSize<std::uint16_t>() );
        std::uint16_t const deinterleaveBlockSize( Math::alignIndex( processBlockSize ) );
        LE_MAKE_DEINTERLEAVE_BUFFERS( outputs, deinterleaveBlockSize, numberOfChannels );
        mainInputs = outputs; // we support in-place processing (so we can reuse the buffer)
        if ( interleavedSideInputs ) { LE_MAKE_DEINTERLEAVE_BUFFERS( sideInputs, deinterleaveBlockSize, numberOfChannels ); }
        else                         { sideInputs = nullptr; }
    #ifdef LE_SW_PURE_ANALYSIS
        outputs = nullptr;
    #endif // LE_SW_PURE_ANALYSIS
    }

    LE_DISABLE_LOOP_VECTORIZATION()
    while ( samples )
    {
        if ( numberOfChannels == 1 )
        {
            LE_ASSUME( processBlockSize == samples );
        }
        else
        {
            processBlockSize = std::min<std::uint32_t>( processBlockSize, samples );
                                         Math::deinterleave( interleavedMainInputs, const_cast<float * const *>( mainInputs ), processBlockSize, numberOfChannels );
            if ( interleavedSideInputs ) Math::deinterleave( interleavedSideInputs, const_cast<float * const *>( sideInputs ), processBlockSize, numberOfChannels );
        }

        ProcessParameters processParameters
        (
            mainInputs,
            sideInputs,
            outputs   ,
            channels_ ,
            processBlockSize,
            outputGain,
            mixAmount
        );

        do
        {
            processSingleChannel( processParameters );
        }
        while ( processParameters.setNextChannel( numberOfChannels ) );

        if ( numberOfChannels != 1 )
        {
        #ifndef LE_SW_PURE_ANALYSIS
            Math::interleave( outputs, interleavedOutputs, processBlockSize, numberOfChannels );
        #endif // LE_SW_PURE_ANALYSIS

                                         interleavedMainInputs += processBlockSize * numberOfChannels;
            if ( interleavedSideInputs ) interleavedSideInputs += processBlockSize * numberOfChannels;
        #ifndef LE_SW_PURE_ANALYSIS
                                         interleavedOutputs    += processBlockSize * numberOfChannels;
        #endif // LE_SW_PURE_ANALYSIS

            samples -= processBlockSize;
        }
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// Processor::processSingleChannel()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW
void Processor::processSingleChannel( ProcessParameters const & processParameters ) /// \throws nothing
{
    auto const stepSize        ( engineSetup().stepSize  <std::uint16_t>() );
    auto const windowSizeFactor( engineSetup().windowSizeFactor         () );
    auto const windowSize      ( engineSetup().windowSize<std::uint16_t>() );
    BOOST_ASSERT( windowSize == static_cast<std::uint16_t>( analysisWindow().size() ) );

#ifndef LE_SW_PURE_ANALYSIS
    // Implementation note:
    //   ChannelBuffers::readyOutputDataSize() does not take into account the
    // samples that are not yet ready/'fully complete' (have not gone through
    // all the OLA steps) but are nonetheless real data. As each OLA step adds
    // windowSize new samples of which only the first hopSize become 'fully
    // complete' (i.e. constitute the final OLA step for that hop-sized chunk of
    // samples) it follows that we always have (windowSize - hopSize) of such
    // 'valid but not complete' samples following the
    // ChannelBuffers::outputOLAPosition_ (readyOutputDataSize()).
    //                                        (11.02.2010.) (Domagoj Saric)
    std::uint16_t const incompleteOutputDataSizeFromPreviousSteps( windowSize - stepSize );
#endif // !LE_SW_PURE_ANALYSIS

    float const    * LE_RESTRICT        pCompleteNewInput      ( processParameters.mainChannel    () );
    float const    * LE_RESTRICT        pCompleteNewSideChannel( processParameters.sideChannel    () );
    std::uint32_t                       inputSamples           ( processParameters.numberOfSamples() );
    bool                          const useSideChannel         ( processParameters.haveSideChannel() );
    ChannelBuffers &                    channelBuffers         ( processParameters.channelBuffers () );
    float          * LE_RESTRICT        pOutput                ( processParameters.output         () );

#ifdef LE_SW_PURE_ANALYSIS
    LE_ASSUME( useSideChannel          == false   );
    LE_ASSUME( pCompleteNewSideChannel == nullptr );
    LE_ASSUME( pOutput                 == nullptr );
#endif // LE_SW_PURE_ANALYSIS

    using namespace Math;

    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, ReadOnlyDataRange( pCompleteNewInput      , pCompleteNewInput       +                             inputSamples       ), "main input" );
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, ReadOnlyDataRange( pCompleteNewSideChannel, pCompleteNewSideChannel + ( pCompleteNewSideChannel ? inputSamples : 0 ) ), "side input" );

    while ( inputSamples )
    {
        // Fill the input FIFO buffers just right up to the window size, the
        // minimum we need for one pass of processing.
        std::uint16_t const previousData ( channelBuffers.inputDataSize() );
        std::uint16_t const neededData   ( windowSize - previousData      );
        std::uint16_t const sizeToConsume( static_cast<std::uint16_t>( std::min<std::uint32_t>( neededData, inputSamples ) ) );

        channelBuffers.addNewData( pCompleteNewInput, pCompleteNewSideChannel, sizeToConsume, useSideChannel );
        BOOST_ASSERT_MSG( channelBuffers.inputDataSize() <= windowSize, "Too much data consumed." );
        inputSamples -= sizeToConsume;

        /// \note Assertion failures/crashes occur with 0% overlap due to the
        /// output OLA buffer overruns. As a workaround, the second check is
        /// performed. This needs further investigation...
        ///                                   (04.03.2015.) (Domagoj Saric)
        if
        (   // Process if:
            ( channelBuffers.inputDataSize      () == windowSize                                     ) && // - we have enough input data
            ( channelBuffers.readyOutputDataSize() <= channelBuffers.outputBufferSize() - windowSize )    // - we have space for output data
        )
        {
            // The Window+FFT phase:
            // Implementation note:
            //   As we cannot window the input data directly (because we
            // need it non windowed for later OLA steps) we have to copy it
            // to a new location before windowing. To reduce the number of
            // buffers and data copies we use the fact that the current FFT
            // implementation also requires copying of input data (because
            // it supports only in-place operation so it would overwrite
            // input data if it was not first copied to the output location
            // before performing the FFT) so the two copy operations are
            // merged into one: input data is copied into the destination
            // buffer, windowed and then the FFT is performed.
            //                                (11.02.2010.) (Domagoj Saric)
            channelBuffers.setCurrentDataToChannelData( useSideChannel, fft_, analysisWindow(), windowSizeFactor );

            // The processing phase:
            {
                auto   const channel    ( processParameters.currentChannel() );
                auto &       data       ( channelBuffers   .channelData   () );
                auto &       engineSetup( this->engineSetup()                );
                modules().forEach<ModuleDSP>
                (
                    [&, channel]( ModuleDSP const & module )
                    {
                        module.process( channel, data, engineSetup );
                    }
                );
            }

        #ifndef LE_SW_PURE_ANALYSIS
            // The IFFT+Window+Overlap-Add phase:
            //  Get the time-domain results, window them and add with/to the
            // output FIFO buffer at the current position.
            float * const pOutput( channelBuffers.putNewTimeDomainDataToOutput( fft_, synthesisWindow(), windowSizeFactor ) );

            // Scale the results to:
            // - apply the user selected post/output gain
            // - compensate for the WOLA gain.
            multiply( pOutput, processParameters.outputScaling() / engineSetup().wolaGain(), stepSize );

            if ( processParameters.doMix() )
            {
                // Implementation note:
                //   To avoid redundant buffers and data copying we scale
                // the input data in-place (as it was already consumed and
                // will be discarded) and add it to output data.
                //                            (11.02.2010.) (Domagoj Saric)
                float const inputScaling( 1 - processParameters.mixPercentage() );
                multiply( channelBuffers.inputBuffer(), inputScaling, stepSize );
                add     ( channelBuffers.inputBuffer(), pOutput     , stepSize );
            }
        #endif // LE_SW_PURE_ANALYSIS

            channelBuffers.moveForwardByHopSize( stepSize, useSideChannel );
        } // if ( channelBuffers.inputDataSize() == windowSize )

    #ifndef LE_SW_PURE_ANALYSIS
        std::uint16_t const availableOutputData( channelBuffers.readyOutputDataSize() );
        std::uint16_t const sizeToProduce      ( sizeToConsume                        );
        if ( BOOST_UNLIKELY( sizeToProduce > availableOutputData ) )
        {
            // We have just started processing and the latency time has not yet
            // passed so we do not have enough data and therefore simply zero
            // the part of the output that we have no data for.
            std::uint16_t const amountToZero( sizeToProduce - availableOutputData );
            Math::clear( pOutput, amountToZero );
            pOutput += amountToZero;
        }
        BOOST_ASSERT_MSG
        (
            availableOutputData <= ( sizeToConsume + engineSetup().frameSize<unsigned int>() ),
            "Produced too much data."
        );
        auto const amountToExtract( std::min( sizeToProduce, availableOutputData ) );
        channelBuffers.extractChunkOfReadyOutputData
        (
            pOutput,
            amountToExtract,
            incompleteOutputDataSizeFromPreviousSteps
        );

        LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, ReadOnlyDataRange( pOutput, pOutput + amountToExtract ), "output" );

        pOutput += amountToExtract;
    #endif // LE_SW_PURE_ANALYSIS
    } // while ( inputSamples )
}


LE_OPTIMIZE_FOR_SIZE_BEGIN()

////////////////////////////////////////////////////////////////////////////////
//
// Processor::calculateWindowAndWOLAGain()
// ---------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
// - general information
//  http://sipl.technion.ac.il/Info/new/Staff/Academic/Malah/Publications/Shpiro_Algebraic_ICASSP84.pdf
// - polyphase DFT/Window presum DFT/Weighted-Overlap-Add
//  http://www.dsprelated.com/showmessage/123311/1.php
//  http://www.dsprelated.com/showmessage/45449/1.php
//  http://web.archive.org/web/20010210052902/http://www.chipcenter.com/dsp/DSP000315F1.html
//  http://hdl.lib.byu.edu/1877/etd157
//  http://eetimes.com/design/embedded/4007611/DSP-Tricks-Building-a-practical-spectrum-analyzer
//  http://www.littleendian.com/shared/papers/Time_Aliasing_Methods_of_Spectrum_Estimation.pdf
//  http://groups.yahoo.com/group/softrock40/message/1299
//  http://www.rfel.com/download/D02003-Polyphase%20DFT%20data%20sheet.pdf
//  http://www.rfel.com/download/w03006-comparison_of_fft_and_polydft_transient_response.pdf
//  http://www.ee.cityu.edu.hk/~hcso/canadian97_1.pdf
//  http://www.eurasip.org/Proceedings/Eusipco/Eusipco2005/defevent/papers/cr1183.pdf
//  http://dev.vinux-project.org/time-aliased-hann
////////////////////////////////////////////////////////////////////////////////

namespace
{
    LE_COLD
    void LE_FASTCALL sincWindow( float * LE_RESTRICT const pWindow, std::uint16_t const halfWindowSize, std::uint16_t const sincPeriod )
    {
        double const period( Math::Constants::pi_d / Math::convert<double>( sincPeriod ) );
        double const half  ( Math::convert<double>( halfWindowSize                     ) );

        float * LE_RESTRICT pWindowRight( &pWindow[ halfWindowSize + 1 ] );
        float * LE_RESTRICT pWindowLeft ( pWindowRight - 2               );

        for ( double i( 1 ); i < half; ++i )
        {
            double const x   ( i * period                              );
            float  const sinc( static_cast<float>( std::sin( x ) / x ) );
            *pWindowRight++ *= sinc;
            *pWindowLeft--  *= sinc;
        }
    }
} // anonymous namespace

void LE_COLD Processor::calculateWindowAndWOLAGain()
{
    auto const windowSize( engineSetup().windowSize<std::uint16_t>() );
    auto const stepSize  ( engineSetup().stepSize  <std::uint16_t>() );

    auto const analysisWindowFunction( engineSetup().windowFunction() );

    Math::calculateWindow( analysisWindow_, analysisWindowFunction );

    /// \note
    ///   The WOLA (Weighted Overlap and Add) method requires that a window be
    /// applied to the signal both before and after the DFT, these are the
    /// analysis and synthesis windows respectively. The COLA (Constant Overlap
    /// and Add) condition must therefore apply to the product of the analysis
    /// and synthesis windows. The simplest solution (as 'prescribed' by J.O.S
    /// in Spectral Audio Signal Processing,
    /// http://www.dsprelated.com/dspbooks/sasp/Choice_WOLA_Window.html
    /// http://www.dsprelated.com/dspbooks/sasp/Overlap_Add_Decomposition.html)
    /// is to take the square root of a chosen window (that obeys the COLA
    /// condition) and use that for both the analysis and synthesis windowing.
    /// This approach is not good enough for our purposes because taking the
    /// square root "deforms" the window and it looses its spectral qualities
    /// which in turn hinders phase vocoder performance.
    ///   As discussed in the "WOLA and the phase vocoder" thread on the
    /// music-dsp list the solution is to either use a power complementary
    /// window (such as the Vorbis window, or the Hann window with overlap
    /// factors larger than 2) or to use different analysis and synthesis
    /// windows and to divide the synthesis window with the analysis window
    /// (e.g. use Hamming for analysis and Hann-divided-by-Hamming for
    /// synthesis). We use the latter as a general solution with special
    /// handling for windows that don't work well with the default approach.
    ///                                       (25.04.2012.) (Domagoj Saric)

    /// \todo Power complementary windows do not actually need two windows (when
    /// window presumming is not used). Refactor the relevant code so that it
    /// does not allocate and initialise the (duplicated) synthesis window
    /// (rather it should simply alias the analysis window).
    ///                                       (25.04.2012.) (Domagoj Saric)
    /// \note As a quick-workaround/optimisation we make the synthesis window
    /// alias the analysis window when possible to improve locality of reference
    /// (but the extra wasted allocation is still performed).
    ///                                       (04.03.2015.) (Domagoj Saric)

    Engine::Constants::Window synthesisWindowFunction( Engine::Constants::Hann );
    /// \note Quick-hack: 'reset'/clear the synthesisWindow_ range so that its
    /// status can be used as a signal whether to skip automatic synthesis
    /// window generation (required for the flat top window which needs its own
    /// logic).
    ///                                       (05.03.2015.) (Domagoj Saric)
    synthesisWindow_.alias( FFTWindow() );
    // https://ccrma.stanford.edu/~jos/parshl/Choice_Hop_Size.html
    auto const overlapFactor( engineSetup().windowOverlappingFactor<std::uint8_t>() );
    switch ( analysisWindowFunction )
    {
        namespace Engine = LE::SW::Engine;

        /// \note Hann is power complementary for overlap factors > 2 so reuse
        /// the analysis window for those cases, otherwise fallback to the old
        /// sqrt approach (the "automatic synthesis window generation" approach
        /// does not seem to work no matter what other 'output' window is
        /// chosen).
        ///                                   (05.03.2015.) (Domagoj Saric)
        case Engine::Constants::Hann:
            if ( overlapFactor <= 2 )
                //synthesisWindowFunction = Engine::Constants::Triangle;
                Math::squareRoot( analysisWindow_ );
            BOOST_ASSERT( synthesisWindowFunction == Engine::Constants::Hann );
            break;

        // Blackman and Blackman-Harris windows seem to be power complementary
        // at high overlap factors.
        case Engine::Constants::Blackman:
        case Engine::Constants::BlackmanHarris:
            if
            (
                ( overlapFactor > 3 ) ||
                ( overlapFactor > 2  && engineSetup().windowSizeFactor() >= 4 )
            )
                synthesisWindowFunction = analysisWindowFunction;
            break;

        /// \note Flat top does not seem to work with the "automatic synthesis
        /// window generation" (at overlaps below 75% it just sounds bad and
        /// at higher overlaps the sound 'breaks down' as soon as any
        /// modification is done in the frequency domain). The fallback sqrt
        /// procedure also requires special handling because flat top windows
        /// use negative values that cannot have their square root taken.
        ///                                   (05.03.2015.) (Domagoj Saric)
        case Engine::Constants::FlatTop:
        {
            synthesisWindow_.alias( synthesisWindowBackup_ );
            // Take the square root of the absolute values of the window and
            // restore the signs only to the analysis window:
            float * LE_RESTRICT pAnalysisWindowSample ( analysisWindow_ .begin() );
            float * LE_RESTRICT pSynthesisWindowSample( synthesisWindow_.begin() );
            while ( pAnalysisWindowSample != analysisWindow_.end() )
            {
                auto const inputSample( *pAnalysisWindowSample               );
                auto const sample     ( std::sqrt( std::abs( inputSample ) ) );
                *pAnalysisWindowSample ++ = Math::copySign( sample, inputSample );
                *pSynthesisWindowSample++ =                 sample               ;
            }
            break;
        }

        case Engine::Constants::Rectangle:
            /// \note We want 'intuitive'/expected behaviour (no amplitude
            /// modulation) for the rectangle window so we must disable the
            /// "automatic synthesis window generation" @ 0% overlap.
            ///                               (04.03.2015.) (Domagoj Saric)
            if ( overlapFactor == 1 )
                synthesisWindowFunction = analysisWindowFunction;
            break;

        default: break;
    }

    if ( synthesisWindowFunction == analysisWindowFunction )
    {
        synthesisWindow_.alias( analysisWindow_ );
    }
    else
    if ( !synthesisWindow_ )
    {   // Default solution: synthesis window = Hann / analysis window.
        synthesisWindow_.alias( synthesisWindowBackup_ );
        Math::calculateWindow( synthesisWindow_, synthesisWindowFunction );
        BOOST_ASSERT( synthesisWindow_.back() != 0 );
        float const * LE_RESTRICT pAnalysisWindowSample ( analysisWindow_ .begin() );
        float       * LE_RESTRICT pSynthesisWindowSample( synthesisWindow_.begin() );
        if ( *pAnalysisWindowSample == 0 )
        {
            pAnalysisWindowSample ++;
            pSynthesisWindowSample++;
            //*pSynthesisWindowSample++ = 0; //...mrmlj...?
        }
        while ( pSynthesisWindowSample != synthesisWindow_.end() )
            *pSynthesisWindowSample++ /= *pAnalysisWindowSample++;
    }

    if ( engineSetup().windowSizeFactor() > 1 )
    {
        /// \note When window presumming we need to apply the sinc function to
        /// both the analysis and synthesis windows (with different periods) in
        /// order to avoid the echo/flanging caused by adding the delayed signal
        /// to itself. This step was simply taken from Richard Dobson's
        /// open-sourced VST plugins but it is not yet clear why or how this
        /// works and it is not prescribed in any of the papers and so it
        /// requires further research.
        ///                                   (24.04.2012.) (Domagoj Saric)
        /// \note The original RWD code uses different sinc "periods" for the
        /// analysis and synthesis windows (DFT and step sizes respectively).
        /// This has been found to produce more ripple than using the DFT
        /// size for both windows but it seems to do a better job in eliminating
        /// the echo/flanging so we use this approach also.
        ///                                   (25.04.2012.) (Domagoj Saric)
        auto const halfWindowSize( windowSize / 2 );
        sincWindow( analysisWindow_ .begin(), halfWindowSize, engineSetup().fftSize<unsigned int>() );
        sincWindow( synthesisWindow_.begin(), halfWindowSize, stepSize                              );
    }

    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, analysisWindow_ , "analysis  window" );
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, synthesisWindow_, "synthesis window" );

    // Calculate the WOLA gain and ripple/variation:

    // Fill a temporary buffer with overlap-added copies of the window(s) to
    // determine the total gain and whether the COLA condition is (sufficiently)
    // satisfied (the gain variation is sufficiently small).
    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( wolaBuffer, real_t, windowSize );
    Math::clear( wolaBuffer );
    for ( DataRange::iterator pBufferPosition( wolaBuffer.begin() ); ;pBufferPosition += stepSize )
    {
        auto const bufferSpaceLeft( static_cast<std::uint16_t>( wolaBuffer.end() - pBufferPosition ) );
        Math::addProduct( &analysisWindow_[ 0 ], &synthesisWindow_[ 0 ], pBufferPosition, std::min( windowSize, bufferSpaceLeft ) );
        if ( bufferSpaceLeft <= stepSize )
            break;
    }
    // Implementation note:
    //   We take the mean value here instead of just 'any' value from the valid
    // range (where all elements should be equal/constant if the COLA condition
    // is ideally fulfilled) as this will give a better value for non-COLA
    // window + overlap factor combinations.
    //                                        (25.01.2010.) (Domagoj Saric)
    {
        float minimum( std::numeric_limits<float>::max() );
        float maximum( 0 );
        float mean   ( 0 );
        float const * LE_RESTRICT       pWOLAValue( &wolaBuffer[ windowSize - stepSize ] );
        float const *             const pWOLAEnd  ( &wolaBuffer[ windowSize - 1 ] + 1    );
        while ( pWOLAValue != pWOLAEnd )
        {
            float const value( *pWOLAValue++ );
            mean   += std::abs( value );
            minimum = std::min( value, minimum );
            maximum = std::max( value, maximum );
        }
        mean /= Math::convert<float>( stepSize );

        float const wolaGain ( mean );
        float const variation( ( maximum - minimum ) / maximum / wolaGain );

        engineSetup().setWOLAGainAndRipple( wolaGain, variation );

    #if !( defined( LE_SW_SDK_BUILD ) && !defined( __MSVC_RUNTIME_CHECKS ) ) && 0 //...mrmlj...can be noisy/not needed that much anymore...
        LE_TRACE
        (
            "\tWindow ID: %u, window overlap factor: %u, gain: %f, variation: %f%%.",
            engineSetup().windowFunction(), windowSize / stepSize, wolaGain, variation * 100
        );
    #endif // _DEBUG
    }
}

LE_COLD
void Processor::setNumberOfChannels( std::uint8_t const numberOfMainChannels, std::uint8_t const numberOfSideChannels )
{
    engineSetup().setNumberOfChannels( numberOfMainChannels, numberOfSideChannels );
}

LE_COLD LE_NOTHROW
bool LE_FASTCALL Processor::setSampleRate( float const sampleRate, StorageFactors & currentStorageFactors )
{
    float const currentSampleRate( engineSetup().sampleRate<float>() );
    //...mrmlj...assert that we are in a non-processing state...
    engineSetup().setSampleRate( sampleRate );

    StorageFactors const storageFactors
    {
        currentStorageFactors.fftSize,
    #if LE_SW_ENGINE_WINDOW_PRESUM
        currentStorageFactors.windowSizeFactor,
    #endif // LE_SW_ENGINE_WINDOW_PRESUM
        currentStorageFactors.overlapFactor,
        currentStorageFactors.numberOfChannels,
        engineSetup().sampleRate<std::uint32_t>()
    };

    if ( !storageFactors.complete() )
    {
        // See the related note in resize().
        return true;
    }


    BOOST_ASSERT_MSG
    (
        Processor::requiredStorage( storageFactors ) == Processor::requiredStorage( currentStorageFactors ),
        "Processor storage assumed not to depend on the sampling rate."
    );

    if ( modules().resizeAll( storageFactors, currentStorageFactors ) )
    {
        currentStorageFactors = storageFactors;
        return true;
    }
    else
    {
        engineSetup().setSampleRate( currentSampleRate );
        return false;
    }
}

LE_COLD
bool Processor::resize
(
    StorageFactors                  &       currentStorageFactors,
    StorageFactors            const &       newStorageFactors,
    Setup::Window                     const window,
    Engine::HeapSharedStorage       &       sharedStorage
)
{
    /// \note If not all storage factors have been set yet, simply save the new
    /// values and return true (in expectation of a future resize() with
    /// complete storage factors when the actual allocation will be attempted).
    /// This is required in the plugin for certain hosts that set certain
    /// storage factor related parameters before calling initialise():
    ///  - Ableton - setSampleRate()
    ///  - n-Track - setNumberOfChannels().
    ///                                       (24.01.2013.) (Domagoj Saric)
    if ( !newStorageFactors.complete() )
    {
        currentStorageFactors = newStorageFactors;
        engineSetup().setWindowFunction( window );
        return true;
    }
    else
    if ( newStorageFactors == currentStorageFactors )
    {
        BOOST_ASSERT( Processor::requiredStorage( newStorageFactors ) == Processor::requiredStorage( currentStorageFactors ) );
        BOOST_ASSERT( analysisWindow_ && synthesisWindow_ && channels_ );
        if ( window != engineSetup().windowFunction() )
        {
            changeWindowFunction( window );
            BOOST_ASSERT( window == engineSetup().windowFunction() );
        }
        return true;
    }

    auto const currentMainStorageSize( sharedStorage.size()                            );
    auto const requiredStorage       ( Processor::requiredStorage( newStorageFactors ) );

    auto const currentSampleRate( engineSetup().sampleRate<float>() );
    engineSetup().setSampleRate( newStorageFactors.samplerate );

    bool allocationSucceeded( sharedStorage.resize( requiredStorage ) );
    if ( allocationSucceeded )
    {
        if ( modules().resizeAll( newStorageFactors, currentStorageFactors ) )
        {
            currentStorageFactors = newStorageFactors;
        }
        else
        {
            allocationSucceeded = false;
            engineSetup().setSampleRate( currentSampleRate );
            BOOST_VERIFY( sharedStorage.resize( currentMainStorageSize ) );
        }

        changeWOLAParameters
        (
            currentStorageFactors,
            window,
            sharedStorage
        );

        //...mrmlj...rethink whether we should do this at all here (if the user
        //...mrmlj...has to reset the processor anyway...
        clearSideChannelData();
        resetChannelBuffers ();
    }
    return allocationSucceeded;
}

LE_COLD
StorageFactors Processor::makeFactors
(
    std::uint16_t const fftSize         ,
#if LE_SW_ENGINE_WINDOW_PRESUM
    std::uint8_t  const windowSizeFactor,
#endif // LE_SW_ENGINE_WINDOW_PRESUM
    std::uint8_t  const overlapFactor   ,
    std::uint8_t  const numberOfChannels,
    std::uint32_t const sampleRate
)
{
    StorageFactors const storageFactors =
    {
        fftSize,
    #if LE_SW_ENGINE_WINDOW_PRESUM
        windowSizeFactor,
    #endif // LE_SW_ENGINE_WINDOW_PRESUM
        overlapFactor,
        numberOfChannels,
        sampleRate
    };
    return storageFactors;
}


void LE_COLD Processor::changeWOLAParameters( StorageFactors const & storageFactors, Setup::Window const window, Storage storage )
{
    BOOST_ASSERT( storage );
    this->resize( storageFactors, storage );
    BOOST_ASSERT_MSG
    (
        unsigned( storage.size() ) <= storageFactors.numberOfChannels * Utility::Constants::vectorAlignment, //...mrmlj...
        "Requested storage space not consumed."
    );

    engineSetup().setFFTSize          ( storageFactors.fftSize          );
    engineSetup().setOverlappingFactor( storageFactors.overlapFactor    );
#if LE_SW_ENGINE_WINDOW_PRESUM
    engineSetup().setWindowSizeFactor ( storageFactors.windowSizeFactor );
#endif // LE_SW_ENGINE_WINDOW_PRESUM
    changeWindowFunction( window );
}


void LE_COLD Processor::changeWindowFunction( Setup::Window const window )
{
    engineSetup().setWindowFunction( window );
    calculateWindowAndWOLAGain();
}


void LE_COLD Processor::clearSideChannelData()
{
    for ( auto & channel : channels_ )
        channel.channelData().clearSideChannelData();
}


void LE_COLD Processor::resetChannelBuffers()
{
    std::uint16_t const initialSilenceSamples
    (
        engineSetup().windowSize<std::uint16_t>() -
        engineSetup().stepSize  <std::uint16_t>()
    );
    for ( auto & channel : channels_ )
        channel.reset( initialSilenceSamples );
}


Processor       & Processor::fromEngineSetup( Setup       & engineSetup ) { return Utility::ParentFromMember<Processor, Setup, &Processor::engineSetup_>()( engineSetup ); }
Processor const & Processor::fromEngineSetup( Setup const & engineSetup ) { return fromEngineSetup( const_cast<Setup &>( engineSetup ) ); }

LE_COLD LE_CONST_FUNCTION
std::uint32_t Processor::requiredStorage( StorageFactors const & factors )
{
    return
        Math::FFT_float_real_1D::requiredStorage( factors ) +
        FFTWindow              ::requiredStorage( factors ) + // analysis
        FFTWindow              ::requiredStorage( factors ) + // synthesis
        Channels               ::requiredStorage( factors );
}

LE_COLD
void Processor::resize( StorageFactors const & factors, Storage & storage )
{
    fft_            .resize( factors, storage );
    analysisWindow_ .resize( factors, storage );
    synthesisWindow_.resize( factors, storage );
    channels_       .resize( factors, storage );

    synthesisWindowBackup_.alias( synthesisWindow_ );
}

LE_COLD LE_CONST_FUNCTION
std::uint32_t Processor::Channels::requiredStorage( StorageFactors const & factors )
{
    using Utility::align;
    std::uint16_t const channelBuffersBaseSize   ( sizeof( value_type ) );
    std::uint32_t const channelBuffersStorageSize( value_type::requiredStorage( factors ) );
    BOOST_ASSERT( align( channelBuffersStorageSize ) == channelBuffersStorageSize );
    std::uint32_t const totalSizePerChannel( align( channelBuffersBaseSize ) + channelBuffersStorageSize );
    return factors.numberOfChannels * totalSizePerChannel;
}

LE_COLD
void Processor::Channels::resize( StorageFactors const & factors, Storage & storage )
{
    Utility::SharedStorageBuffer<ChannelBuffers>::resize( factors.numberOfChannels * sizeof( value_type ), storage );

#if LE_SW_ENGINE_WINDOW_PRESUM
    std::uint8_t const windowSizeFactor      ( factors.windowSizeFactor                );
#else
    std::uint8_t const windowSizeFactor      ( 1                                       );
#endif // LE_SW_ENGINE_WINDOW_PRESUM
    std::uint16_t const windowSize           ( factors.fftSize * windowSizeFactor      );
    std::uint16_t const stepSize             ( factors.fftSize / factors.overlapFactor );
    std::uint16_t const initialSilenceSamples( windowSize - stepSize                   );
    for ( auto  & channelBuffers : *this )
    {
        ChannelBuffers * LE_RESTRICT const pNewChannelBuffers( new ( &channelBuffers ) ChannelBuffers() );
        LE_ASSUME( pNewChannelBuffers );
        pNewChannelBuffers->resize( factors, storage      );
        pNewChannelBuffers->reset ( initialSilenceSamples );
    }
}

LE_OPTIMIZE_FOR_SIZE_END()

namespace
{
    float const * const dummyNullSidePointer( nullptr );
} // anonymous namespace

LE_FORCEINLINE
Processor::ProcessParameters::ProcessParameters
(
    InputData             const inputs,
    InputData             const sideChannels,
    OutputData            const outputs,
    Channels      const &       channels,
    std::uint32_t         const numberOfSamples,
    float                 const outputGain,
#ifdef __APPLE__
    float                 const volatile mixAmount ///...mrmlj...!? broken codegen by Xcode 7.1(.1) Clang...
#else
    float                 const mixAmount
#endif // __APPLE__
)
    :
    ppMainChannels_( inputs                                              ),
    ppSideChannels_( sideChannels ? sideChannels : &dummyNullSidePointer ),
    pOutput_       ( outputs                                             ),

    channelBuffers_( channels ),

    currentChannel_ ( 0               ),
    numberOfSamples_( numberOfSamples ),

    mixPercentage_( mixAmount              ),
    outputScaling_( outputGain * mixAmount ),
    doMix_        ( mixPercentage_ < 1     )
{
}

#ifndef LE_NO_LFOs
void Processor::setPosition( std::uint32_t const absolutePositionInSamples )
{
    lfoTimer().setPosition( absolutePositionInSamples, engineSetup().sampleRate<float>() );
}

LE_NOTHROW
void Processor::updatePosition( std::uint32_t const deltaSamples )
{
    handleTimingInformationChange
    (
        lfoTimer().updatePositionAndTimingInformation
        (
            deltaSamples,
            engineSetup().sampleRate<float>()
        )
    );
}

Processor::LFO::Timer::TimingInformationChange LE_FASTCALL Processor::updatePositionAndTimingInformation( float const positionInBars, float const barDuration, std::uint8_t const measureNumerator )
{
    auto const timingChange
    (
        lfoTimer().updatePositionAndTimingInformation
        (
            positionInBars,
            barDuration   ,
            measureNumerator
        )
    );
    handleTimingInformationChange( timingChange );
    return timingChange;
}

Processor::LFO::Timer::TimingInformationChange LE_FASTCALL Processor::updatePositionAndTimingInformation( std::uint32_t const deltaNumberOfSamples )
{
    auto const timingChange
    (
        lfoTimer().updatePositionAndTimingInformation
        (
            deltaNumberOfSamples,
            engineSetup().sampleRate<float>()
        )
    );
    handleTimingInformationChange( timingChange );
    return timingChange;
}

LE_NOTHROW
void Processor::handleTimingInformationChange( LFO::Timer::TimingInformationChange const timingInformationChange )
{
    if ( timingInformationChange.timingInfoChanged() )
        updateModuleLFOs( timingInformationChange );
}


void Processor::updateModuleLFOs( LFO::Timer::TimingInformationChange const timingInformationChange )
{
    modules().forEach<Engine::ModuleDSP>
    (
        [&]( Engine::ModuleDSP & module ) { module.updateLFOs( timingInformationChange ); }
    );
}
#endif // LE_NO_LFOs

/// \note Processor instances do not actually hold ModuleChainImpl instances as
/// different project types have different requirements/usage patterns regarding
/// module chains (e.g. multiple chains/programs for VST2.4 plugins). For this
/// reason each project type must define the non-const getter in the appropirate
/// module.
///                                           (21.03.2015.) (Domagoj Saric)
ModuleChainImpl const & Processor::modules() const { return const_cast<Processor &>( *this ).modules(); }

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
