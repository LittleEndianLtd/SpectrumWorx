////////////////////////////////////////////////////////////////////////////////
///
/// gui.mm
/// ------
///
/// Copyright (c) 2013 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin.hpp"

#include "le/utility/trace.hpp"

#include "boost/preprocessor/stringize.hpp"

#import "AudioUnit/AUCocoaUIView.h"
//------------------------------------------------------------------------------

#define LE_PLUGIN_AU_VIEW_FACTORY LEPluginAUViewFactory
#define LE_PLUGIN_AU_VIEW         LEPluginAUView

@interface LE_PLUGIN_AU_VIEW : NSView { ::AudioUnit effect_; }

- initWithFrame: (NSRect) frame lePlugin: (::AudioUnit) audioUnit;
//- (void) detach;
- (void) dealloc;
- (void) applicationWillTerminate: (NSNotification*) aNotification;
- (void) close;
@end

@implementation LE_PLUGIN_AU_VIEW

- initWithFrame:(NSRect) frame lePlugin: (::AudioUnit) audioUnit
{
    BOOST_ASSERT( audioUnit );
    self = [super initWithFrame: frame];
    if ( self )
    {
        NSView * const pParentView( self );
        if ( AudioUnitSetProperty( audioUnit, LE::Plugins::Detail::auViewPropertyID, kAudioUnitScope_Global, 0, &pParentView, sizeof( pParentView ) ) != noErr )
            return nil;

        [[NSNotificationCenter defaultCenter] addObserver: self
                                                 selector: @selector (applicationWillTerminate:)
                                                     name: NSApplicationWillTerminateNotification
                                                   object: nil];

        effect_ = audioUnit;
    }

    return self;
}


- (void) dealloc
{
    [self  close  ];
    [super dealloc];
}


- (void) applicationWillTerminate: (NSNotification*) aNotification
{
    LE_TRACE( "\t SW AU: host terminating w/o closing the plugin." );
    [self close];
}


- (void) close
{
    BOOST_ASSERT( effect_ );
    ::NSView * const pParentView( nullptr );
    ::OSStatus const result( ::AudioUnitSetProperty( effect_, LE::Plugins::Detail::auViewPropertyID, kAudioUnitScope_Global, 0, &pParentView, sizeof( pParentView ) ) );
    LE_TRACE_IF( result != noErr, "\t SW AU: LEPluginAUView failed (%ld) to close the effect GUI (maybe the effect was destroyed beforehand).", result );
    [[NSNotificationCenter defaultCenter] removeObserver: self];
    BOOST_ASSERT( ( effect_ = nullptr ) == nullptr );
}

@end


@interface LE_PLUGIN_AU_VIEW_FACTORY : NSObject <AUCocoaUIBase> {}
- ( unsigned   ) interfaceVersion;
- ( NSString * ) description;
- ( NSView   * ) uiViewForAudioUnit: (::AudioUnit) leAudioUnit withSize: (NSSize) inPreferredSize;
@end


@implementation LE_PLUGIN_AU_VIEW_FACTORY

- ( unsigned   ) interfaceVersion { return 0; }

- ( NSString * ) description      { return @"LE.AU.SpectrumWorx"; }

- ( NSView   * ) uiViewForAudioUnit: (::AudioUnit) leAudioUnit withSize: (NSSize) preferredSize
{
    ::NSRect const frame = { ::NSPoint(), preferredSize };
    return
    [
        [
            [LE_PLUGIN_AU_VIEW alloc]
            initWithFrame: frame
            lePlugin     : leAudioUnit
        ] autorelease
    ];
}

@end

//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------
namespace Detail
{
//------------------------------------------------------------------------------

::CFURLRef auBundlePath()
{
    @autoreleasepool
    {
        // https://developer.apple.com/library/mac/#documentation/CoreFOundation/Conceptual/CFBundles/AccessingaBundlesContents/AccessingaBundlesContents.html
        ::Class const auViewFactoryClass( [LE_PLUGIN_AU_VIEW_FACTORY class] );
      //::Class const auViewFactoryClass( ::objc_getClass( BOOST_PP_STRINGIZE( LE_PLUGIN_AU_VIEW_FACTORY ) ) );
        LE_ASSUME( auViewFactoryClass );
        NSBundle * const auBundle    ( [NSBundle bundleForClass: auViewFactoryClass] );
        NSString * const auBundlePath( [auBundle bundlePath                        ] );
        LE_ASSUME( auBundlePath );
        return reinterpret_cast<::CFURLRef>( [[NSURL fileURLWithPath: auBundlePath] retain] );
    }
}

::CFStringRef auCocoaViewFactoryClassName()
{
    //return ::CFStringCreateWithCString( nullptr, LE_PLUGIN_AU_VIEW_FACTORY, kCFStringEncodingUTF8 );
    return CFSTR( BOOST_PP_STRINGIZE( LE_PLUGIN_AU_VIEW_FACTORY ) );
}

//------------------------------------------------------------------------------
} // namespace Detail
//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
