////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleGUI.hpp
///
///    SW plugin module interface and implementation.
///
/// Copyright � 2009 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleGUI_hpp__E42344C7_8515_44B3_9C24_6F88CC5840FA
#define moduleGUI_hpp__E42344C7_8515_44B3_9C24_6F88CC5840FA
#pragma once
//------------------------------------------------------------------------------
#include "core/modules/automatedModuleImpl.hpp"

#include "gui/modules/moduleUI.hpp"

#include "le/spectrumworx/engine/moduleParameters.hpp"
#include "le/utility/cstdint.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Module
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE ModuleGUI
    :
    public GUI::ModuleUI,
    public Engine::ModuleParameters,
    public AutomatedModuleImpl<ModuleGUI>
{
public: // Automation
    template <class AutomatedParameter>
    boost::optional2<std::pair<std::uint_fast8_t, LFO::value_type>> LE_NOTHROW
    setAutomatedLFOParameter( std::uint_fast8_t const parameterIndex, std::uint_fast8_t const lfoParameterIndex, Plugins::AutomatedParameterValue const value )
    {
        auto const result( Automation::setAutomatedLFOParameter<AutomatedParameter>( parameterIndex, lfoParameterIndex, value, *this ) );
        GUI::ModuleUI::updateLFOParameter( parameterIndex, lfoParameterIndex, value );
        return result;
    }

    LE_NOTHROW float LE_FASTCALL getSharedParameter( std::uint_fast8_t sharedParameterIndex                       ) const               ;
    LE_NOTHROW void  LE_FASTCALL setSharedParameter( std::uint_fast8_t sharedParameterIndex, float parameterValue )                     ;

    LE_NOTHROW float LE_FASTCALL getEffectParameter( std::uint_fast8_t effectParameterIndex                       ) const override final;
    LE_NOTHROW float LE_FASTCALL setEffectParameter( std::uint_fast8_t effectParameterIndex, float parameterValue )       override final;

public:
    static ModuleGUI & fromGUI( GUI::ModuleUI & moduleUI ) { LE_ASSUME( &moduleUI ); return static_cast<ModuleGUI &>( moduleUI ); }

    GUI::ModuleUI * gui() { LE_ASSUME( this ); return this; }

public:
    template <class Effect> class Impl;

protected:
    template <typename ... T>
    ModuleGUI( T && ... args ) : Engine::ModuleParameters( std::forward<T>( args )... ) {}
    ~ModuleGUI();

private: friend class AutomatedModuleImpl<ModuleGUI>;
    void LE_FASTCALL setSharedParameterFromLFO( std::uint_fast8_t parameterIndex, LFO::value_type );
    void LE_FASTCALL setEffectParameterFromLFO( std::uint_fast8_t parameterIndex, LFO::value_type );
}; // class ModuleGUI

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleGUI_hpp
