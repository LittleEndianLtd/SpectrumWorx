////////////////////////////////////////////////////////////////////////////////
///
/// module.cpp
/// ----------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "moduleDSPAndGUI.hpp"

#include "automatedModuleImpl.inl"

#include "le/utility/parentFromMember.hpp"
#include "le/spectrumworx/engine/moduleNode.hpp" // for intrusive_ptr_release_deleter
#include "gui/editor/spectrumWorxEditor.hpp"

#include "boost/smart_ptr/intrusive_ptr.hpp"
#include "boost/utility/typed_in_place_factory.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

// Implementation note:
//   This is required to prevent Clang from inlining the base destructor into
// each ModuleDSP<> destructor.
//                                            (13.12.2011.) (Domagoj Saric)
LE_NOINLINE LE_NOTHROW LE_COLD
Module::~Module()
{
    LE_ASSUME( !ui_.is_initialized() );
}


void LE_NOTHROW Module::createGUI( GUI::SpectrumWorxEditor & editor, std::uint8_t const moduleIndex )
{
    struct ModuleUIInplaceConstructor : public boost::typed_in_place_factory_base
    {
        ModuleUIInplaceConstructor( GUI::SpectrumWorxEditor & editor ) : editor_( editor ) {}

        GUI::ModuleUI * apply( void * const pAddress ) const
        {
            LE_ASSUME( pAddress );
            GUI::ModuleUI * const pModuleUI( new ( pAddress ) GUI::ModuleUI() );
            /// \note We initiate the creation of effect specific widgets from
            /// within this specialized inplace factory so that the module's GUI
            /// does not get marked as constructed until all the controls have
            /// actually been constructed and setup.
            ///                               (23.04.2013.) (Domagoj Saric)
            pModuleUI->module().doCreateGUI( *pModuleUI );
            BOOST_ASSERT_MSG
            (
                pModuleUI->getNumChildComponents() == ( GUI::ModuleUI::baseWidgets + pModuleUI->module().numberOfEffectSpecificParameters() ),
                "Unexpected number of child widgets at end of ModuleUI constructor."
            );
            pModuleUI->updateForEngineSetupChanges( editor_.engineSetup() );
            return pModuleUI;
        }

        GUI::SpectrumWorxEditor & editor_;
    }; // struct ModuleUIInplaceConstructor

    if ( !GUI::isThisTheGUIThread() )
    {
        boost::intrusive_ptr<Module> pModule( this );
        GUI::postMessage
        (
            [=, &editor](){ pModule->createGUI( editor, moduleIndex ); }
        );
        return;
    }

    try
    {
        BOOST_ASSERT( GUI::isThisTheGUIThread() || juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager() );
        BOOST_ASSERT( !ui_ );
        ui_ = ModuleUIInplaceConstructor( editor );
        BOOST_ASSERT( gui() );

    #ifndef NDEBUG
        /// \note Certain debug-only sanity checks may require early access to
        /// the editor (e.g. for access to the Engine::Setup instance).
        ///                                   (28.03.2014.) (Domagoj Saric)
        editor.addChildComponent( &*gui() );
    #endif // nDEBUG

        /// \note It is assumed that right upon (GUI) creation a module is not
        /// selected and that therefor its shared parameters UI is not active
        /// and does not need to be updated.
        ///                                   (07.02.2014.) (Domagoj Saric)
        BOOST_ASSERT( !gui()->selected() );
        gui()->setBypass( bypass() );

        using GUI::ModuleControlBase;
        std::uint8_t const numberOfEffectParameters( numberOfEffectSpecificParameters() );
        for ( std::uint8_t effectParameterIndex( 0 ); effectParameterIndex < numberOfEffectParameters; ++effectParameterIndex )
            gui()->setEffectParameter
            (
                effectParameterIndex,
                getEffectParameter( effectParameterIndex ),
                GUI::ModuleUI::AutomationOrPreset
            );

        gui()->moveToSlot( moduleIndex );
        GUI::addToParentAndShow( editor, *gui() );
    } catch ( ... ) {}
}


