////////////////////////////////////////////////////////////////////////////////
//
// ---
// GUI
// ---
//
//   SpectrumWorx GUI implementation (based on our patched JUCE GIT tip).
//
// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
//
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef gui_hpp__3B545D5D_569F_4056_BEA8_159015782EC9
#define gui_hpp__3B545D5D_569F_4056_BEA8_159015782EC9
#pragma once
//------------------------------------------------------------------------------
#include "le/parameters/enumerated/tag.hpp"
#include "le/parameters/powerOfTwo/tag.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/rvalueReferences.hpp"
#include "le/utility/tchar.hpp"

// Boost sandbox
#include "boost/mmap/mapped_view/mapped_view.hpp"
#include "boost/optional/optional.hpp"

#ifndef _MSC_VER
#include "boost/mpl/at.hpp"
#endif // _MSC_VER
#include "boost/smart_ptr/intrusive_ptr.hpp"

#include "juce/juce_gui_basics/juce_gui_basics.h"

#if defined( _WIN32 )
    #include "windows.h"
#elif defined( __APPLE__ )
    #ifdef __LP64__
        typedef unsigned int  ATSFontContainerRef;
    #else
        typedef unsigned long ATSFontContainerRef;
    #endif // __LP64__
#endif // _WIN32
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace Parameters
{
    template <class Parameter> struct DiscreteValues;
    template <class Parameter> struct Name;
}

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

namespace Engine { class Setup; }
class SpectrumWorx;

