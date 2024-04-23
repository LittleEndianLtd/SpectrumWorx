////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleDSPAndGUI.hpp
///
///    SW plugin module interface and implementation.
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleDSPAndGUI_hpp__C1E04A83_2733_44D6_A484_9F03A08AD6CD
#define moduleDSPAndGUI_hpp__C1E04A83_2733_44D6_A484_9F03A08AD6CD
#pragma once
//------------------------------------------------------------------------------
#include "automatedModuleImpl.hpp"

#include "gui/modules/moduleUI.hpp"

#include "le/spectrumworx/engine/module.hpp"
#include "le/utility/cstdint.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

namespace GUI { class SpectrumWorxEditor; }


////////////////////////////////////////////////////////////////////////////////
///
/// \class Module
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE Module
    :
    public  Engine::ModuleDSP,
    public  AutomatedModuleImpl<Module>,
    private GUI::ParameterWidgetsVTable<Module>
{
public: // GUI
    LE_NOTHROW void LE_FASTCALL createGUI ( GUI::SpectrumWorxEditor &, std::uint8_t moduleIndex );
    LE_NOTHROW bool LE_FASTCALL destroyGUI(                                                     );

public: // Automation
    template <class AutomatedParameter>
    boost::optional<std::pair<std::uint8_t, LFO::value_type>> LE_NOTHROW LE_FASTCALL
    setAutomatedLFOParameter( std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, Plugins::AutomatedParameterValue const value )
    {
        auto const result( Automation::setAutomatedLFOParameter<AutomatedParameter>( parameterIndex, lfoParameterIndex, value, *this ) );
        updateLFOGUI( parameterIndex, lfoParameterIndex, value );
        return result;
    }

public:
    using OptionalUI = boost::optional<GUI::ModuleUI>;

    OptionalUI       & gui()       { return ui_; }
    OptionalUI const & gui() const { return ui_; }

    float LE_FASTCALL setParameterValueFromUI( std::uint8_t parameterIndex, float value );

    static Module & fromGUI( GUI::ModuleUI & );

public:
    template <class Effect> class Impl;

protected:
    template <class Effect, typename ... T>
    Module( Impl<Effect> * const pImpl, T && ... args )
        :
        ModuleDSP( std::forward<T>( args )... ),
        GUI::ParameterWidgetsVTable<Module>( *pImpl )
    {}

public: //...mrmlj...(delete pModule)...
    ~Module();

private: friend class AutomatedModuleImpl<Module>;
    float LE_FASTCALL setBaseParameter  ( std::uint8_t sharedParameterIndex, float parameterValue ) override final;
    float LE_FASTCALL setEffectParameter( std::uint8_t effectParameterIndex, float parameterValue ) override final;

private:
    void LE_FASTCALL setBaseParameterFromLFO  ( std::uint8_t parameterIndex, LFO::value_type ) override final;
    void LE_FASTCALL setEffectParameterFromLFO( std::uint8_t parameterIndex, LFO::value_type ) override final;

    void LE_FASTCALL updateLFOGUI( std::uint8_t parameterIndex, std::uint8_t lfoParameterIndex, Plugins::AutomatedParameterValue );

private:
    OptionalUI ui_;
}; // class Module

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleDSPAndGUI_hpp
