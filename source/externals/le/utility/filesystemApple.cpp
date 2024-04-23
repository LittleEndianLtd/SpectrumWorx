////////////////////////////////////////////////////////////////////////////////
///
/// filesystemApple.cpp/mm
/// ----------------------
///
/// Target platform specific boilerplate code.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "filesystemImpl.hpp"

#include "objc.hpp"
#include "trace.hpp"

#include <boost/assert.hpp>
#include <boost/config.hpp>

#import <TargetConditionals.h>
#import <CoreFoundation/CoreFoundation.h>
#import <Foundation/Foundation.h>
#if !( TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR )
    #include <sys/syslog.h>
#endif

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

bool LE_COLD isBundled()
{
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    return true;
#else
    CFBundleRef const mainBundle( CFBundleGetMainBundle() );
    if ( !mainBundle ) return false;
    CFBundleRefNum dummy0( 0 ), dummy1( 0 );
    auto const result( CFBundleOpenBundleResourceFiles( mainBundle, &dummy0, &dummy1 ) );
    //CFBundleRefNum const resourceMap( CFBundleOpenBundleResourceMap( mainBundle ) );
    CFBundleCloseBundleResourceMap( mainBundle, dummy0 );
    CFBundleCloseBundleResourceMap( mainBundle, dummy1 );
    CFRelease( mainBundle );
    return result == noErr;
#endif
}

bool LE_COLD isSandboxed()
{
    // http://www.cultofmac.com/113977/os-x-lion-sandboxing-is-a-killjoy-destined-to-ruin-our-mac-experience/
    // http://stackoverflow.com/questions/12177948/how-do-i-detect-if-my-app-is-sandboxed
    return ::getenv( "APP_SANDBOX_CONTAINER_ID" ) != nullptr;
}

namespace
{
    //...mrmlj...consider converting to carbon...
    // https://developer.apple.com/library/mac/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/AccessingFilesandDirectories/AccessingFilesandDirectories.html#//apple_ref/doc/uid/TP40010672-CH3-SW11
    // https://developer.apple.com/library/mac/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html#//apple_ref/doc/uid/TP40010672-CH2-SW28
    // http://www.taffysoft.com/pages/20120905-01.html
    // http://stackoverflow.com/questions/12714022/whats-the-equivalent-of-nshomedirectory-in-corefoundation
    // http://stackoverflow.com/questions/21291193/nshomedirectory-nsbundle-mainbundle
    NSString * LE_COLD domainDirectory( NSSearchPathDirectory const directory )
    {
        NSArray  * const dirPaths( NSSearchPathForDirectoriesInDomains( directory, NSUserDomainMask, YES ) );
        NSString * const dir     ( [dirPaths objectAtIndex: 0] );
        return dir;
    }

    template <SpecialLocations> NSString * pathFor();

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    template <> NSString * LE_COLD pathFor<AppData        >() { return ::NSHomeDirectory(); }
#else // OS X
    template <> NSString * LE_COLD pathFor<AppData        >() { return [[NSBundle mainBundle] bundlePath]; } // LE_VERIFY( ::_NSGetExecutablePath( appPath, &appPathSize ) == 0 );
#endif
    template <> NSString * LE_COLD pathFor<Documents      >() { return domainDirectory( NSDocumentDirectory ); }
    template <> NSString * LE_COLD pathFor<SharedDocuments>() { return ::NSHomeDirectory(); }
    template <> NSString * LE_COLD pathFor<Library        >() { return domainDirectory( NSLibraryDirectory  ); }
    template <> NSString * LE_COLD pathFor<Resources      >() { return [[NSBundle mainBundle] resourcePath]; }
  //template <> NSString * LE_COLD pathFor<SDCard         >() { return nullptr; }
    template <> NSString * LE_COLD pathFor<ExternalStorage>() { return nullptr; }
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    template <> NSString * LE_COLD pathFor<Temporaries    >() { return [pathFor<AppData>() stringByAppendingPathComponent: @"tmp"]; }
#else // OS X
    template <> NSString * LE_COLD pathFor<Temporaries    >() { return @"/var/tmp"; }
#endif
} // anonymous namespace

template <SpecialLocations rootDirectory>
template <class Result, class Functor>
Result LE_COLD PathResolver<rootDirectory>::apply( char const * const relativePath, Functor const f )
{
    @autoreleasepool
    {
        // http://developer.apple.com/library/mac/documentation/FileManagement/Conceptual/FileSystemProgrammingGuide/FileSystemOverview/FileSystemOverview.html
        NSString * const absolutePath( [pathFor<rootDirectory>() stringByAppendingPathComponent: ObjC::utf8String( relativePath )] );
        return f( [absolutePath UTF8String] );
    }
}

template <>
template <class Result, class Functor>
Result LE_COLD PathResolver<Resources>::apply( char const * const relativePath, Functor const f )
{
    @autoreleasepool
    {
        NSString * const filename( ObjC::utf8String( relativePath ) );
        NSString * const filePath( [ [NSBundle mainBundle] pathForResource: filename ofType: nullptr ] );
        return f( filePath ? [filePath UTF8String] : relativePath ); //...mrmlj...revisit: why are we supporting (erroneusly passed?) absolute paths here?
    }
}

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "filesystemImpl.inl"