//------------------------------------------------------------------------------
namespace GUI
{
//------------------------------------------------------------------------------

enum ResourceBitmaps
{
    EditorBackground        = '01',
    EditorKnobStrip         = '02',
    ModuleKnobStrip         = '03',
    ModuleOn                = '04',
    ModuleMuted             = '05',
    AddModule               = '06',
    PresetBackground        = '07',
    PresetOff               = '08',
    PresetOn                = '09',
    SettingsOff             = '10',
    SettingsOn              = '11',
    SymmetricKnobStrip      = '12',
    TriggerBtnOff           = '13',
    TriggerBtnOn            = '14',
    Eject                   = '16',
    SettingsEngineBg        = '17',
    SettingsIntrfcBg        = '17', // ...currently the same as the engine background...
    SettingsRegBg           = '19',
    SettingsAboutBg         = '20',
    SettingsEngineOff       = '21',
    SettingsEngineOn        = '22',
    SettingsGUIOff          = '23',
    SettingsGUIOn           = '24',
    SettingsRegOff          = '25',
    SettingsRegOn           = '26',
    SettingsAboutOff        = '27',
    SettingsAboutOn         = '28',
    SettingsRegDoneBg       = '29',
    PresetSaveUp            = '30',
    PresetSaveDown          = '31',
    PresetDeleteUp          = '32',
    PresetDeleteDown        = '33',
    PresetSaveAsUp          = '34',
    PresetSaveAsDown        = '35',
    AuthorizeDown           = '36',
    AuthorizeUp             = '37',
    BuyNowDown              = '38',
    BuyNowUp                = '39',
    LFOSliderThumb          = '40',
    LEDOff                  = '41',
    LEDOn                   = '42',
    LFOSine                 = '43',
    LFOTriangle             = '44',
    LFOSawtooth             = '45',
    LFOReverseSaw           = '46',
    LFOSquare               = '47',
    LFOExponent             = '48',
    LFORandomHold           = '49',
    LFORandomSlide          = '50',
    LFORandomWhacko         = '51',
    LFODirac                = '52',
    LFOdIRAC                = '53',
    ModuleBg                = '55',
    ModuleBgSelected        = '56',
    ChangeWaveform          = '57',
    ModuleKnobSelected      = '58',
    ModuleCombo             = '59',
    ModuleComboOn           = '60',
    SettingsCombo           = '61',
    SettingsComboOn         = '62',
    SmallLinearKnobStrip    = '63',
    SmallSymmetricKnobStrip = '64',
    SmallModuleKnobSelected = '65',
    UsersGuideUp            = '66',
    UsersGuideDown          = '67',
    ModuleKnobLFOed         = '68',
}; // enum ResourceBitmaps


////////////////////////////////////////////////////////////////////////////////
///
/// \class ReferenceCountedGUIInitializationGuard
///
/// \brief Responsible for GUI library initialization and cleanup handling.
///
////////////////////////////////////////////////////////////////////////////////

class ReferenceCountedGUIInitializationGuard
{
public:
    static LE_NOTHROWNOALIAS bool isGUIInitialised();

protected:
     ReferenceCountedGUIInitializationGuard();
    ~ReferenceCountedGUIInitializationGuard();

private:
    static std::uint8_t guiInitializationReferenceCount;
}; // class ReferenceCountedGUIInitializationGuard


////////////////////////////////////////////////////////////////////////////////
//
// resourceBitmap()
// ----------------
//
//  Lazy-constructs a static juce::Image object from a resource ID. Implemented
// as a template function so that a different object would be constructed for
// different resource ID's.
//
////////////////////////////////////////////////////////////////////////////////


juce::Image resourceBitmap( char const (&bitmapNumber)[ 2 + 1 ] );

template <int bitmapID>
#ifdef __clang__
LE_NOALIAS
#else
LE_PURE_FUNCTION
#endif // __clang__
juce::Image const & resourceBitmap()
{
    using stringID = boost::mpl::string<bitmapID>;
    static char const bitmapNumber[ 2 + 1 ] =
    {
        boost::mpl::at_c<stringID, 0>::type::value,
        boost::mpl::at_c<stringID, 1>::type::value,
        '\0'
    };
    static juce::Image const image( resourceBitmap( bitmapNumber ) );
    return image;
}

void LE_FASTCALL paintImage( juce::Graphics &, juce::Image const &               );
void LE_FASTCALL paintImage( juce::Graphics &, juce::Image const &, int x, int y );

void setSizeFromImage( juce::Component &, juce::Image const & );


class SpectrumWorxEditor;

LE_NOTHROWNOALIAS bool isThisTheGUIThread();
LE_NOTHROWNOALIAS bool isGUIInitialised  ();

LE_NOTHROWNOALIAS float displayScale();


////////////////////////////////////////////////////////////////////////////////
// Global paths.
//...mrmlj...clean this up...
////////////////////////////////////////////////////////////////////////////////

boost::mmap::mapped_view<char const> mapPathsFile(                          );
boost::mmap::mapped_view<char      > mapPathsFile( unsigned int desiredSize );

bool LE_NOTHROW initializePaths         ();
bool LE_NOTHROW havePathsBeenInitialised();

LE_NOTHROWNOALIAS juce::File const & rootPath     ();
LE_NOTHROWNOALIAS juce::File       & presetsFolder();
LE_NOALIAS        juce::File         resourcesPath();
LE_NOALIAS        juce::File         licencesPath ();


#ifdef __APPLE__
FSRef makeFSRefFromPath( juce::String const & path );
#endif // __APPLE__


////////////////////////////////////////////////////////////////////////////////
///
/// \class Theme
///
/// \brief JUCE LookAndFeel class for SpectrumWorx
///
////////////////////////////////////////////////////////////////////////////////

class ModuleControlBase;
class ModuleUI;

class Theme final : public juce::LookAndFeel, boost::noncopyable
{
public:
    enum ModuleUIMouseOverReaction
    {
        Never,
        WhenParentModuleSelected,
        WhenParentOrNothingSelected
    };

    enum LFOUpdateBehaviour
    {
        NoUpdate,
        WhenControlSelected,
        WhenControlActive,
        Always
    };

    struct Settings
    {
        Settings();

        float                     globalOpacity            ;
        ModuleUIMouseOverReaction moduleUIMouseOverReaction;
        LFOUpdateBehaviour        lfoUpdateBehaviour       ;
        bool                      hideCursorOnKnobDrag     ;
    };

public:
    static void createSingleton ();
    static void destroySingleton();

    static Theme & singleton();

     Theme();
    ~Theme();

public:
    static juce::Colour blueColour() { return juce::Colour( 19, 181, 234 ); }

    juce::Font const & blueFont () const { return blueFont_ ; }
    juce::Font const & whiteFont() const { return whiteFont_; }

    static Settings & settings() { return settings_; }

