////////////////////////////////////////////////////////////////////////////////
///
/// module.cpp
/// ----------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "moduleControl.hpp"

#include "core/modules/automatedModuleImpl.inl" //...mrmlj...

#include "gui/editor/spectrumWorxEditor.hpp"
#include "gui/modules/moduleUI.hpp"

#include "le/parameters/lfo.hpp"
#include "le/parameters/printer.hpp"

#if LE_SW_SEPARATED_DSP_GUI
#include "core/modules/moduleGUI.hpp"
#else
#include "core/modules/moduleDSPAndGUI.hpp"
#endif // LE_SW_SEPARATED_DSP_GUI

#include "boost/config.hpp"
#if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
    #include "boost/polymorphic_cast.hpp"
#endif // no RTTI

#include "juce/juce_gui_basics/juce_gui_basics.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

// Implementation note:
//   Because no two windows can have focus at the same time and because JUCE
// does not support multithreaded GUI code, it is safe to use a static here even
// if there are multiple effect editor instances open.
//                                            (07.07.2011.) (Domagoj Saric)
/// \todo Verify this on the Mac.
///                                           (07.07.2011.) (Domagoj Saric)
ModuleControlBase * ModuleControlBase::pActiveControl( 0 );


bool ModuleControlBase::tryActivateControl() const
{
    if ( isActive() )
        return false;

    if ( juce::Component::getNumCurrentlyModalComponents() != 0 )
        return false;

    Theme::ModuleUIMouseOverReaction         const desiredReaction  ( Theme::settings().moduleUIMouseOverReaction     );
    juce::Component                  const * const pFocusedComponent( juce::Component::getCurrentlyFocusedComponent() );

    bool const nothingFocused        ( noModuleOrModuleControlFocused() );
    bool const parentFocused         ( pFocusedComponent == &moduleUI() );
    bool const parentOrNothingFocused( parentFocused | nothingFocused   ); // Intentional bitwise or as an optimization.
    bool const controlFocused        ( pFocusedComponent == &widget()   );
    if
    (
        ( controlFocused                                                                  ) ||
        ( desiredReaction == Theme::WhenParentOrNothingSelected && parentOrNothingFocused ) ||
        ( desiredReaction == Theme::WhenParentModuleSelected    && parentFocused          )
    )
        return true;
    else
        return false;
}


bool ModuleControlBase::reportActiveControl( double const minimum, double const maximum, double const interval )
{
    if ( tryActivateControl() )
    {
        /// \note The control is marked as activated only after the editor
        /// has been informed (and the LFO GUI created) to avoid race condition
        /// crashes when a user activates a control that is being automated (and
        /// SpectrumWorxEditor::updateActiveControlValue() gets called as a
        /// response to setParameter() before the LFO gets created).
        ///                                   (25.04.2013.) (Domagoj Saric)
        editor().moduleControlActivated( *this, minimum, maximum, interval );
        pActiveControl = this;
        return true;
    }
    return false;
}


bool LE_NOTHROW ModuleControlBase::reportInactiveControl() const
{
    if ( isActive() && !Detail::hasDirectFocus( widget() ) )
    {
        editor().moduleControlDectivated( *this );
        pActiveControl = nullptr;
        return true;
    }
    else
        return false;
}


void ModuleControlBase::moduleParameterChanged()
{
    BOOST_ASSERT( !ModuleUI::selectedModule() || ModuleUI::selectedModule() == &moduleUI()                              );
    BOOST_ASSERT( noModuleOrModuleControlFocused() || Detail::hasDirectFocus( widget() ) || moduleUI().hasDirectFocus() );
    BOOST_ASSERT( isActive()                                                                                            );
    BOOST_ASSERT( !isLFOEnabled()                                                                                       );

    /// \note Checkout revision 7493 or earlier for (more templated) code
    /// that directly accessed the effect's parameters.
    ///                                       (12.03.2013.) (Domagoj Saric)
    SpectrumWorxEditor & editor( this->editor() );
    std::uint8_t const parameterIndex( moduleParameterIndex() + 1 ); // Bypass
    editor.updateModuleParameterAndNotifyHost( moduleUI(), parameterIndex, getValue() );
    editor.updateActiveControlValue();
}


void ModuleControlBase::configureControl( bool const mouseClickCanGrabFocus )
{
    widget().setWantsKeyboardFocus          ( true                   );
    widget().setMouseClickGrabsKeyboardFocus( mouseClickCanGrabFocus );
}


bool ModuleControlBase::noModuleOrModuleControlFocused( SpectrumWorxEditor const & editor )
{
    // Implementation note:
    //   We ignore focused widgets on auxiliary windows.
    //                                        (07.07.2011.) (Domagoj Saric)
    juce::Component const * const pFocusedComponent( juce::Component::getCurrentlyFocusedComponent() );
    return
        ( pFocusedComponent == nullptr             ) ||
        ( pFocusedComponent == &editor             ) ||
        ( !editor.isParentOf( *pFocusedComponent ) );
}

bool ModuleControlBase::noModuleOrModuleControlFocused() const { return noModuleOrModuleControlFocused( editor() ); }


