////////////////////////////////////////////////////////////////////////////////
///
/// \file finalImplementations.hpp
///
/// Implementations of different module interfaces for a given effect
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef finalImplementations_hpp__A4E1EEF8_DCF0_4AB1_8A68_6767C135FDC6
#define finalImplementations_hpp__A4E1EEF8_DCF0_4AB1_8A68_6767C135FDC6
#pragma once
//------------------------------------------------------------------------------
#include "le/parameters/boolean/tag.hpp"
#include "le/parameters/enumerated/tag.hpp"
#include "le/parameters/linear/tag.hpp"
#include "le/parameters/symmetric/tag.hpp"
#include "le/parameters/trigger/tag.hpp"
#include "le/parameters/fusionAdaptors.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/engine/moduleImpl.hpp"
#if LE_SW_GUI
#include "le/spectrumworx/engine/moduleParameters.hpp"
#endif // LE_SW_GUI
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

#if LE_SW_GUI
////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleWidgets
///
////////////////////////////////////////////////////////////////////////////////

template <class Effect>
class LE_NOVTABLE ModuleWidgets
{
public: // Module GUI interface implementation.
    LE_NOALIAS void LE_FASTCALL create( GUI::ModuleUI & uiBase )
    {
    #if !LE_SW_SEPARATED_DSP_GUI
        BOOST_ASSERT( !uiBase.module().gui() );
    #endif // LE_SW_SEPARATED_DSP_GUI
        uiBase.setUpForEffect( Effect::title, Effect::description );
        parameterWidgets_.construct( uiBase );
    }

    LE_NOTHROWNOALIAS void LE_FASTCALL destroy()
    {
    #if !LE_SW_SEPARATED_DSP_GUI
        //BOOST_ASSERT( !uiBase.module().gui() );
    #endif // LE_SW_SEPARATED_DSP_GUI
        parameterWidgets_.destroy();
    }

private:
    using Parameters       = typename Effect::Parameters      ;
    using ParameterWidgets = GUI::ParameterWidgets<Parameters>;

private:
    ParameterWidgets parameterWidgets_;
}; // class ModuleWidgets
#endif // LE_SW_GUI


#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI

////////////////////////////////////////////////////////////////////////////////
///
/// \class Module::Impl<>
///
////////////////////////////////////////////////////////////////////////////////

template <class Effect>
class Module::Impl LE_SEALED
    :
    public Engine::ModuleEffectImpl<Effect, Module>,
    public ModuleWidgets           <Effect>
{
public:
    template <typename EffectTypeIndex>
    Impl( EffectTypeIndex ) : Impl::ModuleEffectImpl( EffectTypeIndex(), this ) {}
}; // class Module::Impl

#else // LE_SW_GUI, LE_SW_SEPARATED_DSP_GUI

////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleDSP::Impl<>
///
////////////////////////////////////////////////////////////////////////////////

template <class Effect>
class ModuleDSP::Impl LE_SEALED
    :
    public Engine::ModuleEffectImpl<Effect, ModuleDSP>
{
public:
    template <typename EffectTypeIndex>
    Impl( EffectTypeIndex ) : Impl::ModuleEffectImpl( EffectTypeIndex() ) {}
}; // class ModuleDSP::Impl


#if LE_SW_GUI
////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleGUI::Impl<>
///
////////////////////////////////////////////////////////////////////////////////

template <class Effect>
class ModuleGUI::Impl LE_SEALED
    :
    public ModuleGUI,
    public ModuleWidgets<Effect>
{
public:
    // http://www.artima.com/cppsource/nevercall.html
    template <typename EffectTypeIndex>
                Impl( EffectTypeIndex ) : ModuleGUI( Engine::Detail::MakeEffectMetaData<Effect, EffectTypeIndex>::data, lfos_.begin() ) { ModuleWidgets::create ( *gui() ); }
    LE_NOTHROW ~Impl(                 ) LE_OVERRIDE /*LE_SEALED*/                                                                       { ModuleWidgets::destroy(        ); }

private: //...mrmlj...duplicated from ModuleEffectImpl@moduleImpl.hpp
    using LFOStorage = std::array<ModuleParameters::LFOPlaceholder, ModuleParameters::numberOfLFOBaseParameters + Effect::Parameters::static_size>;
    LFOStorage lfos_;
}; // class ModuleGUI::Impl
#endif // LE_SW_GUI

#endif //  LE_SW_GUI, LE_SW_SEPARATED_DSP_GUI

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // finalImplementations_hpp