    static bool shouldUpdateLFOControl      ( ModuleControlBase const & );
    static bool aModuleControlNeedsLFOUpdate( ModuleUI          const & );

public: // juce::LookAndFeel overrides
    void        drawLinearSliderBackground  ( juce::Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider & ) override;
    void        drawLinearSliderThumb       ( juce::Graphics &, int x, int y, int width, int height, float sliderPos, float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle, juce::Slider & ) override;
    void        drawPopupMenuBackground     ( juce::Graphics &, int width, int height ) override;
    void        drawTabAreaBehindFrontButton( juce::TabbedButtonBar &, juce::Graphics &, int w, int h ) override;
    juce::Image getDefaultFolderImage       () /*override*/;
    int         getMenuWindowFlags          () override;
    juce::Font  getPopupMenuFont            () override;
    int         getSliderThumbRadius        ( juce::Slider &   ) override;
    int         getTabButtonSpaceAroundImage(                  ) override { return 0; }
    int         getTabButtonOverlap         ( int /*tabDepth*/ ) override { return 0; }

private:
    juce::Font const blueFont_ ;
    juce::Font const whiteFont_;

    juce::Image folderIcon_;

    static Settings settings_;
}; // class Theme


////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class WidgetBase
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    LE_NOTHROW  void LE_FASTCALL setName( juce::Component & widget, juce::String const & newName );
                void LE_FASTCALL setName( juce::Component & widget, char const * const   newName );

    bool LE_FASTCALL hasDirectFocus( juce::Component const & );
    bool LE_FASTCALL hasFocus      ( juce::Component const & );

    bool LE_FASTCALL isParentOf( juce::Component const & parent, juce::Component const & possibleChild  );
    bool LE_FASTCALL isParentOf( juce::Component const & parent, juce::Component const * pPossibleChild );
}

template <class BaseComponent = juce::Component>
class LE_NOVTABLE WidgetBase : public BaseComponent
{
protected:
                WidgetBase(                                    ) : BaseComponent( juce::String::empty ) {}
    LE_NOINLINE WidgetBase( char const * const   componentName ) : BaseComponent( juce::String::empty ) { setName( componentName ); }
                WidgetBase( juce::String const & componentName ) : BaseComponent( componentName       ) {}
    LE_NOTHROW LE_FORCEINLINE ~WidgetBase() {}

public:
    void setName( char const * const newName ) { Detail::setName( *this, newName ); }
    using BaseComponent::setName;

    void LE_NOTHROWNOALIAS setVisible  (                      ) { BaseComponent::setVisible( true      ); }
    void LE_NOTHROWNOALIAS setInvisible(                      ) { BaseComponent::setVisible( false     ); }
    void LE_NOTHROWNOALIAS setIsVisible( bool const isVisible ) { BaseComponent::setVisible( isVisible ); }

    void LE_NOTHROWNOALIAS setEnabled( bool const isEnabled ) { BaseComponent::setEnabled( isEnabled ); }

    bool LE_NOTHROWNOALIAS hasDirectFocus() const { return Detail::hasDirectFocus( *this ); }
    bool LE_NOTHROWNOALIAS hasFocus      () const { return Detail::hasFocus      ( *this ); }

    bool LE_NOTHROWNOALIAS isParentOf( juce::Component const & control ) const { return Detail::isParentOf( *this, control ); }

    static LE_NOTHROWRESTRICTNOALIAS void * operator new   ( std::size_t               const count      , void * LE_RESTRICT const pStorage     ) { (void)count; LE_ASSUME( pStorage ); return pStorage; }
    static LE_NOTHROWNOALIAS         void   operator delete( void        * LE_RESTRICT const /*pObject*/, void * LE_RESTRICT const /*pStorage*/ ) {}

#ifdef __clang__
    void * operator new   ( std::size_t   const count   ) { return ::operator new   ( count   ); }
    void   operator delete( void        * const pObject ) { return ::operator delete( pObject ); }
#endif // __clang__

private:
    using BaseComponent::isParentOf;
    using BaseComponent::setVisible;
}; // class WidgetBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class OwnedWindowBase
/// \internal
/// \brief OwnedWindow core/non-templated implementation.
///
////////////////////////////////////////////////////////////////////////////////

class PresetBrowser;
class SpectrumWorxEditor;

class OwnedWindowBase
{
protected:
    static void attach( SpectrumWorxEditor &, juce::Component & );
    static void detach( SpectrumWorxEditor &, juce::Component & );

