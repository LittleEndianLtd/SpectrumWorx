////////////////////////////////////////////////////////////////////////////////
///
/// talkBoxImpl.cpp
/// ---------------
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "talkBoxImpl.hpp"

#include "le/math/math.hpp"
#include "le/math/conversion.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelData.hpp"
#include "le/spectrumworx/engine/setup.hpp"
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
// TalkingWind static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const TalkBox::title      [] = "Talk Box";
char const TalkBox::description[] = "Classic vocoding with a synthesized carrier.";


////////////////////////////////////////////////////////////////////////////////
//
// TalkBox UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( TalkBox::ExternalCarrier, "External carrier" )
EFFECT_PARAMETER_NAME( TalkBox::BaseFrequency  , "Base frequency"   )
EFFECT_PARAMETER_NAME( TalkBox::CutOff         , "Cutoff"           )


////////////////////////////////////////////////////////////////////////////////
//
// TalkBox::setup()
// ----------------
//
////////////////////////////////////////////////////////////////////////////////
LE_COLD
void TalkBoxImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    synth_.parameters().set<Synth::Frequency      >( parameters().get<BaseFrequency           >() );
    synth_.parameters().set<Synth::HarmonicSlope  >( parameters().get<TalkBox::HarmonicSlope  >() );
    synth_.parameters().set<Synth::FlangeIntensity>( parameters().get<TalkBox::FlangeIntensity>() );
    synth_.parameters().set<Synth::FlangeOffset   >( parameters().get<TalkBox::FlangeOffset   >() );

    auto const cutOff
    (
        std::min<std::uint32_t>
        (
            parameters().get<TalkBox::CutOff>(),
            engineSetup.sampleRate<std::uint32_t>() / 2
        )
    );
    auto const cutOffBin( static_cast<std::uint16_t>( cutOff / engineSetup.frequencyRangePerBin<float>() ) );
    IndexRange const synthWorkingRange
    (
        std::min( workingRange.begin(), cutOffBin ),
        std::min( workingRange.end  (), cutOffBin )
    );
    synth_.setup( synthWorkingRange, engineSetup );

    vocoder_.parameters().set<Vocoder::FilterMethod  >( Vocoder::FilterMethod::MelEnvelope );
    vocoder_.parameters().set<Vocoder::EnvelopeBorder>( 800 );
    vocoder_.setup( workingRange, engineSetup );
}


////////////////////////////////////////////////////////////////////////////////
//
// TalkBoxImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void TalkBoxImpl::process( ChannelState & cs, Engine::ChannelData_ReIm2AmPh data, Engine::Setup const & setup ) const
{
    auto const input ( data.input  );
    auto       output( data.output );

    /// \note Use ReIm input to avoid redundant main channel phases calculation
    /// since they are simply discarded.
    ///                                       (26.10.2015.) (Domagoj Saric)
    Math::amplitudes
    (
        input .main().reals().begin(),
        input .main().imags().begin(),
        output.main().amps ().begin(),
        output.main().amps ().end  ()
    );

    bool const fullRange( data.input.numberOfBins() == setup.numberOfBins() );
    if ( BOOST_UNLIKELY( !fullRange ) ) //...mrmlj...
    {
    #if 0 //...mrmlj...treat the working range as a 'band pass' control for talkbox4unity...
        Math::rectangular2polar
        (
            input .full().main().reals ().begin(),
            input .full().main().imags ().begin(),
            output.full().main().amps  ().begin(),
            output.full().main().phases().begin(),
            input.beginBin()
        );
        Math::rectangular2polar
        (
            input .main().reals ().end(),
            input .main().imags ().end(),
            output.main().amps  ().end(),
            output.main().phases().end(),
            output.full().main().phases().end()
        );
    #else
        Math::clear( output.full().main().amps().begin(), output.       main().amps().begin() );
        Math::clear( output.       main().amps().end  (), output.full().main().amps().end  () );
    #endif
    }

    if ( !parameters().get<ExternalCarrier>() ) synth_  .process( cs, data.output, setup );
                                                vocoder_.process(     data.output, setup );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
