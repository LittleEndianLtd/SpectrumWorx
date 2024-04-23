////////////////////////////////////////////////////////////////////////////////
///
/// pitchShifterImpl.cpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "pitchShifterImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/parameters/uiElements.hpp"

#include <cstdint>
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
// PitchShifter static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PitchShifter  ::title[] = "Pitch Shifter"      ;
char const PVPitchShifter::title[] = "Pitch Shifter (pvd)";


char const Detail::PitchShifterBase::description[] = "Pitch shifter.";


////////////////////////////////////////////////////////////////////////////////
//
// PitchShifter UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Detail::PitchShifterBase::SemiTones     , "Semitones"             )
EFFECT_PARAMETER_NAME( Detail::PitchShifterBase::Cents         , "Cents"                 )
#ifdef LE_PV_TSS_DYNAMIC_THRESHOLD
EFFECT_PARAMETER_NAME( Detail::PitchShifterBase::TSSSensitivity, "Transient sensitivity" )
#endif // LE_PV_TSS_DYNAMIC_THRESHOLD


////////////////////////////////////////////////////////////////////////////////
//
// PitchShifterImpl::setup()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace
{
    void setPitchScale
    (
        PhaseVocoderShared::PitchShiftParameters       &       pitchShiftParameters,
        Detail::PitchShifterBase::Parameters     const &       parameters,
        std::uint16_t                                    const numberOfBins
    )
    {
        using namespace Detail;
        float const pitchScale
        (
            PhaseVocoderShared::PitchShiftParameters::scaleFromSemiTonesAndCents
            (
                parameters.get<PitchShifterBase::SemiTones>(),
                parameters.get<PitchShifterBase::Cents    >()
            )
        );
        pitchShiftParameters.setScalingFactor( pitchScale, numberOfBins );
    }
} // anonymous namespace

void PitchShifterImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    PhaseVocoderShared::PitchShifter::setup( engineSetup );
    setPitchScale( pitchShiftParameters(), parameters(), engineSetup.numberOfBins() );
#ifdef LE_PV_TSS_DYNAMIC_THRESHOLD
    baseParameters().setTSSDynamicThreshold( 1 - ( parameters().get<TSSSensitivity>() / 100 ) );
#endif // LE_PV_USE_TSS
}


void PVPitchShifterImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    setPitchScale( pitchShiftParameters(), parameters(), engineSetup.numberOfBins() );
#ifdef LE_PV_TSS_DYNAMIC_THRESHOLD
    BOOST_ASSERT_MSG
    (
        parameters().get<TSSSensitivity>() == TSSSensitivity::default_(),
        "PVD PitchShifter does not (yet) support the Transient sensitivity parameter."
    );
#endif // LE_PV_TSS_DYNAMIC_THRESHOLD
}


void PVPitchShifterImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    PhaseVocoderShared::PVPitchShifter::process( std::forward<Engine::ChannelData_AmPh>( data ) );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
