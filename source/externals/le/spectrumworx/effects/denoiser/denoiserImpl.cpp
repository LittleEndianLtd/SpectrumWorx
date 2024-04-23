////////////////////////////////////////////////////////////////////////////////
///
/// denoiserImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "denoiserImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
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
// Denoiser static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Denoiser::title      [] = "Denoiser";
char const Denoiser::description[] = "Denoise using noise footprint.";


////////////////////////////////////////////////////////////////////////////////
//
// Denoiser UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Denoiser::Intensity, "Amount"          )
EFFECT_PARAMETER_NAME( Denoiser::Mode     , "Noise footprint" )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Denoiser, Mode,
    (( Main, "Main"    ))
    (( Side, "Side"    ))
    (( Sum , "Average" ))
)


////////////////////////////////////////////////////////////////////////////////
//
// DenoiserImpl::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void DenoiserImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    float const zerodBLevel( engineSetup.maximumAmplitude() );
    float const maxAmpdB(  0 );
    float const minAmpdB( -1 );
    float const maxAmp  ( zerodBLevel * Math::dB2NormalisedLinear( maxAmpdB ) );
    float const minAmp  ( zerodBLevel * Math::dB2NormalisedLinear( minAmpdB ) );
    LE_ASSUME( maxAmp == zerodBLevel );
    LE_ASSUME( maxAmp > minAmp );

  //factor_ = ( parameters().get<Factor>() / 1000.0f ) * 2.0f;    
    factor_ = Math::percentage2NormalisedLinear( parameters().get<Intensity>() ) * ( maxAmp - minAmp );
    LE_ASSUME( factor_ > 0 );
}


////////////////////////////////////////////////////////////////////////////////
//
// DenoiserImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void DenoiserImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    /// \note Udo Zolzer, DAFX, 8.4.9 Denoising
    /// f(x) = x^2/(x+c)
    /// Stronger components survive, weaker are further attenuated.
    ///                                       (11.09.2013.) (Domagoj Saric)

    Mode::value_type const mode  ( parameters().get<Mode>() );
    float            const factor( factor_                  );

    while ( data )
    {
        float       & mainAmp( data.main().amps().front() );
        float const   sideAmp( data.side().amps().front() );
        ++data;

        /// \note Mode::Side requires special handling because the basic DAFX
        /// formula is not suited for side-chain based denoising, with low side
        /// chain levels it can produce high amplification/correction factors
        /// resulting in clipping or auto-muting. For this reason we limit it so
        /// that the input signal is never amplified (maximum correction factor
        /// is 1).
        ///                                   (11.09.2013.) (Domagoj Saric)
        float c;
        switch ( mode )
        {
            case Mode::Main: c =   mainAmp                   ;                                      break;
            case Mode::Side: c =             sideAmp         ; c = std::max( c, mainAmp - factor ); break;
            case Mode::Sum : c = ( mainAmp + sideAmp ) / 2.0f;                                      break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }

        float const correctionFactor( mainAmp / ( c + factor ) );
        LE_ASSUME( correctionFactor >= 0 );
        LE_ASSUME( correctionFactor <= 1 );
        mainAmp *= correctionFactor;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
