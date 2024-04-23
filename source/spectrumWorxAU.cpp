////////////////////////////////////////////////////////////////////////////////
///
/// spectrumWorxAU.cpp
/// ------------------
///
/// Copyright (c) 2013 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "spectrumWorxAU.hpp"

#include "core/host_interop/parameters.hpp"
#include "core/modules/automatedModule.hpp"
#include "core/modules/moduleDSPAndGUI.hpp"
#include "core/spectrumWorxSharedImpl.inl"

#include "le/spectrumworx/effects/baseParameters.hpp"
#include "le/spectrumworx/presets.hpp"
#include "le/parameters/lfo.hpp"

#include "le/plugins/au/tag.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/assert.hpp"

#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

namespace
{
    namespace BaseParameters = Effects::BaseParameters;

    using LE::Parameters::IndexOf;
    using LFO = LE::Parameters::LFOImpl;

    std::uint8_t const startFrequencyIndex( IndexOf<BaseParameters::Parameters, BaseParameters::StartFrequency>::value );
    std::uint8_t const stopFrequencyIndex ( IndexOf<BaseParameters::Parameters, BaseParameters::StopFrequency >::value );

    std::uint8_t const lowerBoundIndex    ( IndexOf<LFO::Parameters           , LFO::LowerBound                 >::value );
    std::uint8_t const upperBoundIndex    ( IndexOf<LFO::Parameters           , LFO::UpperBound                 >::value );
} // anonymous namespace

LE_NOTHROWNOALIAS
void SpectrumWorxAU::getDependentParameters
(
    ParameterID                                                         const parameterID,
    boost::iterator_range<::AUDependentParameter *>                     const dependentParameters,
    SpectrumWorxCore                                const * LE_RESTRICT const pEffect
)
{
    BOOST_ASSERT_MSG
    (
        dependentParameters.size() == numberOfDependentParameters( parameterID, pEffect ),
        "Invalid dependent parameters buffer size."
    );

    ::AUDependentParameter * LE_RESTRICT pDependentParameter( dependentParameters.begin() );

    switch ( parameterID.type() )
    {
        case ParameterID::ModuleChainParameter:
        {
            std::uint8_t const moduleIndex( parameterID.value._.moduleChain.moduleIndex );

            std::uint8_t numberOfModuleParameters;
            if ( pEffect )
            {
                auto const pModule( pEffect->moduleChain().module( moduleIndex ) );
                if ( pModule )
                    numberOfModuleParameters = pModule->numberOfParameters();
                else
                    numberOfModuleParameters = 0;
            }
            else
            {
                numberOfModuleParameters = Constants::maxNumberOfParametersPerModule;
            }

            ParameterID moduleParameterID; moduleParameterID.value.type = ParameterID::ModuleParameter;
            ParameterID lfoParameterID   ; lfoParameterID   .value.type = ParameterID::LFOParameter   ;

            moduleParameterID.value._.module.moduleIndex = moduleIndex;
            lfoParameterID   .value._.lfo   .moduleIndex = moduleIndex;

            for( std::uint8_t moduleParameterIndex( 0 ); moduleParameterIndex < numberOfModuleParameters; ++moduleParameterIndex )
            {
                moduleParameterID.value._.module.moduleParameterIndex = moduleParameterIndex;
                pDependentParameter->mScope       = kAudioUnitScope_Global;
                pDependentParameter->mParameterID = moduleParameterID.binaryValue;
                ++pDependentParameter;

                if ( moduleParameterIndex != 0 /*Bypass*/ )
                {
                    for ( std::uint8_t lfoParameterIndex( 0 ); lfoParameterIndex < ParameterCounts::lfoExportedParameters; ++lfoParameterIndex, ++pDependentParameter )
                    {
                        lfoParameterID.value._.lfo.moduleParameterIndex = moduleParameterIndex - 1;
                        lfoParameterID.value._.lfo.lfoParameterIndex    = lfoParameterIndex;
                        pDependentParameter->mScope       = kAudioUnitScope_Global;
                        pDependentParameter->mParameterID = lfoParameterID.binaryValue;
                    }
                }
            }
        }
        break;

        case ParameterID::ModuleParameter:
        {
            ParameterID moduleParameterID( parameterID );
            switch ( parameterID.value._.module.moduleParameterIndex )
            {
                case startFrequencyIndex: moduleParameterID.value._.module.moduleParameterIndex = stopFrequencyIndex ; break;
                case stopFrequencyIndex : moduleParameterID.value._.module.moduleParameterIndex = startFrequencyIndex; break;
                LE_DEFAULT_CASE_UNREACHABLE();
            }
            BOOST_ASSERT( !pEffect || pEffect->moduleChain().module( parameterID.value._.lfo.moduleIndex ) );
            pDependentParameter->mScope       = kAudioUnitScope_Global;
            pDependentParameter->mParameterID = moduleParameterID.binaryValue;
            ++pDependentParameter;
        }
        break;

        case ParameterID::LFOParameter:
        {
            ParameterID lfoParameterID( parameterID );
            switch ( parameterID.value._.lfo.lfoParameterIndex )
            {
                case lowerBoundIndex: lfoParameterID.value._.lfo.lfoParameterIndex = upperBoundIndex; break;
                case upperBoundIndex: lfoParameterID.value._.lfo.lfoParameterIndex = lowerBoundIndex; break;
                LE_DEFAULT_CASE_UNREACHABLE();
            }
            BOOST_ASSERT( !pEffect || pEffect->moduleChain().module( parameterID.value._.lfo.moduleIndex ) );
            pDependentParameter->mScope       = kAudioUnitScope_Global;
            pDependentParameter->mParameterID = lfoParameterID.binaryValue;
            ++pDependentParameter;
        }
        break;

        case ParameterID::GlobalParameter:
        default:
            LE_UNREACHABLE_CODE();
    }

    BOOST_ASSERT( pDependentParameter <= dependentParameters.end() );
    BOOST_ASSERT( pDependentParameter == dependentParameters.end() );
}

