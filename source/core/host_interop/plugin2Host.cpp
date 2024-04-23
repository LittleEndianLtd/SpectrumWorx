////////////////////////////////////////////////////////////////////////////////
///
/// plugin2Host.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin2Host.hpp"

#include "core/automatedModuleChain.hpp" // ModuleChainParameter
#include "core/modules/automatedModule.hpp"

#include "le/spectrumworx/effects/configuration/effectNames.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"
//------------------------------------------------------------------------------
namespace LE
{

template <typename Char>
LE_NOTHROWNOALIAS char * LE_FASTCALL copyToBuffer( Char const * string, boost::iterator_range<char *> const & buffer );

namespace Parameters
{
    namespace Detail
    {
        namespace Engine = SW::Engine;

        template <>
        char const * print<Engine::OverlapFactor>
        (
            unsigned int          const parameterValue,
            Engine::Setup const &       engineSetup   ,
            PrintBuffer   const &       buffer        ,
            PowerOfTwoParameterTag
        )
        {
            typedef DisplayValueTransformer<Engine::OverlapFactor> ValueTransformer;
            Utility::lexical_cast
            (
                ValueTransformer::transform( parameterValue, engineSetup ),
                1,
                buffer.begin()
            );
            return buffer.begin();
        }

        template <class Parameter>
        char const * print
        (
            float         const & parameterValue,
            Engine::Setup const &               ,
            PrintBuffer   const &               ,
            PowerOfTwoParameterTag
        );
        template <>
        char const * print<Engine::OverlapFactor>
        (
            float         const & parameterValue,
            Engine::Setup const & engineSetup   ,
            PrintBuffer   const & buffer        ,
            PowerOfTwoParameterTag
        )
        {
            return print<Engine::OverlapFactor>( Math::convert<unsigned int>( parameterValue ), engineSetup, buffer, PowerOfTwoParameterTag() );
        }
    } // namespace Detail
} // namespace Parameters

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

using LFO = Parameters::LFOImpl;

#if LE_SW_GUI
LE_NOTHROW
void Plugin2HostInteropControler::moduleChangedByUser
(
    std::uint8_t                     const chainParameterIndex,
    Module       const * LE_RESTRICT const pModule
) const
{
    BOOST_ASSERT( GUI::isThisTheGUIThread() );

    if ( parameterListChanged() )
        return;

    ParameterID parameterID;
    parameterID.value              .type        = ParameterID::ModuleChainParameter;
    parameterID.value._.moduleChain.moduleIndex = chainParameterIndex;
    ModuleChainParameter const parameter( pModule ? pModule->effectTypeIndex() : noModule );
    ParameterValueForAutomation const automationValue =
    {
        Plugins::FullRangeAutomatedParameter ::convertParameterToAutomationValue( parameter ),
        Plugins::NormalisedAutomatedParameter::convertParameterToAutomationValue( parameter )
    };
    automatedParameterChanged( parameterID, automationValue );
    moduleChanged            ( chainParameterIndex, pModule );
}
#endif // LE_SW_GUI

LE_NOTHROW
void Plugin2HostInteropControler::automatedParameterChanged
(
    Module       const &       module,
    std::uint8_t         const moduleIndex,
    std::uint8_t         const moduleParameterIndex,
    float                const parameterValue
) const
{
    //BOOST_ASSERT( moduleParameterID.moduleParameterIndex < Constants::maxNumberOfParametersPerModule ); //...mrmlj...TuneWorx...
    /// \todo Consider a smarter place to put this check (currently needed only
    /// for TuneWorx).
    ///                                       (18.01.2012.) (Domagoj Saric)
    if ( moduleParameterIndex >= SW::Constants::maxNumberOfParametersPerModule )
        return;
    ParameterID parameterID;
    parameterID.value.type                          = ParameterID::ModuleParameter;
    parameterID.value._.module.moduleIndex          = moduleIndex;
    parameterID.value._.module.moduleParameterIndex = moduleParameterIndex;
    Plugin2HostInteropControler::ParameterValueForAutomation const automationValue =
    {
        Automation::internal2AutomatedValue( moduleParameterIndex, parameterValue, false, module ),
        Automation::internal2AutomatedValue( moduleParameterIndex, parameterValue, true , module )
    };
    automatedParameterChanged( parameterID, automationValue );
}


LE_NOTHROW
void Plugin2HostInteropControler::automatedParameterChanged( ParameterID::LFO const lfoParameterID, float const value ) const
{
    using namespace SW::Constants;
    using namespace ParameterCounts;
    LE_ASSUME( lfoParameterID.moduleIndex          < maxNumberOfModules                     );
    LE_ASSUME( lfoParameterID.moduleParameterIndex < ( maxNumberOfParametersPerModule - 1 ) );
    LE_ASSUME( lfoParameterID.lfoParameterIndex    < lfoExportedParameters                  );

    ParameterID parameterID;
    parameterID.value.type  = ParameterID::LFOParameter;
    parameterID.value._.lfo = lfoParameterID;
    ParameterValueForAutomation const automationValue =
    {
        value,
        LFO::internal2AutomatedValue( lfoParameterID.lfoParameterIndex, value, true )
    };
    automatedParameterChanged( parameterID, automationValue );
}


LE_NOTHROW
void LE_FASTCALL Plugin2HostInteropControler::globalParameterChanged
(
    std::uint8_t                            const index,
    ParameterValueForAutomation::value_type const fullRange,
    ParameterValueForAutomation::value_type const normalised,
    bool const asDiscreteGesture //....mrmlj...ugh cleanup....for distinction between knobs and comboboxes
)
{
    ParameterID parameterID;
    parameterID.value         .type  = ParameterID::GlobalParameter;
    parameterID.value._.global.index = index;
    ParameterValueForAutomation const automationValue = { fullRange, normalised };
    if ( asDiscreteGesture ) automatedParameterBeginEdit( parameterID                  );
                             automatedParameterChanged  ( parameterID, automationValue );
    if ( asDiscreteGesture ) automatedParameterEndEdit  ( parameterID                  );
}

#if LE_SW_GUI
LE_NOTHROW
void Plugin2HostInteropControler::modulesChanged
(
    AutomatedModuleChain const & chain,
    std::uint8_t       /*const*/ firstModuleIndex,
    std::uint8_t       /*const*/ lastModuleIndex
) const
{
    //...mrmlj...BOOST_ASSERT( firstModuleIndex <= lastModuleIndex );

    if ( parameterListChanged() )
        return;

    //...mrmlj...
    if ( firstModuleIndex > lastModuleIndex )
        std::swap( firstModuleIndex, lastModuleIndex );

    std::uint8_t       moduleIndex   ( firstModuleIndex    );
    std::uint8_t const moduleIndexEnd( lastModuleIndex + 1 );
    auto moduleIter( chain.Engine::ModuleChainImpl::module( moduleIndex ) ); //...mrmlj...
    while ( moduleIndex != moduleIndexEnd )
    {
        BOOST_ASSERT_MSG( moduleIter->referenceCount_ >= 3, "Module chain altered while notifying host about current state." );
        Module const * pModule( nullptr );
        if ( !chain.isEnd( moduleIter ) )
        {
            pModule = &Engine::actualModule<Module const>( *moduleIter );
            ++moduleIter;
        }
        moduleChangedByUser( moduleIndex++, pModule );
    }
}
#endif // LE_SW_GUI

////////////////////////////////////////////////////////////////////////////////
//
// Parameters
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin2HostPassiveInteropController::ParameterNameGetter
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
struct Plugin2HostPassiveInteropController::ParameterNameGetter
{
    using result_type = void;

