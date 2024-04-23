////////////////////////////////////////////////////////////////////////////////
///
/// plugin2HostImpl.inl
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin2HostImpl_inl__AB4A42D6_2C95_4C31_B90E_55BB2A91243A
#define plugin2HostImpl_inl__AB4A42D6_2C95_4C31_B90E_55BB2A91243A
#pragma once
//------------------------------------------------------------------------------
#include "plugin2HostImpl.hpp"

#if LE_SW_SEPARATED_DSP_GUI && LE_SW_GUI
    #include "core/modules/moduleGUI.hpp"
#elif LE_SW_GUI
    #include "core/modules/moduleDSPAndGUI.hpp"
#endif // module interface
#include "le/parameters/parametersUtilities.hpp"
#include "le/parameters/runtimeInformation.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"
#include "le/spectrumworx/effects/baseParametersUIElements.hpp" //...mrmlj...required only for getParameterProperties()...

#include "boost/config.hpp"
#if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
    #include "boost/polymorphic_cast.hpp"
#endif // no RTTI
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// http://forum.cockos.com/showthread.php?p=538840
// http://www.kvraudio.com/forum/viewtopic.php?t=253666
// http://www.cockos.com/reaper/sdk/vst/vst_ext.php
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Indexed parameter functors.
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
///
/// \class SpectrumWorxSharedImpl<>::ParameterGetter
///
////////////////////////////////////////////////////////////////////////////////

template <class AutomatedParameter>
struct ParameterGetterBase
{
    typedef Plugins::AutomatedParameterValue result_type;

    result_type LE_NOTHROWNOALIAS LE_FASTCALL operator()( ParameterID::Global const parameterID, Program const * LE_RESTRICT const pProgram ) const
    {
        return LE::Parameters::invokeFunctorOnIndexedParameter( pProgram->parameters(), parameterID.index, typename AutomatedParameter::Getter() );
    }

    result_type LE_NOTHROWNOALIAS LE_FASTCALL operator()( ParameterID::ModuleChain const parameterID, Program const * LE_RESTRICT const pProgram ) const
    {
        return AutomatedParameter::convertParameterToAutomationValue( pProgram->moduleChain().getParameterForIndex( parameterID.moduleIndex ) );
    }

    result_type LE_NOTHROWNOALIAS LE_FASTCALL operator()( ParameterID::LFO const parameterID, Program const * LE_RESTRICT const pProgram ) const
    {
        return (*this)( parameterID, pProgram->moduleChain().module( parameterID.moduleIndex ).get() );
    }

    result_type LE_NOTHROWNOALIAS LE_FASTCALL operator()( ParameterID::LFO const parameterID, Plugin2HostInteropControler::Module const * LE_RESTRICT const pModule ) const
    {
        return pModule
            ? Automation::getAutomatedLFOParameter       <AutomatedParameter>( parameterID.moduleParameterIndex, parameterID.lfoParameterIndex, *pModule )
            : Automation::getDefaultAutomatedLFOParameter<AutomatedParameter>(                                   parameterID.lfoParameterIndex           );
    }
}; // class ParameterGetterBase

template <class ActualModule, class AutomatedParameter>
struct ParameterGetter : ParameterGetterBase<AutomatedParameter>
{
    using Base        = ParameterGetterBase<AutomatedParameter>;
    using result_type = typename Base::result_type;
    using Base::operator();

    result_type LE_NOTHROWNOALIAS LE_FASTCALL operator()( ParameterID::Module const parameterID, Program const * LE_RESTRICT const pProgram ) const
    {
        LE_ASSUME( parameterID.moduleParameterIndex < Constants::maxNumberOfModuleParameters );
        return (*this)( parameterID, pProgram->moduleChain().moduleAs<ActualModule>( parameterID.moduleIndex ).get() );
    }

    result_type LE_NOTHROWNOALIAS LE_FASTCALL operator()( ParameterID::Module const parameterID, ActualModule const * LE_RESTRICT const pModule ) const
    {
        // Implementation note:
        //   If chunks are not used/supported and/or the host generated UI is
        // used, the host application "manually" iterates over all parameters to
        // save/restore them so we must explicitly handle the case(s) of
        // parameters that do not actually exist (e.g. the corresponding module
        // does not currently exist or it does not have the specified
        // parameter).
        //                                    (26.06.2009.) (Domagoj Saric)
        return pModule
            ? pModule->getAutomatedParameter( parameterID.moduleParameterIndex, AutomatedParameter::normalised )
            : result_type();
    }
}; // struct ParameterGetter : ParameterGetterBase<AutomatedParameter>


