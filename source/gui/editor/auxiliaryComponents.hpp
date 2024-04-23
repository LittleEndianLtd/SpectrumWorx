////////////////////////////////////////////////////////////////////////////////
///
/// \file auxiliaryComponents.hpp
/// -----------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef auxiliaryComponents_hpp__66FD7192_4423_4467_8D99_99BF0FD87CB3
#define auxiliaryComponents_hpp__66FD7192_4423_4467_8D99_99BF0FD87CB3
#pragma once
//------------------------------------------------------------------------------
#include "gui/modules/moduleControl.hpp"
#include "gui/modules/moduleUI.hpp"

#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
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

////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class SharedModuleControls
////////////////////////////////////////////////////////////////////////////////

class SharedModuleControls
    :
    public WidgetBase<>
{
private:
    typedef ModuleControlImpl<ModuleKnob> Knob; //...mrmlj...are these logically "module knobs"?

public:
    class FrequencyRange final
        :
        public ModuleControlBase,
        public WidgetBase<juce::Slider>
    {
    public:
        FrequencyRange();

    public: // module control traits
        typedef float value_type;
        typedef float param_type;

        value_type LE_NOTHROWNOALIAS LE_FASTCALL getStartValue(                           ) const { return static_cast<value_type>( juce::Slider::getMinValue() );                                       }
        void       LE_NOTHROW        LE_FASTCALL setStartValue( param_type const newValue )       { juce::Slider::setMinValue( static_cast<value_type>( newValue ), juce::dontSendNotification, false ); }

        value_type LE_NOTHROWNOALIAS LE_FASTCALL getStopValue (                           ) const { return static_cast<value_type>( juce::Slider::getMaxValue() );                                        }
        void       LE_NOTHROW        LE_FASTCALL setStopValue ( param_type const newValue )       { juce::Slider::setMaxValue( static_cast<value_type>( newValue ), juce::dontSendNotification, false );  }

        void LE_FASTCALL updateForEngineSetupChanges( Engine::Setup const & ) override;

        ModuleControlBase & startControl();
        ModuleControlBase & stopControl ();

    protected: // ModuleControlBase overrides
        LE_NOTHROW void LE_FASTCALL lfoStateChanged() override {}

        LE_NOTHROW        void  LE_FASTCALL setValue( float value )       override;
        LE_NOTHROWNOALIAS float LE_FASTCALL getValue(             ) const override;

    private:
        void focusGained( juce::Component::FocusChangeType ) override;
        void focusLost  ( juce::Component::FocusChangeType ) override;

        void mouseDown ( juce::MouseEvent const & ) noexcept override;
        void mouseUp   ( juce::MouseEvent const & ) noexcept override;

        void mouseEnter( juce::MouseEvent const & ) noexcept override;
        void mouseExit ( juce::MouseEvent const & ) noexcept override;

        void mouseMove( juce::MouseEvent const & ) noexcept override;

        void valueChanged() noexcept override;

        void paint( juce::Graphics & ) override;

        LE_IMPLEMENT_ASYNC_REPAINT

    private:
        void reportActiveControl  ();
        void reportInactiveControl();

        void updateSliderSelection( juce::MouseEvent const & );

        std::uint8_t activeParameterIndex () const;
        std::uint8_t thumbToParameterIndex() const;
        void verifyThumbAndParameterIndicies() const;

        LFO       & lfo();
        LFO const & lfo() const { return const_cast<FrequencyRange &>( *this ).lfo(); }

        SharedModuleControls & parent();

    private:
        /// \note DIRTY HACK:
        /// Because the (single) FrequencyRange widget/control actually maps to
        /// two module parameters we need a way to discriminate between the two,
        /// that is to detect which parameter the widget is currently
        /// representing. The real problem is the fact that a FrequencyRange
        /// instance may need to map to both parameters at the same time: in the
        /// GUI thread the user is accessing the StartFrequency parameter while
        /// an LFO is changing the StopFrequency parameter from the processing
        /// thread.
        /// The first solution was to have two ModuleControl instances both
        /// mapping to the same widget (FrequencyRange) but this became no
        /// longer doable after the module widget/control code was refactored
        /// to allow a one-to-one static cross-casting between juce::Component
        /// and GUI::ModuleControlBase classes (see the implementation of the
        /// ModuleControlBase::controlForWidget() member function).
        /// The current solution uses the fact that concurrent access will
        /// always happen through different parts of the API:
        /// - GUI code will call ModuleControlBase::getValue() and change the
        /// "value"/position of the control directly through/from OS/JUCE events
        /// - internal/automation/preset/LFO code will use
        /// ModuleControlBase::setValue() and never call
        /// ModuleControlBase::getValue().
        /// This is why FrequencyRange::getValue() uses the
        /// ModuleControlBase::moduleParameterIndex() member functions while
        /// setValue() uses the below parameterIndexForInternalWriteAccess_
        /// data member.
        ///                                   (12.02.2014.) (Domagoj Saric)
        std::uint8_t parameterIndexForInternalWriteAccess_;
        bool canUseWriteAccessIndex() const;
    }; // class FrequencyRange

public:
    SharedModuleControls();

    void updateForEngineSetupChanges( Engine::Setup const & );

    void updateForActiveModule();

    ModuleControlBase & controlForParameter( std::uint8_t parameterIndex );

private:
    SpectrumWorxEditor       & editor()      ;
    SpectrumWorxEditor const & editor() const;

private: // JUCE Component overrides.
    void focusLost                   ( FocusChangeType ) override;
    void focusOfChildComponentChanged( FocusChangeType ) override;

private:
    Knob           gain_          ;
    Knob           wet_           ;
    FrequencyRange frequencyRange_;
}; // class SharedModuleControls

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // auxiliaryComponents_hpp
