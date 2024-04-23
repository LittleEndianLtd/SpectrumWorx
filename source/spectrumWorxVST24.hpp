////////////////////////////////////////////////////////////////////////////////
///
/// \file spectrumWorxVST24.hpp
/// ---------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef spectrumWorxVST24_hpp__92FDD21B_5D70_4DBB_8CE3_63498D30A98D
#define spectrumWorxVST24_hpp__92FDD21B_5D70_4DBB_8CE3_63498D30A98D
#pragma once
//------------------------------------------------------------------------------
#include "spectrumWorx.hpp"
#include "core/spectrumWorxSharedImpl.hpp"

#include "le/plugins/vst/2.4/plugin.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class SpectrumWorxVST24 LE_SEALED
    :
    public SpectrumWorxSharedImpl<SpectrumWorxVST24, Plugins::Protocol::VST24>
{
private:
    typedef Plugins::Protocol::VST24 Protocol;

    typedef SpectrumWorxSharedImpl<SpectrumWorxVST24, Protocol> Base;

    typedef Plugins::Plugin<SpectrumWorxVST24, Protocol> PluginPlatform    ;
    typedef PluginPlatform::AutomatedParameter           AutomatedParameter;

public: // Plugin framework interface
    LE_NOTHROW SpectrumWorxVST24( PluginPlatform::ConstructionParameter const pluginBaseParam ) : Base( pluginBaseParam ) {}

    bool LE_NOTHROW initialise();

    bool setSpeakerArrangement( ::VstSpeakerArrangement const & input, ::VstSpeakerArrangement const & output )      ;
    bool getSpeakerArrangement( ::VstSpeakerArrangement       & input, ::VstSpeakerArrangement       & output ) const;

	void getInputProperties ( std::uint8_t index, ::VstPinProperties & properties );
	void getOutputProperties( std::uint8_t index, ::VstPinProperties & properties );

    using SpectrumWorx::canParameterBeAutomated;
}; // class SpectrumWorxVST24;

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // spectrumWorxVST24.hpp
