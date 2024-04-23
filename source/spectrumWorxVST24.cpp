////////////////////////////////////////////////////////////////////////////////
///
/// spectrumWorxVST24.cpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "spectrumWorxVST24.hpp"

#include "core/spectrumWorxSharedImpl.inl"

#include "le/plugins/vst/2.4/plugin.hpp"
#include "le/plugins/vst/2.4/tag.hpp"

#include "boost/assert.hpp"

#include <array>
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4389 ) // Signed/unsigned mismatch.
bool LE_NOTHROW SpectrumWorxVST24::initialise()
{
    bool success( Base::initialise() == Success );
    BOOST_ASSERT( success );
    auto const hostInputs   ( host().getNumInputs () );
    auto const hostOutputs  ( host().getNumOutputs() );
    auto const pluginInputs ( engineSetup().numberOfChannels() + engineSetup().numberOfSideChannels() );
    auto const pluginOutputs( engineSetup().numberOfChannels()                                        );
#if 0 // disabled
    /// \note A VST 2.4 specific workaround for situations where the host does
    /// not allow us to switch from the default maximum number of channels
    /// specified at construction to the actual desired IO mode (the default or
    /// last-session-specified one). As this change is attempted in
    /// Base::initialise() we have to check for success here, afterwards.
    ///                                       (04.10.2013.) (Domagoj Saric)
    if
    (
        ( hostInputs  != pluginInputs  ) ||
        ( hostOutputs != pluginOutputs )
    )
    {
        bool const changeSuccessful( SpectrumWorxCore::setNumberOfChannels( hostInputs, hostOutputs ) );
    #if LE_SW_ENGINE_INPUT_MODE >= 2
        BOOST_ASSERT( hostTryIOConfigurationChange( engineSetup().numberOfChannels(), engineSetup().numberOfSideChannels() ) );
    #endif // #if LE_SW_ENGINE_INPUT_MODE >= 2
        success &= changeSuccessful;
    }
#else // current workaround
    /// \note The above "try to be nice" solution does not play 'user friendly'
    /// (e.g. with Ableton Live 8&9) because the plugin then starts up with the
    /// maximum number of IO channels and eats up a lot of CPU. For this reason
    /// we ignore the host's cry and go with the defaults (or last session
    /// settings) which is at least crash-wise safe (because at worst we will
    /// simply be ignoring the data for the extra channels).
    ///                                       (24.09.2014.) (Domagoj Saric)
    BOOST_ASSERT( hostInputs  == maxNumberOfInputs  || hostInputs  == pluginInputs  );
    BOOST_ASSERT( hostOutputs == maxNumberOfOutputs || hostOutputs == pluginOutputs );
    aEffect().numInputs  = pluginInputs ;
    aEffect().numOutputs = pluginOutputs;
#endif

    return success;
}
#pragma warning( pop )

namespace
{
    LE_MSVC_SPECIFIC( LE_WEAK_SYMBOL_CONST ) std::array<::VstSpeakerArrangementType const, 8> const speakerArrangementTypes = {{ kSpeakerArrMono, kSpeakerArrStereo, kSpeakerArr30Music, kSpeakerArr40Music, kSpeakerArr50, kSpeakerArr51, kSpeakerArr70Music, kSpeakerArr71Music }};
    LE_MSVC_SPECIFIC( LE_WEAK_SYMBOL_CONST ) std::array<::VstSpeakerType            const, 8> const speakerTypes            = {{ kSpeakerL, kSpeakerR, kSpeakerC, kSpeakerLfe, kSpeakerLs, kSpeakerRs, kSpeakerUndefined, kSpeakerUndefined }};

    void fillMultiChannelSpeakerArrangement( ::VstSpeakerArrangement & speakers, std::uint8_t const numberOfChannels )
    {
        speakers.numChannels = numberOfChannels;
        speakers.type        = ( numberOfChannels < speakerArrangementTypes.size() ) ? speakerArrangementTypes[ numberOfChannels - 1 ] : kSpeakerArrUserDefined;

        std::uint8_t speaker( 0 );
        std::uint8_t const definedSpeakerTypes( std::min( numberOfChannels, static_cast<std::uint8_t>( speakerTypes.size() ) ) );
        while ( speaker < definedSpeakerTypes )
        {
            speakers.speakers[ speaker ].type = speakerTypes[ speaker ];
            ++speaker;
        }
        while ( speaker < numberOfChannels )
        {
            speakers.speakers[ speaker ].type = kSpeakerUndefined;
            ++speaker;
        }
    }


    void fillMultiChannelSpeakerArrangements
    (
        ::VstSpeakerArrangement & input ,
        ::VstSpeakerArrangement & output,
        std::uint8_t const numberOfMainChannels,
        std::uint8_t const numberOfSideChannels
    )
    {
        if
        (
            ( numberOfMainChannels == 1 ) &&
            ( numberOfSideChannels == 0 )
        )
        {   // mono
            input.type        = kSpeakerArrMono;
            input.numChannels = 1;
            input.speakers[ 0 ].type = kSpeakerM;
            output.type        = kSpeakerArrMono;
            output.numChannels = 1;
            output.speakers[ 0 ].type = kSpeakerM;
        }
        else
        if
        (
            ( numberOfMainChannels == 1 ) &&
            ( numberOfSideChannels == 1 )
        )
        {   // mono with side chain
            input.type        = kSpeakerArrUserDefined; //kSpeakerArrStereo;
            input.numChannels = 2;
            input.speakers[ 0 ].type = kSpeakerM;
            input.speakers[ 1 ].type = kSpeakerSl;
            output.type        = kSpeakerArrMono;
            output.numChannels = 1;
            output.speakers[ 0 ].type = kSpeakerM;
        }
        else
        {
            fillMultiChannelSpeakerArrangement( input , numberOfMainChannels + numberOfSideChannels );
            fillMultiChannelSpeakerArrangement( output, numberOfMainChannels                        );
        }
    }

