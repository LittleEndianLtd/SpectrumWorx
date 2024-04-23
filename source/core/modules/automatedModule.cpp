////////////////////////////////////////////////////////////////////////////////
///
/// automatedModule.cpp
/// -------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "automatedModule.hpp"

#include "core/host_interop/parameters.hpp"

#include "le/parameters/conversion.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/parameters/printer.hpp" // BaseParameters printers
#include "le/plugins/plugin.hpp" //...ugh...mrmlj...for Plugins::*AutomatedParameter usage in printer.hpp...clean this up...
#include "le/spectrumworx/effects/baseParametersUIElements.hpp" // BaseParameters printers
#include "le/spectrumworx/engine/moduleParameters.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Automation
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

using BaseParameters = Effects::BaseParameters::Parameters;

LE_NOTHROWNOALIAS
char const * getParameterValueString( std::uint8_t const index, ParameterPrinter const & printer, ModuleParameters const & module )
{
    if ( index >= module.numberOfParameters() )
    {
        BOOST_ASSERT( printer.printer.buffer[ 0 ] == 0 );
        return nullptr;
    }

    return
        ( index < BaseParameters::static_size )
            ? LE::Parameters::invokeFunctorOnIndexedParameter<BaseParameters>( index, std::forward<ParameterPrinter const>( printer ) )
            : module.metaData().getParameterValueString( module.effectSpecificParameterIndex( index ), printer );
}


LE_NOTHROWNOALIAS
char const * getParameterUnit( std::uint8_t const parameterIndex, ModuleParameters const * LE_RESTRICT const pModule )
{
    if ( parameterIndex < BaseParameters::static_size )
    {
        return ModuleParameters::parameterInfos()[ parameterIndex ].unit;
    }
    else
    {
        if ( pModule && parameterIndex < pModule->numberOfParameters() )
            return pModule->effectSpecificParameterInfo( pModule->effectSpecificParameterIndex( parameterIndex ) ).unit;
    }
    return nullptr;
}


Plugins::AutomatedParameterValue internal2AutomatedValue( std::uint8_t const parameterIndex, float const internalValue, bool const normalised, ModuleParameters const & module )
{
    BOOST_ASSERT( parameterIndex < module.numberOfParameters() );
    if ( !normalised )
        return internalValue;
    if ( parameterIndex < module.numberOfBaseParameters )
        return sharedInternal2AutomatedValue( parameterIndex, internalValue, true );
    return effectInternal2AutomatedValue( module.effectSpecificParameterIndex( parameterIndex ), internalValue, true, module );
}
float                            automated2InternalValue( std::uint8_t const parameterIndex, Plugins::AutomatedParameterValue const automatedValue, bool const normalised, ModuleParameters const & module )
{
    BOOST_ASSERT( parameterIndex < module.numberOfParameters() );
    if ( !normalised )
        return automatedValue;
    if ( parameterIndex < module.numberOfBaseParameters )
        return sharedAutomated2InternalValue( parameterIndex, automatedValue, true );
    return effectAutomated2InternalValue( module.effectSpecificParameterIndex( parameterIndex ), automatedValue, true, module );
}
Plugins::AutomatedParameterValue sharedInternal2AutomatedValue( std::uint8_t const sharedParameterIndex, float const internalValue, bool const normalised )
{
    if ( !normalised )
        return internalValue;
    return ModuleParameters::parameterToNormalisedValue( internalValue, ModuleParameters::parameterInfos()[ sharedParameterIndex ] );
}
float                            sharedAutomated2InternalValue( std::uint8_t const sharedParameterIndex, Plugins::AutomatedParameterValue const automatedValue, bool const normalised )
{
    if ( !normalised )
        return automatedValue;
    return ModuleParameters::normalisedToParameterValue( automatedValue, ModuleParameters::parameterInfos()[ sharedParameterIndex ] );
}
Plugins::AutomatedParameterValue effectInternal2AutomatedValue( std::uint8_t const effectParameterIndex, float const internalValue, bool const normalised, ModuleParameters const & module )
{
    if ( !normalised )
        return internalValue;
    return module.parameterToNormalisedValue( internalValue, module.effectSpecificParameterInfo( effectParameterIndex ) );
}
float                            effectAutomated2InternalValue( std::uint8_t const effectParameterIndex, Plugins::AutomatedParameterValue const automatedValue, bool const normalised, ModuleParameters const & module )
{
    if ( !normalised )
        return automatedValue;
    return module.normalisedToParameterValue( automatedValue, module.effectSpecificParameterInfo( effectParameterIndex ) );
}

boost::optional<AutoAdjustedLFOParameter> LE_COLD LE_FASTCALL
Detail::autoAdjustedLFOParameter( LFO & lfo, std::uint8_t const lfoParameterIndex )
{
    //LFO::value_type const * __restrict pSourceBound;
    //LFO::value_type       * __restrict pTargetBound;
    using LE::Parameters::IndexOf;
    auto const lowerBoundIndex( IndexOf<LFO::Parameters, LFO::LowerBound>::value );
    auto const upperBoundIndex( IndexOf<LFO::Parameters, LFO::UpperBound>::value );
    LE_ASSUME( lfoParameterIndex < ParameterCounts::lfoExportedParameters );
    switch ( lfoParameterIndex )
    {
        case lowerBoundIndex:
            if ( lfo.upperBound() < lfo.lowerBound() )
            {
                lfo.setUpperBound( lfo.lowerBound() );
                return AutoAdjustedLFOParameter( lowerBoundIndex, lfo.upperBound() );
            }
            break;

        case upperBoundIndex:
            if ( lfo.lowerBound() > lfo.upperBound() )
            {
                lfo.setLowerBound( lfo.upperBound() );
                return AutoAdjustedLFOParameter( upperBoundIndex, lfo.lowerBound() );
            }
            break;

        default:
            break;
    }
    return boost::none;
}

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Automation
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
