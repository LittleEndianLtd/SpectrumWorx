////////////////////////////////////////////////////////////////////////////////
///
/// exImploderImpl.cpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
/// \note Dobson says: "Spectral accumulation. The most extreme effect of
///       the three! Applies feedback echo to each analysis channel, with
///       the possibility also to apply a pitch glissando up or down. Echo
///       is amplitude dependent, so this effect is most apparent when
///       applied to percussive sounds, or any sound with distinct changes."
///
///       http://people.bath.ac.uk/masrwd/pvplugs.html
///                                           (23.03.2010.) (Danijel Domazet)
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "exImploderImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"

#include <cmath>
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
// ExImploder UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PVImploder::title      [] = "Imploder (pvd)";
char const PVImploder::description[] = "Spectral implosion with glissando.";

char const PVExploder::title      [] = "Exploder (pvd)";
char const PVExploder::description[] = "Spectral explosion with glissando.";

char const Imploder::title[] = "Imploder";
char const Exploder::title[] = "Exploder";

EFFECT_PARAMETER_NAME( PVImploder::Decay    , "Decay"     )
EFFECT_PARAMETER_NAME( PVImploder::Gliss    , "Glissando" )
EFFECT_PARAMETER_NAME( PVImploder::Threshold, "Limit"     )
//EFFECT_PARAMETER_NAME( PVImploder::Gate     , "Gate"      )

EFFECT_PARAMETER_NAME( PVExploder::Growth   , "Growth"    )
EFFECT_PARAMETER_NAME( PVExploder::Gliss    , "Glissando" )
EFFECT_PARAMETER_NAME( PVExploder::Threshold, "Limit"     )
EFFECT_PARAMETER_NAME( PVExploder::Gate     , "Gate"      )


namespace Detail
{
    float LE_FASTCALL dB2BinAmplitude( int const dBAmplitude, Engine::Setup const & engineSetup )
    {
        return Math::dB2NormalisedLinear( dBAmplitude ) * engineSetup.maximumAmplitude();
    }

LE_OPTIMIZE_FOR_SIZE_BEGIN()

    float LE_FASTCALL LE_COLD ExImPloderImpl::setup
    (
        int const glissando,
        int const threshold,
        int const gate,
        Engine::Setup const & engineSetup
    )
    {
        /// \note We use the same gating logic for both Exploder and Imploder.
        ///                                   (18.04.2013.) (Domagoj Saric)
             if ( gate == ExImPloder::Gate::minimum() ) gate_ = 0;
      //else if ( gate == ExImPloder::Gate::maximum() ) gate_ = std::numeric_limits<float>::max();
        else                                            gate_ = dB2BinAmplitude( gate, engineSetup );
        threshold_ = dB2BinAmplitude( threshold, engineSetup );
        nyquist_   = engineSetup.sampleRate<float>() * 0.95f / 2.0f;

        float const frameTime( engineSetup.stepTime() );

        // Get cents per second from input and transform to octaves per frame:
        int   const centsPerSecond    ( glissando                    );
        float const semitonesPerSecond( centsPerSecond     / 100.0f  );
        float const octavesPerSecond  ( semitonesPerSecond / 12.0f   );
        float const octavesPerFrame   ( octavesPerSecond * frameTime );
        gliss_ = Math::exp2( octavesPerFrame );
        return frameTime;
    }

LE_OPTIMIZE_FOR_SIZE_END()

    ////////////////////////////////////////////////////////////////////////////
    //
    // ExImPloderImpl::process()
    // -------------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Shared process procedure for the Exploder and Imploder
    /// effects.
    ///
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////
    // Implementation note:
    //   The only two differences between the two effects/procedures are in
    // the growth-decay values and accumulated magnitude-threshold comparison.
    // The first difference is easily solved by simply passing in the desired
    // value (as the 'magnitudeScale'). The accumulated magnitude-threshold
    // comparison on the other hand always uses the same value( the threshold_
    // data member) but the Imploder resets the accumulated magnitude if it is
    // less than the threshold while the Exploder resets it if it is greater
    // than the threshold. This is solved with the additional
    // thresholdIsLowerBound parameter.
    //                                        (10.10.2011.) (Domagoj Saric)
    ////////////////////////////////////////////////////////////////////////////

LE_OPTIMIZE_FOR_SPEED_BEGIN()

