////////////////////////////////////////////////////////////////////////////////
///
/// host2PluginImpl.inl
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef host2PluginImpl_inl__9BFA5CD4_B26C_4469_8F03_0D71BD78D43A
#define host2PluginImpl_inl__9BFA5CD4_B26C_4469_8F03_0D71BD78D43A
#pragma once
//------------------------------------------------------------------------------
#include "host2PluginImpl.hpp"

#ifdef __GNUC__
#include "plugin2Host.hpp"
#endif // __GNUC__

#include "core/modules/automatedModuleImpl.inl"

#include "le/parameters/conversion.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/plugins/plugin.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"
#include "le/utility/cstdint.hpp"

#include "boost/assert.hpp"
#include "boost/get_pointer.hpp"

#ifdef _DEBUG
    #include <cstdio>
#endif // _DEBUG
#include <cstring>
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
/// \class SpectrumWorxSharedImpl<>::ParameterSetter
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    template <class OptionalGUI>
    void updateGUIForChangedModule( OptionalGUI * const pGUI, bool const addModule, bool const targetSlotFull )
    {
        if ( pGUI )
        {
                 if ( !addModule &&  targetSlotFull ) pGUI->moduleRemoved();
            else if (  addModule && !targetSlotFull ) pGUI->moduleAdded  ();
        }
    }

    inline
    void updateGUIForChangedModule( void const * /*guiless*/, bool /*const addModule*/, bool /*const targetSlotFull*/ ) {}
} // namespace Detail

class ModuleChainParameter;

#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

template <class Impl, class Protocol>
class Host2PluginInteropImpl<Impl, Protocol>::ParameterSetter
{
private:
    /// \todo Consider solving this with a wrapper, like it was done for
    /// AutomatedModuleChain parameters.
    ///                                       (07.07.2010.) (Domagoj Saric)
    struct GlobalParameterSetter
    {
        typedef ErrorCode result_type;

        template <class Parameter>
        result_type operator()() const
        {
            bool const success
            (
                Impl:: template setGlobalParameter<Parameter>
                (
                    effect_,
                    AutomatedParameter:: template convertAutomationToParameterValue<Parameter>( value_ )
                )
            );
            return success ? Plugins::ErrorCode<Protocol>::Success : Plugins::ErrorCode<Protocol>::OutOfMemory;
        }

    #if LE_SW_ENGINE_INPUT_MODE == 1
        template <> result_type operator()<GlobalParameters::InputMode>() const { LE_UNREACHABLE_CODE(); return result_type(); }
    #endif // LE_SW_ENGINE_INPUT_MODE == 1