    static void adjustPositions( juce::Component * pFirstWindow, juce::Component * pSecondWindow, unsigned int editorX, unsigned int editorY, unsigned int flags );
    static void adjustPositions( SpectrumWorxEditor & parent, juce::Component * pFirstWindow, juce::Component * pSecondWindow );

    static void adjustPositionsForPresetBrowser( SpectrumWorxEditor & parent, juce::Component * pCurrentWindowState );
    static void adjustPositionsForSettings     ( SpectrumWorxEditor & parent, juce::Component * pCurrentWindowState );

private:
#ifdef _WIN32
    static LRESULT __stdcall callWndHookProc( int, WPARAM, LPARAM );

    static HHOOK wndProcHook;
#endif // _WIN32
}; // class OwnedWindowBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class OwnedWindow
///
/// \brief A base class for windows/widgets/components that need "owned window"
/// behaviour with automatic main (editor) window position tracking/following.
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

template <class Window>
class OwnedWindow : private OwnedWindowBase
{
private:
    static bool const isPresetBrowser  = std::is_same<Window, PresetBrowser>::value;
    static bool const isSettingsWindow = !isPresetBrowser;

public:
    // Implementation note:
    //   Unfortunately we cannot make the attachment automatic in the
    // constructor because, under OSX, attachment causes the 'paint' method to
    // be immediately invoked which can cause undefined behaviour/crashes
    // because the wrapping object's constructor still has not finished (and
    // thus the 'paint' method gets called on an incomplete object). Because of
    // this, the only reliable usage (for now) is for the wrapping class
    // (Window) to call the attach() function manually at the end of its
    // constructor.
    //                                        (07.10.2010.) (Domagoj Saric)
    /// \todo Think of a smarter solution or assert that attach() gets called
    /// exactly once.
    ///                                       (07.10.2010.) (Domagoj Saric)
    void attach()
    {
    #ifdef _MSC_VER
        static_assert( std::is_same<Window, SpectrumWorxEditor::Settings>::value == isSettingsWindow, "" );
        LE_ASSUME( this != 0 );
    #endif // _MSC_VER

        SpectrumWorxEditor & parent( static_cast<Window *>( this )->editor() );
        juce::Component    & window( static_cast<Window *>( this )->window() );

        OwnedWindowBase::attach( parent, window );

        if ( isPresetBrowser )
            adjustPositionsForPresetBrowser( parent, &window );
        else
            adjustPositionsForSettings     ( parent, &window );

        BOOST_ASSERT( window.isOnDesktop() && window.getPeer() );
    }

    ~OwnedWindow()
    {
    #ifdef _MSC_VER
        LE_ASSUME( this != 0 );
    #endif // _MSC_VER
        SpectrumWorxEditor & parent( static_cast<Window *>( this )->editor() );
        juce::Component    & window( static_cast<Window *>( this )->window() );
        OwnedWindowBase::detach( parent, window );
        if ( isPresetBrowser )
            adjustPositionsForPresetBrowser( parent, 0 );
        else
            adjustPositionsForSettings     ( parent, 0 );
    }
}; // class OwnedWindow
#pragma warning( pop )


void LE_NOTHROW warningMessageBox ( boost::string_ref title, boost::string_ref message, bool canBlock );
bool LE_NOTHROW warningOkCancelBox( TCHAR const * title, TCHAR const * question );

void addToParentAndShow( juce::Component & parent, juce::Component & childToBe );

LE_NOTHROW
void fadeOutComponent( juce::Component &, float finalAlpha, unsigned int duration, bool useProxyComponent );


class Lock : private juce::MessageManagerLock
{
public:
    LE_NOINLINE
    Lock() : juce::MessageManagerLock( nullptr ) { BOOST_ASSERT( lockWasGained() || !ReferenceCountedGUIInitializationGuard::isGUIInitialised() ); }
}; // class Lock


namespace Detail
{
    template <class GUIHolder, class Functor>
    class Message LE_SEALED : public juce::MessageManager::MessageBase
    {
    public:
        Message( GUIHolder & guiHolder, Functor && functor ) : pGUIHolder_( &guiHolder ), functor_( std::move( functor ) ) {}

    private:
        LE_NOTHROW LE_COLD void messageCallback() LE_SEALED
        {
            if ( pGUIHolder_->gui() )
                if ( !functor_( *pGUIHolder_->gui() ) )
                    this->post();
        }

