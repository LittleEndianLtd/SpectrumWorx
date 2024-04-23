////////////////////////////////////////////////////////////////////////////////
///
/// slewLimiterImpl.cpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "slewLimiterImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
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
// SlewLimiter static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const SlewLimiter::title      [] = "Slew Limiter";
char const SlewLimiter::description[] = "Limit magnitude change speed.";

/// \note Alex says: "Created by Bram de Jong @ Smartelectronix (www.smartelectronix.com) 
/// and Gabe «LoGreyBeam» Morley (www.soundmangle.com).  This is just well known 
/// time domain slew limiter algorithm adapted to spectral domain.  
/// Limit control ranges from  -80db to -6db because the slew limit value gives 
/// best results on very small values.  
/// Slew limiter applied to magnitudes produce interesting effects."
///                                    (20.12.2009.) (Danijel Domazet)

////////////////////////////////////////////////////////////////////////////////
//
// SlewLimiter UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( SlewLimiter::SlewRate , "Slew rate" )
EFFECT_PARAMETER_NAME( SlewLimiter::Direction, "Direction" )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    SlewLimiter, Direction,
    (( RiseFall, "Rise&Fall" ))
    (( Rise    , "Rise"      ))
    (( Fall    , "Fall"      ))
)


////////////////////////////////////////////////////////////////////////////////
//
// SlewLimiterImpl::setup()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

void SlewLimiterImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    float const dBLimit( parameters().get<SlewRate>() / engineSetup.stepsPerSecond() );
    float const limit  ( Math::dB2NormalisedLinear( dBLimit )                        );

    switch ( parameters().get<Direction>().getValue() )
    {
        case Direction::RiseFall: gainLowerBound_ = 1 / limit; gainUpperBound_ = limit                            ; break;
        case Direction::Rise    : gainLowerBound_ = 0        ; gainUpperBound_ = limit                            ; break;
        case Direction::Fall    : gainLowerBound_ = 1 / limit; gainUpperBound_ = std::numeric_limits<float>::max(); break;
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// SlewLimiterImpl::process()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////
/// \note Alex says: "Created by Bram de Jong @ Smartelectronix
/// (www.smartelectronix.com) and Gabe «LoGreyBeam» Morley
/// (www.soundmangle.com). This is just well known time domain slew limiter
/// algorithm adapted to spectral domain."
///                                           (20.12.2009.) (Danijel Domazet)
////////////////////////////////////////////////////////////////////////////////

void SlewLimiterImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    using namespace Math;

    if ( cs.isInitialised )
    {
        float const gainLowerBound( gainLowerBound_ );
        float const gainUpperBound( gainUpperBound_ );

        float       * LE_RESTRICT pCurrentAmp ( data.amps().begin()             );
        float const * LE_RESTRICT pPreviousAmp( &cs.magsPrev[ data.beginBin() ] );

        float const * const pEnd( data.amps().end() );
        while ( pCurrentAmp != pEnd )
        {
            /// \note previousAmp must be kept above zero in order for the
            /// signal to be able to ever rise from initial silence (because
            /// previousAmp * clampedGain will alwas yield zero if previousAmp
            /// is zero). Additionally, it helps avoiding floating point
            /// exceptions in the currentAmp / previousAmp expression.
            ///                               (19.06.2012.) (Domagoj Saric)
            float const previousAmp( std::max( *pPreviousAmp++, std::numeric_limits<float>::epsilon() ) );
            float const currentAmp (           *pCurrentAmp                                             );

            float const gain( currentAmp / previousAmp );

            float const clampedGain( clamp( gain, gainLowerBound, gainUpperBound ) );

            *pCurrentAmp++ = previousAmp * clampedGain;
        }
    }

    // Save new and unused amplitudes for next comparison:
    copy( data.full().amps(), cs.magsPrev );
    cs.isInitialised = true;
}


void SlewLimiterImpl::ChannelState::reset()
{
    // Implementation note:
    //   No need to call magsPrev.clear() as limiting is skipped for
    // uninitialized channel states.
    //                                        (14.10.2011.) (Domagoj Saric)
    isInitialised = false;
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