////////////////////////////////////////////////////////////////////////////////
///
/// \class ParameterInfoGetter
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

template <class Protocol>
class ParameterInfoGetter : public Plugins::ParameterInformation<Protocol>
{
public:
    using Base        = Plugins::ParameterInformation<Protocol>;
    using result_type = void;

    result_type operator()( ParameterID::Global const id, Program const * )
    {
        LE::Parameters::invokeFunctorOnIndexedParameter<GlobalParameters::Parameters>( id.index, std::forward<Base>( *this ) );
    }

    result_type operator()( ParameterID::ModuleChain /*const id*/, Program const * )
    {
        Base:: LE_GNU_SPECIFIC( template ) operator()<ModuleChainParameter>();
        this->markAsMeta();
    }

    result_type operator()( ParameterID::Module const id, Program const * LE_RESTRICT const pProgram )
    {
        auto const pModule( pProgram ? pProgram->moduleChain().module( id.moduleIndex ) : nullptr );

        auto const moduleParameterIndex( id.moduleParameterIndex );

        using namespace LE::Parameters::Traits;
        typedef LE::Parameters::LinearUnsignedInteger::Modify<Minimum<0>, Maximum<0>, Default<0>>::type NotAvailableParameter;

        if ( pProgram && ( !pModule || ( moduleParameterIndex >= pModule->numberOfParameters() ) ) )
        {   // Dynamic parameter list:
            Base:: LE_GNU_SPECIFIC( template ) operator()<NotAvailableParameter>();
            return;
        }

        typedef Effects::BaseParameters::Parameters BaseParams;

        if ( moduleParameterIndex < BaseParams::static_size )
        {
            LE::Parameters::invokeFunctorOnIndexedParameter<BaseParams>( moduleParameterIndex, std::forward<Base>( *this ) );
            switch ( moduleParameterIndex )
            {
                using LE::Parameters::IndexOf;
                default:
                    break;
                case IndexOf<BaseParams, Effects::BaseParameters::StartFrequency>::value:
                case IndexOf<BaseParams, Effects::BaseParameters::StopFrequency >::value:
                    this->markAsMeta();
            }
        }
        else
        {
            if ( pModule )
            {
                this->set( pModule->parameterInfo( moduleParameterIndex ) );
            }
            else
            {
                typedef LE::Parameters::LinearFloat::Modify<Minimum<0>, Maximum<1>, Default<0>>::type GenericModuleChainParameter;
                Base:: LE_GNU_SPECIFIC( template ) operator()<GenericModuleChainParameter>();
            }
        }
    }

    result_type operator()( ParameterID::LFO const id, Program const * LE_RESTRICT const pProgram )
    {
        auto const pModule( pProgram ? pProgram->moduleChain().module( id.moduleIndex ) : nullptr );

        if ( pProgram && ( !pModule || ( id.moduleParameterIndex >= pModule->numberOfParameters() ) ) )
        {   // Dynamic parameter list:
            using namespace LE::Parameters::Traits;
            typedef LE::Parameters::LinearUnsignedInteger::Modify<Minimum<0>, Maximum<0>, Default<0>>::type NotAvailableParameter;
            Base:: LE_GNU_SPECIFIC( template ) operator()<NotAvailableParameter>();
        }
        else
        {
            using LFO = LE::Parameters::LFOImpl;
            using LFOParameters = LFO::Parameters;
            auto const lfoParameterIndex( id.lfoParameterIndex );
            LE::Parameters::invokeFunctorOnIndexedParameter<LFOParameters>( lfoParameterIndex, std::forward<Base>( *this ) );
            switch ( lfoParameterIndex )
            {
                using LE::Parameters::IndexOf;
                default:
                    break;
                case IndexOf<LFOParameters, LFO::LowerBound>::value:
                case IndexOf<LFOParameters, LFO::UpperBound>::value:
                    this->markAsMeta();
            }
        }
    }

     ParameterInfoGetter( ParameterInfoGetter const & ) = delete;
    ~ParameterInfoGetter(                             ) = delete;
}; // class ParameterInfoGetter

#pragma warning( pop )

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// Parameters and automation
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl, class Protocol>
Plugins::AutomatedParameterValue LE_NOTHROWNOALIAS
Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameter( ParameterID const parameterID ) const
{
    return invokeFunctorOnIdentifiedParameter( parameterID, ParameterGetter<typename Impl::Module, AutomatedParameter>(), &impl().program() );
}

