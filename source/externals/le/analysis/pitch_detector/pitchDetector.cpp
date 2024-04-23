////////////////////////////////////////////////////////////////////////////////
///
/// pitchDetector.cpp
/// -----------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "pitchDetector.hpp"

#include "le/analysis/peak_detector/peakDetector.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/spectrumworx/engine/setup.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include <boost/assert.hpp>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( LE )
//------------------------------------------------------------------------------

// https://www.littleendian.com/wiki/index.php/PitchDetectorSDK

// Papers/lectures:
// http://en.wikipedia.org/wiki/Pitch_detection_algorithm
// http://audition.ens.fr/adc/pdf/2002_JASA_YIN.pdf
// http://www.nicholson.com/rhn/dsp.html#1 (Frequency/Pitch Measurement/Estimation Techniques)
// https://ccrma.stanford.edu/~pdelac/154/m154paper.htm
// http://cnx.org/content/m11714/latest
// http://www.cse.ohio-state.edu/~dwang/papers/Li-Wang.icassp05.pdf (DETECTING PITCH OF SINGING VOICE IN POLYPHONIC AUDIO)
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.79.7433&rep=rep1&type=pdf (A Perceptual Pitch Detector)
// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.11.302&rep=rep1&type=pdf (Multi-pitch Detection)
// http://www.ee.columbia.edu/~dpwe/papers/KarjT99-pitch.pdf (Multi-pitch and Periodicity Analysis)
// http://faculty.ksu.edu.sa/ghulam/Documents/Downloads/PitchExtraction_NoisyEnvironment.pdf
// http://www.researchgate.net/publication/221478674_Multi-microphone_periodicity_function_for_robust_F0_estimation_in_real_noisy_and_reverberant_environments
// http://www.cs.otago.ac.nz/tartini/papers/A_Smarter_Way_to_Find_Pitch.pdf
// http://www.eecs.qmul.ac.uk/~emmanouilb/papers/benetosdixon_jstsp_postprint.pdf
// http://courses.physics.illinois.edu/phys406/NSF_REU_Reports/2005_reu/Real-Time_Time-Domain_Pitch_Tracking_Using_Wavelets.pdf
// http://en.wikipedia.org/wiki/Detection_theory
// http://www.scribd.com/doc/50529329/48653343-digital-processing
// http://repository.kulib.kyoto-u.ac.jp/dspace/bitstream/2433/52615/1/soa004_081.pdf (Pitch extraction by Peak detection 1966)
// http://ieeexplore.ieee.org/xpl/login.jsp?tp=&arnumber=5556836&url=http%3A%2F%2Fieeexplore.ieee.org%2Fiel5%2F5535008%2F5556742%2F05556836.pdf%3Farnumber%3D5556836


// Discussions:
// http://stackoverflow.com/questions/7181630/fft-on-iphone-to-ignore-background-noise-and-find-lower-pitches
// http://dsp.stackexchange.com/questions/2882/pitch-detection-algorithm-for-minsample-size
// - HPS:
//   http://www.hydrogenaudio.org/forums/index.php?showtopic=91250
//   http://dsp.stackexchange.com/questions/572/harmonic-product-spectrum-limitations-in-pitch-detection

// 3rd party SDKs/libraries:
// - free:
// http://www.schmittmachine.com/dywapitchtrack.html
// https://github.com/irtemed88/PitchDetector
// https://github.com/alexbw/novocaine
// http://pitchtracker.codeplex.com
// http://tarsos.0110.be/tag/TarsosDSP
// http://www.fon.hum.uva.nl/praat
// http://code.google.com/p/freqazoid
//
// http://kevmdev.com/developer.php
// http://code.google.com/p/ctuner/source/browse/trunk/mac/Tuner.c
// http://www.audiocontentanalysis.org/code
// http://sig.sapp.org/src/sigAudio/Pitch_HPS.cpp
// - GPL:
// https://github.com/JorenSix/TarsosDSP
// - commercial:


// 3rd party end products:
// - free:
// https://play.google.com/store/apps/details?id=com.ntrack.tuner
// http://tombaran.info/autotalent.html (+pitch correction)
// http://mtg.upf.edu/technologies/melodia
// https://getinstinct.com/tuner
// https://developer.microsoft.com/en-us/microsoft-edge/testdrive/demos/webaudiotuner
// - GPL:
// http://www.sonicvisualiser.org
// - commercial:
// http://www.youtube.com/watch?v=5s2psD2sntY (Pitch Primer)
// http://www.celemony.com/cms/index.php?id=dna
// http://www.celemony.com/cms/index.php?id=videos#top
// http://www.youtube.com/watch?v=2tZKy1W58iw
// http://www.singandsee.com/forsingers.php
//------------------------------------------------------------------------------

using namespace SW; //...mrmlj...

