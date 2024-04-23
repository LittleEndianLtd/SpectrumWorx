////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin2HostImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin2HostImpl_hpp__945F8003_3F61_4CE0_9870_C44C37E5AD66
#define plugin2HostImpl_hpp__945F8003_3F61_4CE0_9870_C44C37E5AD66
#pragma once
//------------------------------------------------------------------------------
#include "plugin2Host.hpp"

#ifdef __GNUC__
#include "core/automatedModuleChain.hpp" //...mrmlj...SW::Program...
#endif // __GNUC__


#include "le/plugins/plugin.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin2HostActiveInteropImpl
///
/// \brief Main SpectrumWorx plugin implementation class. Functionality shared
/// for all protocols.
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

template <class Impl, class Protocol>
class LE_NOVTABLE Plugin2HostPassiveInteropImpl
    :
    public Plugin2HostPassiveInteropController
{
public:
    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    using AutomatedParameter = typename Plugins::AutomatedParameterFor<Protocol>::type;

    Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS LE_FASTCALL getParameter( ParameterID ) const;

    static bool getParameterProperties( ParameterID, Plugins::ParameterInformation<Protocol> &, Program const * );

    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    struct ParameterValueStringGetter //...mrmlj...aggregate initialisation...: Plugin2HostPassiveInteropController::ParameterValueStringGetter
    {
        using result_type = Plugin2HostPassiveInteropController::ParameterValueStringGetter::result_type;

        result_type operator()( ParameterID::Module const parameterID, Program const * LE_RESTRICT const pProgram ) const
        {
        #if defined( _WIN32 ) && !defined( LE_SW_FMOD )
            LE_ASSUME( baseGetter.printer.valueSource == Parameters::AutomatedParameterPrinter::Internal );
        #endif // _WIN32 && ! FMOD
            auto const pModule( pProgram->moduleChain().moduleAs<typename Impl::Module>( parameterID.moduleIndex ) );
            if ( pModule )
                return pModule->getParameterValueString( parameterID.moduleParameterIndex, baseGetter.printer );
            return nullptr;
        }

        result_type operator()( ParameterID::Global      const parameterID, Program const * const pProgram ) const { return baseGetter( parameterID, pProgram ); }
        result_type operator()( ParameterID::ModuleChain const parameterID, Program const * const pProgram ) const { return baseGetter( parameterID, pProgram ); }
        result_type operator()( ParameterID::LFO         const parameterID, Program const * const pProgram ) const { return baseGetter( parameterID, pProgram ); }

        Plugin2HostPassiveInteropController::ParameterValueStringGetter const baseGetter;
    }; // struct ParameterValueStringGetter
    #pragma warning( pop )

    LE_NOTHROWNOALIAS
    void getParameterDisplay
    (
        ParameterID                                          const parameterID,
        boost::iterator_range<char *>                        const text,
        Plugins::AutomatedParameterValue const * LE_RESTRICT const pValue
    ) const
    {
        //...mrmlj...duplicated in SpectrumWorxEditorFMOD as a quick-hack around fmod ugliness
        //...mrmlj...(printing required both in the DSP and UI)...

        // http://www.juce.com/forum/topic/juce-module-automatically-handle-plugin-parameters
    #if defined( _WIN32 ) && !defined( LE_SW_FMOD )
        LE_ASSUME( pValue == nullptr );
    #endif // _WIN32 && !FMOD
        using AutomatedParameter = typename Plugins::AutomatedParameterFor<Protocol>::type;
        ParameterValueStringGetter const getter =
        {{
            pValue ? *pValue : 0,
            pValue
                ? std::is_same<AutomatedParameter, Plugins::FullRangeAutomatedParameter>::value
                    ? LE::Parameters::AutomatedParameterPrinter::Linear
                    : LE::Parameters::AutomatedParameterPrinter::NormalisedLinear
                : LE::Parameters::AutomatedParameterPrinter::Internal,
            text,
            impl().engineSetup()
        }};
        char const * const pValueString( invokeFunctorOnIdentifiedParameter( parameterID, std::forward<ParameterValueStringGetter const>( getter ), &impl().program() ) );
        copyToBuffer( pValueString, text );
    }

    /* </Parameters> */

    template <Plugins::PluginCapability pluginCapability>
    bool queryDynamicCapability() const { return false; }

protected:
    Impl       & impl()       { LE_ASSUME( this ); return static_cast<Impl &>( *this );             }
    Impl const & impl() const { return const_cast<Plugin2HostPassiveInteropImpl &>( *this ).impl(); }
}; // class Plugin2HostPassiveInteropImpl