    private:
        boost::intrusive_ptr<GUIHolder> const pGUIHolder_;
        Functor                               functor_   ;
    }; // class Message

    template <class Functor>
    class MessageDirect LE_SEALED : public juce::MessageManager::MessageBase
    {
    public:
        MessageDirect( Functor && functor ) : functor_( std::move( functor ) ) {}

    private:
        LE_NOTHROW LE_COLD void messageCallback() LE_SEALED { functor_(); }

    private:
        Functor const functor_;
    }; // class MessageDirect

    LE_NOTHROW inline
    void postMessage( juce::MessageManager::MessageBase * LE_RESTRICT const pMessage )
    {
        if ( pMessage ) pMessage->post();
    }
} // namespace Detail

template <class GUIHolder, class Functor>
LE_NOTHROW void postMessage( GUIHolder & guiHolder, Functor && functor )
{
    Detail::postMessage( new ( std::nothrow ) Detail::Message<GUIHolder, Functor>( guiHolder, std::move( functor ) ) );
}

template <class Functor>
LE_NOTHROW void postMessage( Functor && functor )
{
    Detail::postMessage( new ( std::nothrow ) Detail::MessageDirect<Functor>( std::move( functor ) ) );
}

template <class GUIHolder, class Functor>
LE_NOTHROW void postOrExecuteMessage( GUIHolder & guiHolder, Functor && functor )
{
    if
    (
        isThisTheGUIThread() &&
        functor( *guiHolder.gui() )
    )
        return;
#if LE_SW_SEPARATED_DSP_GUI
    LE_UNREACHABLE_CODE();
#else
    postMessage( /*std::forward<GUIHolder>*/( guiHolder ), std::move( functor ) );
#endif
}


#if 0 //...mrmlj...does not work with the latest juce...cleanup...
////////////////////////////////////////////////////////////////////////////////
///
/// \class AsyncRepainter
/// \internal
/// \brief Helper for creating asynchronous/'on-GUI-thread' repainting widgets.
///...mrmlj...thoroughly document...
////////////////////////////////////////////////////////////////////////////////

// http://lachand.free.fr/cocoa/Threads.html
// https://developer.apple.com/library/mac/documentation/Cocoa/Conceptual/Multithreading/Introduction/Introduction.html
// http://www.cocoawithlove.com/2009/08/safe-threaded-design-and-inter-thread.html

class LE_NOVTABLE AsyncRepainter : public juce::Component
{
public:
    static void repaint( juce::Component &, int x, int y, int w, int h );

#define LE_IMPLEMENT_ASYNC_REPAINT                                                                                                                   \
private:                                                                                                                                             \
    void internalRepaint( int const x, int const y, int const w, int const h ) noexcept LE_OVERRIDE { AsyncRepainter::repaint( *this, x, y, w, h ); }

private:
    AsyncRepainter();
};
#else
#define LE_IMPLEMENT_ASYNC_REPAINT
#endif

////////////////////////////////////////////////////////////////////////////////
///
/// \class DrawableText
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE DrawableText : public juce::GlyphArrangement
{
public:
    DrawableText
    (
        char const * text,
        unsigned int     x, unsigned int      y,
        unsigned int width, unsigned int height,
        juce::Justification = juce::Justification::centredLeft,
        juce::Font const & font = defaultFont()
    );

    static juce::Font defaultFont();
}; // class DrawableText


////////////////////////////////////////////////////////////////////////////////
///
/// \class BitmapButton
///
////////////////////////////////////////////////////////////////////////////////

class BitmapButton : public WidgetBase<juce::ImageButton>
{
public: // ModuleUI control traits
    typedef bool       value_type;
    typedef value_type param_type;

    static value_type valueRangeMinimum() { return false; }
    static value_type valueRangeMaximum() { return true ; }
    static value_type valueRangeQuantum() { return true - false; }

    LE_NOTHROWNOALIAS value_type LE_FASTCALL getValue(                           ) const { return getToggleState(                                      ); }
    LE_NOTHROWNOALIAS void       LE_FASTCALL setValue( param_type const newValue )       {        setToggleState( newValue, juce::dontSendNotification ); }

public:
    BitmapButton
    (
        juce::Component       & parent,
        juce::Image     const & on    ,
        juce::Image     const & off   ,
        juce::Colour    const & overlayColourWhenOver = defaultOverOverlay(),
        bool                    toggled = true
    );

public: //...mrmlj...as a workaround for the juce::ImageButton class that hides all this information...
    juce::Image getCurrentImage() const;

