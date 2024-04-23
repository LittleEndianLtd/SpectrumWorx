////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleUI.hpp
/// ------------------
///
/// Module UI related functionality.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleUI_hpp__8228E5F3_535E_4B08_9AD0_072C9fA7AD93
#define moduleUI_hpp__8228E5F3_535E_4B08_9AD0_072C9fA7AD93
#pragma once
//------------------------------------------------------------------------------
#include "gui/gui.hpp"
#include "gui/modules/moduleControl.hpp"

#include "le/math/conversion.hpp"
#include "le/parameters/linear/parameter.hpp"
#include "le/parameters/boolean/tag.hpp"
#include "le/parameters/enumerated/tag.hpp"
#include "le/parameters/powerOfTwo/tag.hpp"
#include "le/parameters/trigger/tag.hpp"
#include "le/parameters/symmetric/tag.hpp"
#include "le/parameters/printer_fwd.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tchar.hpp"

#include "boost/mpl/fold.hpp"
#include "boost/polymorphic_cast.hpp"

#include "juce/juce_gui_basics/juce_gui_basics.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility { class CriticalSectionLock; }
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class Module;
class ModuleGUI;

namespace Effects
{
    namespace Detail
    {
        struct EmptyParameters;
    }

    namespace BaseParameters
    {
        class Bypass        ;
        class Gain          ;
        class Wet           ;
        class StartFrequency;
        class StopFrequency ;
    }
}

namespace Engine
{
    class Setup;
}

//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleKnob
///
////////////////////////////////////////////////////////////////////////////////

class ModuleUI;

class LE_NOVTABLE ModuleKnob
    :
    public Knob,
    public ModuleControl<ModuleKnob>
{
protected:
    ModuleKnob
    (
        juce::Component & parent,
        unsigned int      x,
        unsigned int      y
    );

public:
    #pragma warning( push )
    #pragma warning( disable : 4480 ) // Nonstandard extension used: specifying underlying type for enum 'SW::Effects::PhaseVocoderShared::pitchShiftAndScale::TransientBins'.
    enum Quantization : std::uint8_t
    {
        Fixed,
        FrequencyInHertz,
        TimeInMilliseconds
    };
    #pragma warning( pop )

private:
    typedef boost::mpl::string<' Hz'> Hertz;
    typedef boost::mpl::string<' ms'> Millisecond;

    template <typename Unit> struct QuantizationImpl;

public:
    template <class Parameter>
    struct QuantizationFor
        :
        QuantizationImpl
        <
            typename LE::Parameters::Detail::GetTraitDefaulted
            <
                LE::Parameters::Traits::Tag::Unit,
                typename Parameter::Traits,
                typename Parameter::Defaults
            >::type
        >
    {}; // struct QuantizationFor

    void LE_FASTCALL setupForParameter
    (
        juce::Image const & imageStrip      ,
        Quantization        quantizationType,
        std::uint8_t        quantizationStep
    );

private: // juce::Component overrides
    void mouseDown ( juce::MouseEvent const & ) noexcept LE_OVERRIDE;
    void mouseUp   ( juce::MouseEvent const & ) noexcept LE_OVERRIDE;

    void valueChanged() noexcept LE_OVERRIDE;

    void paint( juce::Graphics & ) LE_OVERRIDE;

protected: // ModuleControl interface.
    void LE_FASTCALL lfoStateChanged();

    void LE_FASTCALL updateForEngineSetupChanges( Engine::Setup const & );

    void moduleControlActivated  ();
    void moduleControlDeactivated();

    double valueRangeMinimum() const { return getMinimum (); }
    double valueRangeMaximum() const { return getMaximum (); }
    double valueRangeQuantum() const { return getInterval(); }

    static bool const mouseClickCanGrabFocus = true;

public:
    typedef Knob BaseWidget;

private:
    void syncMouseWheelAndLFOState();

private:
    Quantization                    quantization_;
    juce::Image const * LE_RESTRICT pImageStrip_ ;

private:
    static unsigned int const marginForGlow =  4;
    static unsigned int const spaceForText  = 18;
}; // class ModuleKnob