LE_NOTHROWNOALIAS
std::uint16_t SpectrumWorxAU::numberOfDependentParameters( ParameterID const parameterID, SpectrumWorxCore const * LE_RESTRICT const pEffect )
{
    switch ( parameterID.type() )
    {
        case ParameterID::ModuleChainParameter:
            if ( pEffect )
            {
                auto const pModule( pEffect->moduleChain().module( parameterID.value._.moduleChain.moduleIndex ) );
                if ( pModule )
                    return pModule->numberOfParameters() + pModule->numberOfLFOControledParameters() * ParameterCounts::lfoExportedParameters;
                else
                    return 0;
            }
            else
            {
                return Constants::maxNumberOfParametersPerModule + ( Constants::maxNumberOfParametersPerModule - 1 ) * ParameterCounts::lfoExportedParameters;
            }

        case ParameterID::ModuleParameter:
            BOOST_ASSERT
            (
                ( parameterID.value._.module.moduleParameterIndex == startFrequencyIndex ) ||
                ( parameterID.value._.module.moduleParameterIndex == stopFrequencyIndex  )
            );
            return 1;

        case ParameterID::LFOParameter:
            BOOST_ASSERT
            (
                ( parameterID.value._.lfo.lfoParameterIndex == lowerBoundIndex ) ||
                ( parameterID.value._.lfo.lfoParameterIndex == upperBoundIndex )
            );
            if ( pEffect ) return ( pEffect->moduleChain().module( parameterID.value._.lfo.moduleIndex ) ) ? 1 : 0;
            else           return 1;

        case ParameterID::GlobalParameter:
        default:
            LE_UNREACHABLE_CODE();
            return 0;
    }
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

LE_PLUGIN_AU_ENTRY_POINT( LE::SW::SpectrumWorxAU );