    static juce::Colour const & normalOverlay     ();
    static juce::Colour const & downOverlay       ();
    static juce::Colour const & defaultOverOverlay();

    static float normalOpacity() { return 1.00f; }
    static float downOpacity  () { return 1.00f; }
    static float overOpacity  () { return 0.88f; }

protected:
    static char const * getTextFromValue( unsigned int const booleanValue )
    {
        static char const * const strings[] = { "off", "on" };
        return strings[ booleanValue ];
    }

private:
    LE_IMPLEMENT_ASYNC_REPAINT

    using juce::Button::getToggleStateValue;
}; // class BitmapButton


////////////////////////////////////////////////////////////////////////////////
///
/// \class PopupMenu
///
////////////////////////////////////////////////////////////////////////////////

class PopupMenu : boost::noncopyable
{
public:
    using ItemID     = std::uint32_t;
    using OptionalID = boost::optional<ItemID>;

public:
    LE_NOTHROW  PopupMenu();
    LE_NOTHROW ~PopupMenu() {}

    void addItem         ( ItemID, char const * newItemText, juce::Image const & icon = juce::Image::null, bool enabled = true );
    void addSubMenu      ( PopupMenu &, char const * name );
    void addSectionHeader( char const * title );

    void clear();

    unsigned int numberOfItems() const;

    OptionalID showCenteredAtRight( juce::Component const & ) const;
    OptionalID showCenteredBelow  ( juce::Component const & ) const;

    static bool menuActive() { return menuActive_; };

protected:
    typedef unsigned int MangledID;
    static MangledID mangleID  ( ItemID    );
    static ItemID    unmangleID( MangledID );

private:
    void updateDimensionsForNewItem( juce::String const & itemText );

    OptionalID showAt( unsigned int x, unsigned int y, unsigned int width, unsigned int height ) const;

protected: //...mrmlj...
    juce::PopupMenu menu_;

private:
    static unsigned int const zeroIDMaskWorkaround = 0xFF000000;

    unsigned short menuHeight_;
    unsigned short menuWidth_ ;

    static bool menuActive_;
}; // class PopupMenu


////////////////////////////////////////////////////////////////////////////////
///
/// \class PopupMenuWithSelection
///
////////////////////////////////////////////////////////////////////////////////

class PopupMenuWithSelection : public PopupMenu
{
public:
    PopupMenuWithSelection();

    unsigned int getSelectedID(                             ) const;
    void         setSelectedID( unsigned int newSelectionID )      ;

    unsigned int getSelectedIndex(                                ) const;
    void         setSelectedIndex( unsigned int newSelectionIndex )      ;

    juce::String const & getSelectedItemText() const;
    juce::Image  const & getSelectedItemIcon() const;

    bool showCenteredAtRight( juce::Component const & );
    bool showCenteredBelow  ( juce::Component const & );

    void clear();

    bool hasValidSelection() const;

    juce::String const & getItemText( unsigned int itemIndex ) const;

private:
    bool handleNewSelection( OptionalID const & chosenMenuEntryID );
    void updateSelection   ( unsigned int newSelectionIndex       );

private:
    int currentSelection_  ;
    int currentSelectionID_;
}; // class PopupMenuWithSelection


////////////////////////////////////////////////////////////////////////////////
///
/// \class ComboBox
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE ComboBox
    :
    public WidgetBase<>,
    public PopupMenuWithSelection
{
public: // ModuleUI control traits
    typedef unsigned int value_type;
    typedef value_type   param_type;

    static value_type valueRangeMinimum()       { return 0              ; }
           value_type valueRangeMaximum() const { return numberOfItems(); }
    static value_type valueRangeQuantum()       { return 1              ; }

    value_type getValue(                           ) const { return getSelectedID(          ); }
    void       setValue( param_type const newValue )       {        setSelectedID( newValue ); }

public:
    void setSelectedID   ( unsigned int newSelectionID    );
    void setSelectedIndex( unsigned int newSelectionIndex );

protected:
    ComboBox
    (
        juce::Component   & parent,
        juce::Image const & normalBackground,
        juce::Image const & selectedBackground
    );

    bool showMenu();

protected: // juce::Component overrides
    void paint( juce::Graphics & ) LE_OVERRIDE;

	LE_IMPLEMENT_ASYNC_REPAINT

private:
    juce::Image const & normalBackground_  ;
    juce::Image const & selectedBackground_;
}; // class ComboBox