template <typename Unit> struct ModuleKnob::QuantizationImpl                          { static Quantization const value = Fixed             ; };
template <             > struct ModuleKnob::QuantizationImpl<ModuleKnob::Hertz      > { static Quantization const value = FrequencyInHertz  ; };
template <             > struct ModuleKnob::QuantizationImpl<ModuleKnob::Millisecond> { static Quantization const value = TimeInMilliseconds; };


////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleLEDTextButton
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE ModuleLEDTextButton
    :
    public LEDTextButton,
    public ModuleControl<ModuleLEDTextButton>
{
protected:
    ModuleLEDTextButton
    (
        juce::Component & parent,
        unsigned int x,
        unsigned int y
    );

private: // juce::Component overrides
    void mouseDown( juce::MouseEvent const & ) LE_OVERRIDE;
    void paintButton( juce::Graphics &, bool isMouseOverButton, bool isButtonDown ) LE_OVERRIDE;

protected: // ModuleControl interface.
    void focusChanged() { repaint(); }

    // Implementation note:
    //   We allow a smooth LFO range control for boolean parameters.
    //                                        (21.07.2011.) (Domagoj Saric)
    static double valueRangeQuantum() { return 0; }

    using BitmapButton::getTextFromValue;
    char const * getValueText() const { return getTextFromValue( getValue() ); }

public:
    typedef BitmapButton BaseWidget;

private:
    void clicked() LE_OVERRIDE;
}; // class ModuleLEDTextButton


////////////////////////////////////////////////////////////////////////////////
///
/// \class TriggerButton
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE TriggerButton
    :
    public BitmapButton,
    public ModuleControl<TriggerButton>
{
protected:
    TriggerButton
    (
        juce::Component & parent,
        unsigned int x,
        unsigned int y
    );

public:
    value_type getValue(            ) const { return isDown(); }
    void       setValue( param_type );

protected: // ModuleControl interface.
    void lfoStateChanged() { setValue( false ); }
    void focusChanged   () { repaint(); }

    // Implementation note:
    //   We allow a smooth LFO range control for boolean parameters.
    //                                        (21.07.2011.) (Domagoj Saric)
    static double valueRangeQuantum() { return 0; }

    using BitmapButton::getTextFromValue;
    char const * getValueText() const { return getTextFromValue( getValue() ); }

public:
    typedef TriggerButton BaseWidget;

private: // juce::Component overrides
    void mouseDown( juce::MouseEvent const & )          LE_OVERRIDE;
    void mouseUp  ( juce::MouseEvent const & ) noexcept LE_OVERRIDE;

    void paintButton( juce::Graphics &, bool isMouseOverButton, bool isButtonDown ) LE_OVERRIDE;
}; // class TriggerButton


////////////////////////////////////////////////////////////////////////////////
///
/// \class DiscreteParameter
///
/// \brief Module UI widget for parameters with special discrete values.
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE DiscreteParameter
    :
    public ComboBox,
    public ModuleControl<DiscreteParameter>
{
protected:
    DiscreteParameter
    (
        juce::Component & parent,
        unsigned int x,
        unsigned int y
    );

private:
    void mouseDown( juce::MouseEvent const & ) LE_OVERRIDE;

protected: // ModuleControl interface.
    void focusChanged();

    juce::String const & getTextFromValue( value_type const valueIndex ) const { return getItemText( valueIndex ); }
    juce::String const & getValueText    (                             ) const { return getSelectedItemText();     }

    double valueRangeMaximum() const { return Math::convert<double>( numberOfItems() - 1 ); }

public:
    typedef ComboBox BaseWidget;

private:
    static unsigned int const horizontalMargin =  8;
    static unsigned int const textHeight       = 11;
}; // class DiscreteParameter


////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleUI
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    template <typename ParameterTag> struct WidgetForParameterAux                { typedef ModuleKnob          type; };
    template <> struct WidgetForParameterAux<Parameters::BooleanParameterTag   > { typedef ModuleLEDTextButton type; };
    template <> struct WidgetForParameterAux<Parameters::EnumeratedParameterTag> { typedef DiscreteParameter   type; };
    template <> struct WidgetForParameterAux<Parameters::TriggerParameterTag   > { typedef TriggerButton       type; };

    template <class Parameter>
    using WidgetForParameter = WidgetForParameterAux<typename Parameter::Tag>;
} // namespace Detail

