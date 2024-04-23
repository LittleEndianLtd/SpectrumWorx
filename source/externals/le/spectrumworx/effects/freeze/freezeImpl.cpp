////////////////////////////////////////////////////////////////////////////////
///
/// freezeImpl.cpp
/// --------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "freezeImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/math.hpp"
#include "le/math/conversion.hpp"
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
// Freeze static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Freeze::title      [] = "Freeze";
char const Freeze::description[] = "Time freeze.";


////////////////////////////////////////////////////////////////////////////////
//
// Freeze UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Freeze::FreezeTrigger , "Freeze"          )
EFFECT_PARAMETER_NAME( Freeze::MeltTrigger   , "Melt"            )
EFFECT_PARAMETER_NAME( Freeze::TransitionTime, "Transition time" )


////////////////////////////////////////////////////////////////////////////////
//
// FreezeImpl::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void FreezeImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    pvParameters_.setup( engineSetup );

    freeze_ = parameters().get<FreezeTrigger>().consumeValue();
    melt_   = parameters().get<MeltTrigger  >().consumeValue();
    LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();
    inverseTransitionTime_ = 1 / Math::convert<float>( engineSetup.milliSecondsToSteps( parameters().get<TransitionTime>() ) );
}


////////////////////////////////////////////////////////////////////////////////
//
// FreezeImpl::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////
/// \todo Create a PV version of Freeze.
///                                           (09.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

void FreezeImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    if ( !freeze_ & !melt_ & !cs.frozen & cs.freezeDone & cs.meltDone )
        return;

    using namespace Math;

    //------------------------------------------------------------------------//

    DataRange const & currentFullMag ( data.full().amps  () );
    DataRange const & currentFullFreq( data.full().phases() );

    // To PV domain:
    PhaseVocoderShared::analysis( cs.pvState, data.full(), pvParameters_ );

    DataRange const & currentMag ( data.amps  () );
    DataRange const & currentFreq( data.phases() );

    auto const fullNumberOfBins( data.full().numberOfBins() );
    auto const beginBin        ( data.beginBin()            );

    //------------------------------------------------------------------------//

    // Freeze command received?
    //...mrmlj...quick-workaround for a non-deterministic relationship between
    //...mrmlj...setup() and process() calls...
    bool const freeze( freeze_ & ( freeze_ != cs.previousFreezeFlag ) );
    bool const melt  ( melt_   & ( melt_   != cs.previousMeltFlag   ) );
    cs.previousFreezeFlag = freeze_;
    cs.previousMeltFlag   = melt_  ;

    BOOST_ASSERT_MSG( !( freeze && melt )                          , "Freezing and melting at the same time?" );
  //BOOST_ASSERT_MSG( cs.frameCounter <= 1 / inverseTransitionTime_, "Frame counter overflow."                ); //...mrmlj...

    if ( freeze )
    {
        cs.freezeDone = false; // Signal that freezing is in process.
        cs.frozen     = false; // We don't have a frozen signal yet.

        cs.frameCounter = 0;

        // Freeze current frame, and save the previous frozen frame, so we can
        // make a transition between two frozen frames:

        copy( cs.frozenMagNew .begin(), cs.frozenMagOld .begin(), fullNumberOfBins );
        copy( cs.frozenFreqNew.begin(), cs.frozenFreqOld.begin(), fullNumberOfBins );

        copy( currentFullMag , cs.frozenMagNew  );
        copy( currentFullFreq, cs.frozenFreqNew );
    }

    // Melt command received?
    if ( melt & cs.freezeDone ) // Ignore if freezing process is active.
    {
        cs.meltDone = false; // Signal that melting is in process.
        cs.frozen   = false; // We don't have a frozen signal for output since
                             // we are melting.
        cs.frameCounter = 0;
    }

    // Both freezing and melting can be smoothed in a way that current state is
    // blended with future state.
    // Here we find a "blending" ratio between source and target signals, which
    // depends on the chosen period:

    // If period is zero, then there is no transition, output formula
    // "out = blendFactor * ( in2 - in1 ) + in1" amounts to "out = in2".
    LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();
    float const blendFactor( std::min( cs.frameCounter++ * inverseTransitionTime_, 1.0f ) );
    LE_ASSUME( blendFactor >= 0 );
    LE_ASSUME( blendFactor <= 1 );
    bool const blendFactorIsOne( blendFactor == 1 );

    // Is melting in process?
    if ( !cs.meltDone )
    {
        if ( cs.normal )
        {
            // If we are in "normal" mode then nothing is frozen, there is
            // nothing to melt:
            cs.meltDone = true;
        }
        else
        {
            // Let's melt the frozen frame but slowly, we need to do the
            // transition from last frozen frame to current input:
            mix( currentMag .begin(), &cs.frozenMagNew [ beginBin ], currentMag .begin(), currentMag .end(), blendFactor );
            mix( currentFreq.begin(), &cs.frozenFreqNew[ beginBin ], currentFreq.begin(), currentFreq.end(), blendFactor );

            if ( blendFactorIsOne )
            {
                // If transition is over then enter "normal" mode:
                cs.meltDone = true;
                cs.normal   = true;
            }
            else
            {
                cs.meltDone = false;
            }
        }
    }

    // Is freezing in process?
    if ( !cs.freezeDone )
    {
        // If we are in "normal" mode then we need to do a transition from
        // normal input to the first frozen frame:
                
        if ( cs.normal )
        {
            mix( &cs.frozenMagNew [ beginBin ], currentMag .begin(), currentMag .begin(), currentMag .end(), blendFactor );
            mix( &cs.frozenFreqNew[ beginBin ], currentFreq.begin(), currentFreq.begin(), currentFreq.end(), blendFactor );

            if ( blendFactorIsOne )
            {
                // If transition is over then...
                cs.freezeDone = true;  // ...done with freezing.
                cs.frozen     = true;  // ...state is "frozen".
                cs.normal     = false; // ...state isn't "normal".
            }
            else
            {
                cs.freezeDone = false;
                cs.frozen     = false;
            }
        }
        else
        {
            // Since we are not in "normal" mode, we need to do a transition
            // from last frozen frame to current frozen frame:
            mix( &cs.frozenMagNew [ beginBin ], &cs.frozenMagOld [ beginBin ], currentMag .begin(), currentMag .end(), blendFactor );
            mix( &cs.frozenFreqNew[ beginBin ], &cs.frozenFreqOld[ beginBin ], currentFreq.begin(), currentFreq.end(), blendFactor );

            if ( blendFactorIsOne )
            {
                // If transition is over then...
                cs.freezeDone = true; // ...done with freezing.
                cs.frozen     = true; // ...state is "frozen".
            }
            else
            {
                cs.freezeDone = false;
                cs.frozen     = false;
            }
        }
    }

    // If state is frozen then we just need to play the frozen frame over and
    // over again:
    if ( cs.frozen )
    {
        unsigned int const numberOfBins( data.numberOfBins() );
        copy( &cs.frozenMagNew [ beginBin ], currentMag .begin(), numberOfBins );
        copy( &cs.frozenFreqNew[ beginBin ], currentFreq.begin(), numberOfBins );
    }

    //------------------------------------------------------------------------//

    // Back from the PV domain:
    PhaseVocoderShared::synthesis( cs.pvState, data.full().phases(), pvParameters_ );
}


////////////////////////////////////////////////////////////////////////////////
//
// FreezeImpl::ChannelState::clear()
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////
        
void FreezeImpl::ChannelState::reset()
{
    frameCounter = 0;
    
    normal     = true ; // Start in normal state, not frozen one.
    frozen     = false;    
   
    freezeDone = true; // Nothing is in process of freezing or melting.
    meltDone   = true;

    previousFreezeFlag = false;
    previousMeltFlag   = false;

    /// \note Frozen data does not need to be cleared so we don't call
    /// DynamicChannelState::reset().
    ///                                       (16.09.2013.) (Domagoj Saric)
    pvState.reset();
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
