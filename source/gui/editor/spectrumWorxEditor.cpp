////////////////////////////////////////////////////////////////////////////////
///
/// spectrumWorxEditor.cpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "spectrumWorxEditor.hpp"

#if LE_SW_SEPARATED_DSP_GUI
    #include "core/modules/automatedModuleImpl.inl" //...mrmlj...loadPresetParameters
    #include "core/modules/moduleGUI.hpp"
#else
    #include "core/modules/moduleDSPAndGUI.hpp"
    #include "spectrumWorx.hpp"
#endif // LE_SW_SEPARATED_DSP_GUI

#include "le/parameters/lfo.hpp"
#include "le/parameters/printer.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/presets.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/objcfwdhelpers.hpp"
#include "le/utility/parentFromMember.hpp"

#include "juce/beginIncludes.hpp"
  //#include "juce/juce_core/network/juce_URL.h"
    #include "juce/juce_core/threads/juce_Process.h"
    #include "juce/juce_gui_basics/windows/juce_ComponentPeer.h"
#include "juce/endIncludes.hpp"

#include "boost/assert.hpp"
#include "boost/polymorphic_cast.hpp"
#include "boost/smart_ptr/intrusive_ptr.hpp"
#include "boost/utility/in_place_factory.hpp"

#include <array>
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

//...mrmlj...the specialized/optimized fillComboBoxForParameter<>() helpers no
//...longer go through the generic Parameters::print<>() function so we also
//...have to provide a specialization of the fillComboBoxForParameter()
//...function template to get the overlap factor in percentages in the settings
//...window...
//...clean this up...
template <>
void fillComboBoxForParameter<Engine::OverlapFactor>( ComboBox & comboBox )
{
    //...mrmlj...
#if defined( __clang__ ) && defined( _DEBUG )
    Engine::Setup const * pEngineSetup( nullptr ); ++pEngineSetup; //...mrmlj...workaround for clang's -fcatch-undefined-behavior...
#else
    static Engine::Setup const * const pEngineSetup( nullptr );
#endif // _DEBUG

    using Parameter = Engine::OverlapFactor;
    std::array<char, 20> buffer;
    Parameter::value_type value( Parameter::minimum() );
    while ( value <= Parameter::maximum() )
    {
        using LE::Parameters::print;
        using LE::Parameters::DisplayValueTransformer;
        print<Parameter>( value, const_cast<Engine::Setup const &>( *pEngineSetup ), boost::make_iterator_range_n( &buffer[ 0 ], buffer.size() ) );
        std::strcat( &buffer[ 0 ], boost::mpl::c_str<DisplayValueTransformer<Engine::OverlapFactor>::Suffix>::value );
        comboBox.addItem( value, &buffer[ 0 ] );
        value *= 2;
    }
}

#ifdef __APPLE__
    // gui.mmm forward declarations.
    extern void             attachComponentToHostWindow  ( juce::Component &, ObjC::NSView   * );
    extern ObjC::NSWindow * attachComponentToHostWindow  ( juce::Component &, WindowRef        );
    extern void             detachComponentFromHostWindow( juce::Component &, ObjC::NSWindow * );
#endif


////////////////////////////////////////////////////////////////////////////////
//
// Constants private to this module.
// ---------------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace Constants
{
    namespace Layout
    {
        unsigned int const textBoxHorizontalOffset =  76;
        unsigned int const textBoxHeight           =  22;
        unsigned int const textBoxWidth            = 113;

        unsigned int const moduleNameVerticalOffset   =  13;
        unsigned int const controlNameVerticalOffset  =  42;
        unsigned int const controlValueVerticalOffset =  53;
        unsigned int const sampleNameVerticalOffset   = 306;
    }
} //namespace Constants


#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

SpectrumWorxEditor::SpectrumWorxEditor()
    :
    nextAvailableModuleSlot_( 0 ),

    in_ ( *this, 18,  37 ),
    out_( *this, 18, 110 ),
    mix_( *this, 18, 185 ),

    moduleMenuButton_( *this ),
    gradient_        ( *this ),

    /// \todo Reimplement these Alex's widgets.
    ///                                       (22.09.2009.) (Domagoj Saric)
    //cSamplerDisplay ( CRect( 0, 0, 235, 129 ).offset( 222, 17 ), this, kBankSelect, 0, &resourceBitmap<kbLoad>(), &resourceBitmap<kbDel>(), &resourceBitmap<kbLock>() ),
    //cSpectrumDisplay( CRect( 0, 0, 235, 129 ).offset( 222, 17 ), this, SpectrumDisplay, 0, capture ),

    // buttons...
    preset_        ( *this, resourceBitmap<PresetOn  >(), resourceBitmap<PresetOff  >() ),
    settingsButton_( *this, resourceBitmap<SettingsOn>(), resourceBitmap<SettingsOff>() ),

    holdSharedModuleControls_( false ),
    holdLFODisplay_          ( false )
{
    using LE::Parameters::IndexOf;
    using namespace GlobalParameters;
    using GlobalParameters::Parameters;
    in_ .setupForParameter( IndexOf<Parameters, InputGain    >::value, InputGain    ::minimum(), InputGain    ::maximum(), InputGain    ::default_() );
    out_.setupForParameter( IndexOf<Parameters, OutputGain   >::value, OutputGain   ::minimum(), OutputGain   ::maximum(), OutputGain   ::default_() );
    mix_.setupForParameter( IndexOf<Parameters, MixPercentage>::value, MixPercentage::minimum(), MixPercentage::maximum(), MixPercentage::default_() );

    updateMainKnobs();
    BOOST_ASSERT( !settings_ );

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    // Implementation note:
    //   A sample may have already been loaded, either at startup using the last
    // session "preset" or through the GUI that was then destroyed and is now
    // being recreated.
    //                                        (10.06.2010.) (Domagoj Saric)
    updateSampleNameAsync();
#endif // LE_SW_DISABLE_SIDE_CHANNEL

    setDefaultFocusHandling();
    grabKeyboardFocus();

    // Resizable VST GUI discussions:
    // http://www.kvraudio.com/forum/viewtopic.php?t=141313
    // http://lists.steinberg.net:8100/Lists/vst-plugins/Message/17785.html
    // http://www.u-he.com/vstsource
    setSizeFromImage( *this, resourceBitmap<EditorBackground>() );

    gradient_.setInvisible();
    gradient_.setSize( ModuleUI::width, ModuleUI::height );
    moduleMenuButton_.moveToSlot( 0 );

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    sampleArea_.setBounds( 75, 307, 115, 20 );
#endif // LE_SW_DISABLE_SIDE_CHANNEL

    preset_        .setTopLeftPosition(  74, 338 );
    settingsButton_.setTopLeftPosition( 134, 338 );

    preset_        .addListener( this );
    settingsButton_.addListener( this );

    createChainGUIs( moduleChain() );

    setOpaque ( true );
    setVisible(      );

#if LE_SW_AUTHORISATION_REQUIRED
    if ( !authorised() )
    {
        // Implementation note:
        //   We cannot call showRegistrationPage() directly/synchronously
        // because of hosts like Audio Mulch where it is possible to detect the
        // editor parent only using geometry logic which works only after the
        // host parent window has resized to fit our editor and this happens
        // only after this constructor exits. For this reason we have to defer
        // the call to a later time.
        //                                    (02.06.2010.) (Domagoj Saric)
    #ifdef __APPLE__
        std::uint8_t resentCount( 0 );
    #endif // __APPLE__
        GUI::postMessage
        (
            effect(),
            [=]( GUI::SpectrumWorxEditor & editor ) mutable
            {
            #ifdef __APPLE__
                //...mrmlj...temporary uber ugly hack to workaround Reaper positioning problems...
                if ( resentCount++ < 50 )
                {
                    juce::Thread::sleep( 8 );
                    return false;
                }
                resentCount = 0;
            #endif // __APPLE__
                editor.showRegistrationPage();
                return true;
            }
        );
    }
#endif // LE_SW_AUTHORISATION_REQUIRED
}

#pragma warning( pop )

LE_NOTHROW
SpectrumWorxEditor::~SpectrumWorxEditor()
{
    BOOST_ASSERT( GUI::isThisTheGUIThread() );

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    effect().deregisterSampleLoadedListener( *this );
#endif // LE_SW_DISABLE_SIDE_CHANNEL

    while ( static_cast<bool const volatile &>( holdLFODisplay_           ) ) {}
    while ( static_cast<bool const volatile &>( holdSharedModuleControls_ ) ) {}

    /// \note
    ///   Take the focus beforehand to workaround JUCE's problematic focus
    /// handling (while taking the focus it will refocus the last focused
    /// component if it was a child of the component that is taking focus, IOW
    /// it will refocus the ModuleUI being destroyed, just what we are trying
    /// to avoid).
    ///                                       (13.01.2012.) (Domagoj Saric)
    BOOST_ASSERT( getWantsKeyboardFocus          () );
    BOOST_ASSERT( getMouseClickGrabsKeyboardFocus() );
    grabKeyboardFocus();
    destroyChainGUIs( moduleChain() );

    /// \note
    ///   Required now that boost::optional does not mark itself as
    /// uninitialised in its destructor so the PresetBrowser would think that
    /// the Settings window still exists (and vice verse) in its destructor.
    ///                                       (12.01.2012.) (Domagoj Saric)
    //...mrmlj...think of a cleaner solution...
    settings_      = boost::none;
    presetBrowser_ = boost::none;

#if defined( __APPLE__ ) && !defined( __x86_64__ )
    if ( pCocoaHostWindow_ )
    {
        //...mrmlj...manual detachment seems to be needed after all...otherwise
        // (background/hidden) ghost window(s) are left hanging sometimes...
        // ...this has to be called _after_ all owned windows are closed...
        // ...reinvestigate...
        detachComponentFromHostWindow( *this, pCocoaHostWindow_ );
    }
#endif // Apple & Carbon
}


#if defined( _WIN32 )

void SpectrumWorxEditor::attachToHostWindow( HWND const parentWindowHandle )
{
    //...mrmlj...taken from the JUCE VST wrapper
    juce::Component::addToDesktop( 0 );
    HWND const thisHWND( reinterpret_cast<HWND>( this->getWindowHandle() ) );
    BOOST_VERIFY( ::SetParent( thisHWND, parentWindowHandle ) );

    DWORD val( ::GetWindowLong( thisHWND, GWL_STYLE ) );
    val = ( val & ~WS_POPUP ) | WS_CHILD;
    ::SetWindowLong( thisHWND, GWL_STYLE, val );
}

#elif defined( __APPLE__ )

void SpectrumWorxEditor::attachToHostWindow( ObjC::NSView * const pParentWindow )
{
#if !defined( __x86_64__ )
    pCocoaHostWindow_ = nullptr;
#endif
    attachComponentToHostWindow( *this, pParentWindow );
}

#if !defined( __x86_64__ )
void SpectrumWorxEditor::attachToHostWindow( WindowRef const parentWindow )
{
    pCocoaHostWindow_ = attachComponentToHostWindow( *this, parentWindow );
}
#endif // 32 bit only Carbon support
#endif // platform


LE_NOTHROW LE_PURE_FUNCTION
SpectrumWorxEditor & SpectrumWorxEditor::fromChild( juce::Component const & widget )
{
    BOOST_ASSERT( widget.getParentComponent() );
    juce::Component * pParent( widget.getParentComponent() );
    while ( pParent->getParentComponent() )
        pParent = pParent->getParentComponent();
    return *boost::polymorphic_downcast<SpectrumWorxEditor *>( pParent );
}


LE_NOTHROW LE_PURE_FUNCTION
SpectrumWorxEditor & SpectrumWorxEditor::fromPresetBrowser( PresetBrowser & presetBrowser )
{
    return Utility::ParentFromOptionalMember<SpectrumWorxEditor, PresetBrowser, &SpectrumWorxEditor::presetBrowser_, false>()( presetBrowser );
}


#ifdef LE_SW_SEPARATED_DSP_GUI

    Engine::Setup const & SpectrumWorxEditor::engineSetup() const { return engineSetup_; }

    AutomatedModuleChain       & SpectrumWorxEditor::moduleChain()       { return /*moduleChain_*/program().moduleChain(); }
    AutomatedModuleChain const & SpectrumWorxEditor::moduleChain() const { return /*moduleChain_*/program().moduleChain(); }

    Program       & SpectrumWorxEditor::program()       { return program_; } //...mrmlj...for moduleChanged
    Program const & SpectrumWorxEditor::program() const { return program_; } //...mrmlj...for moduleChanged

    SpectrumWorxEditor::Host       & SpectrumWorxEditor::host()       { return *this; }
    SpectrumWorxEditor::Host const & SpectrumWorxEditor::host() const { return *this; }

    bool SpectrumWorxEditor::authorised() const { return true; }


    bool SpectrumWorxEditor::ModuleInitialiser::operator()( Module & module, std::uint8_t const slotIndex ) const
    {
        module.moveToSlot( slotIndex );
        module.updateForEngineSetupChanges( editor.engineSetup() );
        addToParentAndShow( editor, module );
        return true;
    }