////////////////////////////////////////////////////////////////////////////////
///
/// \class BackgroundImage
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE BackgroundImage : public WidgetBase<>
{
protected:
    BackgroundImage( juce::Image const & image ) : pImage_( &image ) {};

    juce::Image const & image() const;
    void setImage( juce::Image const & );

protected: // juce::Component overrides
    void paint( juce::Graphics & ) LE_OVERRIDE;

private:
     juce::Image const * pImage_;
}; // class BackgroundImage


////////////////////////////////////////////////////////////////////////////////
///
/// \class LEDTextButton
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    void paintTextButton
    (
        BitmapButton const &,
        juce::Graphics &,
        unsigned int textX , unsigned int textY ,
        unsigned int imageX, unsigned int imageY,
        bool isMouseOverButton,
        bool isButtonDown
    );
} // namespace Detail

class LEDTextButton : public BitmapButton
{
public:
    LEDTextButton
    (
        juce::Component & parent,
        unsigned int x,
        unsigned int y,
        char const * text
    );

protected: // juce::Component overrides
    void paintButton( juce::Graphics &, bool isMouseOverButton, bool isButtonDown ) LE_OVERRIDE;
}; // class LEDTextButton


////////////////////////////////////////////////////////////////////////////////
///
/// \class TextButton
///
////////////////////////////////////////////////////////////////////////////////

class TextButton : public WidgetBase<juce::Button>
{
public:
    TextButton
    (
        juce::Component & parent,
        unsigned int x,
        unsigned int y,
        char const * text
    );

private:
    void paintButton( juce::Graphics &, bool isMouseOverButton, bool isButtonDown ) LE_OVERRIDE;

    static unsigned int const height = 11;
}; // class TextButton


////////////////////////////////////////////////////////////////////////////////
///
/// \class Knob
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE Knob : public WidgetBase<juce::Slider>
{
public:
    typedef double value_type;
    typedef float  param_type;

    LE_COLD value_type LE_NOALIAS LE_FASTCALL getValue(            ) const { return static_cast<value_type>( juce::Slider::getValue() ); }
    LE_COLD void       LE_NOALIAS LE_FASTCALL setValue( param_type );

    param_type getNormalisedValue() const;

    void setupForParameter
    (
        char        const * title             ,
        juce::Image const & filmStripToSizeFor,
        param_type          defaultValue
    );

protected:
    Knob
    (
        juce::Component & parent ,
        unsigned int      x      ,
        unsigned int      y      ,
        unsigned int      xMargin,
        unsigned int      yMargin
    );

public:
    static void LE_FASTCALL removeValueListeners( juce::Slider &, juce::ValueListener & );

protected:
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Woverloaded-virtual"
#endif // __clang__
    void paint( juce::Image const & filmStrip, unsigned int xMargin, unsigned int yMargin, juce::Graphics & );
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

protected: // juce::Component overrides
    void startedDragging() noexcept LE_OVERRIDE;
#ifndef NDEBUG
    void stoppedDragging() noexcept LE_OVERRIDE;
#endif // NDEBUG

    LE_IMPLEMENT_ASYNC_REPAINT

private:
    using juce::Slider::getValueObject   ;
    using juce::Slider::getMinValueObject;
    using juce::Slider::getMaxValueObject;

protected:
    static unsigned int const numberOfKnobSubbitmaps = 127;
}; // class Knob


////////////////////////////////////////////////////////////////////////////////
///
/// \class EditorKnob
///
////////////////////////////////////////////////////////////////////////////////

class EditorKnob LE_SEALED : public Knob
{
public:
    EditorKnob
    (
        SpectrumWorxEditor & parent,
        unsigned int         x,
        unsigned int         y
    );

