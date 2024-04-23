////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleControl.hpp
/// -----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleControl_hpp__F2848AE1_5E1F_4FB9_81AB_541E17F40DD2
#define moduleControl_hpp__F2848AE1_5E1F_4FB9_81AB_541E17F40DD2
#pragma once
//------------------------------------------------------------------------------
#include "le/math/conversion.hpp"
#include "le/utility/cstdint.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters { class LFOImpl; struct RuntimeInformation; }
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class Module;
class ModuleGUI;

namespace Engine { class Setup; }

//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

class SpectrumWorxEditor;

class ModuleControlBase;
template <class ImplWidget>
class ModuleControlImpl;

template <class ImplWidget>
class LE_NOVTABLE ModuleControl
{
private:
    using Impl = ModuleControlImpl<ImplWidget>;

protected: // Utility functions
    bool isLFOEnabled          () const { return impl().ModuleControlBase::isLFOEnabled          (); } //...mrmlj...try to access the effect-specific lfo directly...
    void moduleParameterChanged()       {        impl().ModuleControlBase::moduleParameterChanged(); }

    ModuleControlBase & control() { /*BOOST_ASSERT( &ModuleControlBase::controlForWidget( impl() ) == &impl() );*/ return impl(); }

protected: // Default implementations for the module control interface
    static bool const mouseClickCanGrabFocus = false;

protected: // Default implementations for the module control interface callbacks
    static void lfoStateChanged         () {};

    static void focusChanged            () {}

    static void moduleControlActivated  () {}
    static void moduleControlDeactivated() {}

    static void updateForEngineSetupChanges( Engine::Setup const & ) {}

private:
    Impl       & impl()       { LE_MSVC_SPECIFIC( LE_ASSUME( this != 0 ) ); return static_cast<Impl       &>( *this ); }
    Impl const & impl() const { LE_MSVC_SPECIFIC( LE_ASSUME( this != 0 ) ); return static_cast<Impl const &>( *this ); }
}; // class ModuleControl


class ModuleUI;

////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleControlBase
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE ModuleControlBase
{
protected:
    ModuleControlBase( std::uint8_t const parameterIndex, ModuleUI & moduleUI )
        : parameterIndex_( parameterIndex ), pModuleUI_( &moduleUI ) {}

    // Implementation note:
    //   ModuleControls do not need to explicitly deactivate themselves in the
    // destructor because all the info for the corresponding ModuleUI will be
    // erased from the main window when the parent ModuleUI deactivates in its
    // destructor.
    //                                        (14.11.2011.) (Domagoj Saric)

public:
    virtual LE_NOTHROW        void  LE_FASTCALL setValue( float )       = 0;
    virtual LE_NOTHROWNOALIAS float LE_FASTCALL getValue(       ) const = 0;

    virtual LE_NOTHROW void LE_FASTCALL lfoStateChanged() = 0;

    virtual LE_NOTHROW void LE_FASTCALL updateForEngineSetupChanges( Engine::Setup const & ) {}

    // Implementation note:
    //   The names (and signatures) of these two functions were chosen to match
    // the existing virtual functions of the Knob widget class (in order to
    // reuse them more easily).
    //                                        (07.07.2011.) (Domagoj Saric)
    juce::String getValueText    (                   ) const { return getValueString( nullptr ); }
    juce::String getTextFromValue( float const value ) const { return getValueString( &value  ); }

    juce::String getValueString( float const * LE_RESTRICT pValue ) const;

    Parameters::RuntimeInformation const & info() const;
    char                           const * name() const;

    //...mrmlj...excludes non-lfoable parameters (Bypass)...
    std::uint8_t moduleParameterIndex() const { return parameterIndex_; }

    bool isActive() const { return this == activeControl(); }

    bool isLFOEnabled() const;

    void moduleParameterChanged();

#if LE_SW_SEPARATED_DSP_GUI
    using Module = SW::ModuleGUI;
#else
    using Module = SW::Module   ;
#endif // LE_SW_SEPARATED_DSP_GUI

    using LFO = Parameters::LFOImpl;

    LFO                      & lfo     ();
    LFO                const & lfo     () const { return const_cast<ModuleControlBase &>( *this ).lfo     (); }
    Module                   & module  ();
    Module             const & module  () const { return const_cast<ModuleControlBase &>( *this ).module  (); }
    juce::Component          & widget  ();
    juce::Component    const & widget  () const { return const_cast<ModuleControlBase &>( *this ).widget  (); }
    ModuleUI                 & moduleUI();
    ModuleUI           const & moduleUI() const { return const_cast<ModuleControlBase &>( *this ).moduleUI(); }
    SpectrumWorxEditor       & editor  () const;

    static ModuleControlBase & controlForWidget( juce::Component & );

    static ModuleControlBase * activeControl() { return pActiveControl; }

    static bool isActive( juce::Component const & );

           bool noModuleOrModuleControlFocused(                            ) const; //...mrmlj...
    static bool noModuleOrModuleControlFocused( SpectrumWorxEditor const & )      ;

protected:
    bool            reportActiveControl  ( double minimum, double maximum, double interval )      ;
    bool LE_NOTHROW reportInactiveControl(                                                 ) const;

    void LE_FASTCALL configureControl( bool mouseClickCanGrabFocus );

private:
    /// \note DIRTY HACKS:
    /// See the note in the
    /// SharedModuleControls::FrequencyRange::reportActiveControl() member
    /// function.
    ///                                       (12.02.2014.) (Domagoj Saric)
    friend class SharedModuleControls;
    void clearActiveControl();

    void reassignTo( ModuleUI &                  );
    void reassignTo( std::uint8_t parameterIndex );

    bool isASharedModuleControl() const;

private:
    bool LE_NOTHROW tryActivateControl() const;

private:
    std::uint8_t               parameterIndex_;
    ModuleUI     * LE_RESTRICT pModuleUI_     ;

    static ModuleControlBase * pActiveControl;
}; // class ModuleControlBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleControlImpl
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