#else
    Engine::Setup const & SpectrumWorxEditor::engineSetup() const { return effect().uncheckedEngineSetup(); }
    char_t        const & SpectrumWorxEditor::authorised () const { return effect().authorised          (); }

    AutomatedModuleChain       & SpectrumWorxEditor::moduleChain()       { return effect().moduleChain(); }
    AutomatedModuleChain const & SpectrumWorxEditor::moduleChain() const { return effect().moduleChain(); }

    SpectrumWorx       & SpectrumWorxEditor::effect()       { return SpectrumWorx::effect( *this ); }
    SpectrumWorx const & SpectrumWorxEditor::effect() const { return const_cast<SpectrumWorxEditor &>( *this ).effect(); }

    SpectrumWorxEditor::Host       & SpectrumWorxEditor::host()       { return effect(); }
    SpectrumWorxEditor::Host const & SpectrumWorxEditor::host() const { return effect(); }

    Program       & SpectrumWorxEditor::program()       { return effect().program(); }
    Program const & SpectrumWorxEditor::program() const { return effect().program(); }

    Utility::CriticalSectionLock LE_NOTHROW SpectrumWorxEditor::getProcessingLock() const { return effect().getProcessingLock(); }
#endif // LE_SW_SEPARATED_DSP_GUI


void SpectrumWorxEditor::togglePresetBrowser( juce::Button const & button )
{
    auto & editor( SpectrumWorxEditor::fromChild( button ) );
    BOOST_ASSERT( editor.getPeer() );
    bool const open( button.getToggleState() );
    if ( open )
        editor.presetBrowser_ = boost::in_place();
    else
        editor.presetBrowser_ = boost::none;
}


void SpectrumWorxEditor::setDefaultFocusHandling()
{
    setWantsKeyboardFocus          ( true );
    setMouseClickGrabsKeyboardFocus( true );
}


void SpectrumWorxEditor::moduleDrag( ModuleUI & moduleUI, juce::MouseEvent const & event )
{
    if ( isDragAndDropActive() )
    {
        unsigned int const halfModuleWidth( ModuleUI::width / 2 );

        juce::Rectangle<int> const & sourceRect( moduleUI.getBounds() );

        juce::Rectangle<int> const leftRect
        (
            ModuleUI::horizontalOffset,
            ModuleUI::verticalOffset,
            sourceRect.getX() - ( halfModuleWidth + ModuleUI::distance ) - 1 - ModuleUI::horizontalOffset,
            ModuleUI::height
        );

        unsigned int const emptyModuleSpaceBegin( moduleMenuButton_.getX() - 4 );
        unsigned int const rightRectStart( sourceRect.getRight() + ModuleUI::distance + halfModuleWidth + 1 );
        unsigned int const rightRectEnd  ( emptyModuleSpaceBegin + halfModuleWidth );
        juce::Rectangle<int> const rightRect
        (
            rightRectStart,
            ModuleUI::verticalOffset,
            rightRectEnd - rightRectStart,
            ModuleUI::height
        );

        juce::Point<int> const mousePosition( this->getLocalPoint( nullptr, event.getScreenPosition() ) );

        bool const showGradient
        (
            leftRect .contains( mousePosition ) ||
            rightRect.contains( mousePosition )
        );

        if ( showGradient )
        {
            unsigned int const firstGradientOffset
            (
                ModuleUI::horizontalOffset -
                ( halfModuleWidth + ( ModuleUI::distance / 2 ) )
            );

            unsigned int const gradientIndex
            (
                ( mousePosition.getX() - firstGradientOffset )
                    /
                ( ModuleUI::width + ModuleUI::distance )
            );
            BOOST_ASSERT( gradient_.getHeight() == ModuleUI::height );
            BOOST_ASSERT( gradient_.getWidth () == ModuleUI::width  );

            unsigned int const gradientOffset
            (
                firstGradientOffset +
                ( gradientIndex * ( ModuleUI::width + ModuleUI::distance ) )
            );
            gradient_.setTopLeftPosition
            (
                gradientOffset,
                ModuleUI::verticalOffset
            );
            BOOST_ASSERT( !gradient_.getBounds().intersects( sourceRect ) );
        }
        gradient_.setIsVisible( showGradient );
    }
    else
    {
        gradient_.toFront       ( true );
        gradient_.setAlwaysOnTop( true );

        startDragging( juce::var::null, &moduleUI );
    }
}

void SpectrumWorxEditor::moduleDragEnd( ModuleUI & moduleUI, juce::MouseEvent const & event )
{
    juce::Point<int> const mousePosition( this->getLocalPoint( nullptr, event.getScreenPosition() ) );

    bool const dragAborted
    (
        !gradient_.isVisible() ||
        !gradient_.getBounds().contains( mousePosition )
    );
    gradient_.setInvisible();
    if ( dragAborted )
        return;

    /// \note We have to block automation here because of FMOD's MVC
    /// implementation in which it responds to
    /// EDITOR_TO_HOST_SET_PARAMETER_VALUE calls (part of the below
    /// host().modulesChanged() calls) by immediately calling
    /// HOST_TO_EDITOR_UPDATE_PARAMETER_VALUE which in turn, coupled with the
    /// "dependent parameter caching hack-mechanism", breaks the module chain
    /// contents while it is being traversed.
    ///                                       (20.10.2014.) (Domagoj Saric)
    Host2PluginInteropControler::AutomationBlocker const automationBlocker( /*host*/moduleChainOwner/*mrmlj*/() );

    BOOST_ASSERT( !gradient_.getBounds().intersects( moduleUI.getBounds() ) );
    unsigned int const slotWidth( ModuleUI::width + ModuleUI::distance );
    unsigned int const sourceX  ( moduleUI .getX() );
    unsigned int       targetX  ( gradient_.getX() );
    bool         const moveLeft ( static_cast<unsigned int>( gradient_.getRight() ) < sourceX );
    int const gradientToTargetOffset( ( ModuleUI::width + ModuleUI::distance ) / 2 );
    targetX -= moveLeft
                ? - gradientToTargetOffset
                : + gradientToTargetOffset;
    BOOST_ASSERT( ( signed( targetX - sourceX ) % signed( slotWidth ) ) == 0 );
    moduleUI.setTopLeftPosition( targetX, ModuleUI::verticalOffset );

    int const offset
    (
        moveLeft
            ? + static_cast<int>( slotWidth )
            : - static_cast<int>( slotWidth )
    );

    std::uint8_t const sourceIndex( ( sourceX - ModuleUI::horizontalOffset ) / slotWidth );
    std::uint8_t const targetIndex( ( targetX - ModuleUI::horizontalOffset ) / slotWidth );
    //...mrmlj...
    //if ( sourceIndex > targetIndex )
    //    std::swap( sourceIndex, targetIndex );

    moveModules( moduleUI, Math::abs( targetIndex - sourceIndex ), offset );
    auto & moduleChain( this->moduleChain() );
    moduleChain.moveModule    (              sourceIndex, targetIndex );
    host().gestureBegin( "Drag module" );
    host()     .modulesChanged( moduleChain, sourceIndex, targetIndex );
    host().gestureEnd();
}


void SpectrumWorxEditor::setLastModulePosition( std::uint_fast8_t const slotIndex )
{
    BOOST_ASSERT( slotIndex <= SW::Constants::maxNumberOfModules );
    nextAvailableModuleSlot_ = slotIndex;
    moduleMenuButton_.moveToSlot( slotIndex );
}


namespace
{
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

    struct EditorMainAreaText
    {
        juce::String        const *       pText         ;
        juce::Font          const *       pFont         ;
        juce::Colour                const colour        ;
        unsigned int                const verticalOffset;
        juce::Justification         const justification ;
        unsigned int                const textLinesToUse;
    }; // struct EditorMainAreaText

    #pragma warning( pop )

    // Implementation note:
    //   To prevent the "static initialisation order fiasco" (occurring with
    // Clang 2.8 on OS X) we do not use the JUCE static Colours::white object
    // but construct our own white juce::Colour here.
    //                                        (25.01.2011.) (Domagoj Saric)
    EditorMainAreaText mainAreaTexts[] =
    {
        { 0, 0, Theme::blueColour()       , Constants::Layout::moduleNameVerticalOffset  , juce::Justification::centred                                       , 1 }, // active module name
        { 0, 0, juce::Colour( 0xFFFFFFFF ), Constants::Layout::controlNameVerticalOffset , juce::Justification::top | juce::Justification::horizontallyCentred, 2 }, // control name
        { 0, 0, juce::Colour( 0xFFFFFFFF ), Constants::Layout::controlValueVerticalOffset, juce::Justification::centred                                       , 1 }, // control value
        { 0, 0, Theme::blueColour()       , Constants::Layout::sampleNameVerticalOffset  , juce::Justification::centred                                       , 1 }, // sample name
    };

    void drawMainAreaText( juce::Graphics & graphics, EditorMainAreaText const & text )
    {
        using namespace Constants::Layout;

        graphics.setColour(  text.colour );
        graphics.setFont  ( *text.pFont  );
        graphics.drawFittedText
        (
            *text.pText,
            textBoxHorizontalOffset, text.verticalOffset,
            textBoxWidth           , textBoxHeight * text.textLinesToUse,
            text.justification,
            text.textLinesToUse
        );
    }
} //anonymous namespace

void SpectrumWorxEditor::paint( juce::Graphics & graphics )
{
    GUI::paintImage( graphics, resourceBitmap<EditorBackground>() );

    juce::Font const & moduleNameFont ( Theme::singleton().blueFont () );
    juce::Font const & sampleNameFont ( DrawableText::defaultFont   () );
    juce::Font const & controlTextFont( Theme::singleton().whiteFont() );

    mainAreaTexts[ 0 ].pText = &string( activeModuleName   ); mainAreaTexts[ 0 ].pFont = &moduleNameFont ;
    mainAreaTexts[ 1 ].pText = &string( activeControlName  ); mainAreaTexts[ 1 ].pFont = &controlTextFont;
    mainAreaTexts[ 2 ].pText = &string( activeControlValue ); mainAreaTexts[ 2 ].pFont = &controlTextFont;
    mainAreaTexts[ 3 ].pText = &string( currentSampleName  ); mainAreaTexts[ 3 ].pFont = &sampleNameFont ;

    for ( auto const & text : mainAreaTexts )
        drawMainAreaText( graphics, text );

#if LE_SW_AUTHORISATION_REQUIRED
    if ( !authorised() )
        graphics.drawSingleLineText( "DEMO", 20, 287 );
#endif // LE_SW_AUTHORISATION_REQUIRED
}


void SpectrumWorxEditor::buttonClicked( juce::Button * const pButton )
{
    if ( pButton == &settingsButton_ )
    {
        if ( settingsButton_.getToggleState() )
        {
            showSettings( 0 );
        }
        else
        {
            BOOST_ASSERT( settings_ );
            settings_ = boost::none;
        }
    }
    else
    {
        BOOST_ASSERT( pButton == &preset_ );
        togglePresetBrowser( *pButton );
    }
}


void LE_NOINLINE SpectrumWorxEditor::updateString
(
    String               const stringID,
    unsigned int         const stringVerticalOffset,
    unsigned int         const stringHeight,
    juce::String const &       updatedString
)
{
    string( stringID ) = updatedString;

    using namespace Constants::Layout;
    repaint
    (
        textBoxHorizontalOffset, stringVerticalOffset,
        textBoxWidth           , stringHeight
    );
}

void SpectrumWorxEditor::setActiveModuleName( juce::String const & newName )
{
    using namespace Constants::Layout;
    updateString( activeModuleName, moduleNameVerticalOffset, textBoxHeight, newName );
}


void SpectrumWorxEditor::setActiveControlName( juce::String const & newName )
{
    using namespace Constants::Layout;
    updateString( activeControlName, controlNameVerticalOffset, textBoxHeight * 4, newName );
}


void SpectrumWorxEditor::setActiveControlValue( juce::String const & newValue )
{
    using namespace Constants::Layout;
    updateString( activeControlValue, controlValueVerticalOffset, textBoxHeight, newValue );
}


LE_NOTHROW
void SpectrumWorxEditor::updateActiveControlValue()
{
    try
    {
        BOOST_ASSERT( lfoDisplay_ );
        LFODisplay const & lfoDisplay( /*static_cast<LFODisplay const &>*/( *lfoDisplay_ ) );
        if ( lfoDisplay.lfo().enabled() ) setActiveControlValue( "[LFO]"                             );
        else                              setActiveControlValue( lfoDisplay.control().getValueText() );
    }
    catch ( ... ) {}
}

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
void SpectrumWorxEditor::updateSampleName( juce::String const & newSampleName )
{
    using namespace Constants::Layout;
    updateString( currentSampleName, sampleNameVerticalOffset, textBoxHeight, newSampleName );
}


void SpectrumWorxEditor::updateSampleName()
{
    updateSampleName( effect().sample_.sampleFile().getFileNameWithoutExtension() );
}