class SharedModuleControls;
class SpectrumWorxEditor;

class ModuleUI
#if !LE_SW_SEPARATED_DSP_GUI
LE_SEALED
#endif // LE_SW_SEPARATED_DSP_GUI
    :
    public  WidgetBase<>,
    private juce::ButtonListener
{
public:
    enum ParameterChangeSource
    {
        AutomationOrPreset,
        LFOValue
    }; // enum ParameterChangeSource

    void LE_FASTCALL setBaseParameter  ( std::uint8_t sharedParameterIndex, float parameterValue, ParameterChangeSource );
    void LE_FASTCALL setEffectParameter( std::uint8_t effectParameterIndex, float parameterValue, ParameterChangeSource );
    void LE_FASTCALL setParameter      ( std::uint8_t       parameterIndex, float parameterValue, ParameterChangeSource );

    void LE_FASTCALL setBypass( bool );

#if LE_SW_SEPARATED_DSP_GUI
    bool  LE_FASTCALL bypass() const;
    float LE_FASTCALL getBaseParameter  ( std::uint8_t parameterIndex ) const;
    float LE_FASTCALL getEffectParameter( std::uint8_t parameterIndex ) const;
#endif // LE_SW_SEPARATED_DSP_GUI

    void LE_FASTCALL updateForEngineSetupChanges( Engine::Setup const & );

    void LE_FASTCALL updateLFOParameter( std::uint8_t parameterIndex, std::uint8_t lfoParameterIndex, float/*Parameters::RuntimeInformation::value_type*/ value );

public:
    juce::String const & description() const { return description_; }

    SpectrumWorxEditor         & editor        ()      ;
    SpectrumWorxEditor   const & editor        () const;
    SharedModuleControls       & sharedControls()      ;

#if LE_SW_SEPARATED_DSP_GUI
    static boost::none_t processingLock() { return boost::none; }
#else
    Utility::CriticalSectionLock getProcessingLock() const; //...mrmlj...quick-fix...
#endif // LE_SW_SEPARATED_DSP_GUI

    //...mrmlj...quick-fix...
    void holdSharedControls( bool doHold ) const;
    bool sharedControlsLocked() const;

#if LE_SW_SEPARATED_DSP_GUI
    typedef SW::ModuleGUI Module;
#else
    typedef SW::Module    Module;
#endif // LE_SW_SEPARATED_DSP_GUI

    Module       & module()      ;
    Module const & module() const;

    static ModuleUI * selectedModule() { return pSelectedModule_; }

    bool selected() const { return this == selectedModule(); }

public:
                ModuleUI();
    LE_NOTHROW ~ModuleUI();

    void setUpForEffect
    (
        char const * effectName,
        char const * effectDescription
    );

    void LE_FASTCALL moveToSlot( std::uint8_t slotIndex );

    ModuleControlBase       & effectSpecificParameterControl( std::uint8_t parameterIndex )      ;
    ModuleControlBase const & effectSpecificParameterControl( std::uint8_t parameterIndex ) const;

private: friend class SpectrumWorxEditor; friend class SharedModuleControls; //...mrmlj...
    void activate  ();
    void deactivate();
    bool selectionTracksMouseMovements() const;

private: // JUCE Component overrides.
    void paint( juce::Graphics & ) LE_OVERRIDE;

    void mouseDrag ( juce::MouseEvent const & )          LE_OVERRIDE;
    void mouseEnter( juce::MouseEvent const & )          LE_OVERRIDE;
    void mouseExit ( juce::MouseEvent const & ) noexcept LE_OVERRIDE;
    void mouseUp   ( juce::MouseEvent const & ) noexcept LE_OVERRIDE;

    void focusGained                 ( FocusChangeType ) LE_OVERRIDE;
    void focusLost                   ( FocusChangeType ) LE_OVERRIDE;
    void focusOfChildComponentChanged( FocusChangeType ) LE_OVERRIDE;

private: // JUCE ButtonListener overrides.
    void buttonClicked( juce::Button * ) LE_OVERRIDE;

private:
    BitmapButton bypass_;
    BitmapButton eject_ ;

    juce::String description_;

public:
    static std::uint8_t  const horizontalOffset = 213;
    static std::uint8_t  const verticalOffset   =   9;
    static std::uint16_t const height           = 358;
    static std::uint8_t  const width            =  68;
    static std::uint8_t  const distance         =   0;
    static std::uint8_t  const border           =   4;

    static std::uint8_t const baseWidgets = 2;

private:
    static ModuleUI * pSelectedModule_;
}; // class ModuleUI


