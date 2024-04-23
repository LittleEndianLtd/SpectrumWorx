////////////////////////////////////////////////////////////////////////////////
///
/// gui.mm
/// ------
///
///   Cocoa/Objective-C(++) implementation details required for Mac OSX. Based
/// on original JUCE code from the juce_VST_wrapper.mm file.
///
/// Copyright (c) 2010.-2013. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// Useful links:
// http://www.otierney.net/objective-c.html
// http://developer.apple.com/library/mac/#DOCUMENTATION/Cocoa/Conceptual/ObjectiveC/Introduction/introObjectiveC.html
// http://stackoverflow.com/questions/2097294/what-do-the-plus-and-minus-signs-mean-in-objective-c-next-to-a-method
// https://developer.apple.com/library/ios/#referencelibrary/GettingStarted/Learning_Objective-C_A_Primer/
// http://serenity.uncc.edu/web/ADC/2005/Developer_DVD_Series/April/ADC%20Reference%20Library/documentation/Carbon/index.html
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "le/utility/objcfwdhelpers.hpp"

#include "boost/assert.hpp"

#include <Carbon/Carbon.h>
#include <Cocoa/Cocoa.h>

#include "juce/AppConfig.h"
#include "juce/juce_gui_basics/juce_gui_basics.h"

#if ! JUCE_64BIT
    #define JUCE_MAC_WINDOW_VISIBITY_BODGE 1 // see note below..
#endif
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

void initialiseMac() noexcept
{
#if ! JUCE_64BIT
    BOOST_VERIFY( ::NSApplicationLoad() );
#endif
    BOOST_ASSERT( [NSThread isMultiThreaded] );

#if 0 //...mrmlj...experiments to 'fix' the fat fonts issue under ML...
    NSUserDefaults * pStandardUserDefaults = [NSUserDefaults standardUserDefaults];
    [pStandardUserDefaults setBool: true forKey: @"NSFontDefaultScreenFontSubstitutionEnabled"];
    //[pStandardUserDefaults registerDefaults: pStandardUserDefaults ];
    [pStandardUserDefaults registerDefaults:@{@"NSFontDefaultScreenFontSubstitutionEnabled" : @YES}];
    [NSUserDefaults resetStandardUserDefaults];
#endif // _DEBUG
}

#if !JUCE_64BIT
static pascal OSStatus windowVisibilityBodge( EventHandlerCallRef, EventRef const e, void * const user ) noexcept
{
    NSWindow * const hostWindow( static_cast<NSWindow *>( user ) );

    switch ( GetEventKind (e) )
    {
        case kEventWindowInit:   [hostWindow display        ]; break;
        case kEventWindowShown:  [hostWindow orderFront: nil]; break;
        case kEventWindowHidden: [hostWindow orderOut  : nil]; break;
    }

    return eventNotHandledErr;
}

/* When you wrap a WindowRef as an NSWindow, it seems to bugger up the HideWindow
   function, so when the host tries (and fails) to hide the window, this catches
   the event and does the job properly.
*/

static void attachWindowHidingHooks( juce::Component & comp, void * const hostWindowRef, NSWindow * const nsWindow )
{
    using namespace juce;

    // Adds a callback bodge to work around some problems with wrapped
    // carbon windows..
    static EventTypeSpec const eventsToCatch[] =
    {
        { kEventClassWindow, kEventWindowInit   },
        { kEventClassWindow, kEventWindowShown  },
        { kEventClassWindow, kEventWindowHidden }
    };

    EventHandlerRef ref;
    InstallWindowEventHandler( (WindowRef) hostWindowRef,
        NewEventHandlerUPP (windowVisibilityBodge),
        GetEventTypeCount (eventsToCatch), eventsToCatch,
        (void*) nsWindow, &ref );

    comp.getProperties().set( "carbonEventRef", String::toHexString( (pointer_sized_int)(void*) ref ) );
}

static void removeWindowHidingHooks( juce::Component & comp )
{
    RemoveEventHandler
    (
        (EventHandlerRef)(void*)(juce::pointer_sized_int) comp.getProperties()[ "carbonEventRef" ].toString().getHexValue64()
    );
}