bool operator<( HPS const & left, HPS const & right )
{
    LE_ASSUME( left .harmonicProduct >= 0 );
    LE_ASSUME( right.harmonicProduct >= 0 );
    /// \note We want a descending sort.
    ///                                       (05.04.2016.) (Domagoj Saric)
    return left.harmonicProduct > right.harmonicProduct;
}


////////////////////////////////////////////////////////////////////////////////
//
// PitchDetector::findPitch()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

float LE_FASTCALL PitchDetector::findPitch
(
    SW::Engine::ReadOnlyDataRange const &       amplitudes,
    ChannelState                        &       cs,
    float                                 const lfb,
    float                                 const hfb,
    SW::Engine::Setup             const &       engineSetup
)
{
    auto const numberOfBins( static_cast<std::uint16_t>( amplitudes.size() ) );

    // Peak detector settings:
    PeakDetector pd;
    // Find all peaks, of any strength:
    pd.setStrengthThreshold(  0 );
    // Relax the threshold to 80 dB:
    pd.setGlobalThreshold  ( 80 );
    pd.setZeroDecibelValue ( engineSetup.maximumAmplitude() );

    // Find peaks:
    pd.findPeaksAndEstimateFrequency( amplitudes.begin(), numberOfBins, engineSetup.sampleRate<std::uint32_t>() );
    // Delete non-peaks to make it easier for HPS:
    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( filteredAmps, float, numberOfBins );
    Math::copy( amplitudes, filteredAmps );
    pd.attenuateNonPeaks( filteredAmps.begin(), 0, numberOfBins - 1, 300.0f );

    // Find HPS spectrum:
    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( hps, HPS, numberOfBins );
    findHarmonicProductSpectrumAndSort( filteredAmps, hps );
    // Estimate pitch:
    float pitch( estimatePitch( cs.lastPitch, lfb, hfb, hps, pd ) );

#ifdef LE_SW_PURE_ANALYSIS
    std::uint8_t const maximumConfidence        ( 5    );
    float        const maximumAllowedPitchChange( 0.2f );

    if ( pitch )
    { // Valid new pitch:
        if ( !cs.lastPitch )
        { // No previous pitch - simply use the new one:
            cs.confidence = 1;
        }
        else
        if ( ( Math::abs( cs.lastPitch - pitch ) ) / pitch < maximumAllowedPitchChange )
        { // Pitch changed within limits - save it and increase confidence:
            cs.confidence = std::min<std::uint8_t>( maximumConfidence, cs.confidence + 1 );
        }
        else
        { // Pitch changed significantly/out of limits:
            if ( cs.confidence > 2 )
            { // we have a somewhat confident previous pitch - use it but decrease confidence:
                pitch = cs.lastPitch;
                --cs.confidence;
            }
            else
            { // no previous pitch - use the new one:
                cs.confidence = 1;
            }
        }

        BOOST_ASSERT( pd.getNumPeaks() != 0 );
        bool const newPitchIgnored( pitch == cs.lastPitch );
        if ( !newPitchIgnored )
        {
            for ( std::uint16_t peakIndex( 0 ); ; ++peakIndex )
            {
                Peak const & peak( *pd.getPeak( peakIndex ) );
                if ( peak.freq == pitch )
                {
                    cs.amplitude = peak.amplitude;
                    break;
                }
                BOOST_ASSERT( peakIndex < pd.getNumPeaks() );
            }
        }
    }
    else
    { // No pitch detected for current frame:
        if ( cs.lastPitch && cs.confidence )
        { // we have a previous pitch - use it but decrease confidence:
            pitch         = cs.lastPitch;
            cs.confidence = std::max<std::uint8_t>( 0, cs.confidence - 1 );
        }
        else
            cs.reset();
    }
#endif // LE_SW_PURE_ANALYSIS

    cs.lastPitch = pitch;

    return pitch;
}


////////////////////////////////////////////////////////////////////////////////
//
// PitchDetector::findHarmonicProductSpectrumAndSort()
// ---------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SPEED_BEGIN()

namespace
{
    using SW::Engine::ReadOnlyDataRange;

    float LE_FASTCALL LE_HOT harmonic( std::uint8_t const harmonic, std::uint16_t const bin, ReadOnlyDataRange const & amplitudes )
    {
        auto       pHarmonicAmp    ( amplitudes.begin() + ( harmonic * bin ) );
        auto const pHarmonicAmpsEnd( pHarmonicAmp + harmonic                 );
        if ( pHarmonicAmpsEnd > amplitudes.end() )
            return 0;
        return std::accumulate( pHarmonicAmp, pHarmonicAmpsEnd, 0.0f ) / harmonic;
    }
} // anonymous namespace