        Impl                             &       effect_;
        Plugins::AutomatedParameterValue   const value_ ;
    }; // struct GlobalParameterSetter

public:
    using result_type = ErrorCode;
    ////////////////////////////////////////////////////////////////////////////
    result_type operator()( ParameterID::Global const parameterID, Impl * LE_RESTRICT const pEffect ) const
    {
        return LE::Parameters::invokeFunctorOnIndexedParameter<GlobalParameters::Parameters>( parameterID.index, GlobalParameterSetter { *pEffect, value_ } );
    }
    ////////////////////////////////////////////////////////////////////////////
    result_type operator()( ParameterID::ModuleChain const parameterID, Impl * LE_RESTRICT const pImpl ) const
    {
        // Implementation note:
        //   Unconditional setting of module chain parameters through indices
        // would allow for creation of a non-contiguous module rack (with empty
        // slots between modules), this is something we do not want so here we
        // explicitly allow changes only for the last filled or the first
        // unfilled slot.
        //                                    (15.03.2010.) (Domagoj Saric)

        //...mrmlj...consider issuing group begin/end gesture notifications
        //...mrmlj...here (as well as for presets)...
        //...https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/TheAudioUnitView/TheAudioUnitView.html
        //...https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html
        //...http://web.archive.org/web/20111010140734/http://developer.apple.com/library/mac/#/web/20111011163210/http://developer.apple.com/library/mac/samplecode/FilterDemo/Listings/Source_CocoaUI_AppleDemoFilter_UIView_m.html
        //...http://www.juce.com/forum/topic/au-support-begin-and-end-gesture
        //...https://groups.google.com/forum/#!msg/coreaudio-api/2YMCPgKmcmw/7uiKqRU1SdEJ
        //...http://web.archiveorange.com/archive/v/q7bubMk8ylQlGENSdNUr
        //...http://osdir.com/ml/coreaudio-api/2010-06/msg00209.html
        //...https://developer.apple.com/library/mac/technotes/tn2104/_index.html

        std::int8_t const noModule( -1 );
        auto const moduleIndex( parameterID.moduleIndex );
        auto const effectIndex
        (
            AutomatedParameter:: template convertAutomationToParameterValue<ModuleChainParameter>( value_ )
        );
        auto &       moduleChain( pImpl->moduleChain() );
        auto   const chainLength( moduleChain.size() );
        bool const addModule       ( effectIndex != noModule );
        bool const targetSlotFull  ( moduleIndex     <  chainLength );
        bool const previousSlotFull( moduleIndex - 1 <  chainLength );
        bool const nextSlotEmpty   ( moduleIndex + 1 == chainLength );
        if
        (
            (  addModule == targetSlotFull   ) ||
            (  addModule && previousSlotFull ) ||
            ( !addModule && nextSlotEmpty    )
        )
        {
            auto const result( moduleChain.setParameter( moduleIndex, effectIndex, pImpl->moduleInitialiser() ) );
            if ( result.second == effectIndex )
            {
                Detail::updateGUIForChangedModule( boost::get_pointer( pImpl->gui() ), addModule, targetSlotFull );
                //...mrmlj...http://lists.apple.com/archives/coreaudio-api/2005/Oct/msg00164.html
                if ( pImpl->host().wantsManualDependentParameterNotifications() )
                    pImpl->moduleChanged( moduleIndex, result.first.get() );
                return Plugins::ErrorCode<Protocol>::Success;
            }
            return Plugins::ErrorCode<Protocol>::OutOfMemory;
        }
        else
        {
            LE_TRACE( "SW: attempt to change a 'non-tail' module (index %d).\n", parameterID.moduleIndex );
            return Plugins::ErrorCode<Protocol>::OutOfRange;
        }
    }
    ////////////////////////////////////////////////////////////////////////////
    result_type operator()( ParameterID::Module const parameterID, Impl * LE_RESTRICT const pEffect ) const
    {
        auto const pModule( pEffect->moduleChain(). template moduleAs<typename Impl::Module>( parameterID.moduleIndex ) );
        if ( pModule )
        {
            pModule->setAutomatedParameter( parameterID.moduleParameterIndex, value_, AutomatedParameter::normalised );
            return Plugins::ErrorCode<Protocol>::Success;
        }
        return Plugins::ErrorCode<Protocol>::OutOfRange;
    }
    ////////////////////////////////////////////////////////////////////////////
    result_type operator()( ParameterID::LFO const parameterID, Impl * LE_RESTRICT const pEffect ) const
    {
        auto const pModule( pEffect->moduleChain(). template moduleAs<typename Impl::Module>( parameterID.moduleIndex ) );
        if ( pModule && ( parameterID.moduleParameterIndex < pModule->numberOfLFOControledParameters() ) )
        {
            // Implementation note:
            //   Updating a bounds parameter can cause an implicit update/fixup
            // to its counterpart parameter so we notify the host about the
            // possible change.
            //                                (04.07.2011.) (Domagoj Saric)
            auto const implicitlyUpdatedParameter
            (
                Automation:: template setAutomatedLFOParameter<AutomatedParameter>( parameterID.moduleParameterIndex, parameterID.lfoParameterIndex, value_, *pModule )
            );
            if ( pEffect->host().wantsManualDependentParameterNotifications() && implicitlyUpdatedParameter )
            {
                ParameterID::LFO const implicitlyUpdatedLFOParameterID = { implicitlyUpdatedParameter->first, parameterID.moduleParameterIndex, parameterID.moduleIndex };
                ParameterID implicitlyUpdatedParameterID;
                implicitlyUpdatedParameterID.value.type  = ParameterID::LFOParameter;
                implicitlyUpdatedParameterID.value._.lfo = implicitlyUpdatedLFOParameterID;
                auto const parameterSelector( Plugin2HostInteropControler::make<typename Impl::ParameterSelector>( implicitlyUpdatedParameterID ) );
                pEffect->host().automatedParameterChanged( parameterSelector, implicitlyUpdatedParameter->second );
            }
            return Plugins::ErrorCode<Protocol>::Success;
        }
        return Plugins::ErrorCode<Protocol>::OutOfRange;
    }
    ////////////////////////////////////////////////////////////////////////////
    Plugins::AutomatedParameterValue const value_;
}; // ParameterSetter

#pragma warning( pop )

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// Parameters and automation
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl, class Protocol>
typename Plugins::ErrorCode<Protocol>::value_type LE_NOTHROW
Host2PluginInteropImpl<Impl, Protocol>::setParameter
(
             ParameterID             const parameterID,
    Plugins::AutomatedParameterValue const value
)
{
    BOOST_ASSERT( parameterIndexFromBinaryID( parameterID.binaryValue ) < ParameterCounts::maxNumberOfParameters );

    // VST threading issues discussion:
    // http://forum.cockos.com/showthread.php?t=60633
    if ( impl().blockAutomation() )
    {
        LE_TRACE( "\t SW: blocked automation of parameter %u.", parameterIndexFromBinaryID( parameterID.binaryValue ) );
        return Plugins::ErrorCode<Protocol>::CannotDoInCurrentContext;
    }

    // Implementation note:
    //   Ableton Live (7, 8 & 9) sometimes calls setParameter() after being
    // notified about a parameter change. This is completely redundant/wrong and
    // and the plugin has to be able to handle this. Otherwise an
    // if ( value == getParameter( parameterID ) return; like check has to be
    // inserted here in order to ignore the redundant call.
    // http://www.kvraudio.com/forum/viewtopic.php?t=230479
    //                                        (07.07.2010.) (Domagoj Saric)

    impl().markCurrentProgramAsModified();

    ParameterSetter const setter = { value };
    return invokeFunctorOnIdentifiedParameter( parameterID, std::forward<ParameterSetter const>( setter ), &impl() );
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // host2PluginImpl_inl