    void LE_FASTCALL LE_HOT ExImPloderImpl::process
    (
        ChannelState             &       cs,
        Engine::ChannelData_AmPh &       data,
        bool                       const thresholdIsLowerBound
    ) const
    {
        auto const fullNumberOfBins( data.full().numberOfBins() );

        /// \todo Reinvestigate this algorithm and this pitch shift in
        /// particular, simply multiplying the frequencies is not the correct
        /// way to do it (although Dobson does it that way in his open source
        /// version).
        ///                                   (27.04.2012.) (Domagoj Saric)
        Math::multiply( cs.accumMagn .begin(), magnitudeScale_, fullNumberOfBins );
        Math::multiply( cs.accumFreqs.begin(), gliss_         , fullNumberOfBins );

        float const threshold( threshold_ );
        float const nyquist  ( nyquist_   );
        float const gate     ( gate_      );

        for
        (
            float * LE_RESTRICT pAccumMagn       ( cs  .accumMagn      .begin() ),
                  * LE_RESTRICT pAccumFreq       ( cs  .accumFreqs     .begin() ),
                  * LE_RESTRICT pCurrentAmplitude( data.full().amps  ().begin() ),
                  * LE_RESTRICT pCurrentFreq     ( data.full().phases().begin() ),
                  * LE_RESTRICT const pAccumMagnEnd( pAccumMagn + fullNumberOfBins )
            ;
            pAccumMagn != pAccumMagnEnd
            ;
            ++pAccumMagn,
            ++pAccumFreq,
            ++pCurrentAmplitude,
            ++pCurrentFreq
        )
        {
            /// \note We use the same gating logic for both Exploder and
            /// Imploder.
            ///                               (18.04.2013.) (Domagoj Saric)
          //if ( ( *pCurrentAmplitude > gate ) == thresholdIsLowerBound )
            if ( *pCurrentAmplitude < gate )
                continue;

            if
            (
                ( *pCurrentAmplitude > *pAccumMagn                     ) || // if current amp > amp in accumulator
                ( ( *pAccumMagn < threshold ) == thresholdIsLowerBound ) || // threshold exceeded
                (   *pAccumFreq > nyquist                              )    // Nyquist breached
            )
            {   // replace amp in accumulator with current amp
                *pAccumMagn = *pCurrentAmplitude;
                *pAccumFreq = *pCurrentFreq     ;
            }
            else
            if
            (   // bin within frequency range
                ( pCurrentAmplitude >= data.amps().begin() ) &&
                ( pCurrentAmplitude <  data.amps().end  () )
            )
            {   // else replace current amp with amp in accumulator
                *pCurrentAmplitude = *pAccumMagn;
                *pCurrentFreq      = *pAccumFreq;
            }
        }
    }

LE_OPTIMIZE_FOR_SPEED_END()

LE_OPTIMIZE_FOR_SIZE_BEGIN()

    void ExImPloderImpl::setMagnitudeScale( float const magnitudeScale )
    {
        magnitudeScale_ = magnitudeScale;
    }


    void ExImPloderImpl::ChannelState::reset()
    {
        accumMagn .clear();
        accumFreqs.clear();
    }
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
//
// PVImploderImpl::setup()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void PVImploderImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    float const frameTime
    (
        Detail::ExImPloderImpl::setup
        (
            parameters().get<Gliss    >(),
            parameters().get<Threshold>(),
            parameters().get<Gate     >(),
            engineSetup
        )
    );

    // Transfer decay time from 0 to -120 dB to frame decay constant:
    float const decay
    (
        Math::exp
        (
            ( frameTime / parameters().get<Decay>() )
                *
            Math::ln( Math::dB2NormalisedLinear( -120 ) )
        )
    );
    setMagnitudeScale( decay );
}


////////////////////////////////////////////////////////////////////////////////
//
// PVImploderImpl::process()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PVImploderImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    Detail::ExImPloderImpl::process( cs, data, true );
}


////////////////////////////////////////////////////////////////////////////////
//
// PVExploderImpl::setup()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void PVExploderImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    float const frameTime
    (
        Detail::ExImPloderImpl::setup
        (
            parameters().get<Gliss    >(),
            parameters().get<Threshold>(),
            parameters().get<Gate     >(),
            engineSetup
        )
    );

    // Transfer growth time from 0 to -120 dB to frame growth constant:
    float const growth
    (
        2.0f -
        Math::exp
        (
            ( frameTime / parameters().get<Growth>() )
                *
            Math::ln( Math::dB2NormalisedLinear( -120 ) )
        )
    );
    setMagnitudeScale( growth );
}


////////////////////////////////////////////////////////////////////////////////
//
// PVExploderImpl::process()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PVExploderImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
   Detail::ExImPloderImpl::process( cs, data, false );
}

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