void SpectrumWorxEditor::updateSampleNameAsync()
{
    if ( effect().isSampleLoadInProgress() )
    {
        effect().registerSampleLoadedListener( *this );
        setSampleLoadingStatus();
        BOOST_ASSERT( effect().isSampleLoadInProgress() ); //...mrmlj...handle this threading issue properly....
    }
    else
        updateSampleName();
}


void SpectrumWorxEditor::setSampleLoadingStatus()
{
    sampleArea_.setInvisible();
    updateSampleName( "Loading..." );
}


void SpectrumWorxEditor::newSampleFileSelected( juce::File const & file )
{
    effect().setNewSample( file );
    updateSampleNameAsync();
}
#endif // LE_SW_DISABLE_SIDE_CHANNEL

void LE_NOTHROW SpectrumWorxEditor::removeModule( ModuleUI & moduleUI )
{
    static_assert( ModuleUI::width    % 2 == 0, "Only even width modules supported." );
    static_assert( ModuleUI::distance % 2 == 0, "Only even width modules supported." );

    /// \note See the note for the equivalent statement in the moduleDragEnd()
    /// member function.
    ///                                       (20.10.2014.) (Domagoj Saric)
    Host2PluginInteropControler::AutomationBlocker const automationBlocker( /*host*/moduleChainOwner/*mrmlj*/() );

    LE_ASSUME( nextAvailableModuleSlot_ != 0 );
    auto const slotWidth       ( ModuleUI::width + ModuleUI::distance                         );
    auto const offset          ( - signed( slotWidth )                                        );
    auto const slot            ( ( moduleUI.getX() - ModuleUI::horizontalOffset ) / slotWidth );
    auto const firstModuleIndex( slot                                                         );
    auto const lastModuleIndex ( nextAvailableModuleSlot_ - 1                                 );
    moveModules( moduleUI, lastModuleIndex - firstModuleIndex, offset );
    moduleRemoved();
    BOOST_VERIFY( setModuleInSlot( slot, AutomatedModuleChain::noModule ).first == nullptr );
    host().gestureBegin( "Remove module" );
    host().modulesChanged( moduleChain(), firstModuleIndex, lastModuleIndex );
    host().gestureEnd();
}


void LE_NOTHROW SpectrumWorxEditor::moveModules( ModuleUI & targetSlotUI, std::uint8_t numberOfModules, std::int16_t const offset )
{
    //...mrmlj...
    typedef Engine::ModuleNode                                             ModuleNode;
    typedef std::remove_reference<decltype( targetSlotUI.module() )>::type Module;
    ModuleNode::NodePtr ModuleNode::* const pNextPtr
    (
        ( offset < 0 )
            ? &ModuleNode::next_
            : &ModuleNode::previous_
    );
    //...mrmlj...internal module chain knowledge...
    auto * LE_RESTRICT pMovedModule( &targetSlotUI.module() );
    while ( numberOfModules-- )
    {
        pMovedModule = &Engine::actualModule<Module>( *( Engine::node( *pMovedModule ).*pNextPtr ) );
        LE_ASSUME( pMovedModule ); //...msvc...
        auto & gui( *pMovedModule->gui() );
        gui.setTopLeftPosition( gui.getX() + offset, ModuleUI::verticalOffset );
    }
}


std::pair<boost::intrusive_ptr<SpectrumWorxEditor::Module>, std::int8_t> LE_NOTHROW
SpectrumWorxEditor::setModuleInSlot( std::uint8_t const slotIndex, std::int8_t const effectIndex )
{
    return moduleChainOwner().moduleChain().setParameter( slotIndex, effectIndex, moduleChainOwner().moduleInitialiser() );
}


void LE_NOTHROW SpectrumWorxEditor::addUserAddedModule( std::uint8_t const effectIndex )
{
    // Implementation note:
    //   This is certainly executed from the GUI thread so this function expects
    // the module creation to be done synchronously, in order for the focus
    // grabbing to be safe.
    //                                        (06.07.2011.) (Domagoj Saric)
    BOOST_ASSERT( isThisTheGUIThread() );
    // Implementation note:
    //   We want any user-added module (using the add module menu) to
    // automatically gain focus.
    //                                        (09.02.2010.) (Domagoj Saric)
#ifdef _WIN32
    /// \note Force emptying the message queue as a quick-fix for weird crashes
    /// in SoundForge 10 that happen when the first (any) module is added. These
    /// crashes seem to be caused by a loadProgramState() call "in the middle"
    /// of the grabKeyboardFocus() call (when JUCE calls SetFocus() Windows
    /// starts pumping messages seemingly queued by SoundForge at some point,
    /// one of which causes a call to loadProgramState()).
    ///                                       (24.01.2013.) (Domagoj Saric)
    BOOST_ASSERT( getWantsKeyboardFocus          () );
    BOOST_ASSERT( getMouseClickGrabsKeyboardFocus() );
    juce::MessageManager::getInstance()->runDispatchLoopUntil( 2 );
    this->grabKeyboardFocus();
#endif // _WIN32

    auto const result( setModuleInSlot( nextAvailableModuleSlot_, effectIndex ) );
    std::int8_t const actualEffectIndex( result.second );
    if ( actualEffectIndex == effectIndex ) //...mrmlj...
    {
        BOOST_ASSERT( result.first );
        std::uint8_t const changedSlot( nextAvailableModuleSlot_ );
        moduleAdded();
        result.first->gui()->grabKeyboardFocus();
        host().gestureBegin( "Add module" );
        host().moduleChangedByUser( changedSlot, result.first.get() );
        host().gestureEnd();
    }
    else
    {   // failed module creation
        BOOST_ASSERT( result.first  == nullptr  );
        BOOST_ASSERT( result.second == noModule );
    }
}


#if LE_SW_SEPARATED_DSP_GUI
#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

struct SpectrumWorxEditor::PresetLoader
{
    typedef ModuleInitialiser::Module Module;

    AutomationBlocker automationBlocker() const { return AutomationBlocker( editor ); }

    ModuleInitialiser moduleInitialiser() { return editor.moduleInitialiser(); }

    static boost::none_t processingLock() { return boost::none; }

    static bool onlySetParameters() { return false; }

    GlobalParameters::Parameters & targetGlobalParameters() { LE_UNREACHABLE_CODE(); return editor.program().parameters(); }
    AutomatedModuleChain         & targetChain           () { return editor.moduleChain(); }

private: //...mrmlj...MSVC(12) still does not support generic lambdas...
    struct Notifyer
    {
        template <class Parameter>
        void operator()( Parameter const & parameter ) const { editor.globalParameterChanged<Parameter>( parameter, std::is_integral<typename Parameter::value_type>::value ); }
        SpectrumWorxEditor & editor;
    }; // struct Notifyer
public:
    bool setNewGlobalParameters( GlobalParameters::Parameters const & newParameters )
    {
        editor.program().parameters() = newParameters;
        editor.updateForGlobalParameterChange();
        editor.updateForEngineSetupChanges   ();
        Notifyer const notifyer = { editor };
        boost::fusion::for_each( newParameters, notifyer );
        return true;
    }

    void moduleChainFinished( std::uint8_t const moduleCount, bool const syncedLFOFound )
    {
        editor.setLastModulePosition( moduleCount );
        if
        (
            syncedLFOFound &&
            !LE::Parameters::LFOImpl::Timer::hasTempoInformation()
        )
        {
            GUI::warningMessageBox
            (
                MB_WARNING,
                "Loaded preset uses tempo-synced LFOs but the host does not provide tempo information.",
                false
            );
        }
        Host2PluginInteropControler::AutomationBlocker const automationBlocker( editor./*host*/moduleChainOwner/*mrmlj*/() );
        editor.host().modulesChanged( editor.moduleChain(), 0, SW::Constants::maxNumberOfModules - 1 );
    }

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    bool wantsSampleFile() const { return false; }
    void setSample( boost::string_ref /*const sampleFileName*/ ) { LE_UNREACHABLE_CODE(); }
    bool const ignoreSampleFile;
#endif // LE_SW_DISABLE_SIDE_CHANNEL

    SpectrumWorxEditor & editor;
}; // struct PresetLoader

#pragma warning( push )

SpectrumWorxEditor::PresetLoader SpectrumWorxEditor::presetLoader( bool const ignoreExternalSample )
{
#ifdef LE_SW_DISABLE_SIDE_CHANNEL
    LE_ASSUME( ignoreExternalSample );
    PresetLoader const loader = {                       *this };
#else
    PresetLoader const loader = { ignoreExternalSample, *this };
#endif // LE_SW_DISABLE_SIDE_CHANNEL
    return loader;
}
#endif // LE_SW_SEPARATED_DSP_GUI

bool SpectrumWorxEditor::loadPreset( juce::File const & presetFile, bool const ignoreExternalSample, juce::String & comment, juce::String const & presetName )
{
    auto const pPresetName( presetName.getCharPointer().getAddress() );
#if LE_SW_SEPARATED_DSP_GUI
    return SW::     loadPreset( presetFile, ignoreExternalSample, &comment, pPresetName, *this );
#else
    return effect().loadPreset( presetFile, ignoreExternalSample, &comment, pPresetName        );
#endif // LE_SW_SEPARATED_DSP_GUI
}


void SpectrumWorxEditor::savePreset( juce::File const & presetFile, bool const ignoreExternalSample, juce::String const & comment ) const
{
#if LE_SW_SEPARATED_DSP_GUI
    LE_ASSUME( ignoreExternalSample );
    SW::savePreset( presetFile,                        juce::File::nonexistent                               , comment, program() );
#else
    SW::savePreset( presetFile, ignoreExternalSample ? juce::File::nonexistent : effect().currentSampleFile(), comment, program() );
#endif // LE_SW_SEPARATED_DSP_GUI
}


bool SpectrumWorxEditor::presetLoadingInProgress() const { return static_cast<Host2PluginInteropControler const &>( moduleChainOwner() )/*...mrmlj...*/.presetLoadingInProgress(); }


char const * SpectrumWorxEditor::currentProgramName() const
{
#if LE_SW_SEPARATED_DSP_GUI
    return program().name().begin();
#else
    return effect().currentProgramName();
#endif // LE_SW_SEPARATED_DSP_GUI
}


void SpectrumWorxEditor::moduleActivated()
{
    BOOST_ASSERT( isThisTheGUIThread()       );
    BOOST_ASSERT( ModuleUI::selectedModule() );
    ModuleUI const & module( *ModuleUI::selectedModule() );
    setActiveModuleName( module.getName() );
    if ( !ModuleControlBase::activeControl() )
    {
        setActiveControlName ( module.description() );
        setActiveControlValue( juce::String::empty  );
    }

    /// \note
    ///   See the note in the moduleDeactivated() member function for an
    /// explanation as to why we expect the SharedModuleControls instance to
    /// possibly be already created.
    ///                                       (17.01.2012.) (Domagoj Saric)
    if ( !sharedModuleControls_ )
        sharedModuleControls_ = boost::in_place();
    else
        sharedModuleControls_->setEnabled( true );
    sharedModuleControls_->updateForActiveModule();
}


void SpectrumWorxEditor::moduleDeactivated()
{
    BOOST_ASSERT( ModuleUI::selectedModule() );

    // Implementation note:
    //   We need to prevent JUCE from transferring focus to other module UIs
    // when it destroys the currently active ModuleUI and/or the
    // SharedModuleControls instance as that would cause a call to
    // moduleActivated() while we are still in the SharedModuleControls
    // destructor which in turn would cause another (reentrant) call to the
    // SharedModuleControls destructor. This is accomplished by first
    // transferring focus to the editor window if the module being deactivated
    // (and possibly destroyed) is currently focused.
    //                                        (03.01.2012.) (Domagoj Saric)
    BOOST_ASSERT( this->getWantsKeyboardFocus() );
    if ( ModuleUI::selectedModule()->juce::Component::isParentOf( getCurrentlyFocusedComponent() ) )
    {
        this->grabKeyboardFocus();
        BOOST_ASSERT( hasDirectFocus() );
    }

    BOOST_ASSERT_MSG( !lfoDisplay_ || !lfoDisplay_->isEnabled(), "Module controls not deactivated." );

    setActiveModuleName  ( juce::String::empty );
    setActiveControlName ( juce::String::empty );
    setActiveControlValue( juce::String::empty );

    if ( sharedModuleControls_ )
    {
        /// \note We defer the destruction of the SharedModuleControls instance
        /// so that we can avoid the destruction+recreation in case the user is
        /// actually only activating a different module.
        ///                                   (17.01.2012.) (Domagoj Saric)
        sharedModuleControls_->setEnabled( false );
        GUI::postMessage
        (
            owner(),
            []( GUI::SpectrumWorxEditor & editor )
            {
                auto & sharedModuleControls( editor.sharedModuleControls_ );
                if ( sharedModuleControls && !sharedModuleControls->isEnabled() )
                {
                    if ( static_cast<bool const volatile &>( editor.holdSharedModuleControls_ ) )
                        return false;
                    else
                        sharedModuleControls = boost::none;
                }
                return true;
            }
        );
    }
}