template <class Impl, class Protocol, class Base = Plugin2HostInteropControler>
class LE_NOVTABLE Plugin2HostActiveInteropImpl
    :
    public Base
{
public:
    using AutomatedParameter = typename Plugins::AutomatedParameterFor<Protocol>::type;

protected:
#ifdef _MSC_VER
    Plugin2HostActiveInteropImpl() {}
    template <typename ConstructionParameter>
    Plugin2HostActiveInteropImpl( ConstructionParameter const constructionParameter ) : Base( constructionParameter ) {}
#else
    using Base::Base;
#endif // _MSC_VER

    using ErrorCode = Plugins::ErrorCode<Protocol>;
    static typename ErrorCode::value_type makeErrorCode( ErrorCode const errorCode ) { return errorCode                                            ; }
    static typename ErrorCode::value_type makeErrorCode( bool      const success   ) { return success ? ErrorCode::Success : ErrorCode::OutOfMemory; }

protected:
    void LE_NOTHROW LE_FASTCALL automatedParameterChanged( ParameterID const parameterID, Plugins::AutomatedParameterValue const newValue ) const
    {
        impl().host().automatedParameterChanged( Plugin2HostInteropControler::make<typename Impl::ParameterSelector>( parameterID ), newValue );
        impl().markCurrentProgramAsModified();
    }

protected:
    Impl       & impl()       { LE_ASSUME( this ); return static_cast<Impl &>( *this );            }
    Impl const & impl() const { return const_cast<Plugin2HostActiveInteropImpl &>( *this ).impl(); }

protected: // Plugin2HostInteropControler virtual overrides
    friend class Host2PluginInteropImpl<Impl, Protocol>;
    typedef Plugin2HostInteropControler::ParameterValueForAutomation ParameterValueForAutomation;
    LE_NOTHROW void LE_FASTCALL automatedParameterBeginEdit( ParameterID const parameterID                                         ) const LE_OVERRIDE LE_SEALED { return impl().host().automatedParameterBeginEdit( Plugin2HostInteropControler::make<typename Impl::ParameterSelector>( parameterID ) ); }
    LE_NOTHROW void LE_FASTCALL automatedParameterEndEdit  ( ParameterID const parameterID                                         ) const LE_OVERRIDE LE_SEALED { return impl().host().automatedParameterEndEdit  ( Plugin2HostInteropControler::make<typename Impl::ParameterSelector>( parameterID ) ); }
    LE_NOTHROW void LE_FASTCALL gestureBegin               ( char const * const description                                        ) const LE_OVERRIDE LE_SEALED { return impl().host().gestureBegin( description ); }
    LE_NOTHROW void LE_FASTCALL gestureEnd                 (                                                                       ) const LE_OVERRIDE LE_SEALED { return impl().host().gestureEnd  (             ); }
    LE_NOTHROW void LE_FASTCALL automatedParameterChanged  ( ParameterID, ParameterValueForAutomation                              ) const LE_OVERRIDE LE_SEALED;
    LE_NOTHROW void LE_FASTCALL moduleChanged              ( std::uint8_t moduleIndex, Plugin2HostInteropControler::Module const * ) const LE_OVERRIDE LE_SEALED;
    LE_NOTHROW bool LE_FASTCALL parameterListChanged       (                                                                       ) const LE_OVERRIDE LE_SEALED { return impl().host().parameterListChanged(); }
    LE_NOTHROW void LE_FASTCALL presetChangeBegin          (                                                                       ) const LE_OVERRIDE LE_SEALED { return impl().host().presetChangeBegin   (); }
    LE_NOTHROW void LE_FASTCALL presetChangeEnd            (                                                                       ) const LE_OVERRIDE LE_SEALED { return impl().host().presetChangeEnd     (); }
    LE_NOTHROW bool LE_FASTCALL latencyChanged             (                                                                       )       LE_OVERRIDE LE_SEALED;

#if LE_SW_ENGINE_INPUT_MODE >= 2
    LE_NOTHROW        bool LE_FASTCALL hostTryIOConfigurationChange      ( std::uint8_t numberOfMainChannels, std::uint8_t numberOfSideChannels )       LE_OVERRIDE LE_SEALED;
    LE_NOTHROWNOALIAS bool LE_FASTCALL hostSupportsIOConfigurationChanges(                                                                      ) const LE_OVERRIDE LE_SEALED;
#endif // LE_SW_ENGINE_INPUT_MODE
}; // class Plugin2HostActiveInteropImpl

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#endif // plugin2HostImpl_hpp
