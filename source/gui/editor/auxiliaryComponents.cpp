////////////////////////////////////////////////////////////////////////////////
///
/// auxiliaryComponents.cpp
/// -----------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "auxiliaryComponents.hpp"

#if LE_SW_SEPARATED_DSP_GUI
    #include "core/modules/moduleGUI.hpp"
#else
    #include "core/modules/moduleDSPAndGUI.hpp"
#endif // LE_SW_SEPARATED_DSP_GUI
#include "spectrumWorxEditor.hpp"

#include "le/math/math.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/parameters/printer.hpp"
#include "le/spectrumworx/effects/baseParameters.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/parentFromMember.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters { class LFOImpl; }
namespace SW
{
//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

using Effects::BaseParameters::Parameters;
using Effects::BaseParameters::Bypass;
using Effects::BaseParameters::Gain;
using Effects::BaseParameters::Wet;
using Effects::BaseParameters::StartFrequency;
using Effects::BaseParameters::StopFrequency;

using LE::Parameters::IndexOf;
using LE::Parameters::LFOImpl;

namespace Constants
{
    static std::uint8_t const startFrequencyIndex      = IndexOf<Parameters, StartFrequency>::value - 1 /*Bypass*/;
    static std::uint8_t const stopFrequencyIndex       = IndexOf<Parameters, StopFrequency >::value - 1 /*Bypass*/;
    static std::uint8_t const invalidIndex             = static_cast<std::uint8_t>( -1 );
    static std::uint8_t const startFrequencyThumbIndex = 1;
    static std::uint8_t const stopFrequencyThumbIndex  = 2;
} // namespace Constants


#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

SharedModuleControls::SharedModuleControls()
    :
    /// \note In order to enable a simple parameter-to-control (index) mapping,
    /// the control widgets must be added to the parent SharedModuleControls
    /// component in the correct order.
    ///                                       (11.02.2014.) (Domagoj Saric)
    gain_( *this, *ModuleUI::selectedModule(), 05, 4, IndexOf<Parameters, Gain>::value - 1 ), //...mrmlj...ModuleControlBase::moduleParameterIndex() excludes bypass...
    wet_ ( *this, *ModuleUI::selectedModule(), 75, 4, IndexOf<Parameters, Wet >::value - 1 )
{
    gain_.setupForParameter( resourceBitmap<SmallSymmetricKnobStrip>(), ModuleKnob::Fixed, Gain::discreteValueDistance );
    wet_ .setupForParameter( resourceBitmap<SmallLinearKnobStrip   >(), ModuleKnob::Fixed, Wet ::discreteValueDistance );

    addToParentAndShow( *this, frequencyRange_ );

    setBounds( 77, 80, 116, 79 );
    addToParentAndShow( editor(), *this );

    updateForEngineSetupChanges( editor().engineSetup() );
}

#pragma warning( pop )


void SharedModuleControls::updateForEngineSetupChanges( Engine::Setup const & setup )
{
    frequencyRange_.FrequencyRange::updateForEngineSetupChanges( setup );
    /// \note Gain and Wet are assumed to be non quantized.
    ///                                       (13.02.2014.) (Domagoj Saric)
}


void SharedModuleControls::updateForActiveModule()
{
    BOOST_ASSERT( ModuleUI::selectedModule() );

    ModuleUI & moduleUI( *ModuleUI::selectedModule() );

    gain_          .reassignTo( moduleUI );
    wet_           .reassignTo( moduleUI );
    frequencyRange_.reassignTo( moduleUI );

#if !LE_SW_SEPARATED_DSP_GUI && 0
    auto const & parameters( moduleUI.module().baseParameters() );

    gain_.ModuleKnob::setValue( parameters.get<Gain>() );
    wet_ .ModuleKnob::setValue( parameters.get<Wet >() );

    frequencyRange_.setStartValue( parameters.get<StartFrequency>() );
    frequencyRange_.setStopValue ( parameters.get<StopFrequency >() );
#else
    auto const & module( moduleUI.module() );

    gain_.ModuleKnob::setValue( module.getBaseParameter( IndexOf<Parameters, Gain>::value ) );
    wet_ .ModuleKnob::setValue( module.getBaseParameter( IndexOf<Parameters, Wet >::value ) );

    frequencyRange_.setStartValue( module.getBaseParameter( IndexOf<Parameters, StartFrequency>::value ) );
    frequencyRange_.setStopValue ( module.getBaseParameter( IndexOf<Parameters, StopFrequency >::value ) );
#endif // LE_SW_SEPARATED_DSP_GUI
}


ModuleControlBase & SharedModuleControls::controlForParameter( std::uint8_t const parameterIndex )
{
    using LE::Parameters::IndexOf;
    using namespace Effects::BaseParameters;
    typedef Effects::BaseParameters::Parameters BaseParams;
    switch ( parameterIndex )
    {
        case IndexOf<BaseParams, Gain          >::value: return gain_                         ;
        case IndexOf<BaseParams, Wet           >::value: return wet_                          ;
        case IndexOf<BaseParams, StartFrequency>::value: return frequencyRange_.startControl();
        case IndexOf<BaseParams, StopFrequency >::value: return frequencyRange_.stopControl ();

        case IndexOf<BaseParams, Bypass>::value: LE_UNREACHABLE_CODE();
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


void SharedModuleControls::focusLost( FocusChangeType )
{
    BOOST_ASSERT( ModuleUI::selectedModule() );
    ModuleUI & moduleUI( *ModuleUI::selectedModule() );
    if ( !moduleUI.hasFocus() )
        moduleUI.deactivate();
}


void SharedModuleControls::focusOfChildComponentChanged( FocusChangeType const type )
{
    if ( !hasFocus() )
        SharedModuleControls::focusLost( type );
}


SpectrumWorxEditor & SharedModuleControls::editor()
{
    return Utility::ParentFromOptionalMember<SpectrumWorxEditor, SharedModuleControls, &SpectrumWorxEditor::sharedModuleControls_, false>()( *this );
}

SpectrumWorxEditor const & SharedModuleControls::editor() const
{
    return const_cast<SharedModuleControls &>( *this ).editor();
}


#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

SharedModuleControls::FrequencyRange::FrequencyRange()
    :
    ModuleControlBase                    ( Constants::invalidIndex, *ModuleUI::selectedModule() ),
    parameterIndexForInternalWriteAccess_( Constants::invalidIndex                              )
{
    /// \note See the note in the Knob constructor.
    ///                                       (12.02.2014.) (Domagoj Saric)
    Knob::removeValueListeners( *this, valueListener() );

    setWantsKeyboardFocus( true );
    setSliderStyle ( juce::Slider::TwoValueHorizontal );
    setTextBoxStyle( juce::Slider::NoTextBox, true, 10, 12 );
    setBounds( 2, 36, 108, 34 );

    // Implementation note:
    //   See the note for the LFODisplay range control in
    // SpectrumWorxEditor.cpp.
    //                                        (07.10.2011.) (Domagoj Saric)
    setRange     ( StartFrequency::minimum(), StopFrequency::maximum(), 0 );
    setStartValue( StartFrequency::minimum() );
    setStopValue ( StopFrequency ::maximum() );
    //...mrmlj...setSkewFactor( ... );
}

#pragma warning( pop )


LE_NOTHROW
void SharedModuleControls::FrequencyRange::setValue( float const value )
{
    BOOST_ASSERT( canUseWriteAccessIndex() );
    switch ( parameterIndexForInternalWriteAccess_ )
    {
        case Constants::startFrequencyIndex: setStartValue( value ); break;
        case Constants::stopFrequencyIndex : setStopValue ( value ); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

LE_NOTHROWNOALIAS
float SharedModuleControls::FrequencyRange::getValue() const
{
    //...mrmlj...see the comment for parameterIndexForInternalWriteAccess_...
    BOOST_ASSERT( isThisTheGUIThread() );
    switch ( moduleParameterIndex() )
    {
        case Constants::startFrequencyIndex: return getStartValue();
        case Constants::stopFrequencyIndex : return getStopValue ();
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

LE_NOTHROW
void SharedModuleControls::FrequencyRange::updateForEngineSetupChanges( Engine::Setup const & engineSetup )
{
    typedef FrequencyRange::param_type value_type;
    value_type const nyquist                  ( engineSetup.sampleRate          <value_type>() / 2       );
    value_type const intervalForFrequencyKnobs( engineSetup.frequencyRangePerBin<value_type>() / nyquist );

    using namespace Effects::BaseParameters;
    this->setRange                 ( StartFrequency::minimum(), StartFrequency::maximum(), intervalForFrequencyKnobs );
    this->setSkewFactorFromMidPoint( 4000.0f / nyquist );
}

ModuleControlBase & SharedModuleControls::FrequencyRange::startControl()
{
    parameterIndexForInternalWriteAccess_ = Constants::startFrequencyIndex;
    return *this;
}

ModuleControlBase & SharedModuleControls::FrequencyRange::stopControl()
{
    parameterIndexForInternalWriteAccess_ = Constants::stopFrequencyIndex;
    return *this;
}


void SharedModuleControls::FrequencyRange::focusGained( juce::Component::FocusChangeType ) { reportActiveControl  (); }
void SharedModuleControls::FrequencyRange::focusLost  ( juce::Component::FocusChangeType ) { reportInactiveControl(); }


void SharedModuleControls::FrequencyRange::mouseEnter( juce::MouseEvent const & event ) noexcept
{
    updateSliderSelection( event );
    juce::Slider::mouseEnter( event );
}


void SharedModuleControls::FrequencyRange::mouseExit( juce::MouseEvent const & event ) noexcept
{
    reportInactiveControl();
    juce::Slider::mouseExit( event );
}


void SharedModuleControls::FrequencyRange::mouseDown( juce::MouseEvent const & event ) noexcept
{
    //...mrmlj...BOOST_ASSERT( hasFocus() == this->isActive() );
    updateSliderSelection( event );
    // Implementation note:
    //   The slider is disabled if the LFO for the activated control/thumb is
    // enabled in order to disable changing the parameter value through the GUI
    // (in juce::Slider mouseDown() or mouseDrag() member functions).
    //                                        (04.10.2011.) (Domagoj Saric)
    if ( getThumbBeingDragged() != Constants::invalidIndex )
    {
        setEnabled( !lfo().enabled() );
    }
    verifyThumbAndParameterIndicies();
    juce::Slider::mouseDown( event );
    verifyThumbAndParameterIndicies();
}


void SharedModuleControls::FrequencyRange::mouseUp( juce::MouseEvent const & event ) noexcept
{
    setEnabled( true );
    juce::Slider::mouseUp( event );
}


void SharedModuleControls::FrequencyRange::mouseMove( juce::MouseEvent const & event ) noexcept
{
    updateSliderSelection( event );
    juce::Slider::mouseMove( event );
}


void SharedModuleControls::FrequencyRange::paint( juce::Graphics & g )
{
    juce::Slider::paint( g );
    g.setFont           ( DrawableText::defaultFont() );
    g.drawSingleLineText( "Frequency range", 15, 32   );
}


void SharedModuleControls::FrequencyRange::valueChanged() noexcept
{
    BOOST_ASSERT( ModuleUI::selectedModule() );
    BOOST_ASSERT( &this->module() == &ModuleUI::selectedModule()->module() );
    /// \note juce::Slider might have updated the active thumb within its
    /// juce::Slider::mouseDown() handler before calling valueChanged().
    ///                                       (12.02.2014.) (Domagoj Saric)
    if ( this->moduleParameterIndex() != thumbToParameterIndex() )
        reportActiveControl();
    verifyThumbAndParameterIndicies();

    moduleParameterChanged();
}


LFOImpl & SharedModuleControls::FrequencyRange::lfo()
{
    auto & selectedModule( ModuleUI::selectedModule()->module() );
    auto & controlModule ( this                      ->module() );
    LE_ASSUME( &selectedModule == &controlModule );
    return controlModule.baseLFO( activeParameterIndex() );
}


void SharedModuleControls::FrequencyRange::reportActiveControl()
{
    BOOST_ASSERT( &parent().editor() == &this->editor() );

    std::uint8_t const currentParameterIndex( moduleParameterIndex() );
    std::uint8_t       newParameterIndex;
    char const * pName;
    switch ( getThumbBeingDragged() )
    {
        using namespace Constants;
        using LE::Parameters::Name;
        case startFrequencyThumbIndex: newParameterIndex = startFrequencyIndex; pName = Name<StartFrequency>::string_; break;
        case stopFrequencyThumbIndex : newParameterIndex = stopFrequencyIndex ; pName = Name<StopFrequency >::string_; break;
        default: return;
    }

    if
    (
        ( ModuleControlBase::activeControl() == this ) &&
        ( currentParameterIndex != newParameterIndex )
    )
    {
        /// \note DIRTY HACK:
        /// If FrequencyRange is already the active control widget
        /// ModuleControlBase::reportActiveControl() will skip activation even
        /// if FrequencyRange has internally changed the parameter it maps to.
        /// As a quick workaround we clear the active control in order to force
        /// reactivation (and updating of the editor window).
        ///                                   (12.02.2014.) (Domagoj Saric)
        clearActiveControl();
    }

    reassignTo( newParameterIndex );
    setName( pName );
    if ( !ModuleControlBase::reportActiveControl( StartFrequency::minimum(), StartFrequency::maximum(), 0 ) )
        reassignTo( currentParameterIndex );
        //...mrmlj...resetting the name should not be necessary
}


void SharedModuleControls::FrequencyRange::reportInactiveControl()
{
    if ( ModuleControlBase::reportInactiveControl() )
    {
        using Constants::invalidIndex;
        sliderBeingDragged()                  = invalidIndex;
        reassignTo                            ( invalidIndex );
        parameterIndexForInternalWriteAccess_ = invalidIndex;
        repaint();
    }
}


std::uint8_t SharedModuleControls::FrequencyRange::activeParameterIndex() const
{
    verifyThumbAndParameterIndicies();
    std::uint8_t const indexFromThumb  ( thumbToParameterIndex()      );
    std::uint8_t const indexFromControl( this->moduleParameterIndex() );
    LE_ASSUME( indexFromThumb == indexFromControl );
    return indexFromControl;
}


void SharedModuleControls::FrequencyRange::updateSliderSelection( juce::MouseEvent const & event )
{
    if ( ModuleControlBase::activeControl() && !this->isActive() )
    {
        sliderBeingDragged() = Constants::invalidIndex;
        return;
    }

    int const mousePos( event.x );

    unsigned int const startPosDistance( Math::abs( Math::convert<int>( getPositionOfValue( getMinValue() ) ) - mousePos ) );
    unsigned int const stopPosDistance ( Math::abs( Math::convert<int>( getPositionOfValue( getMaxValue() ) ) - mousePos ) );

    using namespace Constants;
    int  const newSliderSelection  ( ( startPosDistance < stopPosDistance ) ? startFrequencyThumbIndex : stopFrequencyThumbIndex );
    bool const activeControlChanged( newSliderSelection != getThumbBeingDragged() );

    if ( activeControlChanged )
    {
        sliderBeingDragged() = newSliderSelection;
        reportActiveControl();
        repaint();
    }
}


std::uint8_t SharedModuleControls::FrequencyRange::thumbToParameterIndex() const
{
    return static_cast<std::uint8_t>( getThumbBeingDragged() - 1 + Constants::startFrequencyIndex );
}


void SharedModuleControls::FrequencyRange::verifyThumbAndParameterIndicies() const
{
#ifndef NDEBUG
    std::uint8_t expectedParameterIndex;
    switch ( getThumbBeingDragged() )
    {
        case +1: expectedParameterIndex = Constants::startFrequencyIndex; break;
        case +2: expectedParameterIndex = Constants::stopFrequencyIndex ; break;
        case -1: expectedParameterIndex = Constants::invalidIndex       ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    std::uint8_t const actualParameterIndex( this->moduleParameterIndex() );
    BOOST_ASSERT_MSG( expectedParameterIndex == actualParameterIndex, "Thumb and parameter indicies out of sync." );
#endif // NDEBUG
}


bool SharedModuleControls::FrequencyRange::canUseWriteAccessIndex() const
{
    //...mrmlj...see the comment for parameterIndexForInternalWriteAccess_...
#if LE_SW_SEPARATED_DSP_GUI
    BOOST_ASSERT_MSG( isThisTheGUIThread(), "All calls are expected to be serialized to the GUI thread." );
    return true;
#else
    return ( !isThisTheGUIThread() || editor().presetLoadingInProgress() ) && ( parameterIndexForInternalWriteAccess_ != Constants::invalidIndex );
#endif
}


SharedModuleControls & SharedModuleControls::FrequencyRange::parent()
{
    return Utility::ParentFromMember<SharedModuleControls, FrequencyRange, &SharedModuleControls::frequencyRange_>()( *this );
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