ParameterID SpectrumWorxEditor::moduleControlID( ModuleControlBase const & control ) const
{
    ParameterID parameterID;
    parameterID.value.type                          = ParameterID::ModuleParameter;
    parameterID.value._.module.moduleIndex          = moduleChain().getIndexForModule( control.module() );
    parameterID.value._.module.moduleParameterIndex = control.moduleParameterIndex();
    return parameterID;
}


void SpectrumWorxEditor::moduleControlActivated
(
    ModuleControlBase & control,
    double const minimum,
    double const maximum,
    double const interval
)
{
    /// \note
    ///   In addition to the reason given for the SharedModuleControls instance
    /// in the moduleActivated()/moduleDeactivated() member functions, the
    /// LFODisplay instance can also be expected to be already created here
    /// because of the SharedModuleControls::FrequencyRange control (because
    /// one of its thumbs can be activated w/o first deactivating the other).
    ///                                       (17.01.2012.) (Domagoj Saric)
    if ( !lfoDisplay_ )
        lfoDisplay_ = boost::in_place();
    else
        lfoDisplay_->setEnabled( true );

    lfoDisplay_->setupForControl( control, minimum, maximum, interval );

    setActiveModuleName ( control.moduleUI().getName() );
    setActiveControlName( control.widget  ().getName() );
    updateActiveControlValue();

    host().automatedParameterBeginEdit( moduleControlID( control ) );
}


void SpectrumWorxEditor::moduleControlDectivated( ModuleControlBase const & control )
{
    BOOST_ASSERT( lfoDisplay_ );
    BOOST_ASSERT_MSG
    (
        ( &static_cast<LFODisplay const &>( *lfoDisplay_ ).control() == &control ),
        "Deactivating active module control through a wrong control."
    );
    boost::ignore_unused_variable_warning( control );

    setActiveControlName ( ModuleUI::selectedModule() ? ModuleUI::selectedModule()->description() : juce::String::empty );
    setActiveControlValue( juce::String::empty                                                                          );

    if ( lfoDisplay_ )
    {
        /// \note See the note in the moduleDeactivated() member function.
        ///                                   (17.01.2012.) (Domagoj Saric)
        setDefaultFocusHandling();
        lfoDisplay_->setEnabled( false );
        /// \note We defer LFODisplay destruction so that we can avoid the
        /// destruction+recreation in case the user is actually only switching
        /// between controls.
        ///                                   (02.09.2013.) (Domagoj Saric)
        postMessage
        (
            owner(),
            []( GUI::SpectrumWorxEditor & editor )
            {
                auto & lfoDisplay( editor.lfoDisplay_ );
                if ( lfoDisplay && !lfoDisplay->isEnabled() )
                {
                    if ( static_cast<bool const volatile &>( editor.holdLFODisplay_ ) )
                        return false;
                    else
                        lfoDisplay = boost::none;
                    BOOST_ASSERT( editor.getWantsKeyboardFocus          () );
                    BOOST_ASSERT( editor.getMouseClickGrabsKeyboardFocus() );
                }
                return true;
            }
        );
    }

    host().automatedParameterEndEdit( moduleControlID( control ) );
}


void SpectrumWorxEditor::mainKnobDragStarted( std::uint8_t const index ) const
{
    ParameterID parameterID;
    parameterID.value         .type  = ParameterID::GlobalParameter;
    parameterID.value._.global.index = index;
    host().automatedParameterBeginEdit( parameterID );
}

void SpectrumWorxEditor::mainKnobDragStopped( std::uint8_t const index ) const
{
    ParameterID parameterID;
    parameterID.value         .type  = ParameterID::GlobalParameter;
    parameterID.value._.global.index = index;
    host().automatedParameterEndEdit( parameterID );
}


void LE_NOTHROW SpectrumWorxEditor::createChainGUIs( AutomatedModuleChain & chain )
{
#if LE_SW_SEPARATED_DSP_GUI
    boost::ignore_unused_variable_warning( chain );
#else
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2012/n3424.pdf
    std::uint8_t moduleIndex( 0 );
    chain.forEach<Module>
    (
        [&]( Module & module ) mutable { module.createGUI( *this, moduleIndex++ ); }
    );
    setLastModulePosition( moduleIndex );
#endif
}


void LE_NOTHROW SpectrumWorxEditor::destroyChainGUIs( AutomatedModuleChain & chain )
{
#if LE_SW_SEPARATED_DSP_GUI
    boost::ignore_unused_variable_warning( chain );
#else
    chain.forEach<Module>
    (
        []( Module & module ) { module.destroyGUI(); }
    );
    setLastModulePosition( 0 );
#endif
}


void SpectrumWorxEditor::mouseDown( juce::MouseEvent const & event )
{
    juce::Rectangle<int> const logoArea( 12, 290, 51, 63 );
    if ( logoArea.contains( event.x, event.y ) )
    {
        showSettings( 3 );
    }
}


void SpectrumWorxEditor::showSettings( unsigned int const pageIndexToActivate )
{
    // Implementation note:
    //   Settings can be created by the user pressing the settings button and
    // by the license verification code so it can already be created.
    //                                            (13.04.2010.) (Domagoj Saric)
    if ( !settings_.is_initialized() )
        settings_ = boost::in_place();
    settings_->setCurrentTabIndex( pageIndexToActivate, false );
    settingsButton_.setToggleState( true, juce::dontSendNotification );
}


void SpectrumWorxEditor::updateSettings()
{
    if ( settings_.is_initialized() )
        settings_->updateEnginePage();
}

#if !LE_SW_SEPARATED_DSP_GUI
void SpectrumWorxEditor::updateMainKnobs()
{
    auto const & parameters( effect().parameters() );
    using namespace GlobalParameters;
    in_ .setValue( parameters.get<InputGain    >() );
    out_.setValue( parameters.get<OutputGain   >() );
    mix_.setValue( parameters.get<MixPercentage>() );
}
#endif

void LE_NOTHROW SpectrumWorxEditor::updateForGlobalParameterChange()
{
    updateMainKnobs();
    updateSettings ();
}

using namespace GlobalParameters;
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<FFTSize         >() { updateSettings(); updateForEngineSetupChanges(); }
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<OverlapFactor   >() { updateSettings(); updateForEngineSetupChanges(); }
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<WindowFunction  >() { updateSettings(); updateForEngineSetupChanges(); }
#if LE_SW_ENGINE_WINDOW_PRESUM
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<WindowSizeFactor>() { updateSettings(); updateForEngineSetupChanges(); }
#endif // LE_SW_ENGINE_WINDOW_PRESUM
#if LE_SW_ENGINE_INPUT_MODE >= 2
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<InputMode       >() { updateSettings(); }
#endif // LE_SW_ENGINE_INPUT_MODE >= 2
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<InputGain       >() { updateMainKnobs(); }
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<OutputGain      >() { updateMainKnobs(); }
template <> void SpectrumWorxEditor::updateGlobalParameterWidget<MixPercentage   >() { updateMainKnobs(); }


void SpectrumWorxEditor::updateForEngineSetupChanges()
{
    Engine::Setup const & engineSetup( this->engineSetup() );
    holdSharedModuleControls_ = true;
    if ( sharedModuleControlsActive() )
        sharedModuleControls().updateForEngineSetupChanges( engineSetup );
    holdSharedModuleControls_ = false;
    moduleChain().forEach<Module>
    (
        [&]( Module & module )
        {
        #ifndef LE_SW_FMOD
            //...mrmlj...when switching programs...
            BOOST_ASSERT( module.gui() );
            if ( module.gui() )
        #endif // LE_SW_FMOD
                module.gui()->updateForEngineSetupChanges( engineSetup );
        }
    );
}


void SpectrumWorxEditor::updateForNewTimingInfo()
{
    // This gets called from a non GUI thread.
    BOOST_ASSERT( !holdLFODisplay_ );
    holdLFODisplay_ = true;
    if ( lfoDisplay_ && lfoDisplay_->isEnabled() )
        lfoDisplay_->updateForNewTimingInfo();
    holdLFODisplay_ = false;
}


void SpectrumWorxEditor::updateLFO( ModuleUI const & moduleUI, std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, Plugins::AutomatedParameterValue const value )
{
    // This gets called from a non GUI thread.
    BOOST_ASSERT( !holdLFODisplay_ );
    holdLFODisplay_ = true;
    if ( lfoDisplay_ && lfoDisplay_->isEnabled() )
        lfoDisplay_->updateForChangedParameters( moduleUI, parameterIndex, lfoParameterIndex, value );
    holdLFODisplay_ = false;
}

LE_NOTHROW
void SpectrumWorxEditor::updateModuleParameterAndNotifyHost( ModuleUI & moduleUI, std::uint8_t const moduleParameterIndex, float parameterValue ) const
{
    auto         &       module     ( moduleUI.module()                         );
    std::uint8_t   const moduleIndex( moduleChain().getIndexForModule( module ) );
#if !LE_SW_SEPARATED_DSP_GUI
    auto const snappedParameterValue( module.setParameterValueFromUI( moduleParameterIndex, parameterValue ) );
    parameterValue = snappedParameterValue;
#endif // LE_SW_SEPARATED_DSP_GUI
    host().automatedParameterChanged( module, moduleIndex, moduleParameterIndex, parameterValue );
}


SpectrumWorxEditor::ModuleMenuButton::ModuleMenuButton( SpectrumWorxEditor & parent )
    :
    BitmapButton( parent, resourceBitmap<AddModule>(), resourceBitmap<AddModule>(), Theme::singleton().blueColour() )
{
}


void SpectrumWorxEditor::ModuleMenuButton::moveToSlot( std::uint8_t const slotIndex )
{
    //...mrmlj..."magic number" adjustments...
    setTopLeftPosition
    (
        4 + ModuleUI::horizontalOffset + ( ( ModuleUI::width + ModuleUI::distance ) * slotIndex ),
        ( ModuleUI::verticalOffset - 4 ) + ( ModuleUI::height / 2 ) - ( getHeight() / 2 )
    );
    setIsVisible( slotIndex < SW::Constants::maxNumberOfModules );
}


void SpectrumWorxEditor::ModuleMenuButton::clicked()
{
    SpectrumWorxEditor & editor( *boost::polymorphic_downcast<SpectrumWorxEditor *>( this->getParentComponent() ) );
    BOOST_ASSERT( editor.nextAvailableModuleSlot_ < SW::Constants::maxNumberOfModules );
    PopupMenu::OptionalID const chosenMenuEntryID
    (
        editor.moduleMenu_.topMenu().showCenteredAtRight( *this )
    );
    if ( chosenMenuEntryID.is_initialized() )
    {
        BOOST_ASSERT( editor.moduleMenu_.isOwnerOfEntry( *chosenMenuEntryID ) );
        std::uint8_t const effectIndex( editor.moduleMenu_.effectIndexForEntry( *chosenMenuEntryID ) );
        editor.addUserAddedModule( effectIndex );
    }
}


SpectrumWorxEditor::Gradient::Gradient( juce::Component & parent )
    :
    juce::ColourGradient
    (
        juce::Colours::transparentWhite,                                     0, 0,
        juce::Colours::transparentWhite, static_cast<float>( ModuleUI::width ), 0,
        false
    )
{
    juce::ColourGradient::addColour( 0.5, juce::Colours::darkgrey );
    addToParentAndShow( parent, *this );
}


void SpectrumWorxEditor::Gradient::paint( juce::Graphics & graphics )
{
    graphics.setGradientFill( *this );
    graphics.fillAll();
}


namespace
{
    using LFO = LE::Parameters::LFOImpl;

    LE_NOINLINE LFO::value_type LE_FASTCALL rangeSliderValueToLFOValue( juce::Slider const & slider, double const value )
    {
        return Math::convertLinearRange<LFO::value_type, LFO::minimumValue, LFO::maximumValue - LFO::minimumValue, 1, double>
                    (
                        value,
                        slider.getMinimum(),
                        slider.getMaximum()
                    );
    }

    LE_NOINLINE double LE_FASTCALL lfoValueToRangeSliderValue( juce::Slider const & slider, LFO::value_type const & value )
    {
        return Math::convertLinearRange<double, LFO::value_type, LFO::minimumValue, LFO::maximumValue - LFO::minimumValue, 1>
                    (
                        value,
                        slider.getMinimum(),
                        slider.getMaximum()
                    );
    }


    void fillLFOWaveformsMenu( PopupMenu & menu )
    {
        static juce::Image const * LE_RESTRICT const icons[] =
        {
            &resourceBitmap<LFOSine        >(),
            &resourceBitmap<LFOTriangle    >(),
            &resourceBitmap<LFOSawtooth    >(),
            &resourceBitmap<LFOReverseSaw  >(),
            &resourceBitmap<LFOSquare      >(),
            &resourceBitmap<LFOExponent    >(),
            &resourceBitmap<LFORandomHold  >(),
            &resourceBitmap<LFORandomSlide >(),
            &resourceBitmap<LFORandomWhacko>(),
            &resourceBitmap<LFODirac       >(),
            &resourceBitmap<LFOdIRAC       >()
        };

        //BOOST_ASSERT( menu.getNumItems() == 0 );...mrmlj...add size information to the new ComboBox class...
        unsigned int itemId( 0 );
        juce::Image const * LE_RESTRICT const * ppIcon = icons;
        for ( auto const waveFormName : LE::Parameters::DiscreteValues<LFO::Waveform>::strings )
            menu.addItem( itemId++, waveFormName, **ppIcon++ );
    }
} // namespace anonymous

