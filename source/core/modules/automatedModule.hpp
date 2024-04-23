////////////////////////////////////////////////////////////////////////////////
///
/// \file automatedModule.hpp
/// --------------------------
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef automatedModule_hpp__59B44881_2002_403A_914B_C603B43F9D00
#define automatedModule_hpp__59B44881_2002_403A_914B_C603B43F9D00
#pragma once
//------------------------------------------------------------------------------
#include "configuration/constants.hpp"

#include "le/parameters/conversion.hpp"
#include "le/parameters/lfoImpl.hpp"
#include "le/parameters/printer_fwd.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/plugins/plugin.hpp" // for Plugins::AutomatedParameterValue
#include "le/spectrumworx/engine/moduleParameters.hpp" // Clang
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/optional/optional.hpp" // Boost sandbox

#include "boost/assert.hpp"

#include <utility>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters { struct RuntimeInformation; }
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
using ParameterInfo = Parameters::RuntimeInformation;
namespace Engine { class ModuleParameters; }
//------------------------------------------------------------------------------
namespace Automation
{
//------------------------------------------------------------------------------

using LFO                      = Parameters::LFOImpl;
using ParameterPrinter         = Parameters::AutomatedParameterPrinter;
using ModuleParameters         = Engine::ModuleParameters;
using AutoAdjustedLFOParameter = std::pair<std::uint8_t, LFO::value_type>;

template <class AutomatedParameter>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS LE_FASTCALL
getAutomatedLFOParameter( std::uint8_t parameterIndex, std::uint8_t lfoParameterIndex, ModuleParameters const & );

template <class AutomatedParameter>
boost::optional<AutoAdjustedLFOParameter> LE_NOTHROW LE_FASTCALL
setAutomatedLFOParameter( std::uint8_t parameterIndex, std::uint8_t lfoParameterIndex, Plugins::AutomatedParameterValue, ModuleParameters & );

template <class AutomatedParameter>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS LE_FASTCALL
getDefaultAutomatedLFOParameter( std::uint8_t lfoParameterIndex );

Plugins::AutomatedParameterValue       internal2AutomatedValue( std::uint8_t       parameterIndex, float                             internalValue, bool normalised, ModuleParameters const & );
float                                  automated2InternalValue( std::uint8_t       parameterIndex, Plugins::AutomatedParameterValue automatedValue, bool normalised, ModuleParameters const & );

Plugins::AutomatedParameterValue sharedInternal2AutomatedValue( std::uint8_t sharedParameterIndex, float                             internalValue, bool normalised                           );
float                            sharedAutomated2InternalValue( std::uint8_t sharedParameterIndex, Plugins::AutomatedParameterValue automatedValue, bool normalised                           );

Plugins::AutomatedParameterValue effectInternal2AutomatedValue( std::uint8_t effectParameterIndex, float                             internalValue, bool normalised, ModuleParameters const & );
float                            effectAutomated2InternalValue( std::uint8_t effectParameterIndex, Plugins::AutomatedParameterValue automatedValue, bool normalised, ModuleParameters const & );

LE_NOTHROWNOALIAS char const * getParameterValueString( std::uint8_t parameterIndex, ParameterPrinter const &, ModuleParameters const & );
LE_NOTHROWNOALIAS char const * getParameterUnit       ( std::uint8_t parameterIndex                          , ModuleParameters const * );

namespace Detail
{
    boost::optional<AutoAdjustedLFOParameter> LE_FASTCALL
    autoAdjustedLFOParameter( LFO & lfo, std::uint8_t lfoParameterIndex );

    using value_type = Plugins::AutomatedParameterValue;

    ////////////////////////////////////////////////////////////////////////
    /// \internal
    /// \class LFOParameterSetter
    /// \brief Delinearizes the PeriodScale LFO parameter
    ////////////////////////////////////////////////////////////////////////

    template <class AutomatedParameter>
    class LFOParameterSetter : public AutomatedParameter::Setter
    {
    public:
        using result_type = typename AutomatedParameter::Setter::result_type;
        using LFO         = LE::Parameters::LFOImpl;