    void LE_FASTCALL setupForParameter
    (
        std::uint8_t parameterIndex,
        param_type   minimumValue  ,
        param_type   maximumValue  ,
        param_type   defaultValue
    );

private: // juce::Component overrides
    void paint       ( juce::Graphics & )          LE_OVERRIDE;
    void valueChanged(                  ) noexcept LE_OVERRIDE;

    void startedDragging() noexcept LE_OVERRIDE;
    void stoppedDragging() noexcept LE_OVERRIDE;

private:
    SpectrumWorxEditor & editor();

private:
    std::uint8_t parameterIndex_;
}; // class EditorKnob


////////////////////////////////////////////////////////////////////////////////
///
/// \class TitledComboBox
///
////////////////////////////////////////////////////////////////////////////////

class TitledComboBox : public ComboBox
{
public:
    TitledComboBox
    (
        juce::Component & parent,
        unsigned int x,
        unsigned int y,
        char const * title
    );

private:
    void mouseDown( juce::MouseEvent const & ) LE_OVERRIDE;
    void paint    ( juce::Graphics         & ) LE_OVERRIDE;

private:
    DrawableText const title_;
}; // class TitledComboBox


// Implementation note:
//   The following fillComboBoxForParameter<>() implementation supports only
// enumerated and power-of-two parameters and uses their internal knowledge
// (that they do not use a DisplayValueTransformer and how they are printed,
// a simple 'lexical_cast' for power-of-two parameters or a direct fetch of
// a string from the DiscreteValues<>::strings[] array) in order to slightly
// reduce compile time and runtime overhead. In case it becomes needed again, a
// generic solution was used up to revision 4636.
//                                            (15.07.2011.) (Domagoj Saric)

namespace Detail
{
    void addPowerOfTwoValueStringsToComboBox
    (
        unsigned int   firstValue,
        unsigned int   lastValue,
        ComboBox     & comboBox
    );

    void addEnumeratedParameterValueStringsToComboBox
    (
        boost::iterator_range<char const * LE_RESTRICT const *>   strings,
        ComboBox                                                & comboBox
    );

    template <class Parameter>
    void fillComboBoxForParameter( ComboBox & comboBox, LE::Parameters::PowerOfTwoParameterTag )
    {
        addPowerOfTwoValueStringsToComboBox( Parameter::minimum(), Parameter::maximum(), comboBox );
    }

    template <class Parameter>
    void fillComboBoxForParameter( ComboBox & comboBox, LE::Parameters::EnumeratedParameterTag )
    {
        addEnumeratedParameterValueStringsToComboBox( boost::make_iterator_range( LE::Parameters::DiscreteValues<Parameter>::strings ), comboBox );
    }
} // namespace Detail

template <class Parameter>
void fillComboBoxForParameter( ComboBox & comboBox )
{
    Detail::fillComboBoxForParameter<Parameter>( comboBox, typename Parameter::Tag() );
}


////////////////////////////////////////////////////////////////////////////////
///
/// \class DiscreteParameterComboBox
///
///   A wrapper for the TitledComboBox widget class that helps reduce verbosity/
/// boiler plate code by automatically calling name<Parameter>() and
/// fillComboBoxForParameter<Parameter>.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//  It contains a TitledComboBox instance rather than inheriting from
// TitledComboBox in order to 'emulate' __declspec( novtable ) for non-MSVC
// compilers. This requires the various helper operators and the -> syntax.
//  Rather than making the whole class a template only the constructor is
// templatized so that headers that use the class do not have to also know about
// the actual parameters for which a DiscreteParameterComboBox will be created.
//                                            (18.07.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

struct DiscreteParameterComboBox
{
    template <class Parameter>
    DiscreteParameterComboBox
    (
        juce::Component & parent,
        unsigned int const x,
        unsigned int const y,
        Parameter const * = 0
    )
        :
        comboBox_( parent, x, y, LE::Parameters::Name<Parameter>::string_ )
    {
        fillComboBoxForParameter<Parameter>( comboBox_ );
    }

    TitledComboBox       * operator->()       { return &comboBox_; }
    TitledComboBox const * operator& () const { return &comboBox_; }
    operator TitledComboBox       &  ()       { return  comboBox_; }
    operator TitledComboBox const &  () const { return  comboBox_; }

    TitledComboBox comboBox_;
}; // struct DiscreteParameterComboBox

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // gui_hpp