static void updateComponentPos( juce::Component & component ) noexcept
{
    HIViewRef dummyView = (HIViewRef) (void*) (juce::pointer_sized_int)
                            component.getProperties()[ "dummyViewRef" ].toString().getHexValue64();

    HIRect r;
    HIViewGetFrame (dummyView, &r);
    HIViewRef root;
    HIViewFindByID (HIViewGetRoot (HIViewGetWindow (dummyView)), kHIViewWindowContentID, &root);
    HIViewConvertRect (&r, HIViewGetSuperview (dummyView), root);

    Rect windowPos;
    GetWindowBounds (HIViewGetWindow (dummyView), kWindowContentRgn, &windowPos);

    component.setTopLeftPosition ((int) (windowPos.left + r.origin.x),
                                  (int) (windowPos.top + r.origin.y));
}

static pascal OSStatus viewBoundsChangedEvent( EventHandlerCallRef, EventRef, void * const user ) noexcept
{
    updateComponentPos( *static_cast<juce::Component *>( user ) );
    return noErr;
}


ObjC::NSWindow * attachComponentToHostWindow( juce::Component & component, WindowRef const windowRef )
{
    @autoreleasepool
    {
        using namespace juce;

        NSWindow * hostWindow = [[NSWindow alloc] initWithWindowRef: windowRef];
        [hostWindow retain];
        [hostWindow setCanHide: YES];
        [hostWindow setReleasedWhenClosed: YES];

        HIViewRef parentView = 0;

        WindowAttributes attributes;
        GetWindowAttributes ((WindowRef) windowRef, &attributes);
        if ((attributes & kWindowCompositingAttribute) != 0)
        {
            HIViewRef root = HIViewGetRoot ((WindowRef) windowRef);
            HIViewFindByID (root, kHIViewWindowContentID, &parentView);

            if (parentView == 0)
                parentView = root;
        }
        else
        {
            GetRootControl ((WindowRef) windowRef, (ControlRef*) &parentView);

            if (parentView == 0)
                CreateRootControl ((WindowRef) windowRef, (ControlRef*) &parentView);
        }

        // It seems that the only way to successfully position our overlaid window is by putting a dummy
        // HIView into the host's carbon window, and then catching events to see when it gets repositioned
        HIViewRef dummyView = 0;
        HIImageViewCreate (0, &dummyView);
        HIRect r = { {0, 0}, {static_cast<CGFloat>( component.getWidth() ), static_cast<CGFloat>( component.getHeight() )} };
        HIViewSetFrame (dummyView, &r);
        HIViewAddSubview (parentView, dummyView);
        component.getProperties().set("dummyViewRef", String::toHexString ((pointer_sized_int) (void*) dummyView));

        EventHandlerRef ref;
        const EventTypeSpec kControlBoundsChangedEvent = { kEventClassControl, kEventControlBoundsChanged };
        InstallEventHandler (GetControlEventTarget (dummyView), NewEventHandlerUPP (viewBoundsChangedEvent), 1, &kControlBoundsChangedEvent, &component, &ref);
        component.getProperties().set( "boundsEventRef", String::toHexString ((pointer_sized_int) (void*) ref) );

        updateComponentPos( component );

        component.juce::Component::addToDesktop( ComponentPeer::windowIsTemporary );

        //component.juce::Component::setVisible( true  );
        //component.juce::Component::toFront   ( false );

        NSView   * const pluginView   = (NSView*) component.getWindowHandle();
        NSWindow * const pluginWindow = [pluginView window];
        [pluginWindow setExcludedFromWindowsMenu: YES];
        [pluginWindow setCanHide: YES];

        [hostWindow addChildWindow: pluginWindow
                            ordered: NSWindowAbove];
        [hostWindow   orderFront: nil];
        [pluginWindow orderFront: nil];

        attachWindowHidingHooks( component, (WindowRef) windowRef, hostWindow );

        return reinterpret_cast<ObjC::NSWindow *>( hostWindow );
    }
}


void detachComponentFromHostWindow( juce::Component & comp, ObjC::NSWindow * const hostWindowParam )
{
    using namespace juce;

    @autoreleasepool
    {
        EventHandlerRef ref = (EventHandlerRef) (void*) (pointer_sized_int)
            comp.getProperties() ["boundsEventRef"].toString().getHexValue64();
        RemoveEventHandler (ref);

        removeWindowHidingHooks (comp);

        HIViewRef dummyView = (HIViewRef) (void*) (pointer_sized_int)
            comp.getProperties() ["dummyViewRef"].toString().getHexValue64();

        if (HIViewIsValid (dummyView))
            CFRelease (dummyView);

        NSView   * pluginView   = (NSView*) comp.getWindowHandle();
        NSWindow * pluginWindow = [pluginView window];

        ::NSWindow * const hostWindow( reinterpret_cast<::NSWindow *>( hostWindowParam ) );

        [hostWindow removeChildWindow: pluginWindow];
        comp.removeFromDesktop();

        [hostWindow release];
    }
}
#else
void detachComponentFromHostWindow( juce::Component & comp, ObjC::NSWindow * /*const hostWindowParam*/ )
{
    comp.removeFromDesktop();
}
#endif // 64 bit


