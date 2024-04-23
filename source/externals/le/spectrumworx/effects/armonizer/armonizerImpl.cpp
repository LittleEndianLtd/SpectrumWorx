////////////////////////////////////////////////////////////////////////////////
///
/// armonizerImpl.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "armonizerImpl.hpp"

#include "le/spectrumworx/engine/setup.hpp"
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
// Armonizer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Armonizer::title      [] = "Armonizer";
char const Armonizer::description[] = "Add harmonics.";


////////////////////////////////////////////////////////////////////////////////
//
// Armonizer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Armonizer::Interval, "Interval" )


////////////////////////////////////////////////////////////////////////////////
//
// ArmonizerImpl::setup()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void ArmonizerImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    // Setup pitch shifter:
    PitchShifter::setup                     ( engineSetup                                              );
    PitchShifter::setPitchScaleFromSemitones( parameters().get<Interval>(), engineSetup.numberOfBins() );
}

/// \todo This is just another pitch shifter. Improve it by adding the ability
/// to add more harmonics.
///                                           (24.10.2011.) (Domagoj Saric)

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