    void fillMultiMonoSpeakerArrangement( ::VstSpeakerArrangement & speakers, std::uint8_t const numberOfChannels )
    {
        speakers.numChannels = numberOfChannels;
        speakers.type        = kSpeakerArrUserDefined;

        for ( std::uint8_t speaker( 0 ); speaker < numberOfChannels; ++speaker )
        {
            VstInt32 & speakerType( speakers.speakers[ speaker ].type );
            LE_ASSUME( speakerType == 0 );
            speakerType = kSpeakerM;
        }
    }

    void fillMultiMonoChannelSpeakerArrangements
    (
        ::VstSpeakerArrangement & input ,
        ::VstSpeakerArrangement & output,
        std::uint8_t const numberOfMainChannels,
        std::uint8_t const numberOfSideChannels
    )
    {
        fillMultiMonoSpeakerArrangement( input , numberOfMainChannels + numberOfSideChannels );
        fillMultiMonoSpeakerArrangement( output, numberOfMainChannels                        );
    }
} // anonymous namespace


// http://www.cakewalk.com/DevXchange/Article.aspx?aid=102
bool SpectrumWorxVST24::getSpeakerArrangement( ::VstSpeakerArrangement & input, ::VstSpeakerArrangement & output ) const
{
    auto const numberOfMainChannels( engineSetup().numberOfChannels    () );
    auto const numberOfSideChannels( engineSetup().numberOfSideChannels() );

    BOOST_ASSERT_MSG
    (
        ( ( host().getNumOutputs() ==           numberOfMainChannels                          ) || ( host().getNumOutputs() == maxNumberOfOutputs ) ) &&
        ( ( host().getNumInputs () == unsigned( numberOfMainChannels + numberOfSideChannels ) ) || ( host().getNumInputs () == maxNumberOfInputs  ) ),
        "Performing a latency change notification in the middle of an IO mode change (which cannot be separated in VST 2.4)"
    );

    /// \note SW is still only a "multi mono channel" effect.
    ///                                       (04.10.2013.) (Domagoj Saric)
  //fillMultiChannelSpeakerArrangements    ( input, output, numberOfMainChannels, numberOfSideChannels );
    fillMultiMonoChannelSpeakerArrangements( input, output, numberOfMainChannels, numberOfSideChannels );
    return true;
}


bool SpectrumWorxVST24::setSpeakerArrangement( ::VstSpeakerArrangement const & input, ::VstSpeakerArrangement const & output )
{
    return setNumberOfChannelsFromHost( input.numChannels, output.numChannels );
}

namespace
{
    void LE_FASTCALL fillChannelProperties( ::VstPinProperties & properties, char const * const labelFormatString, std::uint8_t const channelIndex, std::uint8_t const numberOfMainChannels )
    {
        LE_INT_SPRINTFA( properties.label, labelFormatString, channelIndex + 1 );
        std::strcpy( properties.shortLabel, &properties.label[ 3 ] );
        BOOST_ASSERT( std::strlen( properties.shortLabel ) < _countof( properties.shortLabel ) );
        properties.flags = kVstPinIsActive | kVstPinUseSpeaker;
        if ( channelIndex % 2 == 0 )
            properties.flags |= kVstPinIsStereo;
        properties.arrangementType = ( numberOfMainChannels < speakerArrangementTypes.size() ) ? speakerArrangementTypes[ numberOfMainChannels - 1 ] : kSpeakerArrUserDefined;
    }
} // anonymous namespace

void SpectrumWorxVST24::getInputProperties( std::uint8_t const index, ::VstPinProperties & properties )
{
    auto const numberOfMainChannels( engineSetup().numberOfChannels() );
    BOOST_ASSERT( index < unsigned( numberOfMainChannels + engineSetup().numberOfSideChannels() ) );
    BOOST_ASSERT( index < this->host().getNumInputs()                                             );
    if ( index < numberOfMainChannels )
        fillChannelProperties( properties, "SW in %1d"  , static_cast<std::uint8_t>( index )                       , numberOfMainChannels );
    else
        fillChannelProperties( properties, "SW side %1d", static_cast<std::uint8_t>( index ) - numberOfMainChannels, numberOfMainChannels );
}


void SpectrumWorxVST24::getOutputProperties( std::uint8_t const index, ::VstPinProperties & properties )
{
    auto const numberOfMainChannels( engineSetup().numberOfChannels() );
    BOOST_ASSERT( index < numberOfMainChannels         );
    BOOST_ASSERT( index < this->host().getNumOutputs() );
    fillChannelProperties( properties, "SW out %1d", static_cast<std::uint8_t>( index ), numberOfMainChannels );
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

LE_PLUGIN_VST24_ENTRY_POINT( LE::SW::SpectrumWorxVST24 );