#define LE_COMP_PTR( member ) reinterpret_cast<ComponentPtr>( &SpectrumWorxEditor::LFODisplay:: member )
SpectrumWorxEditor::LFODisplay::ComponentPtr const SpectrumWorxEditor::LFODisplay::componentsToDisableKeyboardGrabingFor[] =
{
    LE_COMP_PTR( switch_  ),
    LE_COMP_PTR( phase_   ),
    LE_COMP_PTR( range_   ),
    LE_COMP_PTR( period_  ),
    LE_COMP_PTR( quarter_ ),
    LE_COMP_PTR( triplet_ ),
    LE_COMP_PTR( dotted_  )
    /*, this*/
};
#undef LE_COMP_PTR

#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

SpectrumWorxEditor::LFODisplay::LFODisplay()
    :
    switch_        ( *this, resourceBitmap<LEDOn         >(), resourceBitmap<LEDOff        >()                                         ),
    quarter_       ( *this, 62       , 5, " N "                                                                                        ),
    triplet_       ( *this, 62+18*1  , 5, " T "                                                                                        ),
    dotted_        ( *this, 62+18*2-2, 5, " D "                                                                                        ),
    typeArrow_     ( *this, resourceBitmap<ChangeWaveform>(), resourceBitmap<ChangeWaveform>(), juce::Colours::white.withAlpha( 0.5f ) ),
    pModuleControl_( nullptr                                                                                                           )
{
    for ( auto const pComponent : componentsToDisableKeyboardGrabingFor )
    {
        juce::Component & component( this->*pComponent );
        component.setWantsKeyboardFocus          ( false );
        component.setMouseClickGrabsKeyboardFocus( false );
    }
    this->setWantsKeyboardFocus          ( false );
    this->setMouseClickGrabsKeyboardFocus( false );

    fillLFOWaveformsMenu( type_ );

    switch_.setTopLeftPosition( 25, 3 );

    period_.setBounds( 7, 32, 108, 18 );
    period_.setSliderStyle ( juce::Slider::LinearHorizontal );
    period_.setTextBoxStyle( juce::Slider::NoTextBox, true, 10, 12 );
  //period_.setVelocityBasedMode( true );
    addToParentAndShow( *this, period_ );

    phase_.setBounds( 39, 118, 42, 12 );
    phase_.setSliderStyle ( juce::Slider::LinearHorizontal );
    phase_.setTextBoxStyle( juce::Slider::NoTextBox, true, 10, 12 );
    phase_.setRange( -0.5, +0.5 );
    phase_.setDoubleClickReturnValue( true, 0 );
    addToParentAndShow( *this, phase_ );

    typeArrow_.setTopLeftPosition( 109, 99 );
    typeArrow_.addListener       ( this    );

    range_.setBounds( 7, 73, width - 7, 10 );
    range_.setSliderStyle ( juce::Slider::TwoValueHorizontal );
    range_.setTextBoxStyle( juce::Slider::NoTextBox, true, 90, 20 );
    addToParentAndShow( *this, range_ );

    this->setBounds( 71, 156, width, 128 );

    switch_ .addListener( this );
    quarter_.addListener( this );
    triplet_.addListener( this );
    dotted_ .addListener( this );
    range_  .addListener( this );
    period_ .addListener( this );
    phase_  .addListener( this );
}

#pragma warning( pop )


LE_NOTHROW SpectrumWorxEditor::LFODisplay::~LFODisplay()
{
    editor().setDefaultFocusHandling();
}


void SpectrumWorxEditor::LFODisplay::setupForControl( ModuleControlBase & control, double const minimum, double const maximum, double const interval )
{
    pModuleControl_ = &control;

    range_.setRange( minimum, maximum, interval );

    // Implementation note:
    //   A two-valued juce::Slider does not allow to set a max periodScale that
    // is lower than the current min periodScale and vice verse. As a workaround
    // we first set both values to their respective extremes so that the
    // juce::Slider::set(Max/Min)Value() setter function would not alter our
    // values.
    //                                        (24.03.2010.) (Domagoj Saric)
    range_.setMaxValue( range_.getMaximum(), juce::dontSendNotification, false );
    range_.setMinValue( range_.getMinimum(), juce::dontSendNotification, false );

    //range_.setSkewFactor( control.getSkewFactor() );

    updateAllControls();

    addToParentAndShow( editor(), *this );

    repaint();
}


namespace
{
    bool skipPeriodRatio( SpectrumWorxEditor::LFODisplay::Period const & period )
    {
        return ( period.lastSyncType() == LFO::Free ) || !LFO::Timer::hasTempoInformation();
    }

    juce::String LE_FASTCALL periodRatioString( SpectrumWorxEditor::LFODisplay const & parent, double const & periodScale )
    {
        std::array<char, 16> buffer;

        double numerator;
        double denominator;
        char const * suffix;
        switch ( parent.period().lastSyncType() )
        {
            case LFO::Quarter:
                if ( periodScale < 1 )
                {
                    numerator   = 1;
                    denominator = 1 / periodScale;
                }
                else
                {
                    denominator = 1;
                    numerator   = periodScale;
                }
                suffix = "";
                break;

            case LFO::Triplet:
                if ( periodScale < 1 )
                {
                    numerator   = 1;
                    denominator = 1 / ( periodScale * 3 / 2 );
                }
                else
                {
                    denominator = 1;
                    numerator   = ( periodScale * 3 / 2 );
                }
                suffix = "T";
                break;

            case LFO::Dotted:
                if ( periodScale < 1 )
                {
                    numerator   = 1;
                    denominator = 1 / ( periodScale * 2 / 3 );
                }
                else
                {
                    denominator = 1;
                    numerator   = ( periodScale * 2 / 3 );
                }
                suffix = "D";
                break;

            LE_DEFAULT_CASE_UNREACHABLE();
        }

        unsigned int const charactersWritten
        (
            LE_INT_SPRINTFA
            (
                &buffer[ 0 ],
                "%u/%u%s bars",
                Math::convert<unsigned int>( numerator   ),
                Math::convert<unsigned int>( denominator ),
                suffix
            )
        );
        BOOST_ASSERT( charactersWritten < buffer.size() );
        return juce::String( &buffer[ 0 ], charactersWritten );
    }

    juce::String LE_FASTCALL periodMillisecondsString( SpectrumWorxEditor::LFODisplay const & parent, double const & periodScale )
    {
        LE_ASSUME( parent.period().milliseconds() == periodScale );

        bool          const skipRatio( skipPeriodRatio( parent.period() ) );
        unsigned char const precision( !skipRatio * 2                     );

        std::array<char, 32> buffer;
        auto const charactersWritten
        (
            Utility::lexical_cast( periodScale, precision, &buffer[ 0 ] )
        );
        std::strcpy( &buffer[ charactersWritten ], " ms" );

        return juce::String( &buffer[ 0 ] );
    }

    juce::String LE_FASTCALL phaseString( SpectrumWorxEditor::LFODisplay const & /*parent*/, double const & periodScale )
    {
        std::array<char, 32> buffer;
        auto const numberOfCharactersWritten( Utility::lexical_cast( periodScale * 100, 1, &buffer[ 0 ] ) );
        std::strcpy( &buffer[ numberOfCharactersWritten ], "%" );
        return juce::String( &buffer[ 0 ] );
    }

    juce::String LE_FASTCALL rangeValueString( SpectrumWorxEditor::LFODisplay const & parent, double const & periodScale )
    {
        BOOST_ASSERT( ModuleControlBase::activeControl() );
        BOOST_ASSERT( parent.control().isActive()        );
        return parent.control().getTextFromValue( static_cast<float>( periodScale ) );
    }

    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

    struct LFOTextData
    {
        typedef juce::String (LE_FASTCALL StringGetter)( SpectrumWorxEditor::LFODisplay const &, double const & );

        double                      value;
        StringGetter        * const getString;
        unsigned int          const x;
        unsigned int          const y;
        unsigned int          const width;
        unsigned int          const height;
        juce::Justification   const justification;
    };

    #pragma warning( pop )

    std::size_t const lfoWidth = 116;

    LFOTextData sliderTexts[] =
    {
        { 0, &periodRatioString       ,  9,  24,               105, 12, juce::Justification::right }, // period ratio
        { 0, &periodMillisecondsString,  9,  47,               105, 12, juce::Justification::right }, // period ms
        { 0, &rangeValueString        ,  9,  62, lfoWidth - 10 - 2, 12, juce::Justification::right }, // range max
        { 0, &rangeValueString        , 10,  84, lfoWidth - 10    , 12, juce::Justification::left  }, // range min
        { 0, &phaseString             ,  9, 118,               105, 12, juce::Justification::right }, // period ms
    };

    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

    struct FixedText
    {
        char         const * const string          ;
        unsigned int         const verticalPosition;
    };

    #pragma warning( pop )

    static FixedText const fixedText[] =
    {
        { "Period"  ,  24 + 9 },
        { "Range"   ,  62 + 9 },
        { "Waveform",  99 + 9 },
        { "Phase"   , 118 + 9 },
    };
} // anonymous namespace

void SpectrumWorxEditor::LFODisplay::paint( juce::Graphics & graphics )
{
    static_assert( lfoWidth == width, "" ); //...mrmlj...clean this up...

    //...mrmlj...ugh...2.6.x quick-fix workarounds...reinvestigate and clean this up...
    if ( !this->isEnabled() )
        return;
    BOOST_ASSERT( ModuleControlBase::activeControl() != nullptr    );
    BOOST_ASSERT( ModuleControlBase::activeControl() == &control() );
    BOOST_ASSERT( control().isActive()                             );
    BOOST_ASSERT( getParentComponent() == &editor()                );

    {
        graphics.setFont  ( DrawableText::defaultFont() );
        graphics.setColour( juce::Colours::white        );

        for ( auto const & text : fixedText )
            graphics.drawSingleLineText( text.string, 9, text.verticalPosition );
    }

    sliderTexts[ 0 ].value = period_.getValue    ();
    sliderTexts[ 1 ].value = period_.milliseconds();
    sliderTexts[ 2 ].value = range_ .getMaxValue ();
    sliderTexts[ 3 ].value = range_ .getMinValue ();
    sliderTexts[ 4 ].value = phase_ .getValue    ();

    for
    (
        auto const & text :
        boost::make_iterator_range( sliderTexts ).advance_begin( skipPeriodRatio( period_ ) )
    )
    {
        graphics.drawText
        (
            text.getString( *this, text.value ),
            text.x    , text.y     ,
            text.width, text.height,
            text.justification,
            false
        );
    }

    paintImage( graphics, type_.getSelectedItemIcon(), 79, 96 );
}


void SpectrumWorxEditor::LFODisplay::buttonClicked( juce::Button * const pButton )
{
    auto & lfo( this->lfo() );

    if ( pButton == &switch_ )
    {
        bool const enable( switch_.getToggleState() );
        BOOST_ASSERT( enable != lfo.enabled() );
        updateParameterAndNotifyHost<LFO::Enabled>( enable );
        control().lfoStateChanged         ();
        editor ().updateActiveControlValue();
    }
    else
    if ( pButton == &typeArrow_ )
    {
        //...mrmlj...
        if ( !type_.menuActive() && type_.showCenteredAtRight( typeArrow_ ) )
        {
            updateParameterAndNotifyHost<LFO::Waveform>( type_.getSelectedID() );
            this->repaint();
        }
    }
    else
    {
        BOOST_ASSERT( LFO::Timer::hasTempoInformation() );

        LFO::SyncType syncType;
             if            ( pButton == &quarter_ ) { syncType = LFO::Quarter; }
        else if            ( pButton == &triplet_ ) { syncType = LFO::Triplet; }
        else { BOOST_ASSERT( pButton == &dotted_  );  syncType = LFO::Dotted ; }

        bool const addSyncType( pButton->getToggleState() );

        if ( addSyncType ) lfo.addSyncType   ( syncType );
        else               lfo.removeSyncType( syncType );

        updatePeriodControl              ();
        updateLFOAndHostFromPeriodControl();
        this->repaint();
    }
}


void SpectrumWorxEditor::LFODisplay::sliderValueChanged( juce::Slider * const pSlider ) noexcept
{
    repaint();

    if ( pSlider == &range_ )
    {
        auto const newLowerBound( rangeSliderValueToLFOValue( range_, range_.getMinValue() ) );
        auto const newUpperBound( rangeSliderValueToLFOValue( range_, range_.getMaxValue() ) );

        updateParameterAndNotifyHost<LFO::LowerBound>( newLowerBound );
        updateParameterAndNotifyHost<LFO::UpperBound>( newUpperBound );
    }
    else
    if ( pSlider == &period_ )
    {
        updateLFOAndHostFromPeriodControl();
    }
    else
    {
        BOOST_ASSERT( pSlider == &phase_ );
        updateParameterAndNotifyHost<LFO::Phase>( phase_.getValue() );
    }
}