template <class ImplWidget>
class ModuleControlImpl LE_SEALED
    :
    public ModuleControlBase,
    public ImplWidget
{
public:
    using BaseWidget = typename ImplWidget::BaseWidget;

public:
    ModuleControlImpl
    (
        juce::Component &       parent,
        ModuleUI        &       moduleUI,
        std::uint16_t     const x,
        std::uint16_t     const y,
        std::uint8_t      const parameterIndex
    )
        :
        ModuleControlBase( parameterIndex, moduleUI ),
        ImplWidget       ( parent, x, y             )
    {
        configureControl( ImplWidget::mouseClickCanGrabFocus );
    }

    using ModuleControlBase::lfo;
    using ModuleControlBase::moduleParameterChanged;

    using ImplWidget::setValue;
    using ImplWidget::getValue;

    LE_NOTHROW        void  LE_FASTCALL setValue( float const value )       LE_OVERRIDE { return                       ImplWidget::setValue( Math::convert<typename BaseWidget::param_type>( value ) )  ; }
    LE_NOTHROWNOALIAS float LE_FASTCALL getValue(                   ) const LE_OVERRIDE { return Math::convert<float>( ImplWidget::getValue(                                                ) ); }

    LE_NOTHROW void LE_FASTCALL updateForEngineSetupChanges( Engine::Setup const & engineSetup ) LE_OVERRIDE { ImplWidget::updateForEngineSetupChanges( engineSetup ); }

private:
    LE_NOTHROW void LE_FASTCALL lfoStateChanged() LE_OVERRIDE { ImplWidget::lfoStateChanged(); }

    void reportActiveControl()
    {
        bool const activated( ModuleControlBase::reportActiveControl( ImplWidget::valueRangeMinimum(), ImplWidget::valueRangeMaximum(), ImplWidget::valueRangeQuantum() ) );
        if ( activated )
            ImplWidget::moduleControlActivated();
    }

    void reportInactiveControl()
    {
        bool const deactivated( ModuleControlBase::reportInactiveControl() );
        if ( deactivated )
            ImplWidget::moduleControlDeactivated();
    }

    bool isActive() const { return this == activeControl(); }

private:
    virtual void focusGained( juce::Component::FocusChangeType )          LE_OVERRIDE { BOOST_ASSERT( this->getWantsKeyboardFocus() ); reportActiveControl  (); ImplWidget::focusChanged(); }
    virtual void focusLost  ( juce::Component::FocusChangeType ) noexcept LE_OVERRIDE { BOOST_ASSERT( this->getWantsKeyboardFocus() ); reportInactiveControl(); ImplWidget::focusChanged(); }

    virtual void mouseEnter( juce::MouseEvent const & )          LE_OVERRIDE { reportActiveControl  (); }
    virtual void mouseExit ( juce::MouseEvent const & ) noexcept LE_OVERRIDE { reportInactiveControl(); }
}; // class ModuleControlImpl

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleControl_hpp
