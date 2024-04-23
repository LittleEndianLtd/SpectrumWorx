////////////////////////////////////////////////////////////////////////////////
///
/// moduleUI.cpp
/// ------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "moduleUI.hpp"

#if LE_SW_SEPARATED_DSP_GUI
    #include "core/modules/moduleGUI.hpp"
#else
    #include "core/modules/moduleDSPAndGUI.hpp"
#endif // LE_SW_SEPARATED_DSP_GUI
#include "gui/editor/spectrumWorxEditor.hpp"

#include "le/parameters/lfo.hpp"
#include "le/parameters/printer.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/assert.hpp"
#include "boost/polymorphic_cast.hpp"

#include <cstdio>
#include <cstdlib>
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

ModuleLEDTextButton::ModuleLEDTextButton
(
    juce::Component & parent,
    unsigned int const x,
    unsigned int const y
)
    :
    LEDTextButton( parent, x, y, nullptr )
{
    setName( control().name() );
    //...mrmlj...for temporary test selection...
    setSize( resourceBitmap<ModuleComboOn>().getWidth(), getHeight() + 2 );
}


void ModuleLEDTextButton::clicked()
{
    moduleParameterChanged();
}


void ModuleLEDTextButton::mouseDown( juce::MouseEvent const & event )
{
    if ( !hasDirectFocus() )
    {
        grabKeyboardFocus();
    }
    else if ( !isLFOEnabled() )
    {
        LEDTextButton::mouseDown( event );
    }
}


void ModuleLEDTextButton::paintButton( juce::Graphics & g, bool const isMouseOverButton, bool const isButtonDown )
{
    if ( hasDirectFocus() )
        paintImage( g, resourceBitmap<ModuleComboOn>(), 0, -1 );
    g.setOrigin( 3, 1 );
    LEDTextButton::paintButton( g, isMouseOverButton, isButtonDown );
}


TriggerButton::TriggerButton
(
    juce::Component & parent,
    unsigned int const x,
    unsigned int const y
)
    :
    BitmapButton( parent, resourceBitmap<TriggerBtnOn>(), resourceBitmap<TriggerBtnOff>(), juce::Colours::transparentWhite, false )
{
    setName( control().name() );

    setBounds
    (
        x              , y,
        ModuleUI::width, getHeight() + 13
    );

    setTriggeredOnMouseDown( true );

    addToParentAndShow( parent, *this );
}


void TriggerButton::setValue( param_type const newValue )
{
    setState( newValue ? buttonDown : buttonNormal );
}


void TriggerButton::mouseDown( juce::MouseEvent const & e )
{
    if ( !hasDirectFocus() )
    {
        grabKeyboardFocus();
    }
    else if ( !isLFOEnabled() )
    {
        BitmapButton::mouseDown( e );
        moduleParameterChanged();
    }
}


void TriggerButton::mouseUp( juce::MouseEvent const & e ) noexcept
{
    if ( !isLFOEnabled() )
    {
        BitmapButton::mouseUp( e );
        moduleParameterChanged();
    }
}


void TriggerButton::paintButton( juce::Graphics & graphics, bool const isMouseOverButton, bool const isButtonDown )
{
    unsigned int const imageWidth ( 51 );
    unsigned int const imageHeight( 51 );
    BOOST_ASSERT( getCurrentImage().getWidth () == imageWidth  );
    BOOST_ASSERT( getCurrentImage().getHeight() == imageHeight );
    Detail::paintTextButton
    (
        *this, graphics,
        0, imageHeight + 2,
        ( ModuleUI::width - imageWidth ) / 2, 0,
        isMouseOverButton, isButtonDown
    );
    if ( this->hasDirectFocus() )
    {
        paintImage
        (
            graphics, resourceBitmap<ModuleKnobSelected>(),
            ( ModuleUI::width - imageWidth ) / 2 - 1, -1
        );
    }
}