LE_NOTHROW
void SpectrumWorxEditor::LFODisplay::updateForNewTimingInfo()
{
    updatePeriodControl();
    updateSnapControls ();
    verifyGUIAndLFOConsistency();
    repaint();
}


LE_NOTHROW
void SpectrumWorxEditor::LFODisplay::updateForChangedParameters( ModuleUI const & moduleUI, std::uint8_t const parameterIndex, std::uint8_t const lfoParameterIndex, Plugins::AutomatedParameterValue /*const value*/ )
{
    if
    (
        ( &moduleUI      == &control().moduleUI            () ) &&
        ( parameterIndex ==  control().moduleParameterIndex() )
    )
    {
        if ( lfoParameterIndex == LE::Parameters::IndexOf<LFO::Parameters, LFO::Enabled>::value )
            control().lfoStateChanged();
        updateAutomatableControls();
        repaint();
    }
    verifyGUIAndLFOConsistency();
}


LE_NOTHROWNOALIAS
void SpectrumWorxEditor::LFODisplay::updateAllControls()
{
    updateAutomatableControls();
    updateSnapControls       ();
    type_.setSelectedID( lfo().waveForm() );
    verifyGUIAndLFOConsistency();
}

LE_NOTHROWNOALIAS
void SpectrumWorxEditor::LFODisplay::updateAutomatableControls()
{
    updatePeriodControl();
    updateRangeControl ();
    auto & lfo( this->lfo() );
    switch_.setToggleState( lfo.enabled(), juce::dontSendNotification );
    phase_ .setValue      ( lfo.phase  (), juce::dontSendNotification );
}

LE_NOTHROWNOALIAS
void SpectrumWorxEditor::LFODisplay::updatePeriodControl()
{
    auto & lfo( this->lfo() );

    float  const rangeMinimum  ( LFO::PeriodScale::minimum() );
    float  const rangeMaximum  ( LFO::PeriodScale::maximum() );
    double const rangeBeginning( LFO::snapPeriodScale( rangeMinimum, lfo.syncTypes() ).first );
    double const rangeEnd      ( LFO::snapPeriodScale( rangeMaximum, lfo.syncTypes() ).first );
    double const step
    (
        ( lfo.syncTypes() != LFO::Free )
            ? 0
            : 1 / 1000.0 / LFO::Timer::basePeriod() // 1 ms
    );

    period_.setRange( rangeBeginning, rangeEnd, step );
    period_.setSkewFactorFromMidPoint( 1 );

    double const resnappedValue( period_.Period::snapValue( lfo.periodScale(), false ) );
    lfo.setPeriodScale( static_cast<LFO::value_type>( resnappedValue ) ); //...mrmlj...rethink whether this should be done by the LFO class...
    period_.setValue  ( resnappedValue, juce::dontSendNotification );

    verifyGUIAndLFOConsistency();
}

LE_NOTHROWNOALIAS
void SpectrumWorxEditor::LFODisplay::updateRangeControl()
{
    auto & lfo( this->lfo() );
    range_.setMaxValue( lfoValueToRangeSliderValue( range_, lfo.upperBound() ), juce::dontSendNotification, false );
    range_.setMinValue( lfoValueToRangeSliderValue( range_, lfo.lowerBound() ), juce::dontSendNotification, false );
}

LE_NOTHROWNOALIAS
void SpectrumWorxEditor::LFODisplay::updateSnapControls()
{
    if ( LFO::Timer::hasTempoInformation() )
    {
        auto & lfo( this->lfo() );
        quarter_.setToggleState( lfo.hasEnabledSync( LFO::Quarter ), juce::dontSendNotification );
        triplet_.setToggleState( lfo.hasEnabledSync( LFO::Triplet ), juce::dontSendNotification );
        dotted_ .setToggleState( lfo.hasEnabledSync( LFO::Dotted  ), juce::dontSendNotification );
    }
    else
    {
        quarter_.setEnabled( false );
        triplet_.setEnabled( false );
        dotted_ .setEnabled( false );
    }
}


LE_NOTHROWNOALIAS
void SpectrumWorxEditor::LFODisplay::updateLFOAndHostFromPeriodControl()
{
    updateParameterAndNotifyHost<LFO::PeriodScale>( period_.getValue() );
}


void SpectrumWorxEditor::LFODisplay::automatedParameterChanged( std::uint8_t const lfoParameterIndex, float const parameterValue ) const
{
    auto const moduleParameterIndex( control().moduleParameterIndex() );

    if ( moduleParameterIndex >= ( SW::Constants::maxNumberOfParametersPerModule - 1 ) )
        return;

    ParameterID::LFO const lfoParameterID = { lfoParameterIndex, moduleParameterIndex, moduleIndex() };
#ifdef LE_SW_FMOD
    Host2PluginInteropControler::AutomationBlocker const automationBlocker( const_cast<SpectrumWorxEditor &>( editor() ).moduleChainOwner() );
#endif // LE_SW_FMOD
    editor().host().automatedParameterChanged( lfoParameterID, parameterValue );
}


void SpectrumWorxEditor::LFODisplay::verifyGUIAndLFOConsistency() const
{
#ifndef NDEBUG
    //...mrmlj...
    //...mrmlj...the rounding error difference is too great even for the nearEqual() function...
    //BOOST_ASSERT( Math::nearEqual( lfo().periodScale(), static_cast<LFO::value_type>( period_.getValue() ) ) );
    //BOOST_ASSERT( Math::abs( lfo().periodScale() - period_.getValue() ) < 0.001 );
    double const guiPeriod( lfo().periodScale() );
    double const lfoPeriod( period_.getValue () );
    BOOST_ASSERT( Math::abs( guiPeriod - lfoPeriod ) < 0.001 );
#endif // NDEBUG
}


std::uint8_t SpectrumWorxEditor::LFODisplay::moduleIndex() const
{
    auto const moduleIndex( editor().program().moduleChain().getIndexForModule( control().module() ) );
    return moduleIndex;
}

LE_NOTHROW LE_PURE_FUNCTION
SpectrumWorxEditor & SpectrumWorxEditor::LFODisplay::editor()
{
    SpectrumWorxEditor & editor( Utility::ParentFromOptionalMember<SpectrumWorxEditor, LFODisplay, &SpectrumWorxEditor::lfoDisplay_, false>()( *this ) );
    BOOST_ASSERT( ( &editor == this->getParentComponent() ) || !this->getParentComponent() );
    return editor;
}

SpectrumWorxEditor const & SpectrumWorxEditor::LFODisplay::editor() const
{
    return const_cast<SpectrumWorxEditor::LFODisplay &>( *this ).editor();
}


double SpectrumWorxEditor::LFODisplay::Period::snapValue( double const attemptedValue, bool /*userIsDragging*/ ) LE_GNU_SPECIFIC( noexcept )
{
    LFO::SnappedPeriod const result
    (
        LFO::snapPeriodScale
        (
            static_cast<float>( attemptedValue ),
            parent().lfo().syncTypes()
        )
    );
    lastSyncType_ = result.second;
    return result.first;
}


double SpectrumWorxEditor::LFODisplay::Period::milliseconds() const
{
#ifdef LE_SW_FMOD
    return 1000;
#else
    float  const basePeriod          ( parent().editor().effect().lfoTimer().basePeriod() );
    double const periodInMilliseconds( this->getValue() * basePeriod * 1000               );
    return periodInMilliseconds;
#endif // LE_SW_FMOD

}


SpectrumWorxEditor::LFODisplay const & SpectrumWorxEditor::LFODisplay::Period::parent() const
{
    return Utility::ParentFromMember<LFODisplay, Period, &LFODisplay::period_>()( *this );
}


#ifndef LE_SW_DISABLE_SIDE_CHANNEL
SpectrumWorxEditor::SampleArea::SampleArea()
{
    setMouseCursor( juce::MouseCursor::PointingHandCursor );
    addToParentAndShow( editor(), *this );
}


void SpectrumWorxEditor::SampleArea::mouseUp( juce::MouseEvent const & event )
{
    SpectrumWorxEditor & editor( this->editor() );
    juce::ModifierKeys const mouseButtons( event.mods );
    if ( mouseButtons.isRightButtonDown() )
    {
        editor.newSampleFileSelected( juce::File::nonexistent );
    }
    else
    if ( mouseButtons.isLeftButtonDown() )
    {
        juce::FileChooser fileChooser
        (
            "Choose external audio file",
            //juce::File::getSpecialLocation( juce::File::userMusicDirectory ), //...mrmlj... for testing...
            //juce::File::nonexistent,
            editor.effect().currentSampleFile(),
            Sample::supportedFormats(),
            true
        );
        if ( fileChooser.browseForFileToOpen( 0 ) )
        {
            BOOST_ASSERT( fileChooser.getResults().size() == 1 );
            editor.newSampleFileSelected( fileChooser.getResults().getReference( 0 ) );
        }
    }
}

LE_NOTHROW LE_PURE_FUNCTION
SpectrumWorxEditor & SpectrumWorxEditor::SampleArea::editor()
{
    return Utility::ParentFromMember<SpectrumWorxEditor, SampleArea, &SpectrumWorxEditor::sampleArea_>()( *this );
}
#endif // LE_SW_DISABLE_SIDE_CHANNEL


////////////////////////////////////////////////////////////////////////////////
//
// SpectrumWorxEditor::Settings::Settings()
// ----------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.

SpectrumWorxEditor::Settings::Settings() /// \throws std::bad_alloc Out of memory
    :
    juce::TabbedComponent( juce::TabbedButtonBar::TabsAtTop ),

    fftSize_         ( enginePage_, xMargin, yMargin + yStep * 0, (Engine          ::FFTSize          *)( 0 ) ),
    overlapFactor_   ( enginePage_, xMargin, yMargin + yStep * 1, (Engine          ::OverlapFactor    *)( 0 ) ),
    windowFunction_  ( enginePage_, xMargin, yMargin + yStep * 2, (Engine          ::WindowFunction   *)( 0 ) ),
#if LE_SW_ENGINE_WINDOW_PRESUM
    windowSizeFactor_( enginePage_, xMargin, yMargin + yStep * 3, (Engine          ::WindowSizeFactor *)( 0 ) ),
    #if LE_SW_ENGINE_INPUT_MODE >= 1
    inputMode_       ( enginePage_, xMargin, yMargin + yStep * 4, (GlobalParameters::InputMode        *)( 0 ) ),
    #endif // LE_SW_ENGINE_INPUT_MODE
#else
    #if LE_SW_ENGINE_INPUT_MODE >= 1
    inputMode_       ( enginePage_, xMargin, yMargin + yStep * 3, (GlobalParameters::InputMode        *)( 0 ) ),
    #endif
#endif // LE_SW_ENGINE_WINDOW_PRESUM

    pRegistrationData_( 0 )
{
    this->setSize( resourceBitmap<SettingsEngineBg>().getWidth(), editor().getHeight() );

#if LE_SW_ENGINE_INPUT_MODE >= 2 && defined( __APPLE__ )
    inputMode_->setEnabled( !editor().effect().completelyDisableIOChanges() );
#elif LE_SW_ENGINE_INPUT_MODE == 1
    inputMode_       ->setEnabled( false );
#endif // LE_SW_ENGINE_INPUT_MODE
#if defined( LE_SW_FMOD )
    windowFunction_  ->setEnabled( false );
#endif
#if LE_SW_ENGINE_WINDOW_PRESUM
    windowSizeFactor_->setEnabled( false );
#endif // LE_SW_ENGINE_WINDOW_PRESUM

    updateEnginePage              ();
    updateLoadLastSessionOnStartup();

#if LE_SW_AUTHORISATION_REQUIRED
    if ( editor().authorised() )
    {
        setRegisteredTo( editor().effect().authorizationData() );
    }
    else
    {
        registrationPage_.authorize_.addListener( this );
        registrationPage_.buyNow_   .addListener( this );
    }
#else
    {
        static AuthorisationData authorizationData;
    #ifdef LE_SW_FMOD
        authorizationData.line( 1 ) = "FMOD Studio";
        authorizationData.line( 2 ) = "Firelight Technologies Pty";
        authorizationData.line( 3 ) = "Melbourne";
        authorizationData.line( 4 ) = "Australia";

        authorizationData.licenceType() = "FMOD";
    #else // LE_SW_FMOD
        authorizationData.line( 1 ) = "Everyone";
      //authorizationData.line( 2 ) = "N/A";
      //authorizationData.line( 3 ) = "N/A";
      //authorizationData.line( 4 ) = "N/A";

        authorizationData.licenceType() = "Free";
    #endif
        setRegisteredTo( authorizationData );
    }
#endif

    aboutPage_.showUsersGuide_.addListener( this );

    setOutline( 0 );
    setIndent ( 0 );
    setTabBarDepth( resourceBitmap<SettingsEngineOn>().getHeight() );

    juce::String const dummyName( "a" );
    addTab( dummyName, juce::Colours::transparentBlack, &enginePage_      , false );
    addTab( dummyName, juce::Colours::transparentBlack, &interfacePage_   , false );
    addTab( dummyName, juce::Colours::transparentBlack, &registrationPage_, false );
    addTab( dummyName, juce::Colours::transparentBlack, &aboutPage_       , false );

    OwnedWindow<Settings>::attach();
}


