////////////////////////////////////////////////////////////////////////////////
///
/// pitchMagnetImpl.cpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "pitchMagnetImpl.hpp"

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
// PitchMagnet static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PitchMagnet   ::title[] = "Pitch Magnet"      ;
char const PitchMagnetPVD::title[] = "Pitch Magnet (pvd)";

char const Detail::PitchMagnetBase::description[] = "Force to target pitch.";


////////////////////////////////////////////////////////////////////////////////
//
// PitchMagnet UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Detail::PitchMagnetBase::Target, "Target"   )
EFFECT_PARAMETER_NAME( Detail::PitchMagnetBase::Speed , "Strength" )


namespace Detail
{
    void PitchMagnetBaseImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
    {
        targetFrequency_           = Math::convert<float>( parameters().get<Target>() );
        pitchChangeLimitSemitones_ = parameters().get<Speed>() / engineSetup.stepsPerSecond();
    }


    float PitchMagnetBaseImpl::findTargetPitch( ChannelState & cs, Engine::ChannelData_AmPh const & data, Engine::Setup const & engineSetup ) const
    {
        float const estimatedPitchMain( PitchDetector::findPitch( data.full().amps(), cs, 70, 7000, engineSetup ) );

        float pitchScale
        (
            ( estimatedPitchMain != 0.0f )
                ? targetFrequency_ / estimatedPitchMain
                : 1
        );

        float pitchScaleSemitones( Math::interval12TET2Semitone( pitchScale ) );

        pitchScaleSemitones = Math::clamp
        (
            pitchScaleSemitones,
            cs.prevPitchScaleSemitones - pitchChangeLimitSemitones_,
            cs.prevPitchScaleSemitones + pitchChangeLimitSemitones_
        );

        cs.prevPitchScaleSemitones =                               pitchScaleSemitones;
        pitchScale                 = Math::semitone2Interval12TET( pitchScaleSemitones );

        return pitchScale;
    }

    void PitchMagnetBaseImpl::ChannelState::reset()
    {
        PitchDetector::ChannelState::reset();
        prevPitchScaleSemitones = 0;
    }
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
//
// PitchMagnetImpl::process()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PitchMagnetImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & setup ) const
{
    float const scale( PitchMagnetBaseImpl::findTargetPitch( cs, data, setup ) );
    PitchShifter::process( scale, cs, std::forward<Engine::ChannelData_AmPh>( data ), setup );
}

void PitchMagnetPVDImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & setup ) const
{
    float const scale( PitchMagnetBaseImpl::findTargetPitch( cs, data, setup ) );
    PVPitchShifter::process( scale, std::forward<Engine::ChannelData_AmPh>( data ), setup );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