ModuleControlBase::Module & ModuleControlBase::module() { return moduleUI().module(); }


ModuleUI & ModuleControlBase::moduleUI()
{
    // Implementation note:
    //   Shared module controls are not children of ModuleUI instances so we
    // cannot just return
    // *boost::polymorphic_downcast<ModuleUI *>( widget().getParentComponent() ).
    //                                        (04.10.2011.) (Domagoj Saric)
    BOOST_ASSERT( pModuleUI_ );
    return *pModuleUI_;
}


SpectrumWorxEditor & ModuleControlBase::editor() const
{
#if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
    return *boost::polymorphic_downcast<SpectrumWorxEditor *>( moduleUI().getParentComponent() );
#else
    return *                static_cast<SpectrumWorxEditor *>( moduleUI().getParentComponent() );
#endif
}


LE::Parameters::LFOImpl                  & ModuleControlBase::lfo ()       { return module().lfo          ( moduleParameterIndex()     ); }
LE::Parameters::RuntimeInformation const & ModuleControlBase::info() const { return module().parameterInfo( moduleParameterIndex() + 1 ); }
char                               const * ModuleControlBase::name() const { return info().name; }


bool ModuleControlBase::isActive( juce::Component const & widget )
{
    return activeControl() && ( &widget == &activeControl()->widget() );
}


juce::String ModuleControlBase::getValueString( float const * LE_RESTRICT const pValue ) const
{
    std::array<char, 20> buffer;
    using Printer = Engine::ModuleParameters::ParameterPrinter;
    Printer const printer =
    {
        pValue ? *pValue : 0,
        pValue ? Printer::Unchanged : Printer::Internal,
        boost::make_iterator_range_n( &buffer[ 0 ], buffer.size() ),
        moduleUI().editor().engineSetup()
    };
    std::uint8_t const parameterIndex( moduleParameterIndex() + 1 /*Bypass*/ );
    char const * const pValueString( module().getParameterValueString ( parameterIndex, printer ) );
    char const * const pUnit       ( Automation::getParameterUnit( parameterIndex, &module()    ) );
    juce::String result( pValueString );
    result.appendCharPointer( juce::CharPointer_ASCII( pUnit ) );
    return result;
}


namespace
{
    // Cheap cross-casting between widget and ModuleControlBase instances
    class LE_NOVTABLE ControlWidgetBridge
        :
        public ModuleControlBase,
        public WidgetBase<>
    {
    public:
        static ModuleControlBase & asControl( juce::Component & widget )
        {
            LE_ASSUME( &widget );
            ModuleControlBase & control( static_cast<ControlWidgetBridge &>( widget ) );
            BOOST_ASSERT_MSG
            (
                &control == dynamic_cast<ModuleControlBase *>( &widget ),
                "Widget is not a module control."
            );
            LE_ASSUME( &control );
            return control;
        }

        static juce::Component & asWidget( ModuleControlBase & control )
        {
            LE_ASSUME( &control );
            juce::Component & widget( static_cast<ControlWidgetBridge &>( control ) );
            BOOST_ASSERT_MSG
            (
                &widget == dynamic_cast<juce::Component *>( &control ),
                "Module control detached from its widget."
            );
            LE_ASSUME( &widget );
            return widget;
        }

    private:
         ControlWidgetBridge();
        ~ControlWidgetBridge();
    }; // class ControlWidgetBridge
} // anonymous namespace

juce::Component   & ModuleControlBase::widget          (                          ) { return ControlWidgetBridge::asWidget ( *this  ); }
ModuleControlBase & ModuleControlBase::controlForWidget( juce::Component & widget ) { return ControlWidgetBridge::asControl( widget ); }


bool ModuleControlBase::isLFOEnabled() const { return lfo().enabled(); }


void ModuleControlBase::reassignTo( ModuleUI & moduleUI )
{
    BOOST_ASSERT_MSG
    (
        isASharedModuleControl(),
        "This functionality is meant only for SharedModuleControls where the "
        "controls are not actually owned by the ModuleUI and can thus be "
        "assigned to different module UIs."
    );
    pModuleUI_ = &moduleUI;
}

void ModuleControlBase::reassignTo( std::uint8_t const parameterIndex )
{
    BOOST_ASSERT_MSG
    (
        isASharedModuleControl(),
        "This functionality is meant only for SharedModuleControls where the "
        "frequency control can change the parameter it maps to."
    );
    parameterIndex_ = parameterIndex;
}


void ModuleControlBase::clearActiveControl()
{
    BOOST_ASSERT_MSG
    (
        isASharedModuleControl(),
        "This functionality is meant only for the "
        "SharedModuleControls::FrequencyRange class where the same "
        "ModuleControlBase instance can map to a different logical control."
    );
    pActiveControl = nullptr;
}


bool ModuleControlBase::isASharedModuleControl() const
{
    /// \note Shared module control widgets are not children of the
    /// corresponding ModuleUI instance (but rather of the singleton
    /// SharedModuleControls instance).
    ///                                       (12.02.2014.) (Domagoj Saric)
    return pModuleUI_ != widget().getParentComponent();
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
