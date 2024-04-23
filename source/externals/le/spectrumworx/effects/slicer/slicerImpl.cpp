////////////////////////////////////////////////////////////////////////////////
///
/// slicerImpl.cpp
/// --------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "slicerImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
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
// Slicer static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Slicer::title      [] = "Slicer";
char const Slicer::description[] = "Slice and fill.";


////////////////////////////////////////////////////////////////////////////////
//
// Slicer UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Slicer::TimeOn , "On time"       )
EFFECT_PARAMETER_NAME( Slicer::TimeOff, "Slice time"    )
EFFECT_PARAMETER_NAME( Slicer::Mode   , "Slice content" )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Slicer, Mode,
    (( Hold   , "Sample&Hold" ))
    (( Silence, "Silence"     ))
    (( Side   , "Side"        ))
)


////////////////////////////////////////////////////////////////////////////////
//
// SlicerImpl::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void SlicerImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{  
    // User control
    timeOn_  = engineSetup.milliSecondsToSteps( parameters().get<TimeOn >() );
    timeOff_ = engineSetup.milliSecondsToSteps( parameters().get<TimeOff>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// SlicerImpl::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void SlicerImpl::process( ChannelState & cs, Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    using namespace Math;

    Mode::value_type const mode( parameters().get<Mode>() );

    DataRange const csAmps  ( &cs.mags[ data.main().beginBin() ], &cs.mags[ data.main().endBin() - 1 ] + 1 );
    DataRange const csPhases( &cs.phas[ data.main().beginBin() ], &cs.phas[ data.main().endBin() - 1 ] + 1 );

    if ( cs.silence )
    {
        bool const wrappedAround( cs.counter.nextValueFor( timeOff_ ).second );
        if ( wrappedAround )
        {
            cs.silence = false;
            // Pass through
            return;
        }
        else
        {
            switch ( mode )
            {
                case Mode::Hold:
                    copy( csAmps  , data.main().amps  () );
                    copy( csPhases, data.main().phases() );
                    break;

                case Mode::Silence: 
                    clear( data.main().amps() );
                    break;

                case Mode::Side:
                    copy( data.side().amps  (), data.main().amps  () ); 
                    copy( data.side().phases(), data.main().phases() );
                    break;

                LE_DEFAULT_CASE_UNREACHABLE();
            }

            return;
        }
    }

    if ( !cs.silence )
    {
        bool const wrappedAround( cs.counter.nextValueFor( timeOn_ ).second );
        if ( wrappedAround )
        {
            switch ( mode )
            {
                case Mode::Hold:
                    // Save this frame for next "silent part" and output current
                    // state:
                    swap( data.main().amps  (), csAmps   );
                    swap( data.main().phases(), csPhases );

                    // Save unused amplitudes for next "silent part":
                    data.main().copySkippedRanges( Engine::DataPair::Amps  , cs.mags );
                    data.main().copySkippedRanges( Engine::DataPair::Phases, cs.phas );
                    break;

                case Mode::Silence:
                    clear( data.main().amps() );
                    break;

                case Mode::Side:
                    /// \todo Investigate whether this case really should do
                    /// nothing and document accordingly.
                    ///                       (13.05.2011.) (Domagoj Saric)
                    break;

                LE_DEFAULT_CASE_UNREACHABLE();
            }

            // Silence starts:
            cs.silence = true;

            return; 
        }
        else
        {
            // Pass-through. 
            return;
        }
    }
}


void SlicerImpl::ChannelState::reset()
{
    DynamicChannelState::reset();
    counter.reset();
    silence = false;
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