void LE_HOT PitchDetector::findHarmonicProductSpectrumAndSort( SW::Engine::ReadOnlyDataRange const amps, HPSRange const hps )
{
    BOOST_ASSERT( amps.size() == hps.size() );

    auto         const numberOfBins( static_cast<std::uint16_t>( amps.size() ) );
    std::uint8_t const lastHarmonic( 5                                         );

    for ( std::uint16_t k( 0 ); k < numberOfBins; ++k )
    {
        float harmonicProduct( amps[ k ] );
        for ( std::uint8_t h( 2 ); h <= lastHarmonic; ++h )
        {
            harmonicProduct += harmonic( h, k, amps );
        }

        BOOST_ASSERT( !Math::isNegative( harmonicProduct ) );
        hps[ k ].harmonicProduct = harmonicProduct;
        hps[ k ].bin             = k              ;
    }

    std::sort( hps.begin(), hps.end() );
}


float LE_HOT PitchDetector::estimatePitch
(
    float                const lastPitch,
    float                const lowerBound,
    float                const upperBound,
    HPSRange             const hps,
    PeakDetector const &       pd
)
{
	float         detectedPitch			   ( 0 );
	float         detectedPitchPeakStrength( 0 );
	std::uint16_t detectedPitchBin         ( 0 );
	std::uint8_t  detectedPitchHPSIndex    ( 0 );

    // Search top 30 in the HPS:
    LE_DISABLE_LOOP_UNROLLING()
    for ( std::uint8_t k( 0 ); k < 30; ++k ) //...mrmlj...can two HPS' bins be under the same peak?
    {
        // If HPS bin is inside a peak and within the bounds then it is the pitch:
        auto const hpsBin( hps[ k ].bin );
        auto const pPeak ( binPeak( hpsBin, pd ) );
        if ( pPeak )
        {
            float const pitch       ( pPeak->freq                                  );
            float const clampedPitch( Math::clamp( pitch, lowerBound, upperBound ) );
            if ( clampedPitch == pitch )
			{
                detectedPitch             = pitch;
				detectedPitchBin          = hpsBin;
				detectedPitchHPSIndex     = k;
				detectedPitchPeakStrength = pPeak->strength;
				break;
			}
        }
    }

	// Detection of lower harmonics, if the change in pitch was above 100Hz in a single frame:
	if ( lastPitch )
	{
        std::uint16_t pos( detectedPitchHPSIndex + 1 );
		// Search through lower harmonics if HPS amplitude is large enough
		// and pitch is still over 100Hz from previously detected pitch:
        LE_DISABLE_LOOP_UNROLLING()
		while
        (
            ( pos < 50 /*heuristic*/                                                                        ) &&
            ( std::abs( detectedPitch - lastPitch ) > 100 /*heuristic*/ /*Hz*/                              ) &&
            ( hps[ pos ].harmonicProduct > 0.4 /*heuristic*/ * hps[ detectedPitchHPSIndex ].harmonicProduct )
        )
		{
			auto const lowHarmonicBin( hps[ pos ].bin );
			if ( lowHarmonicBin == 0 )
			{
				++pos;
				continue;
			}
			// Check if current bin is possibly a lower harmonic of originally detected pitch:
			if ( Math::abs( detectedPitchBin - Math::round( detectedPitchBin * 1.0f / lowHarmonicBin ) * lowHarmonicBin ) < 3 )
			{
				auto const pPeak( binPeak( lowHarmonicBin, pd ) );
				auto const lowHarmonicPitch       ( pPeak ? pPeak->freq     : 0 );
				auto const lowHarmonicPeakStrength( pPeak ? pPeak->strength : 0 );
				// Accept the lower harmonic as pitch if it is closer to the
				// previous pitch and if peak strength is large enough:
				if
                (
                    lowHarmonicPitch                                                                     &&
                    ( std::abs( lowHarmonicPitch - lastPitch ) < std::abs( detectedPitch - lastPitch ) ) &&
                    ( lowHarmonicPeakStrength                  > 0.4 * detectedPitchPeakStrength       )
                )
				{
					detectedPitch = lowHarmonicPitch;
				}
			}
			++pos;
		}
	}

	// Return detected pitch:
	float const clampedPitch( Math::clamp( detectedPitch, lowerBound, upperBound ) );
    if ( clampedPitch == detectedPitch )
		return detectedPitch;

    return 0;
}


Peak const * PitchDetector::binPeak( std::uint16_t const bin, PeakDetector const & pd )
{
    auto const numPeaks( pd.getNumPeaks() );

    for ( std::uint8_t k( 0 ); k < numPeaks; ++k )
    {
        auto const pPeak( pd.getPeak( k ) );
        if ( ( bin >= pPeak->startPos ) && ( bin <= pPeak->stopPos ) )
        {
            BOOST_ASSERT( pPeak->freq && pPeak->amplitude );
            return pPeak;
        }
    }

    return nullptr;
}

LE_OPTIMIZE_FOR_SPEED_END()


void PitchDetector::ChannelState::reset()
{
    lastPitch  = 0;
#ifdef LE_SW_PURE_ANALYSIS
    amplitude  = -std::numeric_limits<float>::infinity();
    confidence = 0;
#endif // LE_SW_PURE_ANALYSIS
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( LE )
//------------------------------------------------------------------------------
