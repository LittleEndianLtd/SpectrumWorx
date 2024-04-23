////////////////////////////////////////////////////////////////////////////////
///
/// phlipImpl.cpp
/// -------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "phlipImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
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
// Phlip static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Phlip::title      [] = "Phlip";
char const Phlip::description[] = "Phase flip.";


////////////////////////////////////////////////////////////////////////////////
//
// Phlip UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Phlip::Mode, "Target harmonics" )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Phlip, Mode,
    (( All , "All"  ))
    (( Even, "Even" ))
    (( Odd , "Odd"  ))
)


////////////////////////////////////////////////////////////////////////////////
//
// PhlipImpl::setup()
// ------------------
//
////////////////////////////////////////////////////////////////////////////////

void PhlipImpl::setup( IndexRange const & workingRange, Engine::Setup const & )
{
    unsigned int const startBin     (  workingRange.first() );
    bool         const rangeNotEmpty( !workingRange.empty() );
    switch ( parameters().get<Mode>().getValue() )
    {
        case Mode::All :
            step_              = 1;
            oddEvenAdjustment_ = false;
            break;

        case Mode::Even:
            step_              = 2;
            oddEvenAdjustment_ = rangeNotEmpty & ( startBin != ( startBin & ~1 ) );
            break;

        case Mode::Odd :
            step_              = 2;
            oddEvenAdjustment_ = rangeNotEmpty & ( startBin != ( startBin |  1 ) );
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// PhlipImpl::process()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void PhlipImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    Math::negate( DataRange( data.phases().begin() + oddEvenAdjustment_, data.phases().end() ), step_ );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
