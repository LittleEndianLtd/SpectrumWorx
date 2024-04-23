////////////////////////////////////////////////////////////////////////////////
///
/// sumoPitchImpl.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "sumoPitchImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelData.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/dft/domainConversion.hpp"
#include "le/math/math.hpp"
#include "le/math/conversion.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"
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
// SumoPitch static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const SumoPitch::title      [] = "Sumo Pitch";
char const SumoPitch::description[] = "Pitch fight.";


////////////////////////////////////////////////////////////////////////////////
//
// SumoPitch UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( SumoPitch::Blend, "Blend amount" )
EFFECT_PARAMETER_NAME( SumoPitch::Speed, "Speed"        )


////////////////////////////////////////////////////////////////////////////////
//
// SumoPitchImpl::setup()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void SumoPitchImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    amount_ = 1.0f - Math::percentage2NormalisedLinear( parameters().get<Blend>() );

    pitchChangeLimitSemitones_ = parameters().get<Speed>() / engineSetup.stepsPerSecond();

    ps_.setup( engineSetup );
}


////////////////////////////////////////////////////////////////////////////////
//
// SumoPitchImpl::process()
// ------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace
{
    void limitPitchScale( float & pitchScale, float & previousPitchScaleSemitones, float const pitchScaleLimitSemitones )
    {
        using namespace Math;

        float const lowerLimit( previousPitchScaleSemitones - pitchScaleLimitSemitones );
        float const upperLimit( previousPitchScaleSemitones + pitchScaleLimitSemitones );
        float const pitchScaleSemitones
        (
            clamp
            (
                interval12TET2Semitone( pitchScale ),
                lowerLimit,
                upperLimit
            )
        );

        previousPitchScaleSemitones =                         pitchScaleSemitones  ;
        pitchScale                  = semitone2Interval12TET( pitchScaleSemitones );
    }
} // anonymous namespace

void SumoPitchImpl::process( ChannelState & cs, Engine::ChannelData_AmPh2ReIm data, Engine::Setup const & engineSetup ) const
{
    using namespace Math;

    Engine::MainSideChannelData_AmPh const & amPhData( data.input );

    float pitchScaleMain;
    float pitchScaleSide;

    {
        float const estimatedPitchMain( PitchDetector::findPitch( amPhData.full().main().amps(), cs.pdState, 70, 7000, engineSetup ) );
        float const estimatedPitchSide( PitchDetector::findPitch( amPhData.full().side().amps(), cs.pdState, 70, 7000, engineSetup ) );

        if ( estimatedPitchMain && estimatedPitchSide )
        {
            float const targetPitch( ( estimatedPitchMain + estimatedPitchSide ) / 2.0f );

            pitchScaleMain = targetPitch / estimatedPitchMain;
            pitchScaleSide = targetPitch / estimatedPitchSide;

            //pitchScaleMain = estimatedPitchSide / estimatedPitchMain;
            //pitchScaleSide = estimatedPitchMain / estimatedPitchSide;
        }
        else
        {
            pitchScaleMain = 1.0f;
            pitchScaleSide = 1.0f;
        }

        limitPitchScale( pitchScaleMain, cs.prevPitchScaleMainSemitones, pitchChangeLimitSemitones_ );
        limitPitchScale( pitchScaleSide, cs.prevPitchScaleSideSemitones, pitchChangeLimitSemitones_ );
    }

    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( workBufferStorage, char, Engine::ChannelData_AmPhStorage::requiredStorage( engineSetup.fftSize<unsigned int>() ) );
    Engine::ChannelData_AmPhStorage psWorkBuffer( engineSetup.fftSize<unsigned int>(), amPhData.beginBin(), amPhData.endBin(), workBufferStorage );

    { // Pitch shift:
        // Side
        copy( amPhData.full().side().jointView(), psWorkBuffer.full().jointView() );
        ps_.process( pitchScaleSide, cs.side, std::forward<Engine::ChannelData_AmPh>( psWorkBuffer ), engineSetup );

        // Convert pitch shifted side channel to ReIm form and store it directly
        // to output.
        amph2ReIm
        (
            psWorkBuffer.amps  ().begin(),
            psWorkBuffer.phases().begin(),
            data.output .reals ().begin(),
            data.output .imags ().begin(),
            amPhData.numberOfBins()
        );

        // Main
        copy( amPhData.full().main().jointView(), psWorkBuffer.full().jointView() );
        ps_.process( pitchScaleMain, cs.main, std::forward<Engine::ChannelData_AmPh>( psWorkBuffer ), engineSetup );
    }

    // Blend:
    //  - psWorkBuffer contains pitch shifted main AmPh data
    //  - data.output  contains pitch shifted side ReIm data
    mix( psWorkBuffer.amps(), psWorkBuffer.phases(), data.output.reals(), data.output.imags(), amount_ );
}


void SumoPitchImpl::ChannelState::reset()
{
    DynamicChannelState::reset();

    prevPitchScaleMainSemitones = 0;
    prevPitchScaleSideSemitones = 0;

    pdState.reset();
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