    result_type operator()( ParameterID::Global     , Program const * ) const;
    result_type operator()( ParameterID::ModuleChain, Program const * ) const;
    result_type operator()( ParameterID::Module     , Program const * ) const;
    result_type operator()( ParameterID::LFO        , Program const * ) const;

    typedef boost::iterator_range<char *> Buffer;
    Buffer const buffer_;
}; // struct Plugin2HostPassiveInteropController
#pragma warning( pop )

LE_NOTHROWNOALIAS
void Plugin2HostPassiveInteropController::getParameterLabel( ParameterID const parameterID, boost::iterator_range<char *> const label, Program const * LE_RESTRICT const pProgram )
{
    BOOST_ASSERT( label[ 0 ] == 0 );
    char const * const pUnit( invokeFunctorOnIdentifiedParameter( parameterID, ParameterLabelGetter(), pProgram ) );
    if ( pUnit )
    {
        BOOST_ASSERT( std::strlen( pUnit ) < unsigned( label.size() - 1 ) );
        std::strcpy( label.begin(), pUnit );
    }
}


#if 0
LE_NOTHROWNOALIAS
void Plugin2HostPassiveInteropController::getParameterDisplay( ParameterID const parameterID, boost::iterator_range<char *> const text, Engine::Setup const & engineSetup, Plugins::AutomatedParameterValue const * LE_RESTRICT const pValue, Program const & program )
{
#ifdef _WIN32
    LE_ASSUME( pValue == nullptr );
#endif // _WIN32
    ParameterValueStringGetter const getter = {{ engineSetup, pValue, text }};
    char const * const pValueString( invokeFunctorOnIdentifiedParameter( parameterID, std::forward<ParameterValueStringGetter const>( getter ), &program ) );
    copyToBuffer( pValueString, text );
}
#endif

LE_NOTHROWNOALIAS
void Plugin2HostPassiveInteropController::getParameterName( ParameterID const parameterID, boost::iterator_range<char *> const name, Program const * LE_RESTRICT const pProgram )
{
    ParameterNameGetter const getter = { name };
    invokeFunctorOnIdentifiedParameter( parameterID, std::forward<ParameterNameGetter const>( getter ), pProgram );
}

LE_NOTHROWNOALIAS
void Plugin2HostPassiveInteropController::getParameterIDs( boost::iterator_range<Plugins::ParameterID *> const ids, Program const * LE_RESTRICT const pProgram )
{
    //...mrmlj...parameter IDs used only by AU which doesn't use InputMode...
    BOOST_ASSERT_MSG
    (
        ids.size() >= Program::Parameters::static_size - 1 /*InputMode*/ + Constants::maxNumberOfModules,
        "ParameterIDs buffer too small."
    );

    Plugins::ParameterID * LE_RESTRICT pParameterID( ids.begin() );

    ParameterID parameterID;

    using index_t = std::uint8_t;
    using Module  = Plugin2HostInteropControler::Module;

    /// \note AU does not support plugin initiated IO channel configuration
    /// changes so we do not 'export' the InputMode parameter.
    ///                                       (18.03.2013.) (Domagoj Saric)
#if LE_SW_ENGINE_INPUT_MODE >= 2
    index_t const inputModeIndex( LE::Parameters::IndexOf<Program::Parameters, GlobalParameters::InputMode>::value );
#else
    index_t const inputModeIndex( Program::Parameters::static_size                                                 ); //...mrmlj...
#endif // LE_SW_ENGINE_INPUT_MODE >= 2

    parameterID.binaryValue = 0;
    parameterID.value.type  = ParameterID::GlobalParameter;
    for ( index_t globalParameterIndex( 0 ); globalParameterIndex < inputModeIndex; ++globalParameterIndex, ++pParameterID )
    {
        parameterID  .value._.global.index = globalParameterIndex;
        pParameterID->value                = parameterID.binaryValue;
        ParameterID( *pParameterID ).verify();
    }
    for ( index_t globalParameterIndex( inputModeIndex + 1 ); globalParameterIndex < Program::Parameters::static_size; ++globalParameterIndex, ++pParameterID )
    {
        parameterID  .value._.global.index = globalParameterIndex - 1;
        pParameterID->value                = parameterID.binaryValue;
        ParameterID( *pParameterID ).verify();
    }

    parameterID.binaryValue = 0;
    parameterID.value.type  = ParameterID::ModuleChainParameter;
    for ( index_t moduleChainParameterIndex( 0 ); moduleChainParameterIndex < Constants::maxNumberOfModules; ++moduleChainParameterIndex, ++pParameterID )
    {
        parameterID  .value._.moduleChain.moduleIndex = moduleChainParameterIndex;
        pParameterID->value                           = parameterID.binaryValue;
        ParameterID( *pParameterID ).verify();
    }


    parameterID.binaryValue = 0;
    parameterID.value.type  = ParameterID::ModuleParameter;
    ParameterID lfoParameterID;
    lfoParameterID.value.type = ParameterID::LFOParameter;

    AutomatedModuleChain::const_iterator       pModule        ( pProgram ? pProgram->moduleChain().begin() : nullptr                       );
    index_t                              const numberOfModules( pProgram ? pProgram->moduleChain().size () : Constants::maxNumberOfModules );
    for ( index_t moduleIndex( 0 ); moduleIndex < numberOfModules; ++moduleIndex )
    {
        lfoParameterID.value._.lfo   .moduleIndex = moduleIndex;
        parameterID   .value._.module.moduleIndex = moduleIndex;

        index_t const numberOfModuleParameters( pProgram ? Engine::actualModule<Module>( *pModule++ ).numberOfParameters() : Constants::maxNumberOfParametersPerModule );

        for ( index_t moduleParameterIndex( 0 ); moduleParameterIndex < numberOfModuleParameters; ++moduleParameterIndex )
        {
            parameterID.value._.module.moduleParameterIndex = moduleParameterIndex;
            pParameterID->value = parameterID.binaryValue;
            ParameterID( *pParameterID ).verify();
            ++pParameterID;

            if ( moduleParameterIndex != 0 /*Bypass*/ )
            {
                using ParameterCounts::lfoExportedParameters;
                for ( index_t lfoParameterIndex( 0 ); lfoParameterIndex < lfoExportedParameters; ++lfoParameterIndex, ++pParameterID )
                {
                    lfoParameterID.value._.lfo.moduleParameterIndex = moduleParameterIndex - 1;
                    lfoParameterID.value._.lfo.lfoParameterIndex    = lfoParameterIndex;
                    pParameterID->value                             = lfoParameterID.binaryValue;
                    ParameterID( *pParameterID ).verify();
                }
            }
        }
    }

    BOOST_ASSERT( pParameterID <= ids.end() );
    BOOST_ASSERT( pParameterID == ids.end() );
}

LE_NOTHROWNOALIAS
std::uint16_t Plugin2HostPassiveInteropController::numberOfParameters( Program const * LE_RESTRICT const pProgram )
{
    //...mrmlj...verify that this is used only by AU/"non-InputMode-aware" code...
    std::uint16_t numberOfParameters( Program::Parameters::static_size - 1 /*InputMode*/ + Constants::maxNumberOfModules );

    if ( pProgram )
    {
        pProgram->moduleChain().forEach<Engine::ModuleParameters>
        (
            [&numberOfParameters]( Engine::ModuleParameters const & module )
            {
                std::uint8_t const numberOfModuleParameters   ( module.numberOfParameters() );
                std::uint8_t const numberOfModuleLFOParameters( ( numberOfModuleParameters - 1 /*Bypass*/ ) * ParameterCounts::lfoExportedParameters );
                numberOfParameters += numberOfModuleParameters + numberOfModuleLFOParameters;
            }
        );
    }
    else
    {
        numberOfParameters += Constants::maxNumberOfModules * ( Constants::maxNumberOfParametersPerModule + ParameterCounts::lfoParametersPerModule );
    }

    return numberOfParameters;
}


namespace
{
    struct UnitGetter
    {
        typedef char const * LE_RESTRICT result_type;
        template <class Parameter>
        result_type operator()() const { return boost::mpl::c_str<typename LE::Parameters::DisplayValueTransformer<Parameter>::Suffix>::value; }
    };