void attachComponentToHostWindow( juce::Component & component, ObjC::NSView * const pParentViewParam )
{
    ::NSView * pParentView( reinterpret_cast<::NSView *>( pParentViewParam ) );
    BOOST_ASSERT( pParentView );

#if 0
    // (this workaround is because Wavelab provides a zero-size parent view...)
    if ( [pParentView frame].size.height == 0 )
        [ static_cast<::NSView *>( component.getWindowHandle() ) setFrameOrigin: NSZeroPoint ];
#endif // 0

    [pParentView setHidden                        : NO ];
    [pParentView setPostsFrameChangedNotifications: YES];

    [pParentView setFrameSize: NSMakeSize( component.getWidth(), component.getHeight() )];

    //...mrmlj...
    //[hostView setFrameSize: NSMakeSize
    //(
    //    [hostView frame].size.width  + (newWidth  - component->getWidth()),
    //    [hostView frame].size.height + (newHeight - component->getHeight())
    //)];

    ::NSWindow * const pWindow( [pParentView window] );
    [pWindow setAcceptsMouseMovedEvents: YES];
    //[pWindow makeFirstResponder: pParentView];

    component.juce::Component::addToDesktop( 0, pParentView );
    BOOST_ASSERT( component.isVisible() );
    //component.toFront( true );
}


void makeEditorChild( juce::ComponentPeer & editor, juce::ComponentPeer & childToBe ) noexcept
{
    ::NSWindow * const pEditorWindow( [ static_cast<::NSView *>( editor   .getNativeHandle() ) window ] );
    ::NSWindow * const pChildWindow ( [ static_cast<::NSView *>( childToBe.getNativeHandle() ) window ] );
    BOOST_ASSERT( pEditorWindow );
    BOOST_ASSERT( pChildWindow  );
#if 1
    [pEditorWindow addChildWindow: pChildWindow ordered: NSWindowAbove];
#else
    NSWindow * pRealParent( pEditorWindow );
    NSWindow * pNewParent;
    while ( ( pNewParent = [pRealParent parentWindow] ) )
    {
	    pRealParent = pNewParent;
    }

    [pRealParent addChildWindow: pChildWindow ordered: NSWindowAbove];
#endif
}


void detachFromEditor( juce::ComponentPeer & editor, juce::ComponentPeer & child ) noexcept
{
    ::NSWindow * const pEditorWindow      ( [ static_cast<::NSView *>( editor.getNativeHandle() ) window       ] );
    ::NSWindow * const pChildWindow       ( [ static_cast<::NSView *>( child .getNativeHandle() ) window       ] );
    ::NSWindow * const pActualParentWindow( [ pChildWindow                                        parentWindow ] );
    BOOST_ASSERT( pChildWindow );
    BOOST_ASSERT( pEditorWindow == pActualParentWindow || !pEditorWindow );
    BOOST_ASSERT( ![pChildWindow childWindows] || ![[pChildWindow  childWindows] count] );
    //BOOST_ASSERT( [[pEditorWindow childWindows] count] == 1 || !pEditorWindow );
#if 1
    [pActualParentWindow removeChildWindow: pChildWindow];
#else
    ::NSWindow * pRealParent( pEditorWindow );
    ::NSWindow * pNewParent;
    while ( ( pNewParent = [pRealParent parentWindow] ) )
    {
	    pRealParent = pNewParent;
    }

    [pRealParent removeChildWindow: pChildWindow];
#endif
}


void hideCursor() noexcept
{
    BOOST_VERIFY( ::CGDisplayHideCursor( kCGDirectMainDisplay ) == kCGErrorSuccess );
    //[::NSCursor hide];
}


void showCursor() noexcept
{
    BOOST_VERIFY( ::CGDisplayShowCursor( kCGDirectMainDisplay ) == kCGErrorSuccess );
    //[::NSCursor unhide];
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