bool LE_NOTHROW Module::destroyGUI()
{
    // Implementation note:
    //   The current module has to be removed from the processing chain before
    // its GUI is destroyed or added after its GUI is created to prevent the
    // processing thread from accessing the, possibly partially
    // destroyed/created, GUI in the ModuleDSP<>::doPreProcess() member
    // function.
    //                                        (18.11.2011.) (Domagoj Saric)
    if ( GUI::isThisTheGUIThread() /*&& referenceCount_ == 1*/ )
    {
        BOOST_ASSERT( juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager() );
        BOOST_ASSERT( gui() ); //...might not be true if not part of the "active chain"...checked outside for now...
    #if !LE_SW_SEPARATED_DSP_GUI
        /// \note Make sure the module is not 'used' from the processing thread
        /// (possibly updating its GUI from active LFOs).
        ///                                       (18.03.2014.) (Domagoj Saric)
        auto const processingLock( gui()->getProcessingLock() );
    #endif
        gui() = boost::none;
        doDestroyGUI( *this );
        return true;
    }
    else
    if ( gui() )
    {
        GUI::postMessage
        (
            *this,
            []( GUI::ModuleUI & gui ) { BOOST_VERIFY( gui.module()./*...mrmlj...*/destroyGUI() ); return true; }
        );
        return false;
    }
    // Not created or already destroyed
    return true;
}


LE_NOTHROW
float Module::setBaseParameter( std::uint8_t const sharedParameterIndex, float const parameterValue )
{
    float const setValue( ModuleDSP::setBaseParameter( sharedParameterIndex, parameterValue ) );
    if ( gui() )
        gui()->setBaseParameter( sharedParameterIndex, setValue, GUI::ModuleUI::AutomationOrPreset );
    return setValue;
}

LE_NOTHROW
float Module::setEffectParameter( std::uint8_t const effectParameterIndex, float const parameterValue )
{
    float const setValue( ModuleDSP::setEffectParameter( effectParameterIndex, parameterValue ) );
    BOOST_ASSERT
    (
        ( parameterValue == setValue ) ||
        ( effectSpecificParameterInfo( effectParameterIndex ).type != ParameterInfo::FloatingPoint )
    );
    if ( gui() )
        gui()->setEffectParameter( effectParameterIndex, setValue, GUI::ModuleUI::AutomationOrPreset );
    return setValue;
}


LE_NOTHROW
float Module::setParameterValueFromUI( std::uint8_t const parameterIndex, float const value )
{
    BOOST_ASSERT_MSG
    (
        ( parameterIndex == 0 ) || !lfo( parameterIndex - 1 ).enabled(),
        "Parameter changed from the GUI while its LFO is enabled?"
    );
    return ( parameterIndex < numberOfBaseParameters )
        ? ModuleDSP::setBaseParameter(                                 parameterIndex  , value )
        : ModuleDSP::setEffectParameter( effectSpecificParameterIndex( parameterIndex ), value );
}


LE_NOTHROW
void Module::setBaseParameterFromLFO( std::uint8_t const sharedParameterIndex, LFO::value_type const lfoValue )
{
    auto const parameterValue( ModuleParameters::setBaseParameterFromLFOAux( sharedParameterIndex, lfoValue ) );
    if ( gui() )
        gui()->setBaseParameter( sharedParameterIndex, parameterValue, GUI::ModuleUI::LFOValue );
}

LE_NOTHROW
void Module::setEffectParameterFromLFO( std::uint8_t const effectParameterIndex, LFO::value_type const lfoValue )
{
    auto const parameterValue( ModuleParameters::setEffectParameterFromLFOAux( effectParameterIndex, lfoValue ) );
    if ( gui() )
        gui()->setEffectParameter( effectParameterIndex, parameterValue, GUI::ModuleUI::LFOValue );
}


void LE_COLD Module::updateLFOGUI( std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, Plugins::AutomatedParameterValue const value )
{
    GUI::postMessage
    (
        *this,
        [=]( GUI::ModuleUI & gui ) { gui.updateLFOParameter( parameterIndex, lfoParameterIndex, value ); return true; }
    );
}


Module & Module::fromGUI( GUI::ModuleUI & gui )
{
    return Utility::ParentFromOptionalMember<Module, GUI::ModuleUI, &Module::ui_, false>()( gui );
}


namespace Engine
{
    void LE_NOTHROW LE_NOINLINE LE_FASTCALL intrusive_ptr_release_deleter( ModuleNode const * LE_RESTRICT const pModuleNode )
    {
        auto const & module( actualModule<Module>( *pModuleNode ) );
        if
        (
                                 !module  .gui       () ||
            const_cast<Module &>( module ).destroyGUI()
        )
        {
            LE_ASSUME( module.referenceCount_ == 0 );
            delete &module;
        }
        else
        {
            LE_ASSUME( module.referenceCount_ == 1 );
        }
    }
} // namespace Engine

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