    AutomatedModuleChain::ModuleCPtr LE_FASTCALL module( Program const * LE_RESTRICT const pProgram, std::uint8_t const moduleIndex )
    {
        return pProgram ? pProgram->moduleChain().module( moduleIndex ) : nullptr;
    }
    AutomatedModuleChain::ModulePtr  LE_FASTCALL module( Program       * LE_RESTRICT const pProgram, std::uint8_t const moduleIndex )
    {
        return pProgram ? pProgram->moduleChain().module( moduleIndex ) : nullptr;
    }
} // anonymous namespace

Plugin2HostPassiveInteropController::ParameterLabelGetter::result_type
Plugin2HostPassiveInteropController::ParameterLabelGetter::operator()( ParameterID::Global const id, Program const * ) const
{
    return LE::Parameters::invokeFunctorOnIndexedParameter<GlobalParameters::Parameters>( id.index, UnitGetter() );
}


Plugin2HostPassiveInteropController::ParameterLabelGetter::result_type
Plugin2HostPassiveInteropController::ParameterLabelGetter::operator()( ParameterID::Module const id, Program const * LE_RESTRICT const pProgram ) const
{
    auto const pModule( module( pProgram, id.moduleIndex ) );
    if ( pProgram && !pModule )
        return nullptr;
    auto const pUnitString( Automation::getParameterUnit( id.moduleParameterIndex, pModule.get() ) );
    return pUnitString;
}


Plugin2HostPassiveInteropController::ParameterLabelGetter::result_type
Plugin2HostPassiveInteropController::ParameterLabelGetter::operator()( ParameterID::LFO const id, Program const * LE_RESTRICT const pProgram ) const
{
    using LE::Parameters::IndexOf;

    switch ( id.lfoParameterIndex )
    {
        case IndexOf<LFO::Parameters, LFO::LowerBound>::value: break;
        case IndexOf<LFO::Parameters, LFO::UpperBound>::value: break;
        default: return nullptr;
    }

    ParameterID::Module const moduleParameterID = { ParameterID::Padding(), static_cast<std::uint8_t>( id.moduleParameterIndex + 1U /* Bypass */ ), id.moduleIndex };
    return (*this)( moduleParameterID, pProgram );
}


char const * Plugin2HostPassiveInteropController::ParameterValueStringGetter::operator()( ParameterID::Global const parameterID, Program const * LE_RESTRICT const pProgram ) const
{
#if defined( _WIN32 ) && !defined( LE_SW_FMOD )
    LE_ASSUME( printer.valueSource == Parameters::AutomatedParameterPrinter::Internal );
#endif // _WIN32 && ! FMOD
    return LE::Parameters::invokeFunctorOnIndexedParameter( pProgram->parameters(), parameterID.index, std::forward<Parameters::AutomatedParameterPrinter const>( printer ) );
}


char const * Plugin2HostPassiveInteropController::ParameterValueStringGetter::operator()( ParameterID::ModuleChain const parameterID, Program const * LE_RESTRICT const pProgram ) const
{
#if defined( _WIN32 ) && !defined( LE_SW_FMOD )
    LE_ASSUME( printer.valueSource == Parameters::AutomatedParameterPrinter::Internal );
#endif // _WIN32 && ! FMOD

    ModuleChainParameter parameter;
    switch ( printer.valueSource )
    {
        case Parameters::AutomatedParameterPrinter::NormalisedLinear: parameter = Plugins::NormalisedAutomatedParameter::convertAutomationToParameterValue<ModuleChainParameter>( printer.automationValue ); break;
        case Parameters::AutomatedParameterPrinter::Linear          : parameter = Plugins::FullRangeAutomatedParameter ::convertAutomationToParameterValue<ModuleChainParameter>( printer.automationValue ); break;
        case Parameters::AutomatedParameterPrinter::Unchanged       : parameter = Math::convert<ModuleChainParameter::value_type>                                               ( printer.automationValue ); break;
        case Parameters::AutomatedParameterPrinter::Internal        : parameter = pProgram->moduleChain().getParameterForIndex( parameterID.moduleIndex )                                                  ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }

    return ( parameter.getValue() != noModule )
            ? Effects::effectName( parameter )
            : "<empty>";
}

#if 0
char const * Plugin2HostPassiveInteropController::ParameterValueStringGetter::operator()( ParameterID::Module const parameterID, Program const * LE_RESTRICT const pProgram ) const
{
#if defined( _WIN32 ) && !defined( LE_SW_FMOD )
    LE_ASSUME( printer_.pValue_ == nullptr );
#endif // _WIN32 && ! FMOD

    auto const pModule( pProgram->moduleChain().module( parameterID.moduleIndex ) );
    if ( pModule )
        return pModule->getParameterValueString( parameterID.moduleParameterIndex, printer_ );
    return nullptr;
}
#endif // disabled

char const * Plugin2HostPassiveInteropController::ParameterValueStringGetter::operator()( ParameterID::LFO const parameterID, Program const * LE_RESTRICT const pProgram ) const
{
#if defined( _WIN32 ) && !defined( LE_SW_FMOD )
    LE_ASSUME( printer.valueSource == Parameters::AutomatedParameterPrinter::Internal );
#endif // _WIN32 && ! FMOD

    auto const pModule( pProgram->moduleChain().module( parameterID.moduleIndex ) );
    if ( !pModule || ( parameterID.moduleParameterIndex >= pModule->numberOfLFOControledParameters() ) )
        return nullptr;

    auto const lfoableModuleParameterIndex( parameterID.moduleParameterIndex             );
    auto const moduleParameterIndex       ( lfoableModuleParameterIndex + 1 /* Bypass */ );
    auto const lfoParameterIndex          ( parameterID.lfoParameterIndex                );

    LFO const & lfo( pModule->lfo( lfoableModuleParameterIndex ) );

    using LE::Parameters::IndexOf;
    auto const lowerBoundIndex( IndexOf<LFO::Parameters, LFO::LowerBound>::value );
    auto const upperBoundIndex( IndexOf<LFO::Parameters, LFO::UpperBound>::value );
    switch ( lfoParameterIndex )
    {
        default:
        {
            LE_ASSUME( lfoParameterIndex != lowerBoundIndex );
            LE_ASSUME( lfoParameterIndex != upperBoundIndex );

            return
                LE::Parameters::invokeFunctorOnIndexedParameter
                (
                    lfo.parameters(),
                    lfoParameterIndex,
                    std::forward<Parameters::AutomatedParameterPrinter const>( printer )
                );
        }

        case lowerBoundIndex:
        case upperBoundIndex: break;
    }

    if ( printer.valueSource == Parameters::AutomatedParameterPrinter::Internal )
    {
        printer.valueSource = Parameters::AutomatedParameterPrinter::NormalisedLinear;
        switch ( lfoParameterIndex )
        {
            case lowerBoundIndex: printer.automationValue = lfo.lowerBound(); break;
            case upperBoundIndex: printer.automationValue = lfo.upperBound(); break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }
    }

    return Automation::getParameterValueString( moduleParameterIndex, printer, *pModule );
}


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin2HostPassiveInteropController::ParameterNameGetter
///
////////////////////////////////////////////////////////////////////////////////

namespace
{
    struct NameGetter
    {
        typedef char const * result_type;
        template <class Parameter>
        result_type operator()() const { return LE::Parameters::Name<Parameter>::string_; }
    }; // struct NameGetter
#ifdef LE_SW_FMOD //...mrmlj...temporary quick-fix for short buffers...
    template <> NameGetter::result_type NameGetter::operator()<Effects::BaseParameters::StartFrequency>() const { return "Start"; }
    template <> NameGetter::result_type NameGetter::operator()<Effects::BaseParameters::StopFrequency >() const { return "Stop" ; }
#endif // LE_SW_FMOD
} // anonymous namespace


void Plugin2HostPassiveInteropController::ParameterNameGetter::operator()( ParameterID::Global const parameterID, Program const * ) const
{
    copyToBuffer( LE::Parameters::invokeFunctorOnIndexedParameter<GlobalParameters::Parameters>( parameterID.index, NameGetter() ), buffer_ );
}


void Plugin2HostPassiveInteropController::ParameterNameGetter::operator()( ParameterID::ModuleChain const parameterID, Program const * ) const
{
    // Implementation note:
    //   Previously, a more generic way of generating module chain parameter
    // names (based on the standard "UI elements" functionality) was used.
    // It was however, simplified to a single std::sprintf() after revision
    // 3602 as it was deemed enough.
    //                                    (22.02.2011.) (Domagoj Saric)
    BOOST_VERIFY( unsigned( LE_INT_SPRINTFA( buffer_.begin(), "Module %u", parameterID.moduleIndex + 1 ) ) < buffer_.size() );
}


void Plugin2HostPassiveInteropController::ParameterNameGetter::operator()( ParameterID::Module const parameterID, Program const * LE_RESTRICT const pProgram ) const
{
    auto const pModule( module( pProgram, parameterID.moduleIndex ) );
    if ( pProgram && ( !pModule || ( parameterID.moduleParameterIndex >= pModule->numberOfParameters() ) ) )
    {
        std::strcpy( &buffer_[ 0 ], "N/A" );
        return;
    }

    std::uint8_t const uiModuleIndex( parameterID.moduleIndex + 1 );

    char * LE_RESTRICT pPosition( buffer_.begin() );
    *pPosition++ = 'M';
     pPosition  += Utility::lexical_cast( uiModuleIndex, pPosition );
    *pPosition++ = '.';

    using Module     = Plugin2HostInteropControler::Module;
    using BaseParams = Effects::BaseParameters::Parameters;

    if ( !pModule && ( parameterID.moduleParameterIndex >= BaseParams::static_size ) )
    {
        *pPosition++ = 'P';
         pPosition  += Utility::lexical_cast( Module::effectSpecificParameterIndex( parameterID.moduleParameterIndex ) + 1, pPosition );
         BOOST_ASSERT( pPosition <= buffer_.end() );
    }
    else
    {
        char const * LE_RESTRICT const parameterName
        (
            ( parameterID.moduleParameterIndex < BaseParams::static_size )
                ? LE::Parameters::invokeFunctorOnIndexedParameter<BaseParams>( parameterID.moduleParameterIndex, NameGetter() )
                : pModule->effectSpecificParameterInfo( pModule->effectSpecificParameterIndex( parameterID.moduleParameterIndex ) ).name
        );
        copyToBuffer( parameterName, boost::iterator_range<char *>( pPosition, buffer_.end() ) );
    }
}


void Plugin2HostPassiveInteropController::ParameterNameGetter::operator()( ParameterID::LFO const parameterID, Program const * LE_RESTRICT const pProgram ) const
{
    auto const pModule( module( pProgram, parameterID.moduleIndex ) );
    if ( pProgram && ( !pModule || ( parameterID.moduleParameterIndex + 1U /*Bypass*/ >= pModule->numberOfParameters() ) ) )
    {
        std::strcpy( &buffer_[ 0 ], "N/A" );
        return;
    }

    using namespace ParameterCounts;
    LE_ASSUME( parameterID.lfoParameterIndex < lfoExportedParameters );
    char const * LE_RESTRICT const lfoParameterName( LE::Parameters::invokeFunctorOnIndexedParameter<LFO::Parameters>( parameterID.lfoParameterIndex, NameGetter() ) );
    if ( !pModule && ( parameterID.moduleParameterIndex + 1 >= Effects::BaseParameters::Parameters::static_size ) )
    {
        BOOST_VERIFY
        (
            unsigned( LE_INT_SPRINTFA
            (
                buffer_.begin(),
                "M%u.P%u.LFO.%s",
                parameterID.moduleIndex                      + 1,
                parameterID.moduleParameterIndex - ( 5 - 1 ) + 1,
                lfoParameterName
            ) ) < buffer_.size()
        );
        return;
    }

    ParameterID::Module moduleParameterID;
    moduleParameterID.moduleIndex          = parameterID.moduleIndex;
    moduleParameterID.moduleParameterIndex = parameterID.moduleParameterIndex + 1;
    (*this)( moduleParameterID, pProgram );

    Buffer remainingBuffer( &buffer_[ std::strlen( buffer_.begin() ) ], buffer_.end() );
    remainingBuffer = Buffer( copyToBuffer( ".LFO.", remainingBuffer ), remainingBuffer.end() );
    BOOST_VERIFY( copyToBuffer( lfoParameterName, remainingBuffer ) <= buffer_.end() );
}

#if 0 //...mrmlj...MSVC12u5 bad codegen
template <> Plugins::ParameterID    LE_FASTCALL Plugin2HostInteropControler::make<Plugins::ParameterID   >( SW::ParameterID const selector ) { return                           { selector.binaryValue }; }
template <> Plugins::ParameterIndex LE_FASTCALL Plugin2HostInteropControler::make<Plugins::ParameterIndex>( SW::ParameterID const selector ) { return parameterIndexFromBinaryID( selector.binaryValue ); }
#endif // disabled

//..mrmlj...
ParameterID::ParameterID( Plugins::ParameterID    const parameterID    ) : binaryValue(                       parameterID.value   ) {}
ParameterID::ParameterID( Plugins::ParameterIndex const parameterIndex ) : binaryValue( parameterIDFromIndex( parameterIndex    ) ) {}

void ParameterID::verify() const
{
#ifndef NDEBUG
    switch ( type() )
    {
        case ParameterID::GlobalParameter     :
        case ParameterID::ModuleChainParameter:
        case ParameterID::ModuleParameter     :
        case ParameterID::LFOParameter        :
            break;

        LE_DEFAULT_CASE_UNREACHABLE();
    }
#endif // NDEBUG
}

LE_NOTHROW LE_NOINLINE
ParameterID::BinaryValue LE_FASTCALL parameterIDFromIndex( Plugins::ParameterIndex const parameterIndex )
{
    using Parameters = GlobalParameters::Parameters;
    using namespace ParameterCounts;

    // http://stackoverflow.com/questions/5069489/performance-of-built-in-types-char-vs-short-vs-int-vs-float-vs-double
    ParameterID parameterID;

    LE_ASSUME( parameterIndex < maxNumberOfParameters );
    BOOST_ASSERT( maxNumberOfParameters <= std::numeric_limits<std::uint16_t>::max() );
    auto index( parameterIndex.value );

    if ( index < Parameters::static_size )
    {
        parameterID.value  .type         = ParameterID::GlobalParameter;
        parameterID.value._.global.index = static_cast<std::uint8_t>( index );
    }
    else
    if ( ( index -= Parameters::static_size ) < Constants::maxNumberOfModules )
    {
        parameterID.value  .type                    = ParameterID::ModuleChainParameter;
        parameterID.value._.moduleChain.moduleIndex = static_cast<std::uint8_t>( index );
    }
    else
    if ( ( index -= Constants::maxNumberOfModules ) < Constants::maxNumberOfModuleParameters )
    {
        parameterID.value  .type                        = ParameterID::ModuleParameter;
        parameterID.value._.module.moduleIndex          = static_cast<std::uint8_t>( index ) / Constants::maxNumberOfParametersPerModule;
        parameterID.value._.module.moduleParameterIndex = static_cast<std::uint8_t>( index ) % Constants::maxNumberOfParametersPerModule;
    }
    else
    {
        index -= Constants::maxNumberOfModuleParameters;

        LE_ASSUME( index < lfoParameters );

        std::uint8_t const moduleIndex         ( index          / lfoParametersPerModule ); //...mrmlj...cannot cast index to uint8_t here because of FMOD...
        std::uint8_t const parameterIndex      ( index          % lfoParametersPerModule );
        std::uint8_t const moduleParameterIndex( parameterIndex / lfoExportedParameters  );
        std::uint8_t const lfoParameterIndex   ( parameterIndex % lfoExportedParameters  );

        parameterID.value  .type                     = ParameterID::LFOParameter;
        parameterID.value._.lfo.moduleIndex          = moduleIndex         ;
        parameterID.value._.lfo.moduleParameterIndex = moduleParameterIndex;
        parameterID.value._.lfo.lfoParameterIndex    = lfoParameterIndex   ;
    }

#if LE_SW_ENGINE_INPUT_MODE >= 2
    BOOST_ASSERT_MSG
    (
        ( LE::Parameters::IndexOf<Parameters, GlobalParameters::InputMode>::value == parameterIndex ) ||
        ( parameterIndexFromBinaryID( parameterID.binaryValue ) == parameterIndex ),
        "Parameter index<->ID conversion broken."
    );
#endif // LE_SW_ENGINE_INPUT_MODE >= 2

    return parameterID.binaryValue;
} // parameterIndex2ID()


LE_NOTHROW LE_NOINLINE
Plugins::ParameterIndex LE_FASTCALL parameterIndexFromBinaryID( ParameterID::BinaryValue const parameterIDValue )
{
    using Parameters = GlobalParameters::Parameters;
    using namespace ParameterCounts;

    ParameterID parameterID; parameterID.binaryValue = parameterIDValue;

    //...mrmlj...our VST2.4 code also uses IDs internally...
    //BOOST_ASSERT_MSG
    //(
    //    ( parameterID.value._.global.index != LE::Parameters::IndexOf<Parameters, InputMode>::value ) ||
    //    ( parameterID.value.type           !=  ParameterID::GlobalParameter                         ),
    //    "ParameterID based protocols (AU) should not use/export the InputMode parameter."
    //);

    switch ( parameterID.value.type )
    {
        case ParameterID::GlobalParameter     : return { static_cast<Plugins::ParameterIndex::value_type>(                                                                                                    parameterID.value._.global     .index                                                                                                                                                                         ) };
        case ParameterID::ModuleChainParameter: return { static_cast<Plugins::ParameterIndex::value_type>( Parameters::static_size                                                                          + parameterID.value._.moduleChain.moduleIndex                                                                                                                                                                   ) };
        case ParameterID::ModuleParameter     : return { static_cast<Plugins::ParameterIndex::value_type>( Parameters::static_size + Constants::maxNumberOfModules                                          + parameterID.value._.module     .moduleIndex * Constants::maxNumberOfParametersPerModule + parameterID.value._.module.moduleParameterIndex                                                                     ) };
        case ParameterID::LFOParameter        : return { static_cast<Plugins::ParameterIndex::value_type>( Parameters::static_size + Constants::maxNumberOfModules + Constants::maxNumberOfModuleParameters + parameterID.value._.lfo        .moduleIndex * lfoParametersPerModule                    + parameterID.value._.lfo   .moduleParameterIndex * lfoExportedParameters + parameterID.value._.lfo.lfoParameterIndex ) };

        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