namespace Detail ///< \internal
{
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class ModuleWidgetConstructionState
    ///
    ////////////////////////////////////////////////////////////////////////////

    struct ModuleWidgetConstructionState
    {
    public:
        ModuleWidgetConstructionState( ModuleUI & parent );

        ModuleUI              & parent        ;
        mutable std::uint16_t   yOffset       ;
        mutable std::uint8_t    parameterIndex;

    private:
        ModuleWidgetConstructionState( ModuleWidgetConstructionState const & );
        void operator=               ( ModuleWidgetConstructionState const & );
    }; // struct ModuleWidgetConstructionState


    template <class Widget>
    struct ModuleWidgetHolder
    {
        ModuleWidgetHolder( ModuleWidgetConstructionState & );

        ModuleControlImpl<Widget> widget;
    }; // struct ModuleWidgetHolder

#ifdef __clang__ //...mrmlj...ambiguity compilation errors...
    template <typename Parameter>
    struct ParameterWidgetHolder : ModuleWidgetHolder<typename WidgetForParameter<Parameter>::type>
    {
        ParameterWidgetHolder( ModuleWidgetConstructionState & state )
            :
            ModuleWidgetHolder<typename WidgetForParameter<Parameter>::type>( state ) {}
    };
#endif // __clang__

    template <typename Parameter>
    struct ParameterWidget
    {
    #ifdef __clang__
        typedef ParameterWidgetHolder<Parameter>                                 type;
    #else
        typedef ModuleWidgetHolder<typename WidgetForParameter<Parameter>::type> type;
    #endif // __clang__
    }; // struct ParameterWidget

    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class WidgetInitialiser
    ///
    ////////////////////////////////////////////////////////////////////////////

    struct WidgetInitialiser
    {
        template <class Parameter, class Widget>
        static void setup( Widget const & ) {}

        template <class Parameter>
        static void setup( ModuleControlImpl<DiscreteParameter> & comboBox )
        {
            fillComboBoxForParameter<Parameter>( comboBox );
        }

        template <class Parameter>
        static void setup( ModuleControlImpl<ModuleKnob> & knob )
        {
            knob.setupForParameter
            (
                std::is_base_of<LE::Parameters::SymmetricParameterTag, typename Parameter::Tag>::value
                    ? resourceBitmap<SymmetricKnobStrip>()
                    : resourceBitmap<ModuleKnobStrip   >(),
                ModuleKnob::QuantizationFor<Parameter>::value,
                Parameter::discreteValueDistance
            );
        }
    }; // struct WidgetInitialiser


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class EmptyWidgets
    ///
    ////////////////////////////////////////////////////////////////////////////

    struct EmptyWidgets
    {
        EmptyWidgets( ModuleWidgetConstructionState const & );
        static void setup( WidgetInitialiser const & ) {}

        static LE_NOTHROWRESTRICTNOALIAS void * operator new   ( std::size_t               const count      , void * LE_RESTRICT const pStorage     ) { (void)count; LE_ASSUME( pStorage ); return pStorage; }
        static LE_NOTHROWNOALIAS         void   operator delete( void        * LE_RESTRICT const /*pObject*/, void * LE_RESTRICT const /*pStorage*/ ) {}
    }; // struct EmptyWidgets


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class WidgetsStorage
    ///
    ////////////////////////////////////////////////////////////////////////////

    #pragma warning( push )
    #pragma warning( disable : 4584 ) // base-class <> is already a base-class of WidgetsStorage
    template <typename PreviousWidgets, typename Parameter>
    struct WidgetsStorage
        :
        PreviousWidgets,
        ParameterWidget<Parameter>::type
    {
        WidgetsStorage( ModuleWidgetConstructionState & state )
            :
            PreviousWidgets                 ( state ),
            ParameterWidget<Parameter>::type( state )
        {}

        void setup( WidgetInitialiser const & initialiser )
        {
            PreviousWidgets::setup( initialiser );
            initialiser.setup<Parameter>( ParameterWidget<Parameter>::type::widget );
        }
    }; // struct WidgetsStorage
    #pragma warning( pop )
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
///
/// \class ParameterWidgets
///
////////////////////////////////////////////////////////////////////////////////

template <class ParametersParam>
class ParameterWidgets
{
public:
    typedef ParametersParam Parameters;