template <class Impl, class Protocol>
bool Plugin2HostPassiveInteropImpl<Impl, Protocol>::getParameterProperties
(
    ParameterID                                                 const parameterID,
    Plugins::ParameterInformation<Protocol>       &                   parameterInfo,
    Program                                 const * LE_RESTRICT const pProgram
)
{
    parameterInfo.clear(); //...mrmlj...Audition CS5.5

    using InfoGetter = ParameterInfoGetter<Protocol>;
    invokeFunctorOnIdentifiedParameter( parameterID, std::forward<InfoGetter>( static_cast<InfoGetter &>( parameterInfo ) ), pProgram );
    auto * const pNameBuffer( parameterInfo.nameBuffer() );
    if ( pNameBuffer )
    {
        getParameterName( parameterID, boost::make_iterator_range( *pNameBuffer ), pProgram );
        parameterInfo.nameSet(); //...mrmlj...
    }
    return true;
}


////////////////////////////////////////////////////////////////////////////////
///
/// Plugin2HostInteropControler virtual interface implementation
///
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//
// Plugin2HostInteropImpl<>::hostTryIOConfigurationChange()
// --------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Informs the host about changes to the number of IO channels and/or
/// buses.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation notes:
//   Notifications for latency and IO mode are done with two separate calls
// because this was found to be necessary with some hosts (e.g. Ableton Live 7
// and 8) that would otherwise return true from the ioChanged() call even though
// they accepted only part of the change (e.g. the latency change but not the IO
// mode change).
//                                            (28.06.2010.) (Domagoj Saric)
//   Because frequent ioChanged() calls are known to freeze Ableton Live
// (especially version 8) we check and make the calls only if really necessary.
//                                            (20.07.2010.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////
// Related links:
// http://forums.cockos.com/showthread.php?p=308093
// http://forum.cockos.com/showthread.php?t=36483
// http://forum.cockos.com/showthread.php?t=27875
// http://www.koders.com/delphi/fid5A86C7191CBBB828E612DF16911D8577D1ED6FD1.aspx?s=delphi
// http://www.koders.com/delphi/fidDB7E639EEB1D59388C5CD56BAAF28B40A93CCE45.aspx?s=delphi
// http://forum.ableton.com/viewtopic.php?t=26856&postdays=0&postorder=asc&start=60
////////////////////////////////////////////////////////////////////////////////

#if LE_SW_ENGINE_INPUT_MODE >= 2
template <class Impl, class Protocol, class Base>
LE_NOTHROW
bool Plugin2HostActiveInteropImpl<Impl, Protocol, Base>::
hostTryIOConfigurationChange( std::uint8_t const numberOfMainChannels, std::uint8_t const numberOfSideChannels )
{
    return impl().host().reportNewNumberOfIOChannels( numberOfMainChannels, numberOfSideChannels, numberOfMainChannels );
}


template <class Impl, class Protocol, class Base>
LE_NOTHROWNOALIAS
bool Plugin2HostActiveInteropImpl<Impl, Protocol, Base>::hostSupportsIOConfigurationChanges() const
{
    return impl().host().template canDo<Plugins::AcceptIOChanges>();
}
#endif // LE_SW_ENGINE_INPUT_MODE


////////////////////////////////////////////////////////////////////////////////
//
// Plugin2HostInteropImpl<>::latencyChanged()
// ------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Informs the host about changes to the plugin latency.
///
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4389 ) // Signed/unsigned mismatch.
#endif // _MSC_VER

