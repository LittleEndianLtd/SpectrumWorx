////////////////////////////////////////////////////////////////////////////////
///
/// sharperImpl.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "sharperImpl.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"
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
// Sharper static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Sharper::title      [] = "Sharper";
char const Sharper::description[] = "Sharpen the spectrum.";


////////////////////////////////////////////////////////////////////////////////
//
// Sharper UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Sharper::AveragingWidth, "Sharpness" )
EFFECT_PARAMETER_NAME( Sharper::Intensity     , "Intensity" )
EFFECT_PARAMETER_NAME( Sharper::Limiter       , "Limit"     )


////////////////////////////////////////////////////////////////////////////////
//
// SharperImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void SharperImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    filterLenHalf_ = engineSetup.frequencyInHzToBin( parameters().get<AveragingWidth>() ) / 2 ;
    intensity_     = Math::dB2NormalisedLinear( parameters().get<Intensity>() );
    cutoff_        = Math::dB2NormalisedLinear( parameters().get<Limiter  >() ) * engineSetup.maximumAmplitude();
}


////////////////////////////////////////////////////////////////////////////////
//
// SharperImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////
/// \note Alex says: "Smoother and Sharper – applies 1st order lowpass and
///       highpass filters respectively among the magnitude and/or the phase
///       axis. M_smth and p_smth are smooth (or sharp) factors whose values
///       range from 0...128 bins. Warning: highly recommend using small
///       values 0-10. Algorithm comments:
//        Smooth code is obvious; Sharper is more complex but based on postulate
///       that Sharp = Real signal - Smoothed signal. You can play with other
///       blur/smooth methods."
///                                           (23.12.2009.) (Danijel Domazet)
/// \note Sharpness basic definition: Ratio of high frequency level to overall
///       level.
///                                           (21.12.2009.) (Danijel Domazet)
/// \todo Reinvestigate and fix or document why do we send half the filter
///       length to the smoothing routine.
///                                           (19.10.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

void SharperImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    if ( filterLenHalf_ == 0 )
        return;

    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( smoothedAmplitudes, Engine::real_t, data.size() );
    Math::symmetricMovingAverage( data.amps(), smoothedAmplitudes, filterLenHalf_ );

    // Combine (subtract smoother from original) smoothed amplitudes with original one:
    float const limit    ( cutoff_    );
    float const intensity( intensity_ );
    float       * LE_RESTRICT pAmp        ( data.amps().begin()        );
    float const * LE_RESTRICT pSmoothedAmp( smoothedAmplitudes.begin() );
    while ( pSmoothedAmp != smoothedAmplitudes.end() )
    {
        float       & amp        ( *pAmp++         );
        float const   smoothedAmp( *pSmoothedAmp++ );

        //amp += intensity_ * ( amp - smoothedAmp );

        // More like Alex idea:
        //amp = smoothedAmp - intensity_ * ( amp - smoothedAmp );

        //amp = Math::clamp( intensity_ * ( smoothedAmp - amp ), 0, limit );
        float const newAmp( amp + intensity * ( amp - smoothedAmp ) );
        amp = Math::clamp( newAmp, 0, limit );

        // Interesting bass effect:
        //amp -= smoothedAmp             ;
        //amp *= smoothedAmp * intensity_;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