    typedef typename boost::mpl::fold
    <
        Parameters,
        Detail::EmptyWidgets,
        Detail::WidgetsStorage<boost::mpl::_1, boost::mpl::_2>
    >::type Container;

public:
#ifndef NDEBUG
     ParameterWidgets() : constructed_( false ) {}
    ~ParameterWidgets() { BOOST_ASSERT( !constructed_ ); }
#endif // NDEBUG

    void construct( ModuleUI & parent )
    {
        BOOST_ASSERT( !constructed_ );
        doConstruct( parent );
        container().setup( Detail::WidgetInitialiser() );
        #ifndef NDEBUG
            constructed_ = true;
        #endif // NDEBUG
    }

    void destroy()
    {
        BOOST_ASSERT( constructed_ );
        container().~Container();
        #ifndef NDEBUG
            constructed_ = false;
        #endif // NDEBUG
    }

private:
    void doConstruct( ModuleUI & parent )
    {
        Detail::ModuleWidgetConstructionState constructionState( parent );
        LE_ASSUME( &parameterWidgetsStorage_ );
        Container * const pContainer( new ( &parameterWidgetsStorage_ ) Container( constructionState ) );
        LE_ASSUME( pContainer );
    }

    Container & container()
    {
        Container * LE_RESTRICT const pContainer( &reinterpret_cast<Container &>( parameterWidgetsStorage_ ) );
        LE_ASSUME( pContainer );
        return *pContainer;
    }

private:
    typedef typename std::aligned_storage
    <
        sizeof( Container ),
        std::alignment_of<Container>::value
    >::type ParameterWidgetsStorage;
    ParameterWidgetsStorage parameterWidgetsStorage_;

#ifndef NDEBUG
    bool constructed_;
#endif // NDEBUG
}; // class ParameterWidgets


template <>
class ParameterWidgets<Effects::Detail::EmptyParameters>
{
public:
    static void construct( ModuleUI & ) {}
    static void destroy  (            ) {}
}; // class ParameterWidgets<EmptyParameters>


template <class Interface>
struct ParameterWidgetsVTable
{
    template <class Implementation>
    ParameterWidgetsVTable( Implementation const & )
        :
    #ifndef _MSC_VER //...mrmlj...msvc(12) compilation error bug...
        doCreateGUI ( []( ModuleUI         & uiBase ) { boost::polymorphic_downcast<Implementation *>( &uiBase.module() )->create ( uiBase ); } ),
        doDestroyGUI( []( ModuleUI::Module &   base ) { boost::polymorphic_downcast<Implementation *>( &base            )->destroy(        ); } )
    #else
        doCreateGUI ( &createGUI <Implementation> ),
        doDestroyGUI( &destroyGUI<Implementation> )
    #endif // _MSC_VER
    {}

    void( /*mrmlj clang crash LE_GNU_SPECIFIC( __fastcall )*/ LE_MSVC_SPECIFIC( LE_FASTCALL ) * const doCreateGUI  )( ModuleUI         & )             ;
    void( /*mrmlj clang crash LE_GNU_SPECIFIC( __fastcall )*/ LE_MSVC_SPECIFIC( LE_FASTCALL ) * const doDestroyGUI )( ModuleUI::Module & ) /*noexcept*/;

#ifdef _MSC_VER
private:
    template <class Implementation> LE_NOALIAS        static void LE_FASTCALL createGUI ( ModuleUI         & uiBase ) { boost::polymorphic_downcast<Implementation *>( &uiBase.module() )->create ( uiBase ); }
    template <class Implementation> LE_NOTHROWNOALIAS static void LE_FASTCALL destroyGUI( ModuleUI::Module &   base ) { boost::polymorphic_downcast<Implementation *>( &base            )->destroy(        ); }
#endif
}; // struct ParameterWidgetsVTable

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleUI_hpp
