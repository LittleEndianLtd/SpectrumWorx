////////////////////////////////////////////////////////////////////////////////
///
/// automatedModuleImpl.inl
/// -----------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef automatedModuleImpl_inl__E1CBA39F_51D3_4DF9_A85A_6fA4DD3531DC
#define automatedModuleImpl_inl__E1CBA39F_51D3_4DF9_A85A_6fA4DD3531DC
#pragma once
//------------------------------------------------------------------------------
#include "automatedModuleImpl.hpp"

#include "le/parameters/printer.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

template <class Impl>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS
AutomatedModuleImpl<Impl>::getSharedAutomatedParameter( std::uint8_t const parameterIndex, bool const normalised ) const
{
    float const parameterValue( impl().getBaseParameter( parameterIndex ) );
    return Automation::sharedInternal2AutomatedValue( parameterIndex, parameterValue, normalised );
}

template <class Impl>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS
AutomatedModuleImpl<Impl>::getEffectSpecificAutomatedParameter( std::uint8_t const effectSpecificParameterIndex, bool const normalised ) const
{
#ifdef LE_SW_FMOD //...mrmlj...FMOD has "full range" but completely static parameters...
    const_cast<bool &>( normalised ) = true;
#endif // LE_SW_FMOD
    float const parameterValue( impl().getEffectParameter( effectSpecificParameterIndex ) );
    return Automation::effectInternal2AutomatedValue( effectSpecificParameterIndex, parameterValue, normalised, impl() );
}

template <class Impl>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS
AutomatedModuleImpl<Impl>::getAutomatedParameter( std::uint8_t const parameterIndex, bool const normalised ) const
{
    if ( parameterIndex < impl().numberOfBaseParameters ) return this->getSharedAutomatedParameter        (                                      parameterIndex  , normalised );
    else
    if ( parameterIndex < impl().numberOfParameters()   ) return this->getEffectSpecificAutomatedParameter( impl().effectSpecificParameterIndex( parameterIndex ), normalised );
    else                                                  return Plugins::AutomatedParameterValue();
}

template <class Impl>
void LE_NOTHROW
AutomatedModuleImpl<Impl>::setAutomatedParameter( std::uint8_t const parameterIndex, Plugins::AutomatedParameterValue const value, bool const normalised )
{
    //...mrmlj...LE_ASSUME( parameterIndex < SW::Constants::maxNumberOfParametersPerModule );

    if ( parameterIndex >= impl().numberOfParameters() )
        return; // index out of range

    if ( parameterIndex != 0 && impl().lfo( parameterIndex - 1 ).enabled() ) //...mrmlj...skip bypass
        return; // skip automation if the parameter's LFO is enabled

    if ( parameterIndex < impl().numberOfBaseParameters )
    {
        impl().setBaseParameter
        (
            parameterIndex,
            Automation::sharedAutomated2InternalValue( parameterIndex, value, normalised )
        );
        return;
    }

#ifdef LE_SW_FMOD //...mrmlj...FMOD has "full range" but completely static parameters...
    const_cast<bool &>( normalised ) = true;
#endif // LE_SW_FMOD
    std::uint8_t const effectSpecificParameterIndex( impl().effectSpecificParameterIndex( parameterIndex ) );
    impl().setEffectParameter
    (
        effectSpecificParameterIndex,
        Automation::effectAutomated2InternalValue( effectSpecificParameterIndex, value, normalised, impl() )
    );
} // AutomatedModuleImpl<Impl>::setAutomatedParameter()


template <class Impl> LE_NOTHROWNOALIAS
char const * AutomatedModuleImpl<Impl>::getParameterValueString( std::uint8_t const index, LE::Parameters::AutomatedParameterPrinter const & printer ) const
{
    if ( index >= impl().numberOfParameters() )
        return nullptr;

#ifdef LE_SW_FMOD //...mrmlj...FMOD has "full range" but completely static parameters...
    if
    (
        ( printer.valueSource == LE::Parameters::AutomatedParameterPrinter::Linear     ) &&
        ( index               >= Engine::ModuleParameters::BaseParameters::static_size )
    )
        printer.valueSource = LE::Parameters::AutomatedParameterPrinter::NormalisedLinear;
#endif // LE_SW_FMOD

    if ( printer.valueSource == LE::Parameters::AutomatedParameterPrinter::Internal )
    {
        printer.automationValue = 
            ( index < Engine::ModuleParameters::BaseParameters::static_size )
                ? impl().getBaseParameter  (                                      index   )
                : impl().getEffectParameter( impl().effectSpecificParameterIndex( index ) );
        printer.valueSource = LE::Parameters::AutomatedParameterPrinter::Unchanged;
    }

    return Automation::getParameterValueString( index, printer, impl() );
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#endif // automatedModuleImpl_inl