template <class Impl, class Protocol, class Base>
bool LE_NOTHROW Plugin2HostActiveInteropImpl<Impl, Protocol, Base>::latencyChanged()
{
    /// \note Up to SVN revision 8346 this function tried to perform this
    /// notification asynchronously (posting it to the GUI thread) in order to
    /// workaround for potential deadlocks in some hosts when they get called
    /// back here immediately during automation (for example of the FFTSize
    /// parameter). This however created a new problem in VST 2.4 where the same
    /// callback ("ioChanged") is used both for latency and channel
    /// configuration changes - the asynchronous latency notification could
    /// happen right in the middle of an attempt to change the IO mode. For this
    /// reason this notification is now performed synchronously hoping that
    /// deadlock-problematic hosts have or will be fixed.
    ///                                       (04.10.2013.) (Domagoj Saric)
#if defined( _WIN32 ) && !defined( LE_SW_FMOD ) //...mrmlj...vst specific...threading issues verification...
    BOOST_ASSERT_MSG
    (
        ( ( impl().host().getNumOutputs() ==           impl().engineSetup().numberOfChannels()                                                 ) || ( impl().host().getNumOutputs() == impl().maxNumberOfOutputs ) ) &&
        ( ( impl().host().getNumInputs () == unsigned( impl().engineSetup().numberOfChannels() + impl().engineSetup().numberOfSideChannels() ) ) || ( impl().host().getNumInputs () == impl().maxNumberOfInputs  ) ),
        "Performing a latency change notification in the middle of an IO mode change (which cannot be separated in VST 2.4)"
    );
#endif // _WIN32
    auto const newLatency( impl().engineSetup().latencyInSamples() );
    return impl().host().reportNewLatencyInSamples( newLatency );
}

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

template <class Impl, class Protocol, class Base>
void LE_NOTHROW Plugin2HostActiveInteropImpl<Impl, Protocol, Base>::moduleChanged
(
    std::uint8_t                                            const moduleIndex,
    Plugin2HostInteropControler::Module const * LE_RESTRICT const pModuleBase
) const
{
    BOOST_ASSERT_MSG( !parameterListChanged(), "Should not get here if host supports parameter list changes" );

    using AutomatedParameterValue = typename AutomatedParameter::value_type;

    ParameterID fullModuleParameterID;
    fullModuleParameterID.value  .type               = ParameterID::ModuleParameter;
    fullModuleParameterID.value._.module.moduleIndex = moduleIndex;
    ParameterID::Module & moduleParameterID( fullModuleParameterID.value._.module );

    ParameterID fullLFOParameterID;
    fullLFOParameterID.value  .type            = ParameterID::LFOParameter;
    fullLFOParameterID.value._.lfo.moduleIndex = moduleIndex;
    ParameterID::LFO & lfoParameterID( fullLFOParameterID.value._.lfo );

    BOOST_ASSERT( impl().program().moduleChain().module( moduleIndex ).get() == pModuleBase );

    typedef typename Impl::Module Module;
#if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
    auto const pModule( boost::polymorphic_downcast<Module const *>( pModuleBase ) );
#else
    auto const pModule( static_cast                <Module const *>( pModuleBase ) );
#endif

    ParameterGetter<Module, AutomatedParameter> /*const*/ getParameter; //...mrmlj...

    auto const & host( impl().host() );

    moduleParameterID.moduleParameterIndex = 0;
    while ( moduleParameterID.moduleParameterIndex != Constants::maxNumberOfParametersPerModule )
    {
        typedef typename Impl::ParameterSelector ParameterSelector;

        AutomatedParameterValue const moduleParameterValue( getParameter( moduleParameterID, pModule ) );
        host.automatedParameterChanged( Base:: template make<ParameterSelector>( fullModuleParameterID ), moduleParameterValue );

        if ( moduleParameterID.moduleParameterIndex != 0 /*Bypass*/ )
        {
            lfoParameterID.moduleParameterIndex = moduleParameterID.moduleParameterIndex - 1;
            lfoParameterID.lfoParameterIndex    = 0;
            while ( lfoParameterID.lfoParameterIndex != ParameterCounts::lfoExportedParameters )
            {
                AutomatedParameterValue const lfoParameterValue( getParameter( lfoParameterID, pModule ) );
                host.automatedParameterChanged( Base:: template make<ParameterSelector>( fullLFOParameterID ), lfoParameterValue );
                ++lfoParameterID.lfoParameterIndex;
            }
        }

        ++moduleParameterID.moduleParameterIndex;
    }
}


template <class Impl, class Protocol, class Base>
void LE_NOTHROW
Plugin2HostActiveInteropImpl<Impl, Protocol, Base>::automatedParameterChanged( ParameterID const parameterID, ParameterValueForAutomation const value ) const
{
    bool const normalised
    (
        AutomatedParameter::normalised
    #ifdef LE_SW_FMOD
            ||
        (
            ( parameterID.value.type == ParameterID::ModuleParameter                                              ) &&
            ( parameterID.value._.module.moduleParameterIndex >= Engine::ModuleParameters::numberOfBaseParameters )
        )
    #endif // LE_SW_FMOD
    );
    automatedParameterChanged( parameterID, normalised ? value.normalised : value.fullRange );
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // plugin2HostImpl_inl
