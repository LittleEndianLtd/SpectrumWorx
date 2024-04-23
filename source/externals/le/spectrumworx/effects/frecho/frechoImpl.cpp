////////////////////////////////////////////////////////////////////////////////
///
/// frechoImpl.cpp
/// --------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "frechoImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/channelDataReIm.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/dft/domainConversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"
//------------------------------------------------------------------------------
/// \todo Investigate frequency-domain echo cancelation.
/// http://jmvalin.ca/papers/valin_hscma2008.pdf
///                                           (04.03.2015.) (Domagoj Saric)
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
// Fre(v)cho static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Frecho::title      [] = "Frecho";
char const Frecho::description[] = "Frequency domain echo with pitch.";

char const Frevcho::title      [] = "Frevcho";
char const Frevcho::description[] = "Frequency domain reversed echo with pitch.";


////////////////////////////////////////////////////////////////////////////////
//
// Frecho UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Frecho::Distance  , "Distance"   )
EFFECT_PARAMETER_NAME( Frecho::Absorption, "Absorption" )
EFFECT_PARAMETER_NAME( Frecho::EchoPitch , "Pitch"      )


////////////////////////////////////////////////////////////////////////////////
//
// FrechoImpl::setup()
// -------------------
//
////////////////////////////////////////////////////////////////////////////////

void FrechoImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
{
    gain_ = Math::dB2NormalisedLinear( - parameters().get<Absorption>() );

    // Calculate the time needed for sound to return to the source based on
    // distance:
    echoSizeInSteps_ = static_cast<std::uint8_t>( engineSetup.milliSecondsToSteps( parameters().get<Distance>() * 2 * 1000 / speedOfSound ) );

    // Init pitch shifter:
    ps_.setup( engineSetup );
    ps_.setPitchScaleFromSemitones( parameters().get<EchoPitch>(), engineSetup.numberOfBins() );
}


////////////////////////////////////////////////////////////////////////////////
//
// FrechoImpl::process()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void FrechoImpl::doProcess
(
    ChannelState                   & cs,
    Engine::ChannelData_ReIm       & target,
    Engine::Setup            const & engineSetup
) const
{
    auto const numberOfBins( target.full().numberOfBins() );

    //...mrmlj...reinvestigate this and create a reusable solution...
    //...mrmlj...check ReversedHistoryBufferState::getCurrentStepData...
    auto const alignedNumberOfBins( Math::alignIndex( numberOfBins ) );

    // Echo data to calculate and save from this frame...
    float * LE_RESTRICT const pEchoFromFrameReals( &cs.historyBuffer[ cs.frameCounter * alignedNumberOfBins * 2 ] );
    float * LE_RESTRICT const pEchoFromFrameImags( pEchoFromFrameReals + alignedNumberOfBins                      );

    // Echo data to mix into this frame...
    auto const frame( cs.frameCounter.nextValueFor( echoSizeInSteps() ).first );

    float const * LE_RESTRICT pEchoForFrameReal ( &cs.historyBuffer[ frame * alignedNumberOfBins * 2 ] );
    float const * LE_RESTRICT pEchoForFrameImag ( pEchoForFrameReal + alignedNumberOfBins              );

    float       * LE_RESTRICT pEchoFromFrameReal( pEchoFromFrameReals );
    float       * LE_RESTRICT pEchoFromFrameImag( pEchoFromFrameImags );

    float       * LE_RESTRICT pTargetMainReal( target.full().reals().begin() );
    float       * LE_RESTRICT pTargetMainImag( target.full().imags().begin() );

    float const gain( this->gain() );

    auto binCounter( numberOfBins );

    while ( binCounter-- )
    {
        float       & mainReal( *pTargetMainReal++   );
        float       & mainImag( *pTargetMainImag++   );
        float const   echoReal( *pEchoForFrameReal++ );
        float const   echoImag( *pEchoForFrameImag++ );

        // Calculate echo from: this frame + this echo! And save for future...:
        *pEchoFromFrameReal++ = ( mainReal + echoReal ) * gain;
        *pEchoFromFrameImag++ = ( mainImag + echoImag ) * gain;

        // Calculate current output from current input and saved echo:
        if ( &mainReal >= target.reals().begin() && &mainReal < target.reals().end() )
        {
            mainReal = echoReal;
            mainImag = echoImag;
        }
    }

    // Pitch shift saved echo data.
    if ( !ps_.skipProcessing() )
    {
        unsigned int const fftSize( engineSetup.fftSize<unsigned int>() );
        BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( pitchShiftedEchoStorage, char, Engine::ChannelData_AmPhStorage::requiredStorage( fftSize ) );
        Engine::ChannelData_AmPhStorage pitchShiftedEcho( fftSize, target.beginBin(), target.endBin(), pitchShiftedEchoStorage );

        Math::reim2AmPh
        (
            pEchoFromFrameReals,
            pEchoFromFrameImags,
            pitchShiftedEcho.full().amps  ().begin(),
            pitchShiftedEcho.full().phases().begin(),
            numberOfBins
        );

        ps_.process( cs.pvState, std::forward<Engine::ChannelData_AmPh>( pitchShiftedEcho ) );

        Math::amph2ReIm
        (
            pitchShiftedEcho.full().amps  ().begin(),
            pitchShiftedEcho.full().phases().begin(),
            pEchoFromFrameReals,
            pEchoFromFrameImags,
            numberOfBins
        );
    }
}

void FrechoImpl::process( ChannelState & cs, Engine::ChannelData_ReIm data, Engine::Setup const & engineSetup ) const
{
    doProcess( cs, data, engineSetup );
}


////////////////////////////////////////////////////////////////////////////////
//
// FrevchoImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void FrevchoImpl::process( ChannelState & cs, Engine::ChannelData_ReIm data, Engine::Setup const & engineSetup ) const
{
    using namespace Math;

    auto const fullNumberOfBins( data.full().numberOfBins() );

    //...mrmlj...Reverser
    ReversedHistoryBufferState::HistoryData const historyData
    (
        cs.getCurrentStepData( echoSizeInSteps(), fullNumberOfBins )
    );

    auto const startBin( data.beginBin() );
    swap( data.reals().begin(), data.reals().end(), historyData.targetHistory.pAmplitudesOrReals + startBin );
    swap( data.imags().begin(), data.imags().end(), historyData.targetHistory.pPhasesOrImags     + startBin );

    data.copySkippedRanges( Engine::DataPair::Reals, historyData.targetHistory.pAmplitudesOrReals );
    data.copySkippedRanges( Engine::DataPair::Imags, historyData.targetHistory.pPhasesOrImags     );

    if ( historyData.isEmulated() )
    {
        auto const numberOfBins( data.numberOfBins() );
        copy( historyData.sourceHistory.pAmplitudesOrReals + startBin, data.reals().begin(), numberOfBins );
        copy( historyData.sourceHistory.pPhasesOrImags     + startBin, data.imags().begin(), numberOfBins );
    }

    //...mrmlj...Frecho
    FrechoImpl::doProcess( cs, data, engineSetup );

    //...mrmlj...Reverser...
    negate( historyData.targetHistory.pPhasesOrImags, fullNumberOfBins );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
