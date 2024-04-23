////////////////////////////////////////////////////////////////////////////////
///
/// \file spectrumWorxAU.hpp
/// ------------------------
///
/// Copyright (c) 2013 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef spectrumWorxAU_hpp__2435136B_354C_4FE8_9C85_54A92E1A0917
#define spectrumWorxAU_hpp__2435136B_354C_4FE8_9C85_54A92E1A0917
#pragma once
//------------------------------------------------------------------------------
#include "spectrumWorx.hpp"
#include "core/spectrumWorxSharedImpl.hpp"

#include "le/plugins/au/plugin.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class SpectrumWorxAU LE_SEALED
    :
    public SpectrumWorxSharedImpl<SpectrumWorxAU, Plugins::Protocol::AU>
{
private:
    typedef Plugins::Protocol::AU Protocol;

    typedef SpectrumWorxSharedImpl<SpectrumWorxAU, Protocol> Base;

    typedef Plugins::Plugin<SpectrumWorxAU, Protocol> PluginPlatform    ;
    typedef PluginPlatform::AutomatedParameter        AutomatedParameter;

public: // Plugin framework interface
    explicit SpectrumWorxAU( PluginPlatform::ConstructionParameter const pluginBaseParam ) : Base( pluginBaseParam ) {}
    using SpectrumWorx::uninitialise;

    static LE_NOTHROWNOALIAS void          LE_FASTCALL getDependentParameters     ( ParameterID, boost::iterator_range<::AUDependentParameter *> ids, SpectrumWorxCore const * );
    static LE_NOTHROWNOALIAS std::uint16_t LE_FASTCALL numberOfDependentParameters( ParameterID,                                                      SpectrumWorxCore const * );
}; // class SpectrumWorxAU;

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // spectrumWorxAU.hpp
