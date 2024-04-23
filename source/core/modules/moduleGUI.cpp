////////////////////////////////////////////////////////////////////////////////
///
/// moduleGUI.cpp
/// -------------
///
/// Copyright (c) 2011 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "moduleGUI.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

LE_NOTHROW LE_COLD
ModuleGUI::~ModuleGUI() {}

LE_NOTHROW float ModuleGUI::getSharedParameter( std::uint_fast8_t const sharedParameterIndex ) const
{
    auto const result( ModuleParameters::getSharedParameter( sharedParameterIndex ) );
    BOOST_ASSERT( result == GUI::ModuleUI::getSharedParameter( sharedParameterIndex ) );
    return result;
}

LE_NOTHROW
void ModuleGUI::setSharedParameter( std::uint_fast8_t const sharedParameterIndex, float const parameterValue )
{
    ModuleParameters::setSharedParameter( sharedParameterIndex, parameterValue                                    );
    GUI::ModuleUI   ::setSharedParameter( sharedParameterIndex, parameterValue, GUI::ModuleUI::AutomationOrPreset );
}


LE_NOTHROW float ModuleGUI::getEffectParameter( std::uint_fast8_t const effectParameterIndex ) const
{
    return GUI::ModuleUI::getEffectParameter( effectParameterIndex );
}

LE_NOTHROW
float ModuleGUI::setEffectParameter( std::uint_fast8_t const effectParameterIndex, float const parameterValue )
{
    GUI::ModuleUI::setEffectParameter( effectParameterIndex, parameterValue, GUI::ModuleUI::AutomationOrPreset );
    return parameterValue; //...mrmlj...no snapping/quantization? reinvestigate...
}


LE_NOTHROW
void ModuleGUI::setSharedParameterFromLFO( std::uint_fast8_t const sharedParameterIndex, LFO::value_type const lfoValue )
{
    //...mrmlj...AutomatedModuleImpl duplication
    auto const parameterValue( ModuleParameters::normalisedToParameterValue( lfoValue, parameterInfos()[ sharedParameterIndex ] ) );
    GUI::ModuleUI::setSharedParameter( sharedParameterIndex, parameterValue, GUI::ModuleUI::LFOValue );
}

LE_NOTHROW
void ModuleGUI::setEffectParameterFromLFO( std::uint_fast8_t const effectParameterIndex, LFO::value_type const lfoValue )
{
    //...mrmlj...AutomatedModuleImpl duplication
    auto const parameterValue( ModuleParameters::normalisedToParameterValue( lfoValue, effectSpecificParameterInfo( effectParameterIndex ) ) );
    GUI::ModuleUI::setEffectParameter( effectParameterIndex, parameterValue, GUI::ModuleUI::LFOValue );
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