SpectrumWorxEditor::Settings::~Settings()
{
    //...mrmlj..."desktop window" fade-out does not work with the current JUCE
    //getCurrentContentComponent()->fadeOutComponent( 200, 0, 0, 0.2f );
    //this->fadeOutComponent( 200, 0, 0, 0.2f );
    clearTabs();
}


void SpectrumWorxEditor::Settings::setRegisteredTo( AuthorisationData const & registrationData )
{
    BOOST_ASSERT( !pRegistrationData_ );
    BOOST_ASSERT( registrationData.authorised() );

    pRegistrationData_ = &registrationData;

    registrationPage_.setRegistered();
}


void SpectrumWorxEditor::Settings::sliderValueChanged( juce::Slider * const pSlider ) noexcept
{
    LE_ASSUME( pSlider == &interfacePage_.opacitySlider() );
    Theme::singleton().settings().globalOpacity = pSlider->getValue();

    juce::Colour const tabBackground( juce::Colours::black.withAlpha( static_cast<float>( std::pow( Theme::singleton().settings().globalOpacity, 14 ) ) ) );
    BOOST_ASSERT( getTabbedButtonBar().getNumTabs() == 4 );
    for ( unsigned int i( 0 ); i < 4; ++i )
        getTabbedButtonBar().setTabBackgroundColour( i, tabBackground );

    // Force repaint
    for ( unsigned int i( 0 ); i < static_cast<unsigned int>( juce::ComponentPeer::getNumPeers() ); ++i )
    {
        juce::ComponentPeer & peer( *juce::ComponentPeer::getPeer( i ) );
        juce::Rectangle<int> bounds( peer.getBounds() );
        bounds.setPosition( 0, 0 );
        peer.repaint( bounds );
    }

    boost::ignore_unused_variable_warning( pSlider );
}


#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.

void SpectrumWorxEditor::Settings::comboBoxValueChanged( ComboBox const & comboBox )
{
    auto & settings( *boost::polymorphic_downcast<Settings *>( comboBox.getParentComponent()->getParentComponent() ) );
    auto & editor  ( settings.editor() );

    unsigned int const value( comboBox.getValue() );

    using namespace GlobalParameters;
    typedef GlobalParameters::Parameters Parameters;

         if ( &comboBox == &settings.fftSize_          ) { BOOST_VERIFY( editor.globalParameterChanged<FFTSize         >( static_cast<FFTSize         ::value_type>( value ), true ) ); }
    else if ( &comboBox == &settings.overlapFactor_    ) { BOOST_VERIFY( editor.globalParameterChanged<OverlapFactor   >( static_cast<OverlapFactor   ::value_type>( value ), true ) ); }
    else if ( &comboBox == &settings.windowFunction_   ) { BOOST_VERIFY( editor.globalParameterChanged<WindowFunction  >( static_cast<WindowFunction  ::value_type>( value ), true ) ); }
#if LE_SW_ENGINE_WINDOW_PRESUM
    else if ( &comboBox == &settings.windowSizeFactor_ ) { BOOST_VERIFY( editor.globalParameterChanged<WindowSizeFactor>( static_cast<WindowSizeFactor::value_type>( value ), true ) ); }
#endif // LE_SW_ENGINE_WINDOW_PRESUM
#if LE_SW_ENGINE_INPUT_MODE >= 2
    else if ( &comboBox == &settings.inputMode_ )
    {
        LE_ASSUME( !editor.effect().completelyDisableIOChanges() );
        /*BOOST_VERIFY*/( editor.globalParameterChanged<InputMode>( static_cast<InputMode::value_type>( value ), true ) );
        settings.updateLoadLastSessionOnStartup();
        settings.inputMode_->setValue( editor.program().parameters(). template get<InputMode>().getValue() );
    }
#endif // LE_SW_ENGINE_INPUT_MODE
    else if ( &comboBox == &settings.interfacePage_.mouseOverComboBox() )
    {
        Theme::singleton().settings().moduleUIMouseOverReaction = static_cast<Theme::ModuleUIMouseOverReaction>( value );
    }
    else if ( &comboBox == &settings.interfacePage_.lfoUpdateComboBox() )
    {
        Theme::singleton().settings().lfoUpdateBehaviour        = static_cast<Theme::LFOUpdateBehaviour       >( value );
    }
    else
    {
        LE_UNREACHABLE_CODE();
    }

    settings.enginePage_.setNewQualityFactor( editor.engineSetup().wolaRippleFactor() );
}

#pragma warning( pop )


void SpectrumWorxEditor::Settings::updateEnginePage()
{
    auto const & editor     ( this->editor()                );
    auto const & engineSetup( editor.engineSetup()          );
    auto const & parameters ( editor.program().parameters() );
    // Implementation note:
    //   In rare circumstances this function gets called very often (if engine
    // setup parameters change rapidly, e.g. someone automates them using the
    // Ableton Live's 'dual control') and it gets called asynchronously to the
    // actual Engine::Setup instance updating. This can cause the Engine::Setup
    // instance to get 'out-of-date' which in turn would cause an assertion
    // failure if the SpectrumWorx::engineSetup() getter was used. Because
    // of this the SpectrumWorx::uncheckedEngineSetup() getter is used to
    // avoid the assertion failures.
    //   This is safe to do as a non-up-to-date engine setup is harmless here,
    // there will surely be a next message/asynchronous call when it will be up
    // to date).
    //                                        (15.06.2010.) (Domagoj Saric)

    fftSize_         ->setValue( parameters.get<Engine::FFTSize         >() );
    overlapFactor_   ->setValue( parameters.get<Engine::OverlapFactor   >() );
    windowFunction_  ->setValue( parameters.get<Engine::WindowFunction  >() );
#if LE_SW_ENGINE_WINDOW_PRESUM
    windowSizeFactor_->setValue( parameters.get<Engine::WindowSizeFactor>() );
#endif // LE_SW_ENGINE_WINDOW_PRESUM
#if LE_SW_ENGINE_INPUT_MODE >= 1
    unsigned int const customInputMode( SpectrumWorxCore::InputMode::maximum() + 1 );
    unsigned int const inputModeValue
    (
        ( engineSetup.numberOfChannels() > 2 )
            ? customInputMode
            : parameters.get<SpectrumWorxCore::InputMode>().getValue()
    );
    if ( ( inputModeValue == customInputMode ) && ( inputMode_->numberOfItems() == SpectrumWorxCore::InputMode::numberOfDiscreteValues ) )
    {
        inputMode_->addItem( customInputMode, "<custom>", juce::Image::null, false );
    }
    inputMode_->setValue( inputModeValue );
#endif // LE_SW_ENGINE_INPUT_MODE
    enginePage_.setNewQualityFactor( engineSetup.wolaRippleFactor() );
}


SpectrumWorxEditor::Settings::EnginePage::EnginePage()
    :
    BackgroundImage( resourceBitmap<SettingsEngineBg>() )
{}


void SpectrumWorxEditor::Settings::EnginePage::setNewQualityFactor( float const & qualityFactorParam )
{
    float const qualityFactor( qualityFactorParam );
    // Implementation note:
    //   In this document http://eprints.kfupm.edu.sa/21525/1/21525.pdf (at the
    // end of page 32) it is argued that a variation of 0.03% or less is
    // negligible.
    //                                        (25.01.2010.) (Domagoj Saric)
    char const * description;
    if ( qualityFactor < 0.0003f )
        description = "% (excellent)";
    else
    if ( qualityFactor < 0.01f )
        description = "% (average)";
    else
        description = "% (poor)";
    char buffer[ 32 ];
    BOOST_VERIFY( Utility::lexical_cast( qualityFactor * 100.0f, 2, buffer ) < _countof( buffer ) );
    *engineQuality_.getCharPointer().getAddress() = 0;
    engineQuality_ += "Ripple amount: ";
    engineQuality_ += buffer;
    engineQuality_ += description;
#if LE_SW_SEPARATED_DSP_GUI //...mrmlj...
    engineQuality_ = "N/A";
#endif // LE_SW_SEPARATED_DSP_GUI
}

namespace
{
    void printEngineDiagnostics
    (
        juce::String & buffer,
        char const * const title,
        float        const value,
        char const * const suffix,
        unsigned int const verticalOffset,
        juce::Graphics const & graphics
    )
    {
        char valueStr[ 32 ];
        BOOST_VERIFY( Utility::lexical_cast( value, 1, valueStr ) < _countof( valueStr ) );
        buffer  = title;
        buffer += ": ";
        buffer += valueStr;
        buffer += ' ';
        buffer += suffix;
        graphics.drawFittedText( buffer, SpectrumWorxEditor::Settings::xMargin + 4, verticalOffset, 142, 12, juce::Justification::centred, 1 );
    }
}  // anonymous namespace

void SpectrumWorxEditor::Settings::EnginePage::paint( juce::Graphics & g )
{
    BackgroundImage::paint( g );
    g.setColour( juce::Colours::white );
    g.setFont  ( DrawableText::defaultFont() );
    g.drawFittedText( engineQuality_, xMargin + 4, yMargin + yStep * 5, 142, 12, juce::Justification::centred, 1 );

    Settings            & settings   ( Utility::ParentFromMember<Settings, EnginePage, &Settings::enginePage_>()( *this ) );
    Engine::Setup const & engineSetup( settings.editor().engineSetup() );
    juce::String tmp;
    tmp.preallocateBytes( sizeof( juce::String::CharPointerType::CharType ) * 64 );
    printEngineDiagnostics( tmp, "Frequency resolution", engineSetup.frequencyRangePerBin<float>(), "Hz", yMargin + yStep * 5 + 20, g );
    printEngineDiagnostics( tmp, "Time resolution"     , engineSetup.stepTime() * 1000            , "ms", yMargin + yStep * 5 + 40, g );
    printEngineDiagnostics( tmp, "Latency"             , engineSetup.latencyInMilliseconds()      , "ms", yMargin + yStep * 5 + 60, g );

    //...mrmlj...for testing...
    //g.drawSingleLineText( engineQuality_, xMargin - 5, yMargin + yStep * 6 + 12 );
}


SpectrumWorxEditor::Settings::InterfacePage::InterfacePage()
    :
    BackgroundImage( resourceBitmap<SettingsIntrfcBg>() ),
    opacityTitle_             ( "Side window & menu opacity", xMargin + 7, yMargin + 3 * yStep + 15, opacityWidth + 40, 16, juce::Justification::left ),
    globalOpacity_            ( juce::String::empty                                                                    ),
    moduleUIMouseOverReaction_( *this, xMargin    , yMargin + 0 * yStep     , "Mouse over reaction"          ),
    lfoUpdateBehaviour_       ( *this, xMargin    , yMargin + 1 * yStep     , "LFO update behaviour"         ),
    loadLastSessionOnStartup_ ( *this, xMargin - 4, yMargin + 2 * yStep     , "Load last session on startup" ),
    hideCursorOnKnobDrag_     ( *this, xMargin - 4, yMargin + 3 * yStep - 15, "Hide cursor on knob drag"     )
{
    Settings & parent( Utility::ParentFromMember<Settings, InterfacePage, &Settings::interfacePage_>()( *this ) );

    globalOpacity_.setBounds
    (
        xMargin + 7 , yMargin + 3 * yStep + 12 + 20,
        opacityWidth, 16
    );
    globalOpacity_.setSliderStyle           ( juce::Slider::LinearHorizontal );
    globalOpacity_.setTextBoxStyle          ( juce::Slider::NoTextBox, true, 10, 12 );
    globalOpacity_.setRange                 ( 0.8, 1 );
    globalOpacity_.setValue                 ( Theme::singleton().settings().globalOpacity, juce::dontSendNotification );
    globalOpacity_.setDoubleClickReturnValue( true, 0.9 );
  //globalOpacity_.setTooltip               ( globalOpacity_.getName() );
    globalOpacity_.addListener( &parent );

    moduleUIMouseOverReaction_.addItem( Theme::Never                      , "Never"                      );
    moduleUIMouseOverReaction_.addItem( Theme::WhenParentModuleSelected   , "Module selected"            );
    moduleUIMouseOverReaction_.addItem( Theme::WhenParentOrNothingSelected, "Module/nothing selected"    );
    moduleUIMouseOverReaction_.setSelectedIndex( Theme::singleton().settings().moduleUIMouseOverReaction );

    lfoUpdateBehaviour_.addItem( Theme::NoUpdate           , "Never"            );
    lfoUpdateBehaviour_.addItem( Theme::WhenControlSelected, "Control selected" );
    lfoUpdateBehaviour_.addItem( Theme::WhenControlActive  , "Control active"   );
    lfoUpdateBehaviour_.addItem( Theme::Always             , "Always"           );
    lfoUpdateBehaviour_.setSelectedIndex( Theme::singleton().settings().lfoUpdateBehaviour );

    loadLastSessionOnStartup_.addListener( &parent );

    hideCursorOnKnobDrag_.setToggleState( Theme::singleton().settings().hideCursorOnKnobDrag, juce::dontSendNotification );
    hideCursorOnKnobDrag_.addListener( &parent );

    addToParentAndShow( *this, globalOpacity_ );
}


