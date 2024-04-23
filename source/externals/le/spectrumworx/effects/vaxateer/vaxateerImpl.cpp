////////////////////////////////////////////////////////////////////////////////
///
/// vaxateerImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "vaxateerImpl.hpp"

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
// Vaxateer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Vaxateer::title      [] = "Vaxateer";
char const Vaxateer::description[] = "Combination based on RMS.";


////////////////////////////////////////////////////////////////////////////////
//
// Vaxateer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Vaxateer::RMSTarget, "RMS target"         )
EFFECT_PARAMETER_NAME( Vaxateer::RMSGain  , "RMS threshold gain" ) 
EFFECT_PARAMETER_NAME( Vaxateer::Mode     , "Swap condition"     )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Vaxateer, RMSTarget,
    (( MainRMS, "Main" ))
    (( SideRMS, "Side" ))
)

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Vaxateer, Mode,
    (( M1, "Main: >Thr >Side" ))
    (( M2, "Main: >Thr <Side" ))
    (( M3, "Main: <Thr >Side" ))
    (( M4, "Main: <Thr <Side" ))
    (( M5, "Side: >Thr >Main" ))
    (( M6, "Side: >Thr <Main" ))
    (( M7, "Side: <Thr >Main" ))
    (( M8, "Side: <Thr <Main" ))
)


////////////////////////////////////////////////////////////////////////////////
//
// VaxateerImpl::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void VaxateerImpl::setup( IndexRange const &, Engine::Setup const & )
{  
    rmsGain_ = Math::dB2NormalisedLinear( parameters().get<RMSGain>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// VaxateerImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void VaxateerImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    // Calculate RMS of the input signal:
    Engine::ChannelData_AmPh const * pRMSSource;
    switch ( parameters().get<RMSTarget>().getValue() )
    {
        case RMSTarget::MainRMS: pRMSSource = &data.main(); break;
        case RMSTarget::SideRMS: pRMSSource = &data.side(); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }

    float const thr( rmsGain_ * Math::rms( pRMSSource->amps() ) );

    // Setup threshold comparison sources:
    ReadOnlyDataRange const &       mainAmps ( static_cast<Engine::ChannelData_AmPh const &>( data.main() ).amps() );
    ReadOnlyDataRange const &       sideAmps (                                                data.side()  .amps() );
    ReadOnlyDataRange         const threshold( &thr, &thr + 1                                                      );

    ReadOnlyDataRange const * pThresholdComparisonHigherSource;
    ReadOnlyDataRange const * pThresholdComparisonLowerSource;
    ReadOnlyDataRange const * pAmpSideComparisonHigherSource;
    ReadOnlyDataRange const * pAmpSideComparisonLowerSource;
    
    switch ( parameters().get<Mode>().getValue() )
    {
        case Mode::M1:
            pThresholdComparisonHigherSource = &mainAmps;
            pThresholdComparisonLowerSource  = &threshold;
            pAmpSideComparisonHigherSource   = &mainAmps;
            pAmpSideComparisonLowerSource    = &sideAmps;
            break;

        case Mode::M2:
            pThresholdComparisonHigherSource = &mainAmps;
            pThresholdComparisonLowerSource  = &threshold;
            pAmpSideComparisonHigherSource   = &sideAmps;
            pAmpSideComparisonLowerSource    = &mainAmps;
            break;

        case Mode::M3:
            pThresholdComparisonHigherSource = &threshold;
            pThresholdComparisonLowerSource  = &mainAmps;
            pAmpSideComparisonHigherSource   = &mainAmps;
            pAmpSideComparisonLowerSource    = &sideAmps;
            break;

        case Mode::M4:
            pThresholdComparisonHigherSource = &threshold;
            pThresholdComparisonLowerSource  = &mainAmps;
            pAmpSideComparisonHigherSource   = &sideAmps;
            pAmpSideComparisonLowerSource    = &mainAmps;
            break;

        case Mode::M5:
            pThresholdComparisonHigherSource = &sideAmps;
            pThresholdComparisonLowerSource  = &threshold;
            pAmpSideComparisonHigherSource   = &mainAmps;
            pAmpSideComparisonLowerSource    = &sideAmps;
            break;

        case Mode::M6:
            pThresholdComparisonHigherSource = &sideAmps;
            pThresholdComparisonLowerSource  = &threshold;
            pAmpSideComparisonHigherSource   = &sideAmps;
            pAmpSideComparisonLowerSource    = &mainAmps;
            break;

        case Mode::M7:
            pThresholdComparisonHigherSource = &threshold;
            pThresholdComparisonLowerSource  = &sideAmps;
            pAmpSideComparisonHigherSource   = &mainAmps;
            pAmpSideComparisonLowerSource    = &sideAmps;
            break;

        case Mode::M8:
            pThresholdComparisonHigherSource = &threshold;
            pThresholdComparisonLowerSource  = &sideAmps;
            pAmpSideComparisonHigherSource   = &sideAmps;
            pAmpSideComparisonLowerSource    = &mainAmps;
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    while ( data )
    {
        if
        (
            ( *pThresholdComparisonHigherSource->begin() > *pThresholdComparisonLowerSource->begin() ) &&
            ( *pAmpSideComparisonHigherSource  ->begin() > *pAmpSideComparisonLowerSource  ->begin() )
        )
        {
            data.main().amps().front() = data.side().amps().front();
        }

        ++data;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