        LFOParameterSetter( value_type const value ) : AutomatedParameter::Setter{ value } {}
        using AutomatedParameter::Setter::operator();

        void operator()( LFO::PeriodScale & parameter ) const
        {
            auto sourceValue( this->automationValue );
            if ( AutomatedParameter::normalised )
                    sourceValue = LFO::unlinearisePeriodScale( sourceValue );
            parameter = AutomatedParameter:: template convertAutomationToParameterValue<LFO::PeriodScale>( sourceValue );
            LFO::snapPeriodScaleFromAutomation( parameter );
        }
    }; // class LFOParameterSetter


    ////////////////////////////////////////////////////////////////////////
    /// \internal
    /// \class LFOParameterGetter
    /// \brief Linearizes the PeriodScale LFO parameter
    ////////////////////////////////////////////////////////////////////////

    template <class AutomatedParameter>
    class LFOParameterGetter : public AutomatedParameter::Getter
    {
    public:
        typedef typename AutomatedParameter::Getter::result_type result_type;

        using LFO = LE::Parameters::LFOImpl;

        using AutomatedParameter::Getter::operator();

        result_type operator()( LFO::PeriodScale const & parameter ) const
        {
            result_type result( AutomatedParameter:: template convertParameterToAutomationValue<LFO::PeriodScale>( parameter ) );
            if ( AutomatedParameter::normalised )
                result = LFO::linearisePeriodScale( result );
            return result;
        }
    }; // class LFOParameterGetter
} // namespace Detail


template <class AutomatedParameter>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS LE_FASTCALL
getAutomatedLFOParameter( std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, ModuleParameters const & module )
{
    //...mrmlj...LE_ASSUME( parameterIndex < ( Constants::maxNumberOfParametersPerModule - 1 /*Bypass*/ ) );
    if ( BOOST_UNLIKELY( parameterIndex >= module.numberOfLFOControledParameters() ) )
        return getDefaultAutomatedLFOParameter<AutomatedParameter>( lfoParameterIndex );

    return LE::Parameters::invokeFunctorOnIndexedParameter
    (
        module.lfo( parameterIndex ).parameters(),
        lfoParameterIndex,
        Detail::LFOParameterGetter<AutomatedParameter>()
    );
}

template <class AutomatedParameter>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS LE_FASTCALL
getDefaultAutomatedLFOParameter( std::uint8_t const lfoParameterIndex )
{
    if ( AutomatedParameter::normalised )
        return typename AutomatedParameter::value_type();
    /// \note Return the default value for nonexistent parameters (as 0
    /// might not be an allowed value for the given parameter.
    ///                                   (26.03.2014.) (Domagoj Saric)
    return LE::Parameters::invokeFunctorOnIndexedParameter<LFO::Parameters const>
    (
        LFO::Parameters(),
        lfoParameterIndex,
        Detail::LFOParameterGetter<AutomatedParameter>()
    );
}


template <class AutomatedParameter>
boost::optional<AutoAdjustedLFOParameter> LE_NOTHROW  LE_FASTCALL
setAutomatedLFOParameter( std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, Plugins::AutomatedParameterValue const value, ModuleParameters & module )
{
    LE_ASSUME( parameterIndex < ( Constants::maxNumberOfParametersPerModule - 1 /*Bypass*/ ) );

    auto & lfo( module.lfo( parameterIndex ) );

    LE::Parameters::invokeFunctorOnIndexedParameter
    (
        lfo.parameters(),
        lfoParameterIndex,
        Detail::LFOParameterSetter<AutomatedParameter>{ value }
    );

    // Implementation note:
    //   We have to manually check whether the user/host broke the
    // UpperBound >= LowerBound requirement and fix it accordingly. If and only
    // if (to minimize host calls) a fixup is made this is reported to the
    // calling function.
    //                                        (04.07.2011.) (Domagoj Saric)
    return Detail::autoAdjustedLFOParameter( lfo, lfoParameterIndex );
}

//------------------------------------------------------------------------------
} // namespace Automation
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // automatedModule_hpp