void SpectrumWorxEditor::Settings::InterfacePage::paint( juce::Graphics & graphics )
{
    graphics.setColour( juce::Colours::white );
    BackgroundImage::paint( graphics );
    opacityTitle_.draw( graphics );
}


SpectrumWorxEditor::Settings::RegistrationPage::RegistrationPage()
    :
    BackgroundImage( resourceBitmap<SettingsRegBg>() ),
    authorize_( *this, resourceBitmap<AuthorizeDown>(), resourceBitmap<AuthorizeUp>() ),
    buyNow_   ( *this, resourceBitmap<BuyNowDown   >(), resourceBitmap<BuyNowUp   >() )
{
    authorize_.setTopLeftPosition( 101, 158 );
    buyNow_   .setTopLeftPosition(  13, 158 );

    authorize_.setClickingTogglesState( false );
    buyNow_   .setClickingTogglesState( false );
}

#pragma warning( pop )


void SpectrumWorxEditor::Settings::RegistrationPage::paint( juce::Graphics & graphics )
{
    BackgroundImage::paint( graphics );
    Settings const & parent( Utility::ParentFromMember<Settings, RegistrationPage, &Settings::registrationPage_>()( *this ) );
    AuthorisationData const * const pRegistrationData( parent.registrationData() );
    if ( pRegistrationData )
    {
        BOOST_ASSERT( pRegistrationData->line( 1 ).isNotEmpty() );
        graphics.setColour( juce::Colours::white );
        juce::GlyphArrangement text;

        float const fontSize ( 12           );
        float const rowHeight( fontSize + 2 );
        juce::Font const font( fontSize     );

        juce::Justification const justification( juce::Justification::horizontallyJustified | juce::Justification::verticallyCentred );

        for ( unsigned int i( 0 ); i < 4; ++i )
        {
            text.addFittedText
            (
                font,
                pRegistrationData->line( i + 1 ),
                14 , 56 + ( rowHeight * i ),
                162, rowHeight,
                justification,
                1,
                0.2f
            );
        }

        text.draw( graphics );
    }
}


void SpectrumWorxEditor::Settings::RegistrationPage::setRegistered()
{
    setImage( resourceBitmap<SettingsRegDoneBg>() );
    authorize_.setInvisible();
    buyNow_   .setInvisible();
}


#pragma warning( push )
#pragma warning( disable : 4355 ) // 'this' used in base member initializer list.
SpectrumWorxEditor::Settings::AboutPage::AboutPage()
    :
    BackgroundImage( resourceBitmap<SettingsAboutBg>()                                       ),
    versionText_   ( SW_VERSION_STRING SW_EDITION_STRING, 65, 43, 107, 16                    ),
    showUsersGuide_( *this, resourceBitmap<UsersGuideDown>(), resourceBitmap<UsersGuideUp>() )
{
    showUsersGuide_.setTopLeftPosition     ( 101, 108 );
    showUsersGuide_.setClickingTogglesState( false    );
}
#pragma warning( pop )


void SpectrumWorxEditor::Settings::AboutPage::paint( juce::Graphics & graphics )
{
    BackgroundImage::paint( graphics );
    graphics.setColour( juce::Colours::white );
    versionText_.draw( graphics );

    Settings & parent( Utility::ParentFromMember<Settings, AboutPage, &Settings::aboutPage_>()( *this ) );
    AuthorisationData const * const pRegistrationData( parent.registrationData() );
    graphics.setFont( juce::Font( 11 ) );
    graphics.drawText
    (
        pRegistrationData ? pRegistrationData->licenceType() : "Demo",
         65, 56,
        107, 14,
        juce::Justification::left,
        false
    );
}


class SettingsTab : public juce::TabBarButton
{
public:
    using Images = std::array<juce::Image const *, 2>; // [ inactive, active ]

public:
    SettingsTab( juce::String const & tabName, juce::TabbedButtonBar & ownerBar, Images const & images )
        : TabBarButton( tabName, ownerBar ), images_( images ) {}

private:
    int getBestTabLength( int /*depth*/ ) LE_OVERRIDE
    {
        return images_[ false ]->getWidth();
    }

    bool hitTest( int /*mx*/, int /*my*/ ) LE_OVERRIDE
    {
        return true;
    }

    void paint( juce::Graphics & graphics ) LE_OVERRIDE
    {
        paintImage( graphics, *images_[ getToggleState() ] );
    }

private:
    Images const images_;
};


juce::TabBarButton * SpectrumWorxEditor::Settings::createTabButton( juce::String const & tabName, int const tabIndex )
{
    SettingsTab::Images images;
    switch ( tabIndex )
    {
        case 0: images[ 0 ] = &resourceBitmap<SettingsEngineOff>(); images[ 1 ] = &resourceBitmap<SettingsEngineOn>(); break;
        case 1: images[ 0 ] = &resourceBitmap<SettingsGUIOff   >(); images[ 1 ] = &resourceBitmap<SettingsGUIOn   >(); break;
        case 2: images[ 0 ] = &resourceBitmap<SettingsRegOff   >(); images[ 1 ] = &resourceBitmap<SettingsRegOn   >(); break;
        case 3: images[ 0 ] = &resourceBitmap<SettingsAboutOff >(); images[ 1 ] = &resourceBitmap<SettingsAboutOn >(); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    return new SettingsTab( tabName, getTabbedButtonBar(), images );
}

LE_NOTHROW LE_PURE_FUNCTION
SpectrumWorxEditor & SpectrumWorxEditor::Settings::editor()
{
    return Utility::ParentFromOptionalMember<SpectrumWorxEditor, Settings, &SpectrumWorxEditor::settings_, false>()( *this );
}


void SpectrumWorxEditor::Settings::buttonClicked( juce::Button * const pButton )
{
#if !LE_SW_SEPARATED_DSP_GUI || LE_SW_AUTHORISATION_REQUIRED
    SpectrumWorx & effect( editor().effect() );
#endif
    if ( pButton == &interfacePage_.loadLastSessionOnStartup_ )
    {
    #if LE_SW_SEPARATED_DSP_GUI
        BOOST_ASSERT_MSG( false, "Not yet implemented!" );
    #else
        effect.shouldLoadLastSessionOnStartup( interfacePage_.loadLastSessionOnStartup_.getToggleState() );
    #endif // LE_SW_SEPARATED_DSP_GUI
    }
    else
    if ( pButton == &interfacePage_.hideCursorOnKnobDrag_ )
    {
        Theme::settings().hideCursorOnKnobDrag = interfacePage_.hideCursorOnKnobDrag_.getToggleState();
    }
#if LE_SW_AUTHORISATION_REQUIRED
    else
    if ( pButton == &registrationPage_.authorize_ )
    {
        juce::FileChooser fileChooser
        (
            _T( "Please select your licence file..." ),
            juce::File::nonexistent,
            _T( "*" ) SW_LICENCE_FILE_EXTENSION_STANDARD   _T( ";" ) \
            _T( "*" ) SW_LICENCE_FILE_EXTENSION_OS_UPGRADE
            #if SW_ENABLE_UPGRADE
                _T( ";" ) _T( "*" ) SW_LICENCE_FILE_EXTENSION_VERSION_UPGRADE
            #endif
            ,true
        );
        if ( fileChooser.browseForFileToOpen( 0 ) )
        {
            BOOST_ASSERT( fileChooser.getResults().size() == 1 );
            juce::File const & selectedFile( fileChooser.getResults().getReference( 0 ) );
            char const * LE_RESTRICT pErrorMessage
            (
                selectedFile.moveFileTo( licencesPath().getNonexistentChildFile( selectedFile.getFileNameWithoutExtension(), selectedFile.getFileExtension() ) )
                    ? nullptr
                    : "Failed to move the license file."
            );

            if ( !pErrorMessage )
                pErrorMessage = effect.verifyLicence();

            if ( !pErrorMessage )
            {
                BOOST_ASSERT( effect.authorizationData().authorised() );
                setRegisteredTo( effect.authorizationData() );
                if ( editor().presetBrowser_.is_initialized() )
                    editor().presetBrowser_->authorize();
                registrationPage_.repaint();
                editor()         .repaint(); //...mrmlj...only the "DEMO" region...
            }
            else
            {
                //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
                BOOST_ASSERT( !effect.authorizationData().authorised() );
                effect.authorizationData_.clear();
                static char const * LE_RESTRICT volatile const authorizationFailedTitle( "Authorization failed..." );
                char const * const authorizationFailedTitleAux( authorizationFailedTitle );
                GUI::warningMessageBox( authorizationFailedTitleAux, pErrorMessage, false );
            }
        }
    }
    else
    if ( pButton == &registrationPage_.buyNow_ )
    {
        // Implementation note:
        //   The http:// prefix is required for OSX.
        //                                    (10.11.2010.) (Domagoj Saric)
        //...mrmlj...bloated...BOOST_VERIFY( juce::URL( "http://www.littleendian.com" ).launchInDefaultBrowser() );
        BOOST_VERIFY( juce::Process::openDocument( "http://www.littleendian.com", juce::String::empty ) );
    }
#endif // LE_SW_AUTHORISATION_REQUIRED
    else
    if ( pButton == &aboutPage_.showUsersGuide_ )
    {
        BOOST_VERIFY( juce::Process::openDocument( rootPath().getChildFile( "Documents/User's Guide.PDF" ).getFullPathName(), juce::String::empty ) );
    }
}


void SpectrumWorxEditor::Settings::updateLoadLastSessionOnStartup()
{
#if !LE_SW_SEPARATED_DSP_GUI
    interfacePage_.loadLastSessionOnStartup_.setToggleState
    (
        editor().effect().shouldLoadLastSessionOnStartup(),
        juce::dontSendNotification
    );
#endif
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// AuthorisationData
////////////////////////////////////////////////////////////////////////////////

AuthorisationData::char_t const & AuthorisationData::authorised() const
{
    char_t const * LE_RESTRICT const pString( data_.front().getCharPointer().getAddress() );
    LE_ASSUME( pString );
    return *pString;
}


void AuthorisationData::clear()
{
    for ( auto & string : data_ )
        string = string.empty;
}


juce::String & AuthorisationData::line( unsigned int const lineIndex )
{
    // 1-based index
    BOOST_ASSERT( lineIndex >= 1            );
    BOOST_ASSERT( lineIndex <  data_.size() );
    return data_[ lineIndex - 1 ];
}


juce::String const & AuthorisationData::line( unsigned int const lineIndex ) const
{
    return const_cast<AuthorisationData &>( *this ).line( lineIndex );
}


juce::String & AuthorisationData::licenceType()
{
    return data_.back();
}


juce::String const & AuthorisationData::licenceType() const
{
    return const_cast<AuthorisationData &>( *this ).licenceType();
}

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

/*
    Alex's scrap:

	AudioEffectX *effect = (AudioEffectX*)getEffect ();
	VstFileType aiffType ("AIFF File", "AIFF", "aif", "aiff", "audio/aiff", "audio/x-aiff");
	VstFileType waveType ("Wave File", ".WAV", "wav", "wav",  "audio/wav", "audio/x-wav");
	VstFileType aifcType ("AIFC File", "AIFC", "aif", "aifc", "audio/x-aifc");
	VstFileType sdIIType ("SoundDesigner II File", "Sd2f", "sd2", "sd2");

	VstFileSelect vstFileSelect;
	memset (&vstFileSelect, 0, sizeof (VstFileType));

	vstFileSelect.command     = kVstFileLoad;
	vstFileSelect.type        = kVstFileType;
	strcpy (vstFileSelect.title, "Load sample..");
	vstFileSelect.nbFileTypes = 2;
	vstFileSelect.fileTypes   = &aiffType;
	vstFileSelect.returnPath  = new char[1024];
	sprintf(vstFileSelect.returnPath, "");
	//vstFileSelect.initialPath  = new char[1024];
	vstFileSelect.initialPath = 0;

	CFileSelector* cFile = new CFileSelector(effect);

	if (cFile->run (&vstFileSelect))
	{
		StandardAlert(0, "File", vstFileSelect.returnPath, 0,0);
		UpdateDisplay(vstFileSelect.returnPath);
	}

	delete cFile;

	delete [] vstFileSelect.returnPath;
	if (vstFileSelect.initialPath)	delete []vstFileSelect.initialPath;
*/