ModuleKnob::ModuleKnob
(
    juce::Component &       parent,
    unsigned int      const x,
    unsigned int      const y
 )
    :
    Knob
    (
        parent,
        x,
        y,
        marginForGlow * 2,
        std::max<unsigned int>( marginForGlow * 2, spaceForText )
    ),
    pImageStrip_( nullptr )
{
    //...mrmlj...BOOST_ASSERT( imageStrip.getHeight() / numberOfKnobSubbitmaps == 50 );
    //...mrmlj...BOOST_ASSERT( imageStrip.getWidth ()                          == 50 );

    setScrollWheelEnabled( false );
}


void ModuleKnob::setupForParameter
(
    juce::Image const &       imageStrip      ,
    Quantization        const quantizationType,
    std::uint8_t        const quantizationStep
)
{
    auto const & info( control().info() );
    Knob::setupForParameter( info.name, imageStrip, info.default_ );
    //BOOST_ASSERT( !isLFOEnabled() ); //...mrmlj...when turning the GUI on or off...
    setDoubleClickReturnValue( !isLFOEnabled(), info.default_ );
    quantization_ = quantizationType;
    pImageStrip_  = &imageStrip;
    switch ( quantization_ )
    {
        case Fixed             : setRange( info.minimum, info.maximum, quantizationStep ); break;
        case FrequencyInHertz  : BOOST_ASSERT(                          quantizationStep == 1 ); break;
        case TimeInMilliseconds: BOOST_ASSERT( quantizationStep == 0 || quantizationStep == 1 ); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


void ModuleKnob::mouseDown( juce::MouseEvent const & event ) noexcept
{
    // Implementation note:
    //   In order for the base class to handle the mouseDown() event, the
    // control has to be disabled afterwards.
    //                                        (09.12.2011.) (Domagoj Saric)
    Knob::mouseDown( event );
    setEnabled( !isLFOEnabled() );
}


void ModuleKnob::mouseUp( juce::MouseEvent const & event ) noexcept
{
    BOOST_ASSERT( isEnabled() == !isLFOEnabled() );
    Knob::mouseUp( event );
    setEnabled( true );
}


void ModuleKnob::paint( juce::Graphics & graphics )
{
    unsigned int const imageWidth ( pImageStrip_->getWidth() );
    unsigned int const imageHeight( imageWidth               );

    if ( !control().isLFOEnabled() || Theme::shouldUpdateLFOControl( control() ) )
        Knob::paint( *pImageStrip_, marginForGlow, marginForGlow, graphics );
    else
        paintImage( graphics, resourceBitmap<ModuleKnobLFOed>(), marginForGlow, marginForGlow );
    if ( this->hasDirectFocus() )
    {
        juce::Image const & selection( imageWidth < 51 ? resourceBitmap<SmallModuleKnobSelected>() : resourceBitmap<ModuleKnobSelected>() );
        BOOST_ASSERT(           selection.getWidth()   == selection.getHeight() );
        BOOST_ASSERT( unsigned( selection.getWidth() ) == imageWidth + 2        );
        unsigned int const selectionWidth( imageWidth + 2                                      );
        unsigned int const xy            ( marginForGlow - ( selectionWidth - imageWidth ) / 2 );
        paintImage( graphics, selection, xy, xy );
    }

    graphics.setColour( juce::Colours::lightgrey );
    {
        juce::Font font( Theme::singleton().whiteFont() );
        font.setHeight( 10 );
        graphics.setFont( font );
    }
    graphics.drawFittedText
    (
        getName(),
        0, imageHeight + marginForGlow + ( marginForGlow / 2 ),
        getWidth(), 12,
        juce::Justification::horizontallyCentred,
        2,
        0.6f
    );
}


void ModuleKnob::valueChanged() noexcept
{
    BOOST_ASSERT( isMouseOverOrDragging() );
    moduleParameterChanged();
}


LE_NOTHROW
void ModuleKnob::lfoStateChanged()
{
    bool dontcare;
    double const defaultValue( getDoubleClickReturnValue( dontcare ) );
    setDoubleClickReturnValue( !isLFOEnabled(), defaultValue );
    syncMouseWheelAndLFOState();
}


LE_NOTHROW
void ModuleKnob::updateForEngineSetupChanges( Engine::Setup const & engineSetup )
{
    ModuleKnob::param_type quantization;
    switch ( quantization_ )
    {
        case Fixed             : return;
        case FrequencyInHertz  : quantization = engineSetup.frequencyRangePerBin<ModuleKnob::param_type>(); break;
        case TimeInMilliseconds: quantization = engineSetup.stepTime() * 1000                             ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    ParameterInfo const & parameterInfo( control().info() );
    param_type const minimum( parameterInfo.minimum );
    param_type const maximum( parameterInfo.maximum );
    using namespace Math::PositiveFloats;
    LE_ASSUME( minimum      >= 0 );
    LE_ASSUME( maximum      >  0 );
    LE_ASSUME( quantization >  0 );
    LE_ASSUME( maximum > quantization );
    bool const quantumAsMinimum( ( minimum < quantization ) && ( minimum != 0 ) );
    double const adjustedMinimum( quantumAsMinimum ? quantization : Math::convert<param_type>( ceil ( minimum / quantization ) ) * quantization );
    double const adjustedMaximum(                                   Math::convert<param_type>( floor( maximum / quantization ) ) * quantization );
    BOOST_ASSERT( adjustedMinimum >= minimum                                               );
    BOOST_ASSERT( adjustedMaximum <= maximum + 250 * std::numeric_limits<float>::epsilon() ); //...mrmlj...
    setRange( adjustedMinimum, adjustedMaximum, quantization );
}


void ModuleKnob::moduleControlActivated   () { syncMouseWheelAndLFOState();              }
void ModuleKnob::moduleControlDeactivated () { setScrollWheelEnabled( false           ); }
void ModuleKnob::syncMouseWheelAndLFOState() { setScrollWheelEnabled( !isLFOEnabled() ); }

#ifdef __GNUC__ //...mrmlj... GCC 4.6, Clang 2.8-3.2
    unsigned int const ModuleKnob::spaceForText/* = 18*/;
#endif // __GNUC__


DiscreteParameter::DiscreteParameter
(
    juce::Component & parent,
    unsigned int const x,
    unsigned int const y
)
    :
    ComboBox
    (
        parent,
        resourceBitmap<ModuleCombo  >(),
        resourceBitmap<ModuleComboOn>()
    )
{
    setName( control().name() );
    DiscreteParameter::setTopLeftPosition( x, y );
    BOOST_ASSERT( control().info().default_ == 0 );
}


void DiscreteParameter::mouseDown( juce::MouseEvent const & )
{
    if ( !hasDirectFocus() )
    {
        grabKeyboardFocus();
    }
    else if ( !isLFOEnabled() )
    {
        bool const valueChanged( ComboBox::showMenu() );
        if ( valueChanged )
            moduleParameterChanged();
    }
}


void DiscreteParameter::focusChanged()
{
    repaint();
}


ModuleUI * ModuleUI::pSelectedModule_( 0 );

#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

ModuleUI::ModuleUI()
    :
    bypass_( *this, resourceBitmap<ModuleMuted>(), resourceBitmap<ModuleOn>()                                            ),
    eject_ ( *this, resourceBitmap<Eject      >(), resourceBitmap<Eject   >(), juce::Colours::darkgrey.withAlpha( 0.4f ) )
{
    BOOST_ASSERT( isThisTheGUIThread() || juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager() );

    BOOST_ASSERT( resourceBitmap<ModuleBgSelected>().getWidth () == resourceBitmap<ModuleBg>().getWidth () );
    BOOST_ASSERT( resourceBitmap<ModuleBgSelected>().getHeight() == resourceBitmap<ModuleBg>().getHeight() );
    BOOST_ASSERT( resourceBitmap<ModuleBgSelected>().getWidth () == width                                  );
    BOOST_ASSERT( resourceBitmap<ModuleBgSelected>().getHeight() == height                                 );

    setSize( width, height );

    bypass_.setTopLeftPosition
    (
        ( ModuleUI::width / 2 ) - ( bypass_.getWidth() / 2 ),
        ModuleUI::height - 26 - resourceBitmap<ModuleOn>().getHeight()
    );

    eject_.setTopLeftPosition
    (
        ( ModuleUI::width - eject_.getWidth() ) / 2,
        -3
    );

    bypass_.addListener( this );
    eject_ .addListener( this );

    setMouseClickGrabsKeyboardFocus( true );
    setWantsKeyboardFocus          ( true );

    //...mrmlj...for testing...
    //juce::Desktop::getInstance().getAnimator().animateComponent
    //(
    //    this,
    //    juce::Rectangle<int>
    //    (
    //        myHorizontalOffset, verticalOffset,
    //        width             , height
    //    ),
    //    0, 200, false, 0, 0
    //);

    BOOST_ASSERT_MSG
    (
        unsigned( this->getNumChildComponents() ) == baseWidgets,
        "Unexpected number of child widgets at end of ModuleUI constructor."
    );
}

#pragma warning( pop )


LE_NOTHROW
ModuleUI::~ModuleUI()
{
    BOOST_ASSERT( isThisTheGUIThread() || juce::MessageManager::getInstance()->currentThreadHasLockedMessageManager() );

    if ( selectedModule() == this )
    {
        //...mrmlj...
        //BOOST_ASSERT( hasFocus() || editor()./*...mrmlj...sharedModuleControls().hasFocus()*/ sharedModuleControlsActive() );

        // Implementation note:
        //   Unforunately moveKeyboardFocusToSibling() does not just select the
        // next ModuleUI, if any, but also its 'first' control which is
        // undesired so we simply do nothing for now.
        //                                    (08.07.2011.) (Domagoj Saric)
        //moveKeyboardFocusToSibling( false );

        this->setWantsKeyboardFocus( false );
        editor().moduleDeactivated();
        pSelectedModule_ = nullptr;
    }
    else
    {
        BOOST_ASSERT( !hasFocus() );
    }

    fadeOutComponent( *this, 0, 600, true );
}


void ModuleUI::setUpForEffect
(
    char const * const effectName,
    char const * const effectDescription
)
{
    BOOST_ASSERT( getName()   .isEmpty() );
    BOOST_ASSERT( description_.isEmpty() );
    setName( effectName );
    description_ = effectDescription;
}


void ModuleUI::moveToSlot( std::uint8_t const slotIndex )
{
    std::uint16_t const myHorizontalOffset
    (
        horizontalOffset + slotIndex * ( width + distance )
    );
    setTopLeftPosition( myHorizontalOffset, verticalOffset );
}


void ModuleUI::paint( juce::Graphics & graphics )
{
    bool const isActive( selected() );
    graphics.setOpacity( isActive ? 1.0f : 0.5f );
    paintImage( graphics, isActive ? resourceBitmap<ModuleBgSelected>() : resourceBitmap<ModuleBg>() );
    graphics.setColour( Theme::singleton().blueColour() );
    graphics.drawHorizontalLine
    (
        height - 30,
        static_cast  <float>(              ModuleUI::border ),
        Math::convert<float>( getWidth() - ModuleUI::border )
    );

    graphics.setFont( Theme::singleton().whiteFont() );
    graphics.drawFittedText
    (
        getName(),
        3        , height - 30,
        width - 6,          28,
        juce::Justification::centred,
        3,
        0.6f
    );
}


void ModuleUI::mouseDrag( juce::MouseEvent const & event )
{
    if ( event.mods.isLeftButtonDown() )
        editor().moduleDrag( *this, event );
}


void ModuleUI::mouseUp( juce::MouseEvent const & event ) noexcept
{
    editor().moduleDragEnd( *this, event );
}


void ModuleUI::mouseEnter( juce::MouseEvent const & )
{
    if ( selectionTracksMouseMovements() )
        activate();
}


void ModuleUI::mouseExit( juce::MouseEvent const & event ) noexcept
{
    /// \note In some strange cases (e.g. while a ComboBox drop down menu is
    /// open and the mouse is moved over a module) JUCE will call mouseExit()
    /// without first calling mouseEnter().
    ///                                       (24.05.2012.) (Domagoj Saric)
    if ( !selectedModule() )
        return;

    if
    (
        selectionTracksMouseMovements() &&
        !juce::Rectangle<int>( 0, 0, width, height ).contains( event.x, event.y )
    )
        deactivate();
}


void ModuleUI::focusGained( FocusChangeType )
{
    activate();
    BOOST_ASSERT( selected() );
}


void ModuleUI::focusLost( FocusChangeType )
{
    // Implementation note:
    //   If only transferring focus to a subcontrol or to the shared controls do
    // not deactivate.
    //                                        (14.11.2011.) (Domagoj Saric)
    if ( hasFocus() || editor().sharedModuleControlsActiveAndFocused() )
        return;

    //...mrmlj...rethink this focus changing logic and assumptions
    //BOOST_ASSERT( selected() );
    if ( selected() )
        deactivate();
}


void ModuleUI::focusOfChildComponentChanged( FocusChangeType const changeType )
{
    if ( hasFocus() ) ModuleUI::focusGained( changeType );
    else              ModuleUI::focusLost  ( changeType );
}


void ModuleUI::activate()
{
    BOOST_ASSERT( hasFocus() || selectionTracksMouseMovements() );
    if ( this->selected() )
        return;

    // Implementation note:
    //   If the previously active module wasn't actually focused but the shared
    // controls it will not deactivate (and thus repaint) itself in the
    // focusLost() handler so a repaint must be forced here.
    //                                        (14.11.2011.) (Domagoj Saric)
    if ( selectedModule() )
        selectedModule()->repaint();

    pSelectedModule_ = this;
    editor().moduleActivated();
    repaint();
}


void ModuleUI::deactivate()
{
    BOOST_ASSERT(  selected() );
    BOOST_ASSERT( !hasFocus() );

    editor().moduleDeactivated();
    pSelectedModule_ = nullptr;
    repaint();
}


bool ModuleUI::selectionTracksMouseMovements() const
{
    return
        ( Theme::settings().moduleUIMouseOverReaction == Theme::WhenParentOrNothingSelected ) &&
        ModuleControlBase::noModuleOrModuleControlFocused( editor() );
}


namespace
{
    auto const bypassIndex = LE::Parameters::IndexOf<Effects::BaseParameters::Parameters, Effects::BaseParameters::Bypass>::value;

    void LE_FASTCALL setParameterControl( ModuleControlBase & control, float const parameterValue, ModuleUI::ParameterChangeSource const source )
    {
        if ( ( source != ModuleUI::LFOValue           ) || Theme::shouldUpdateLFOControl( control ) )
        {
            control.setValue( parameterValue );
        }
        if ( ( source == ModuleUI::AutomationOrPreset ) && control.isActive() )
        {
            SpectrumWorxEditor::fromChild( control.widget() ).updateActiveControlValue();
        }
    }
} // anonymous namespace

LE_NOTHROW
void ModuleUI::setBaseParameter( std::uint8_t const sharedParameterIndex, float const parameterValue, ParameterChangeSource const source )
{
    if ( sharedParameterIndex == bypassIndex )
    {
        LE_ASSUME( source == AutomationOrPreset );
        setBypass( Math::convert<bool>( parameterValue ) );
    }
    else
    {
    #if LE_SW_SEPARATED_DSP_GUI
        //module().Engine::ModuleParameters::setBaseParameter( sharedParameterIndex, parameterValue );
        if ( !getParentComponent() ) return;
    #endif // LE_SW_SEPARATED_DSP_GUI
        holdSharedControls( true );
        if ( selected() )
            setParameterControl( sharedControls().controlForParameter( sharedParameterIndex ), parameterValue, source );
        holdSharedControls( false );
    }
}



void ModuleUI::setEffectParameter( std::uint8_t const effectParameterIndex, float const parameterValue, ParameterChangeSource const source )
{
    setParameterControl( effectSpecificParameterControl( effectParameterIndex ), parameterValue, source );
}



void ModuleUI::setParameter( std::uint8_t const parameterIndex, float const parameterValue, ParameterChangeSource const source )
{
    if ( parameterIndex < Effects::BaseParameters::Parameters::static_size )
        setBaseParameter(                                                         parameterIndex  , parameterValue, source );
    else
        setEffectParameter( Engine::ModuleParameters::effectSpecificParameterIndex( parameterIndex ), parameterValue, source );
}


void ModuleUI::setBypass( bool const bypass )
{
    bypass_.setValue( bypass );
}


#if LE_SW_SEPARATED_DSP_GUI
float ModuleUI::getBaseParameter( std::uint8_t const parameterIndex ) const
{
    if ( parameterIndex == 0 ) return bypass();
    /// \note See the note for the ModuleParameters::baseParameters_ data
    /// member.
    ///                                       (20.10.2014.) (Domagoj Saric)
    //return editor().getModuleSharedParameter( module(), parameterIndex );
    return module().Engine::ModuleParameters::getBaseParameter( parameterIndex );
}


float ModuleUI::getEffectParameter( std::uint8_t const parameterIndex ) const
{
    return effectSpecificParameterControl( parameterIndex ).getValue();
}


bool ModuleUI::bypass() const { return bypass_.getValue(); }
#endif // LE_SW_SEPARATED_DSP_GUI

LE_NOTHROW LE_COLD
void ModuleUI::updateForEngineSetupChanges( Engine::Setup const & engineSetup )
{
    /// \note SharedModuleControls are updated in/by
    /// SpectrumWorxEditor::updateForEngineSetupChanges().
    ///                                       (13.02.2014.) (Domagoj Saric)
    std::uint8_t const numberOfControls( module().numberOfEffectSpecificParameters() );
    for ( std::uint8_t parameterIndex( 0 ); parameterIndex < numberOfControls; ++parameterIndex )
    {
        effectSpecificParameterControl( parameterIndex ).updateForEngineSetupChanges( engineSetup );
    }
}


void ModuleUI::updateLFOParameter( std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, Plugins::AutomatedParameterValue const value )
{
    //...mrmlj...value unused - updated from the LFO...
    editor().updateLFO( *this, parameterIndex, lfoParameterIndex, value );
}


void ModuleUI::buttonClicked( juce::Button * LE_RESTRICT const pButton )
{
    if ( pButton == &bypass_ )
    {
        float const value( Math::convert<float>( bypass_.getValue() ) );
        editor().updateModuleParameterAndNotifyHost( *this, bypassIndex, value );
    }
    else
    {
        BOOST_ASSERT( pButton == &eject_ );
        //...mrmlj...investigate why this doesn't work when placed inside the ModuleUI destructor...
        if ( ModuleControlBase::activeControl() && ( this == &ModuleControlBase::activeControl()->moduleUI() ) )
            editor().moduleControlDectivated( *ModuleControlBase::activeControl() );
        editor().removeModule( *this );
    }
}


SpectrumWorxEditor & ModuleUI::editor()
{
    BOOST_ASSERT_MSG( getParentComponent(), "ModuleUI detached from editor." );
    return *boost::polymorphic_downcast<SpectrumWorxEditor *>( getParentComponent() );
}

SpectrumWorxEditor const & ModuleUI::editor() const
{
    return const_cast<ModuleUI &>( *this ).editor();
}


SharedModuleControls & ModuleUI::sharedControls()
{
    BOOST_ASSERT_MSG( selected(), "Inactive modules do not have an active shared controls UI." );
    return editor().sharedModuleControls();
}


void ModuleUI::holdSharedControls( bool const doHold ) const
{
    BOOST_ASSERT( editor().holdSharedModuleControls_ != doHold );
    //...mmrlj...BOOST_ASSERT( selected() == editor().sharedModuleControlsActive() );
    editor().holdSharedModuleControls_ = doHold;
}


bool ModuleUI::sharedControlsLocked() const
{
    BOOST_ASSERT_MSG( selected(), "Inactive modules do not have an active shared controls UI." );
    return editor().holdSharedModuleControls_;
}


ModuleControlBase & ModuleUI::effectSpecificParameterControl( std::uint8_t const parameterIndex )
{
    std::uint8_t const actualChildIndex( parameterIndex + baseWidgets );
    BOOST_ASSERT_MSG( actualChildIndex < unsigned( this->getNumChildComponents() ), "Parameter index out of range." );
    juce::Component * LE_RESTRICT const pWidget( this->getChildComponent( actualChildIndex ) );
    LE_ASSUME( pWidget );
    return ModuleControlBase::controlForWidget( *pWidget );
}
ModuleControlBase const & ModuleUI::effectSpecificParameterControl( std::uint8_t const parameterIndex ) const
{
    return const_cast<ModuleUI &>( *this ).effectSpecificParameterControl( parameterIndex );
}


ModuleUI::Module       & ModuleUI::module()       { return Module::fromGUI( *this ); }
ModuleUI::Module const & ModuleUI::module() const { return const_cast<ModuleUI &>( *this ).module(); }

#if !LE_SW_SEPARATED_DSP_GUI
Utility::CriticalSectionLock ModuleUI::getProcessingLock() const { return editor().getProcessingLock(); }
#endif // LE_SW_SEPARATED_DSP_GUI

//------------------------------------------------------------------------------
namespace Detail
{
ModuleWidgetConstructionState::ModuleWidgetConstructionState( ModuleUI & parent )
    :
    parent        ( parent                                              ),
    yOffset       ( 14                                                  ),
    parameterIndex( Engine::ModuleParameters::numberOfLFOBaseParameters )
{}


EmptyWidgets::EmptyWidgets( ModuleWidgetConstructionState const & state )
{
    BOOST_ASSERT_MSG
    (
        state.yOffset < static_cast<unsigned int>( state.parent.getHeight() ),
        "You added more parameters/controls to the effect than can fit into its UI"
    );
    (void)state;
}


template <>
ModuleWidgetHolder<ModuleLEDTextButton>::ModuleWidgetHolder( ModuleWidgetConstructionState & state )
    :
    widget
    (
        state.parent,
        state.parent,
        ModuleUI::border,
        state.yOffset,
        state.parameterIndex++
    )
{
    state.yOffset += widget.getHeight() + 6;
}

template <>
ModuleWidgetHolder<TriggerButton>::ModuleWidgetHolder( ModuleWidgetConstructionState & state )
    :
    widget
    (
        state.parent,
        state.parent,
        0,
        state.yOffset + 4,
        state.parameterIndex++
    )
{
    state.yOffset += widget.getHeight() + 6;
}

template <>
ModuleWidgetHolder<DiscreteParameter>::ModuleWidgetHolder( ModuleWidgetConstructionState & state )
    :
    widget
    (
        state.parent,
        state.parent,
        ModuleUI::border,
        state.yOffset += 4,
        state.parameterIndex++
    )
{
    state.yOffset += widget.getHeight() + 4;
}

template <>
ModuleWidgetHolder<ModuleKnob>::ModuleWidgetHolder( ModuleWidgetConstructionState & state )
    :
    widget
    (
        state.parent,
        state.parent,
        ModuleUI::border,
        state.yOffset,
        state.parameterIndex++
    )
{
    state.yOffset += widget.getHeight();
    //...mrmlj...
    BOOST_ASSERT( resourceBitmap<ModuleKnobStrip>().getWidth() == 51 );
    state.yOffset += 51;
}

} // namespace Detail

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
