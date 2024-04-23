////////////////////////////////////////////////////////////////////////////////
///
/// pitchSpringImpl.cpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "pitchSpringImpl.hpp"

#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"
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
// PitchSpring static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const PitchSpring   ::title[] = "Pitch Spring";
char const PitchSpringPVD::title[] = "Pitch Spring (pvd)";

char const Detail::PitchSpringBase::description[] = "Oscillating pitch.";


////////////////////////////////////////////////////////////////////////////////
//
// PitchSpring UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Detail::PitchSpringBase::Depth , "Depth"  )
EFFECT_PARAMETER_NAME( Detail::PitchSpringBase::Period, "Period" )


namespace Detail
{
    void PitchSpringBaseImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
    {
        VibratoEffect::setup( parameters().get<Period>(), engineSetup );
    }

    float PitchSpringBaseImpl::calculateNewPitch( ChannelState & cs ) const
    {
        return VibratoEffect::calculateNewPitch( cs, parameters().get<SpringType>(), parameters().get<Depth>() );
    }
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
//
// PitchSpringImpl::process()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

void PitchSpringImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & engineSetup ) const
{
    float const scale( PitchSpringBaseImpl::calculateNewPitch( cs ) );
    PitchShifter::process( scale, cs, std::forward<Engine::ChannelData_AmPh>( data ), engineSetup );
}

void PitchSpringPVDImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & engineSetup ) const
{
    float const scale( PitchSpringBaseImpl::calculateNewPitch( cs ) );
    PVPitchShifter::process( scale, std::forward<Engine::ChannelData_AmPh>( data ), engineSetup );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
