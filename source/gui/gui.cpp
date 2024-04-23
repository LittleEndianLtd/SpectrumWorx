//------------------------------------------------------------------------------
//...mrmlj...cleanup...
//------------------------------------------------------------------------------
#include "gui.hpp"

#include "core/host_interop/plugin2Host.hpp" //...mrmlj...only for Plugin2HostPassiveInteropController::ParameterLabelGetter...
#include "gui/editor/spectrumWorxEditor.hpp"
#if !LE_SW_SEPARATED_DSP_GUI
    #include "spectrumWorx.hpp" //...mrmlj..only for SpectrumWorxEditor::globalParameterChanged()...
#endif

#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/lexicalCast.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/mmap/mappble_objects/file/utility.hpp" // Boost sandbox

#include "boost/assert.hpp"
#include "boost/polymorphic_cast.hpp"
#ifdef _WIN32
#include "boost/range/algorithm/search.hpp"
#endif // _WIN32

#ifdef __APPLE__
    #include "dlfcn.h"
    #include "ApplicationServices/ApplicationServices.h" // only for CoreGraphics...
#endif // __APPLE__

#include <array>
#include <cstdio>
#include <cstdlib>
//------------------------------------------------------------------------------
#ifdef __APPLE__
    void const * swDLLAddress;
#endif // __APPLE__
#ifdef _WIN32
    extern "C" IMAGE_DOS_HEADER __ImageBase;
#endif // _WIN32
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

#ifdef __APPLE__
    // gui.mmm forward declarations.
    void initialiseMac   () noexcept;

    void makeEditorChild ( juce::ComponentPeer & editor, juce::ComponentPeer & childToBe ) noexcept;
    void detachFromEditor( juce::ComponentPeer & editor, juce::ComponentPeer & child     ) noexcept;

    void hideCursor() noexcept;
    void showCursor() noexcept;
#endif

////////////////////////////////////////////////////////////////////////////////
//
// ReferenceCountedGUIInitializationGuard static member definitions.
// -----------------------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

std::uint8_t ReferenceCountedGUIInitializationGuard::guiInitializationReferenceCount( 0 );

namespace
{
    void onGUIInitialization();
    void onGUIShutdown      ();
} // anonymous namespace

ReferenceCountedGUIInitializationGuard::ReferenceCountedGUIInitializationGuard()
{
    if ( guiInitializationReferenceCount && !isThisTheGUIThread() )
    {
        juce::AlertWindow::showNativeDialogBox
        (
            "SpectrumWorx error:",
            "We are sorry but SpectrumWorx does not currently "
            "support multiple editor instances with this host.",
            false
        );
        throw std::exception();
    }

    if ( guiInitializationReferenceCount++ == 0 )
    {
    #if defined( __APPLE__ )
        initialiseMac();
    #elif defined( _WIN32 )
        juce::Process::setCurrentModuleInstanceHandle( &__ImageBase );
    #endif // __APPLE__
        //...mrmlj...
        JUCE_AUTORELEASEPOOL
        {
            juce::initialiseJuce_GUI();
            juce::MessageManager::getInstance()->setCurrentThreadAsMessageThread();
            juce::Desktop::create();
            onGUIInitialization();
            juce::LookAndFeel::setDefaultLookAndFeel( &Theme::singleton() );
        }
    }
}


ReferenceCountedGUIInitializationGuard::~ReferenceCountedGUIInitializationGuard()
{
    BOOST_ASSERT( isThisTheGUIThread() );

#if defined( __APPLE__ ) && !__LP64__
    /// \note See the end note in detachComponentFromHostWindow() in gui.mm.
    ///                                       (23.12.2011.) (Domagoj Saric)
    /// \note Creation of another SW GUI might already be in the message queue
    /// (e.g. when switching between multiple instances of SW in Reaper) so we
    /// have to 'empty' the message queue before the reference count check to
    /// avoid destroying JUCE in after another SW GUI has been constructed (i.e.
    /// avoid the following scenario: reference-check...pump-message-queue-where
    /// -a-new-GUI-is-constructed...destroy-JUCE).
    ///                                       (20.02.2013.) (Domagoj Saric)
    /// \note Original JUCE note from detachComponentFromHostWindow():
    /// The event loop needs to be run between closing the window and deleting
    /// the plugin, presumably to let the cocoa objects get tidied up. Leaving
    /// out this line causes crashes in Live and Reaper when you delete the
    /// plugin with its window open. (Doing it this way rather than using a
    /// single longer timout means that we can guarantee how many messages will
    /// be dispatched, which seems to be vital in Reaper).
    ///                                           (13.09.2013.) (Domagoj Saric)
    for ( unsigned int i( 20 ); i != 0; --i )
        juce::MessageManager::getInstance()->runDispatchLoopUntil( 1 );
#endif // Apple

    if ( --guiInitializationReferenceCount == 0 )
    {
        JUCE_AUTORELEASEPOOL
        {
            onGUIShutdown();
            // Implementation note:
            //   We must manually reset the animator otherwise its timer becomes
            // orphaned when the juce::InternalTimerThread singleton is
            // destroyed (so it thinks it is still running even though its
            // parent juce::InternalTimerThread has been destroyed).
            //                                (15.12.2011.) (Domagoj Saric)
            juce::Desktop::getInstance().getAnimator().cancelAllAnimations( false );
            juce::Desktop::getInstance().getAnimator().stopTimer();
        #if defined( _WIN32 )
            BOOST_ASSERT( juce::Process::getCurrentModuleInstanceHandle() == &__ImageBase );
        #endif // _WIN32
            juce::shutdownJuce_GUI();
            juce::Desktop::destroy();
            juce::MessageManager::destroySingleton();

            BOOST_ASSERT( guiInitializationReferenceCount == 0 );
        }
    }
}

LE_NOTHROWNOALIAS
bool ReferenceCountedGUIInitializationGuard::isGUIInitialised()
{
#if LE_SW_GUI
    return guiInitializationReferenceCount != 0;
#else
    BOOST_ASSERT( guiInitializationReferenceCount == 0 );
    return false;
#endif // LE_SW_GUI
}


////////////////////////////////////////////////////////////////////////////////
//
// warningMessageBox()
// -------------------
//
//    A thread safe nothrow implementation that can be safely called whenever
// and wherever from.
//
////////////////////////////////////////////////////////////////////////////////

namespace
{
    bool LE_NOTHROW useJUCEAlertBox( bool const canBlock )
    {
        return ReferenceCountedGUIInitializationGuard::isGUIInitialised() &&
               ( canBlock || isThisTheGUIThread() );
    }
} // anonymous namespace

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

void LE_NOTHROW warningMessageBox( boost::string_ref const title, boost::string_ref const message, bool const canBlock )
{
    BOOST_ASSERT( ReferenceCountedGUIInitializationGuard::isGUIInitialised() || canBlock );
    JUCE_AUTORELEASEPOOL
    {
        try
        {
            juce::String const alertWindowTitle  ( title  .begin(), title  .size() );
            juce::String const alertWindowMessage( message.begin(), message.size() );
            if ( useJUCEAlertBox( canBlock ) )
            {
                if ( canBlock ) juce::AlertWindow::showMessageBox     ( juce::AlertWindow::WarningIcon, alertWindowTitle, alertWindowMessage         );
                else            juce::AlertWindow::showMessageBoxAsync( juce::AlertWindow::WarningIcon, alertWindowTitle, alertWindowMessage         );
            }
            else                juce::AlertWindow::showNativeDialogBox(                                 alertWindowTitle , alertWindowMessage, false );
        }
        catch (...) {}
    }
}

#pragma warning( pop )

bool LE_NOTHROW warningOkCancelBox( TCHAR const * const title, TCHAR const * const question )
{
    JUCE_AUTORELEASEPOOL
    {
        try
        {
            juce::String const alertWindowTitle  ( title    );
            juce::String const alertWindowMessage( question );
            return useJUCEAlertBox( true )
                        ? juce::AlertWindow::showOkCancelBox    ( juce::AlertWindow::WarningIcon, alertWindowTitle, alertWindowMessage       )
                        : juce::AlertWindow::showNativeDialogBox(                                 alertWindowTitle, alertWindowMessage, true );
        }
        catch (...) { return false; }
    }
}


static juce::File pluginRootPath  ;
static juce::File mruPresetsFolder;

namespace
{
    unsigned int const maxPathLength =
        #if defined( _WIN32 )
            // https://visualstudio.uservoice.com/forums/121579-visual-studio/suggestions/2156195-fix-260-character-file-name-length-limitation
            // http://blogs.msdn.com/b/bclteam/archive/2007/02/13/long-paths-in-net-part-1-of-3-kim-hamilton.aspx
            MAX_PATH;
        #elif defined( __APPLE__ )
            PATH_MAX + 1;
        #endif

    typedef TCHAR path_t[ maxPathLength ];

    unsigned int getBinaryPath( path_t & path )
    {
        #if defined( _WIN32 )

            DWORD const fullPathLength( ::GetModuleFileName( reinterpret_cast<HMODULE>( &__ImageBase ), path, _countof( path ) ) );
            BOOST_ASSERT( ( fullPathLength > 0 ) && ( fullPathLength < _countof( path ) ) );
            BOOST_ASSERT( path[ fullPathLength ] == '\0' );
            static TCHAR const extension[] = _T( ".dll" );

        #elif defined( __APPLE__ )

            // http://developer.apple.com/library/mac/#documentation/DeveloperTools/Reference/MachOReference/Reference/reference.html
            // http://lists.apple.com/archives/xcode-users/2004/Feb/msg00428.html

            //unsigned long fullPathLength( PATH_MAX );
            //NSGetExecutablePath( path, &fullPathLength );
            Dl_info exeInfo;
            BOOST_VERIFY( dladdr( &pluginRootPath, &exeInfo ) != 0 );
            ::swDLLAddress = exeInfo.dli_fbase;
            unsigned long fullPathLength( std::strlen( exeInfo.dli_fname ) );
            BOOST_ASSERT( fullPathLength <= _countof( path ) + _countof( ".paths" ) );
            std::memcpy( path, exeInfo.dli_fname, fullPathLength );
            //static TCHAR const extension[] = _T( ".dylib" );...mrmlj...does not see through symlink...
            static TCHAR const extension[] = _T( "." );
            path[ fullPathLength++ ] = _T( '.'  );
            path[ fullPathLength   ] = _T( '\0' );

        #endif // OS

        unsigned int const dotIndex      ( fullPathLength - sizeof( '\0' ) - ( _countof( extension ) - sizeof( '\0' ) - sizeof( '.' ) ) );
        unsigned int const insertionIndex( dotIndex + sizeof( '\0' ) );

        BOOST_ASSERT( path[ dotIndex ] == _T( '.' ) );
        BOOST_ASSERT( std::_tcscmp( &path[ dotIndex ], extension ) == 0 );
        return insertionIndex;
    }
} // anonymous namespace

#ifdef LE_SW_FMOD

bool LE_NOTHROW initializePaths()
{
    try
    {
        if ( !havePathsBeenInitialised() )
        {
            path_t path;
            getBinaryPath( path );
            juce::File const binaryPath( path );

            pluginRootPath   = binaryPath    .getSiblingFile( _T( "SpectrumWorx" ) );
            mruPresetsFolder = pluginRootPath.getChildFile  ( _T( "Presets"      ) );
        }

        bool const result( pluginRootPath.isDirectory() );
        if ( !result )
        {   //...mrmlj...
            warningMessageBox
            (
                "SpectrumWorx critical error: unable to access installation folder.",
                pluginRootPath.getFullPathName().toUTF8().getAddress(),
                true
            );
        }

        return true;
    }
    catch ( ... )
    {
        return false;
    }
}

#else // FMOD

namespace
{
    path_t & getPathsFilePath( path_t & storage )
    {
        unsigned int const insertionIndex( getBinaryPath( storage ) );

        /// \todo Add a singleton plugin host access/getter providing
        /// functionality available without a plugin instance. Afterwards
        /// uncomment and properly implement the below (commented out)
        /// check/assert.
        ///                                   (22.09.2009.) (Domagoj Saric)
        //assert( !effect().host().getPluginDirectory() || ( effect().host().getPluginDirectory() == path ) );

        static TCHAR const pathsSuffix[] = _T( "paths" );
        /*std*/::_tcscpy( &storage[ insertionIndex ], pathsSuffix );

        return storage;
    }
} // anonymous namespace

boost::mmap::mapped_view<char const> mapPathsFile()
{
    path_t path;
    return boost::mmap::mapped_view<char const>( boost::mmap::map_read_only_file( getPathsFilePath( path ) ) );
}

boost::mmap::mapped_view<char> mapPathsFile( unsigned int const desiredSize )
{
    path_t path;
    return boost::mmap::mapped_view<char>( boost::mmap::map_file( getPathsFilePath( path ), desiredSize ) );
}


bool LE_NOTHROW initializePaths()
{
    try
    {
        if ( !havePathsBeenInitialised() )
        {
            boost::mmap::mapped_view<char const> const pathsFile( mapPathsFile() );
            if ( !pathsFile )
            {   //...mrmlj...
                path_t path;
                warningMessageBox
                (
                    "SpectrumWorx critical error: unable to open a configuration file.",
                #ifdef _UNICODE
                    juce::String( getPathsFilePath( path ) ).toUTF8().getAddress(),
                #else
                    getPathsFilePath( path ),
                #endif // _UNICODE
                    true
                );
                return false;
            }
            boost::string_ref const rootPath   ( reinterpret_cast<char const *>( pathsFile.begin() ), std::find( pathsFile.begin(), pathsFile.end(), '\n' ) - pathsFile.begin() );
            boost::string_ref const presetsPath( rootPath.end() + 1                                 , pathsFile.end() - ( rootPath.end() + 1 )                                  );
            pluginRootPath   = juce::String::fromUTF8( rootPath   .begin(), static_cast<unsigned int>( rootPath   .size() ) );
            mruPresetsFolder = juce::String::fromUTF8( presetsPath.begin(), static_cast<unsigned int>( presetsPath.size() ) );
        #ifdef __APPLE__
            // Implementation note:
            //   A temporary workaround to allow user-folder installations
            // on OS X.
            //                            (01.12.2010.) (Domagoj Saric)
            if ( !pluginRootPath.isDirectory() )
            {
                JUCE_AUTORELEASEPOOL
                {
                    BOOST_ASSERT( !mruPresetsFolder.isDirectory() );
                    BOOST_ASSERT( rootPath   .front() == '/' );
                    BOOST_ASSERT( presetsPath.front() == '/' );
                    juce::File const userFolder( juce::File::getSpecialLocation( juce::File::userHomeDirectory ) );
                    pluginRootPath   = userFolder.getChildFile( juce::String( rootPath   .begin() + 1, rootPath   .size() - 1 ) );
                    mruPresetsFolder = userFolder.getChildFile( juce::String( presetsPath.begin() + 1, presetsPath.size() - 1 ) );
                }
            }
        #endif // __APPLE__
        }

        bool const result( pluginRootPath.isDirectory() );
        if ( !result )
            //...mrmlj...
            warningMessageBox( "SpectrumWorx critical error: unable to access installation folder.", pluginRootPath.getFullPathName().toUTF8()/*getCharPointer()*/.getAddress(), true );

        return result;
    }
    catch ( ... )
    {
        return false;
    }
}
#endif // LE_SW_FMOD


bool LE_NOTHROW havePathsBeenInitialised()
{
    return pluginRootPath != juce::File::nonexistent;
}


LE_NOTHROWNOALIAS juce::File const & rootPath()
{
    BOOST_ASSERT_MSG( ( pluginRootPath != juce::File::nonexistent ), "Not initialized." );
    return pluginRootPath;
}

LE_NOTHROWNOALIAS juce::File & presetsFolder()
{
    BOOST_ASSERT_MSG( ( mruPresetsFolder != juce::File::nonexistent ), "Not initialized." );
    return mruPresetsFolder;
}

LE_NOALIAS juce::File resourcesPath()
{
    return rootPath().getChildFile( "Resources" );
}

LE_NOALIAS juce::File licencesPath()
{
    return rootPath().getChildFile( "Licences" );
}


#ifdef __APPLE__
::FSRef makeFSRefFromPath( juce::String const & path )
{
    // FSPathMakeRef broken on relative paths
    // http://lists.apple.com/archives/carbon-development/2003/Mar/msg00997.html
    FSRef result;
    BOOST_VERIFY( ::FSPathMakeRef( reinterpret_cast<UInt8 const *>( path.getCharPointer().getAddress() ), &result, nullptr ) == noErr );
    return result;
}

::CFURLRef makeCFURLFromPath( juce::File const & path )
{
    //::CFStringRef const pathString( ::CFStringCreateWithCStringNoCopy( nullptr, path.getFullPathName().getCharPointer().getAddress(), kCFStringEncodingUTF8, kCFAllocatorNull ) );
    //::CFURLRef    const pathURL   ( ::CFURLCreateWithFileSystemPath  ( nullptr, pathString, kCFURLPOSIXPathStyle, false ) );
    //BOOST_ASSERT( pathString );
    //BOOST_ASSERT( pathURL    );
    //::CFRelease( pathString );
    ::FSRef    const pathFSRef( makeFSRefFromPath( path.getFullPathName() )   );
    ::CFURLRef const pathURL  ( ::CFURLCreateFromFSRef( nullptr, &pathFSRef ) );
    BOOST_ASSERT( pathURL );
    return pathURL;
}
#endif // __APPLE__


juce::Image resourceBitmap( char const (&bitmapNumber)[ 2 + 1 ] )
{
    char const extension[] = ".png";
    std::array
    <
        char,
        _countof( bitmapNumber ) - 1 + _countof( extension ) - 1
    > fileName;
    BOOST_ASSERT( bitmapNumber[ 2 ] == '\0' );
    fileName[ 0 ] = bitmapNumber[ 0 ];
    fileName[ 1 ] = bitmapNumber[ 1 ];
    fileName[ 2 ] = extension[ 0 ];
    fileName[ 3 ] = extension[ 1 ];
    fileName[ 4 ] = extension[ 2 ];
    fileName[ 5 ] = extension[ 3 ];

    juce::File const file( resourcesPath().getChildFile( juce::String( &fileName[ 0 ], fileName.size() ) ) );
#ifndef NDEBUG
    try
    {
#endif  // NDEBUG
    juce::FileInputStream fileInputStream( file );
    juce::PNGImageFormat pngReader;
    juce::Image const bitmap( pngReader.decodeImage( fileInputStream ) );
    BOOST_ASSERT_MSG( bitmap.isValid() && bitmap.getWidth() && bitmap.getHeight(), "Error loading bitmap from disk." );

#ifdef __APPLE__
    // PC to Mac gamma correction
    // http://www.giassa.net/?page_id=475
    {
        juce::Image::BitmapData pixels( bitmap, juce::Image::BitmapData::readWrite );
        BOOST_ASSERT
        (
            ( ( bitmap.getFormat() == juce::Image::ARGB ) && ( pixels.pixelStride == 4 ) ) ||
            ( ( bitmap.getFormat() == juce::Image::RGB  ) && ( pixels.pixelStride == 3 ) )
        );

        {
            float const pcGamma ( 2.2f );
            float const macGamma( 1.8f );

            unsigned char       * const       p_image_data     ( pixels.getLinePointer( 0 )                                );
            unsigned char       * LE_RESTRICT p_current_channel( p_image_data                                              );
            unsigned char const * const       p_image_end      ( p_image_data + ( pixels.lineStride * bitmap.getHeight() ) );

            while ( p_current_channel != p_image_end )
            {
                *p_current_channel = static_cast<unsigned char>( Math::convert<unsigned int>( /*std*/::powf( *p_current_channel / 255.0f, pcGamma / macGamma ) * 255 ) );
                ++p_current_channel;
            }
        }
    }
#endif // __APPLE__

    return bitmap;
#ifndef NDEBUG
    }
    catch ( ... )
    {
        std::printf( "Error loading bitmap (%s) from disk.\n", file.getFullPathName().toUTF8().getAddress() );
        throw;
    }
#endif  // NDEBUG
}


void LE_NOTHROW LE_FASTCALL paintImage( juce::Graphics & graphics, juce::Image const & image ) { paintImage( graphics, image, 0, 0 ); }

void LE_NOTHROW LE_FASTCALL paintImage
(
    juce::Graphics       & graphics,
    juce::Image    const & image,
    int            const   x,
    int            const   y
)
{
    graphics.drawImage
    (
        image,
        x, y, image.getWidth(), image.getHeight(),
        0, 0, image.getWidth(), image.getHeight()
    );
}


void setSizeFromImage( juce::Component & component, juce::Image const & image )
{
    component.setSize( image.getWidth(), image.getHeight() );
}


void addToParentAndShow( juce::Component & parent, juce::Component & childToBe )
{
    parent.addAndMakeVisible( &childToBe );
}


LE_NOTHROW
void fadeOutComponent( juce::Component & component, float const finalAlpha, unsigned int const duration, bool const useProxyComponent )
{
    try
    {
	    juce::Point<int> const centre( component.getBounds().getCentre() );
	    juce::Desktop::getInstance().getAnimator().animateComponent
        (
            &component, juce::Rectangle<int>( centre.getX(), centre.getY(), 0, 0 ), finalAlpha, duration, useProxyComponent, 0, 0
        );
    }
    catch ( ... ) {}
}


LE_NOTHROWNOALIAS LE_NOINLINE
bool LE_COLD isThisTheGUIThread()
{
#if LE_SW_GUI
#ifndef NDEBUG
    if ( !GUI::isGUIInitialised() ) //...mrmlj...to avoid an LE_ASSUME/assertion failure in juce::MessageManager::getInstance()...
        return false;
#endif // NDEBUG
#ifdef __APPLE__
    if ( !juce::MessageManager::getInstanceWithoutCreating() ) return false; //...mrmlj...quick-hack to fix crashes on OSX when this function is called before the GUI is initialised...
#endif // __APPLE__
    return juce::MessageManager::getInstance()->isThisTheMessageThread();
#else
    return false;
#endif // LE_SW_GUI
}

LE_NOTHROWNOALIAS
bool LE_COLD isGUIInitialised() { return ReferenceCountedGUIInitializationGuard::isGUIInitialised(); }

LE_NOTHROWNOALIAS
float LE_COLD displayScale()
{
    auto const &       desktop( juce::Desktop::getInstance()   );
    auto         const scale  ( desktop.getGlobalScaleFactor() );
#ifndef NDEBUG
    for ( auto const & display : desktop.getDisplays().displays )
        BOOST_ASSERT( display.scale == scale );
#endif // NDEBUG
    return scale;
}


namespace Detail
{
    LE_NOTHROW  LE_COLD void LE_FASTCALL setName( juce::Component & widget, juce::String const & newName ) { widget.juce::Component::setName( newName ); }
    LE_NOINLINE LE_COLD void LE_FASTCALL setName( juce::Component & widget, char const * const   newName ) { setName( widget, juce::String( newName ) ); }

    bool LE_FASTCALL hasDirectFocus( juce::Component const & widget )
    {
        bool const result( &widget == widget.getCurrentlyFocusedComponent() );
        BOOST_ASSERT( result == widget.hasKeyboardFocus( false ) );
        return result;
    }

    bool LE_FASTCALL hasFocus( juce::Component const & widget )
    {
        bool const result
        (
            hasDirectFocus( widget ) ||
            isParentOf( widget, widget.getCurrentlyFocusedComponent() )
        );
        BOOST_ASSERT( result == widget.hasKeyboardFocus( true ) );
        return result;
    }

    bool LE_FASTCALL isParentOf( juce::Component const & parent, juce::Component const & possibleChild )
    {
        juce::Component * pParent( possibleChild.getParentComponent() );
        while ( pParent )
        {
            if ( pParent == &parent )
                return true;
            pParent = pParent->getParentComponent();
        }
        return false;
    }

    bool LE_FASTCALL isParentOf( juce::Component const & parent, juce::Component const * pPossibleChild )
    {
        return pPossibleChild && isParentOf( parent, *pPossibleChild );
    }
} // namespace Detail

#ifdef _WIN32
    HHOOK OwnedWindowBase::wndProcHook( 0 );
#endif // _WIN32

void OwnedWindowBase::attach
(
    SpectrumWorxEditor & parent,
    juce::Component    & window
)
{
    BOOST_ASSERT( !window.isOpaque() );

    //...mrmlj...check these links for mac keystroke handling:
    //http://lists.steinberg.net:8100/Lists/vst-plugins/Message/12066.html
    juce::ComponentPeer & owner( *parent.getPeer() );
#ifdef _WIN32
    // Implementation note:
    //   FL Studio 9 uses a GUI engine that has a 'nonstandard' way of handling/
    // implementing owned-windows, that is, all windows in the FL Studio are
    // real child windows (have the WS_CHILD style set) of the main window.
    // Because of this we cannot simply call GetAncestor() with GA_ROOT to get
    // our 'real' parent (the wrapping window) because this would always return
    // the handle to the main application window (in FL Studio 9). As a
    // workaround we traverse up the window hierarchy searching for windows
    // that have "SpectrumWorx" in their names assuming these would be our
    // wrapping windows. Because of hosts like Audition 3.0 [that have several
    // wrapper windows that have the plugin name in their title and that, in
    // addition, are not 'sequential'/direct ancestors/descendants (e.g.
    // Audition wraps SpectrumWorx in a window called " - SpectrumWorx" that is
    // in turn a child of a window called "EffectView" that is in turn a child
    // of the final wrapper window called "VST Plugin - SpectrumWorx")] we must
    // traverse the entire hierarchy and take the last/highest 'appropriate'
    // window.
    //                                        (25.05.2010.) (Domagoj Saric)
    /// \todo The above approach works for so far tested hosts but will fail to
    /// work for hosts that put the name of the (active) plugin in the title
    /// of the main window. Consider a smarter solution when it becomes
    /// necessary.
    ///                                       (25.05.2010.) (Domagoj Saric)
    /// http://www.codeproject.com/Tips/222075/All-about-owned-windows
    /// http://blogs.msdn.com/b/oldnewthing/archive/2010/03/15/9978691.aspx
    HWND const editorHandle                ( reinterpret_cast<HWND>( owner.getNativeHandle() ) );
    HWND       editorRootParentNativeHandle( editorHandle                                      );
    HWND       lastPossibleHandle          ( 0                                                 );
    boost::string_ref const spectrumWorxTitle( "SpectrumWorx" );
    char                    windowTitleBuffer[ 256 ];
    do
    {
        editorRootParentNativeHandle = ::GetAncestor( editorRootParentNativeHandle, GA_PARENT );
        boost::string_ref const windowTitle
        (
            windowTitleBuffer,
            ::GetWindowTextA( editorRootParentNativeHandle, windowTitleBuffer, _countof( windowTitleBuffer ) )
        );
        if ( boost::search( windowTitle, spectrumWorxTitle ) != windowTitle.end() )
            lastPossibleHandle = editorRootParentNativeHandle;
    } while ( editorRootParentNativeHandle );
    editorRootParentNativeHandle = lastPossibleHandle;
    if ( !editorRootParentNativeHandle )
    {
        editorRootParentNativeHandle = ::GetAncestor( editorHandle, GA_ROOT );
        // Implementation note:
        //   Check if we seem to be dealing with a host like Audio Mulch that
        // uses Qt or something similar and uses only child windows (so
        // GetParent() always returns the main application window) that
        // additionally have no window text set (so we cannot use text search).
        //   If so we have no other option but to search for the first parent
        // whose client area encompasses our window.
        //                                    (02.06.2010.) (Domagoj Saric)
        // Implementation note:
        //   To detect if we reached the main application window we must use the
        // GetParent() function (that also 'returns' owner windows) and not the
        // GetAncestor() function with the GA_PARENT parameter because the main
        // application window can 'just' own our parent/wrapper window. In this
        // case GetAncestor( *, GA_PARENT ) would return the desktop window and
        // we would mistake it for the main application window.
        //                                    (04.06.2010.) (Domagoj Saric)
        HWND const masterParent( ::GetParent( editorRootParentNativeHandle ) );
        if ( !masterParent || ( masterParent == ::GetDesktopWindow() ) )
        {
            RECT editorRect;
            BOOST_VERIFY( ::GetWindowRect( editorHandle, &editorRect ) );
            editorRootParentNativeHandle = editorHandle;
            do
            {
                editorRootParentNativeHandle = ::GetAncestor( editorRootParentNativeHandle, GA_PARENT );
                RECT editorParentRect;
                BOOST_VERIFY( ::GetWindowRect( editorRootParentNativeHandle, &editorParentRect ) );
                if
                (
                    ( editorParentRect.left < editorRect.left ) &&
                    ( editorParentRect.top  < editorRect.top  )
                )
                    break;
            } while ( editorRootParentNativeHandle );
        }
    }

    if ( wndProcHook == 0 )
        wndProcHook = ::SetWindowsHookEx( WH_CALLWNDPROC, &callWndHookProc, 0, ::GetCurrentThreadId() );
    BOOST_ASSERT( wndProcHook ); //...mrmlj...better error handling desired...

    window.juce::Component::addToDesktop( juce::ComponentPeer::windowIsSemiTransparent, owner.getNativeHandle() );
#endif // _WIN32

#ifdef __APPLE__
    // http://www.cocoadev.com/index.pl?MagneticWindows
    // http://www.cocoadev.com/index.pl?WindowFollowingWindow
    // http://web.mac.com/mabi99/marcocoa/blog/Entries/2007/5/30_Watching_a_window%E2%80%99s_frame.html
    window.juce::Component::addToDesktop( juce::ComponentPeer::windowIsSemiTransparent );
    juce::ComponentPeer & ownedWindowPeer( *window.getPeer() );
    makeEditorChild( owner, ownedWindowPeer );
#endif // __APPLE__

    window.juce::Component::setVisible( true );
}


void OwnedWindowBase::detach( SpectrumWorxEditor & editor, juce::Component & ownee )
{
#ifdef _WIN32
    BOOST_ASSERT( wndProcHook != 0 );
    if ( juce::ComponentPeer::getNumPeers() < 3 )
    {
        BOOST_ASSERT( juce::ComponentPeer::getNumPeers() == 2                );
        BOOST_ASSERT( juce::ComponentPeer::getPeer( 0 )  == editor.getPeer() );
        BOOST_ASSERT( juce::ComponentPeer::getPeer( 1 )  == ownee .getPeer() );
        BOOST_VERIFY( ::UnhookWindowsHookEx( wndProcHook ) );
        wndProcHook = 0;
    }
    boost::ignore_unused_variable_warning( editor );
    boost::ignore_unused_variable_warning( ownee  );
#else
    detachFromEditor( *editor.getPeer(), *ownee.getPeer() );
#endif // _WIN32
}


#ifdef _WIN32
juce::ComponentPeer * peerWithParentHandle( HWND const parentWindowHandle )
{
    for ( unsigned int i( 0 ); i < static_cast<unsigned int>( juce::ComponentPeer::getNumPeers() ); ++i )
    {
        juce::ComponentPeer * const pPeer( juce::ComponentPeer::getPeer( i ) );
        BOOST_ASSERT( pPeer );
        if ( ::IsChild( parentWindowHandle, static_cast<HWND>( pPeer->getNativeHandle() ) ) )
            return pPeer;
    }
    return nullptr;
}


LRESULT CALLBACK OwnedWindowBase::callWndHookProc( int const nCode, WPARAM const wParam, LPARAM const lParam )
{
    BOOST_ASSERT( lParam );
    CWPSTRUCT const & info( *reinterpret_cast<CWPSTRUCT const *>( lParam ) );
    if ( info.message == WM_WINDOWPOSCHANGED )
    {
        BOOST_ASSERT( info.lParam );
        WINDOWPOS const & wp( *reinterpret_cast<WINDOWPOS const *>( info.lParam ) );
        BOOST_ASSERT( wp.hwnd == info.hwnd );
        // Implementation note:
        //   We have to update on all parent window adjustments (not just on
        // movement) because some hosts do some non-movement adjustments to the
        // main editor's parent window (that nonetheless affect the relative
        // position of our editor window within the parent) right after creation
        // and that breaks the positioning of the 'registration page as a demo
        // popup'.
        //                                    (20.04.2010.) (Domagoj Saric)
        //if ( !( wp.flags & SWP_NOMOVE ) )
        {
            juce::ComponentPeer * pEditorPeer( peerWithParentHandle( info.hwnd ) );
            if ( pEditorPeer )
            {
                SpectrumWorxEditor & editor( *boost::polymorphic_downcast<SpectrumWorxEditor *>( &pEditorPeer->getComponent() ) );
                RECT parentRect;
                BOOST_VERIFY( ::GetWindowRect( info.hwnd, &parentRect ) );
                RECT editorRect;
                BOOST_VERIFY( ::GetWindowRect( reinterpret_cast<HWND>( pEditorPeer->getNativeHandle() ), &editorRect ) );
                unsigned int const parentHorizontalMargin( editorRect.left - parentRect.left );
                unsigned int const parentVerticalMargin  ( editorRect.top  - parentRect.top  );
                POINT newEditorLocation =
                {
                    wp.x + parentHorizontalMargin,
                    wp.y + parentVerticalMargin
                };
                // Implementation note:
                //   Because adjustPositions()/juce::ComponentPeer::setPosition()
                // requires screen coordinates we must first call
                // ClientToScreen() because of hosts like FL Studio where editor
                // wrapper windows are also child windows of the main window and
                // thus WINDOWPOS contains client coordinates. For other hosts
                // (whose wrapper windows are merely owned/non-child windows)
                // this will simply 'do nothing' so there is no need for
                // separate logic.
                //                            (25.05.2010.) (Domagoj Saric)
                BOOST_VERIFY( ::ClientToScreen( ::GetAncestor( info.hwnd, GA_PARENT ), &newEditorLocation ) );
                /// \note Account for display scaling/"high DPI display".
                /// http://msdn.microsoft.com/en-us/library/windows/desktop/ms633533(v=vs.85).aspx
                /// http://blogs.msdn.com/b/fontblog/archive/2005/11/08/490490.aspx
                /// http://blogs.msdn.com/b/b8/archive/2012/03/21/scaling-to-different-screens.aspx
                /// http://msdn.microsoft.com/en-us/magazine/dn574798.aspx
                /// http://stackoverflow.com/questions/8060280/getting-an-dpi-aware-correct-rect-from-getwindowrect-from-a-external-window
                /// http://www.juce.com/forum/topic/scaling-factor
                /// http://www.juce.com/forum/topic/density-independent-pixels
                /// http://www.juce.com/forum/topic/another-bad-update
                /// http://www.juce.com/forum/topic/windows-81-double-scaling
                ///                           (25.09.2014.) (Domagoj Saric)
                float const scale( 1 / displayScale() );
                newEditorLocation.x = Math::round( Math::convert<float>( newEditorLocation.x ) * scale );
                newEditorLocation.y = Math::round( Math::convert<float>( newEditorLocation.y ) * scale );
                BOOST_ASSERT( Math::abs( newEditorLocation.x - editor.getScreenX() ) <= 1 ); //...mrmlj...rounding error...
                BOOST_ASSERT( Math::abs( newEditorLocation.y - editor.getScreenY() ) <= 1 );
                adjustPositions
                (
                    editor.presetBrowser_.get_ptr(),
                    editor.settings_     .get_ptr(),
                    newEditorLocation.x,
                    newEditorLocation.y,
                    wp.flags
                );
            }
        }
    }

    return ::CallNextHookEx( wndProcHook, nCode, wParam, lParam );
}
#endif // _WIN32

namespace
{
    void adjustOwnedWindow
    (
        juce::Component * const pWindow,
        unsigned int    &       x,
        unsigned int      const y,
        unsigned int      const doHide,
        unsigned int      const doShow
    )
    {
        unsigned int const freeWindowDistance( 6 );

        if ( pWindow )
        {
            x -= freeWindowDistance;
            x -= pWindow->getWidth();

            pWindow->setTopLeftPosition( x, y );

            #ifndef _WIN32
                LE_ASSUME( !doShow );
                LE_ASSUME( !doHide );
            #endif // _WIN32
                 if ( doHide ) pWindow->juce::Component::setVisible( false );
            else if ( doShow ) pWindow->juce::Component::setVisible( true  );
        }
    }
} // anonymous namespace

void OwnedWindowBase::adjustPositions
(
    juce::Component * pFirstWindow, juce::Component * pSecondWindow,
    unsigned int const editorX, unsigned int const editorY,
    unsigned int const flags
)
{
    unsigned int const mainEditorMargin( 7 );

    unsigned int       x( editorX                    );
    unsigned int const y( editorY + mainEditorMargin );

#ifdef _WIN32
    BOOL const doHide( flags & SWP_HIDEWINDOW );
    BOOL const doShow( flags & SWP_SHOWWINDOW );
#else
    //...mrmlj...clean this up...
    BOOST_ASSERT( !flags );
    bool const doHide( false );
    bool const doShow( false );
#endif // _WIN32
    BOOST_ASSERT( ( !doHide && !doShow ) || ( !!doHide != !!doShow ) );

    adjustOwnedWindow( pFirstWindow , x, y, doHide, doShow );
    adjustOwnedWindow( pSecondWindow, x, y, doHide, doShow );
}


void OwnedWindowBase::adjustPositions( SpectrumWorxEditor & parent, juce::Component * const pFirstWindow, juce::Component * const pSecondWindow )
{
    juce::Point<int> const parentPosition( parent.getScreenPosition() );
    adjustPositions
    (
        pFirstWindow         , pSecondWindow        ,
        parentPosition.getX(), parentPosition.getY(),
        0
    );
}

void OwnedWindowBase::adjustPositionsForPresetBrowser( SpectrumWorxEditor & parent, juce::Component * const pCurrentWindowState )
{
    adjustPositions( parent, pCurrentWindowState, parent.settings_.get_ptr() );
}

void OwnedWindowBase::adjustPositionsForSettings( SpectrumWorxEditor & parent, juce::Component * const pCurrentWindowState )
{
    adjustPositions( parent, parent.presetBrowser_.get_ptr(), pCurrentWindowState );
}


#if 0 //...mrmlj...does not work with the latest juce...cleanup...
void AsyncRepainter::repaint( juce::Component & component, int const x, int const y, int const w, int const h )
{
    if ( isThisTheGUIThread() )
    {
        static_cast<AsyncRepainter &>( component ).juce::Component::internalRepaint( x, y, w, h );
    }
    else
    {
        class AsyncRepaintCallback : public juce::CallbackMessage
        {
        public:
            AsyncRepaintCallback( juce::Component & component, int const x, int const y, int const w, int const h )
                : pComponent_( &component ), x_( x ), y_( y ), w_( w ), h_( h )
            {
                juce::CallbackMessage::post();
            }

        private:
            void messageCallback() LE_OVERRIDE
            {
                juce::Component * const pComponent( pComponent_.getComponent() );
                if ( pComponent )
                    static_cast<AsyncRepainter &>( *pComponent ).juce::Component::internalRepaint( x_, y_, w_, h_ );
            }

            juce::Component::SafePointer<juce::Component> pComponent_;
            int const x_;
            int const y_;
            int const w_;
            int const h_;
        };

        new (std::nothrow) AsyncRepaintCallback( component, x, y, w, h );
    }
}
#endif // 0


DrawableText::DrawableText
(
    char const *        const text,
    unsigned int        const x,
    unsigned int        const y,
    unsigned int        const width,
    unsigned int        const height,
    juce::Justification const justification,
    juce::Font const &        font
)
{
    addFittedText
    (
        font,
        text,
        Math::convert<float>(     x ), Math::convert<float>(      y ),
        Math::convert<float>( width ), Math::convert<float>( height ),
        justification,
        1
    );
}


juce::Font DrawableText::defaultFont()
{
    juce::Font font( Theme::singleton().Theme::getPopupMenuFont() );
    font.setHeight( 11 );
    return font;
}


void BackgroundImage::paint( juce::Graphics & graphics )
{
    graphics.setOpacity( Theme::singleton().settings().globalOpacity );
    paintImage( graphics, image() );
}


juce::Image const & BackgroundImage::image() const
{
    BOOST_ASSERT( pImage_ ); return *pImage_;
}


void BackgroundImage::setImage( juce::Image const & image )
{
    pImage_ = &image;
}


BitmapButton::BitmapButton
(
    juce::Component    &       parent,
    juce::Image  const &       on,
    juce::Image  const &       off,
    juce::Colour const &       overlayColourWhenOver,
    bool                 const toggled
)
{
    //...mrmlj...the settings bitmaps are currently broken...
    BOOST_ASSERT( ( on.getHeight() == off.getHeight() ) || ( &on == &resourceBitmap<SettingsOn>() ) );
    BOOST_ASSERT(   on.getWidth () == off.getWidth ()                                               );

    /// \note JUCE's ugly Value chemistry tends to backfire when the button's
    /// 'value' is updated through automation/LFOing: juce::Button registers
    /// itself as a listener of its own Value object and the value we set gets
    /// assigned to the internal Value object which in turn sends an
    /// asynchronous notification which, when arrives, may try to reset an
    /// already changed value and this in turn generates a bogus "value changed"
    /// notification. As we do not use the Value objects/interface directly, it
    /// is safe to simply cut this Button->Value->Button loop.
    ///                                       (20.03.2013.) (Domagoj Saric)
    getToggleStateValue().removeListener( this );

    setWantsKeyboardFocus          ( false );
    setMouseClickGrabsKeyboardFocus( false );

    setImages
    (
        true                 , // resizeButtonNowToFitThisImage,
        false                , // rescaleImagesWhenButtonSizeChanges,
        true                 , // preserveImageProportions,
        off                  , // normalImage
        normalOpacity()      , // imageOpacityWhenNormal,
        normalOverlay()      , // overlayColourWhenNormal,
        juce::Image::null    , // overImage,
        overOpacity()        , // imageOpacityWhenOver,
        overlayColourWhenOver, // overlayColourWhenOver,
        on                   , // downImage,
        downOpacity()        , // imageOpacityWhenDown,
        downOverlay()        , // overlayColourWhenDown,
        0.0f                   // hitTestAlphaThreshold
    );

    setClickingTogglesState( toggled );

    addToParentAndShow( parent, *this );
}


//...mrmlj...copy pasted from the juce::ImageButton class because it hides it...
juce::Image BitmapButton::getCurrentImage() const
{
    if ( isDown() || getToggleState() )
        return getDownImage();

    if ( isOver() )
        return getOverImage();

    return getNormalImage();
}


juce::Colour const & BitmapButton::downOverlay()
{
    return juce::Colours::transparentWhite;
}


juce::Colour const & BitmapButton::defaultOverOverlay()
{
    return juce::Colours::transparentWhite;
}


juce::Colour const & BitmapButton::normalOverlay()
{
    return juce::Colours::transparentWhite;
}


// Implementation note:
//   The built in JUCE ComboBox does not allow enough customization so we had to
// make our own. Just like the original ComboBox we use the PopupMenu class for
// the implementation. Because the juce::PopupMenu class is limited and/or too
// encapsulated we use here extremely dirty trickery to get to its internal
// details so as to be able to modify it according to our needs in manner that
// is easier and more efficient than that of the original juce::ComboBox (e.g.
// recreating the whole menu when the selection changes, holding duplicates of
// all items etc...) or to workaround bugs (e.g. the menu displaying in wrong
// places when in lower and/or right half of the screen)...
//                                            (17.03.2010.) (Domagoj Saric)
/// \todo Properly document the following hacks if they persist.
///                                           (26.02.2010.) (Domagoj Saric)
namespace JuceHackery
{
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4512 ) // Assignment operator could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

    class MenuItemInfo
    {
    public:
        const int itemID;
        juce::String text;
        const juce::Colour textColour;
        /*const*/ bool active, isSeparator, isTicked, usesColour;
        juce::Image image;
        juce::ReferenceCountedObjectPtr <juce::PopupMenu::CustomComponent> customComp;
        juce::ScopedPointer <juce::PopupMenu> subMenu;
        juce::ApplicationCommandManager* const commandManager;
    }; // class MenuItemInfo

    #pragma warning( pop )


    static unsigned int getNumberOfItems( juce::PopupMenu const & menu ) { return menu.items.size(); }

    static MenuItemInfo & getItemInfo( juce::PopupMenu & menu, unsigned int const itemIndex )
    {
        MenuItemInfo & item
        (
            *reinterpret_cast<MenuItemInfo *>
            (
                menu.items.getUnchecked( itemIndex )
            )
        );

        #ifdef _DEBUG
            juce::PopupMenu::MenuItemIterator menuIterator( menu );
            BOOST_VERIFY( menuIterator.next() );
            unsigned int currentItemIndex( 0 );
            while ( currentItemIndex++ != itemIndex )
                BOOST_VERIFY( menuIterator.next() );
            BOOST_ASSERT( menuIterator.itemName == item.text     );
            BOOST_ASSERT( menuIterator.itemId   == item.itemID   );
            BOOST_ASSERT( menuIterator.isTicked == item.isTicked );
        #endif // _DEBUG

        return item;
    }

    static MenuItemInfo const & getItemInfo( juce::PopupMenu const & menu, unsigned int const itemIndex )
    {
        return getItemInfo( const_cast<juce::PopupMenu &>( menu ), itemIndex );
    }

    static unsigned int getItemIndexForItemID( juce::PopupMenu & menu, int const itemID )
    {
        for ( unsigned int index( 0 ); index < getNumberOfItems( menu ); ++index )
        {
            if ( getItemInfo( menu, index ).itemID == itemID )
                return index;
        }
        LE_UNREACHABLE_CODE();
    }
}


bool PopupMenu::menuActive_( false );

LE_NOTHROW
PopupMenu::PopupMenu()
    :
    menuHeight_( 0 ),
    menuWidth_ ( 0 )
{
}


void PopupMenu::addItem( ItemID const newItemId, char const * const newItemText, juce::Image const & icon, bool const enabled )
{
    juce::String const text( newItemText );
    updateDimensionsForNewItem( text );
    menu_.addItem( mangleID( newItemId ), text, enabled, false, icon );
}


void PopupMenu::addSubMenu( PopupMenu & subMenu, char const * const name )
{
    BOOST_ASSERT( name );
    juce::String const text( name );
    updateDimensionsForNewItem( text );
    menu_.addSubMenu( text, subMenu.menu_, true );
}


void PopupMenu::addSectionHeader( char const * const title )
{
    BOOST_ASSERT( title );
    juce::String const text( title );
    updateDimensionsForNewItem( text );
    menu_.addSectionHeader    ( text );
}


void PopupMenu::updateDimensionsForNewItem( juce::String const & itemText )
{
    //...mrmlj...menuWidth_ = std::max( menuWidth_, Theme::singleton().getPopupMenuFont().getStringWidth( text ) );
    int idealWidth, idealHeight;
    Theme::singleton().Theme::getIdealPopupMenuItemSize( itemText, false, -1, idealWidth, idealHeight );
    menuWidth_   = std::max<unsigned int>( menuWidth_, idealWidth + 4 );
    menuHeight_ += idealHeight;
}


PopupMenu::OptionalID PopupMenu::showCenteredAtRight( juce::Component const & owner ) const
{
    juce::Point<int> const ownerPosition( owner.getScreenPosition() );
    unsigned int const ownerRight         ( ownerPosition.getX() +   owner.getWidth ()       );
    unsigned int const ownerVerticalMiddle( ownerPosition.getY() + ( owner.getHeight() / 2 ) );
    return showAt
    (
        ownerRight + 6,
        ownerVerticalMiddle - ( menuHeight_ / 2 ),
        1, 1 //...mrmlj...required with latest juce to actually get the menu on the right side...
    );
}


PopupMenu::OptionalID PopupMenu::showCenteredBelow( juce::Component const & owner ) const
{
    juce::Point<int> point( owner.localPointToGlobal( juce::Point<int>() ) );

    unsigned int const width( owner.getWidth() );
    if ( menuWidth_ > static_cast<unsigned int>( width ) )
    {
        point.setX( point.getX() - ( ( menuWidth_ - width ) / 2 ) );
    }

    return showAt( point.getX(), point.getY(), width, owner.getHeight() );
}


PopupMenu::OptionalID PopupMenu::showAt( unsigned int x, unsigned int const y, unsigned int const width, unsigned int const height ) const
{
    menuActive_ = true;
    //...mrmlj...NEW JUCE juce::Component::leHack_modalComponentsShouldFocusBackToPreviouslyFocusedComponent = false;
    MangledID const chosenMenuEntryID
    (
        const_cast<juce::PopupMenu &>( menu_ ).showMenu
        (
            juce::PopupMenu::Options().withTargetScreenArea( juce::Rectangle<int>( x, y, width, height ) )
                                      .withMinimumWidth    ( width                                       )
        )
    );
    //...mrmlj...NEW JUCE juce::Component::leHack_modalComponentsShouldFocusBackToPreviouslyFocusedComponent = true;
    menuActive_ = false;
    /// \note juce::PopupMenu uses zero to indicate that the user dismissed the
    /// menu/didn't click an item and therefor disallows the use of 0 as an item
    /// ID. Since we want to support 0 as a legal item ID, we work around this
    /// by '(un)mangling' the ID when crossing the JUCE API boundary.
    ///                                       (13.02.2014.) (Domagoj Saric)
    if ( chosenMenuEntryID )
        return unmangleID( chosenMenuEntryID );
    else
        return boost::none;
}


void PopupMenu::clear()
{
    menu_.clear();
    menuHeight_ = 0;
    menuWidth_  = 0;
}


unsigned int PopupMenu::numberOfItems() const
{
    return JuceHackery::getNumberOfItems( menu_ );
}


PopupMenu::MangledID PopupMenu::mangleID( ItemID const id )
{
    BOOST_ASSERT( id < zeroIDMaskWorkaround );
    return id | zeroIDMaskWorkaround;
}


PopupMenu::ItemID PopupMenu::unmangleID( MangledID const mangledID )
{
    return mangledID & ( ~zeroIDMaskWorkaround );
}


PopupMenuWithSelection::PopupMenuWithSelection()
    :
    currentSelection_  ( 0 ),
    currentSelectionID_( 0 )
{
}


unsigned int PopupMenuWithSelection::getSelectedIndex() const
{
    BOOST_ASSERT( hasValidSelection() );
    return currentSelection_;
}


void PopupMenuWithSelection::setSelectedIndex( unsigned int const newSelectionIndex )
{
    updateSelection( newSelectionIndex );
    currentSelectionID_ = JuceHackery::getItemInfo( menu_, newSelectionIndex ).itemID;
}


unsigned int PopupMenuWithSelection::getSelectedID() const
{
    BOOST_ASSERT( hasValidSelection() );
    return unmangleID( currentSelectionID_ );
}


void PopupMenuWithSelection::setSelectedID( unsigned int const newSelectionID )
{
    currentSelectionID_ = mangleID( newSelectionID );
    updateSelection( JuceHackery::getItemIndexForItemID( menu_, currentSelectionID_ ) );
}


juce::String const & PopupMenuWithSelection::getSelectedItemText() const
{
    return getItemText( currentSelection_ );
}


juce::Image const & PopupMenuWithSelection::getSelectedItemIcon() const
{
    return JuceHackery::getItemInfo( menu_, currentSelection_ ).image;
}


void PopupMenuWithSelection::updateSelection( unsigned int const newSelectionIndex )
{
    JuceHackery::getItemInfo( menu_, currentSelection_ ).isTicked = false;
    JuceHackery::getItemInfo( menu_, newSelectionIndex ).isTicked = true ;
    currentSelection_ = newSelectionIndex;
}


void PopupMenuWithSelection::clear()
{
    PopupMenu::clear();
    currentSelection_   = 0;
    currentSelectionID_ = 0;
}


bool PopupMenuWithSelection::hasValidSelection() const
{
    return ( currentSelectionID_ != 0 ) && static_cast<std::size_t>( currentSelection_ ) < numberOfItems();
}


bool PopupMenuWithSelection::handleNewSelection( OptionalID const & chosenMenuEntryID )
{
    if ( chosenMenuEntryID.is_initialized() )
    {
        currentSelectionID_ = mangleID( *chosenMenuEntryID );
        updateSelection( JuceHackery::getItemIndexForItemID( menu_, currentSelectionID_ ) );
        return true;
    }
    else
        return false;
}


bool PopupMenuWithSelection::showCenteredAtRight( juce::Component const & owner )
{
    return handleNewSelection( PopupMenu::showCenteredAtRight( owner ) );

}


bool PopupMenuWithSelection::showCenteredBelow( juce::Component const & owner )
{
    return handleNewSelection( PopupMenu::showCenteredBelow( owner ) );
}


juce::String const & PopupMenuWithSelection::getItemText( unsigned int const itemIndex ) const
{
    return JuceHackery::getItemInfo( menu_, itemIndex ).text;
}


ComboBox::ComboBox
(
    juce::Component   & parent,
    juce::Image const & normalBackground,
    juce::Image const & selectedBackground
)
    :
    normalBackground_  ( normalBackground   ),
    selectedBackground_( selectedBackground )
{
    BOOST_ASSERT( normalBackground.getWidth () == selectedBackground.getWidth () );
    BOOST_ASSERT( normalBackground.getHeight() == selectedBackground.getHeight() );

    setSizeFromImage( *this, normalBackground );
    addToParentAndShow( parent, *this );
}


void ComboBox::paint( juce::Graphics & graphics )
{
    paintImage( graphics, hasDirectFocus() ? selectedBackground_ : normalBackground_ );

    graphics.setColour( juce::Colours::white           );
    graphics.setFont  ( Theme::singleton().whiteFont() );
    graphics.drawFittedText
    (
        getSelectedItemText(),
                     4,                                 2,
        getWidth() - 8, normalBackground_.getHeight() - 3,
        juce::Justification::centred,
        1,
        0.1f
    );
}


bool ComboBox::showMenu()
{
    //...mrmlj...temporary workaround for the temporary zero padding workaround...
    if ( !isEnabled() )
        return false;

    if ( menuActive() )
        return false;

    if ( !showCenteredBelow( *this ) )
        return false;

    grabKeyboardFocus();
    repaint          ();
    return true;
}


LE_NOINLINE
void ComboBox::setSelectedID( unsigned int const newSelectionID )
{
    PopupMenuWithSelection::setSelectedID( newSelectionID );
    repaint();
}

void ComboBox::setSelectedIndex( unsigned int const newSelectionIndex )
{
    PopupMenuWithSelection::setSelectedIndex( newSelectionIndex );
    repaint();
}


void Detail::paintTextButton
(
    BitmapButton  const & button,
    juce::Graphics      & g,
    unsigned int const textX , unsigned int const textY ,
    unsigned int const imageX, unsigned int const imageY,
    bool const isMouseOverButton,
    bool const isButtonDown
)
{
    g.setColour( juce::Colours::white        );
    g.setFont  ( DrawableText::defaultFont() );
    juce::Image const & currentImage( button.getCurrentImage() );
    g.drawFittedText
    (
        button.getName(),
        textX,
        textY,
        button.getWidth() - textX,
        11,
        juce::Justification::horizontallyCentred,
        1
    );

    Theme::singleton().Theme::drawImageButton
    (
        g,
        const_cast<juce::Image *>( &currentImage ),
        imageX, imageY,
        currentImage.getWidth(), currentImage.getHeight(),
        isButtonDown
            ? BitmapButton::downOverlay()
            : ( isMouseOverButton
                    ? BitmapButton::defaultOverOverlay()
                    : BitmapButton::normalOverlay()
              ),
        isButtonDown
            ? BitmapButton::downOpacity()
            : ( isMouseOverButton
                    ? BitmapButton::overOpacity()
                    : BitmapButton::normalOpacity()
              ),
        const_cast<BitmapButton &>( button )
    );
}


LEDTextButton::LEDTextButton
(
    juce::Component & parent,
    unsigned int const x,
    unsigned int const y,
    char const * const text
)
    :
    BitmapButton( parent, resourceBitmap<LEDOn>(), resourceBitmap<LEDOff>() )
{
    setName( text );

    setBounds
    (
        x, y,
        getWidth() + DrawableText::defaultFont().getStringWidth( getName() ), 14
    );
}


void LEDTextButton::paintButton( juce::Graphics & g, bool const isMouseOverButton, bool const isButtonDown )
{
    std::size_t const imageWidth( 25 );
    BOOST_ASSERT( getCurrentImage().getWidth() == imageWidth );
    Detail::paintTextButton( *this, g, imageWidth, 3, 0, 0, isMouseOverButton, isButtonDown );
}


TextButton::TextButton
(
    juce::Component & parent,
    unsigned int const x,
    unsigned int const y,
    char const * const text
)
{
    setName( text );

    juce::Font font( Theme::singleton().whiteFont() );
    font.setHeight( static_cast<float>( height ) );

    setBounds
    (
        x, y,
        font.getStringWidth( getName() ), height
    );

    setClickingTogglesState( true );

    addToParentAndShow( parent, *this );
}


void TextButton::paintButton( juce::Graphics & g, bool const isMouseOverButton, bool /*isButtonDown*/ )
{
    #define INTEGER_ALPHA( alpha ) static_cast<unsigned char>( alpha * 255 )
    static unsigned char const alphas[ 2 ][ 2 ] = /* [not toggled, toggled] [not mouse over, mouse over] */
    {
        { INTEGER_ALPHA( 0.3 ), INTEGER_ALPHA( 0.6 ) },
        { INTEGER_ALPHA( 1.0 ), INTEGER_ALPHA( 0.8 ) }
    };
    #undef INTEGER_ALPHA

    unsigned char const alpha( alphas[ getToggleState() ][ isMouseOverButton ] );

    juce::Font font( Theme::singleton().whiteFont() );
    font.setHeight( static_cast<float>( height ) );

    g.setColour( juce::Colour( ( Theme::blueColour().getARGB() & 0x00FFFFFF ) | ( alpha << 24 ) ) );
    g.setFont  ( font                                                                             );
    g.drawSingleLineText( getName(), 0, height );
}


Knob::Knob
(
    juce::Component & parent  ,
    unsigned int const x      ,
    unsigned int const y      ,
    unsigned int const xMargin,
    unsigned int const yMargin
)
{
    /// \note See the note in the BitmapButton constructor.
    ///                                       (20.03.2013.) (Domagoj Saric)
    removeValueListeners( *this, valueListener() );

    setBounds
    (
        x      , y,
        xMargin, yMargin
    );
  //setTooltip             ( title                 );
    setSliderStyle         ( RotaryVerticalDrag    );
    setTextBoxStyle        ( NoTextBox, true, 0, 0 );
  //setPopupDisplayEnabled ( true, 0               ); //...mrmlj...for testing...
    setPopupMenuEnabled    ( true                  );
    setMouseDragSensitivity( 800                   );
    addToParentAndShow     ( parent, *this         );
}


void Knob::setupForParameter
(
    char        const * const title             ,
    juce::Image const &       filmStripToSizeFor,
    param_type          const defaultValue
)
{
    setName( title );

    unsigned int const imageWidth ( filmStripToSizeFor.getWidth ()                          );
    unsigned int const imageHeight( filmStripToSizeFor.getHeight() / numberOfKnobSubbitmaps );
    BOOST_ASSERT( ( filmStripToSizeFor.getHeight() % numberOfKnobSubbitmaps == 0 ) );
    LE_ASSUME( imageWidth == imageHeight );

    unsigned int const xMargin( getWidth () );
    unsigned int const yMargin( getHeight() );

    unsigned int const width ( imageWidth  + xMargin );
    unsigned int const height( imageHeight + yMargin );
    setSize( width, height );

    setDoubleClickReturnValue( true, defaultValue );
}


void Knob::startedDragging() noexcept
{
    if ( !Theme::singleton().settings().hideCursorOnKnobDrag )
        return;

    BOOST_ASSERT( juce::Desktop::getInstance().getNumMouseSources() == 1 );
    juce::MouseInputSource & mouseSource( juce::Desktop::getInstance().getMainMouseSource() );
    BOOST_ASSERT
    (
        ( juce::Desktop::getInstance().getDraggingMouseSource( 0 ) == nullptr      ) || //...mrmlj...double click...
        ( juce::Desktop::getInstance().getDraggingMouseSource( 0 ) == &mouseSource )
    );

    /// \note setMouseCursor( juce::MouseCursor::NoCursor ) and
    /// enableUnboundedMouseMovement() result in a black box under VMWare.
    ///                                       (10.07.2012.) (Domagoj Saric)
    mouseSource.enableUnboundedMouseMovement( true, false );
    BOOST_ASSERT( mouseSource.canDoUnboundedMovement() );
}


#ifndef NDEBUG
void Knob::stoppedDragging() noexcept
{
    BOOST_ASSERT( juce::Desktop::getInstance().getNumMouseSources() == 1 );
    //juce::MouseInputSource & mouseSource( juce::Desktop::getInstance().getMainMouseSource() );
    // http://www.rawmaterialsoftware.com/viewtopic.php?f=2&t=5628&hilit=enableUnboundedMouseMovement
    //mouseSource.enableUnboundedMouseMovement( false, !Theme::singleton().settings().hideCursorOnKnobDrag );

    //...mrmlj...neither of these works/helps because
    //...mrmlj...enableUnboundedMouseMovement() seems to handle it
    //...mrmlj...automatically (but imprecisely)...
    //juce::Desktop::setMousePosition( juce::Desktop::getLastMouseDownPosition() );
    //juce::Desktop::setMousePosition( this->localPointToGlobal( this->getBounds().getCentre() ) );
}
#endif // NDEBUG


LE_NOTHROW
void Knob::removeValueListeners( juce::Slider & slider, juce::ValueListener & valueListener )
{
    //...mrmlj...Slider::valueListener() is protected so we cannot access it here...
    //juce::ValueListener & valueListener( slider.valueListener() );
    LE_ASSUME( &valueListener );
    slider.getValueObject   ().removeListener( &valueListener );
    slider.getMinValueObject().removeListener( &valueListener );
    slider.getMaxValueObject().removeListener( &valueListener );
}


void LE_NOINLINE LE_NOTHROWNOALIAS Knob::setValue( param_type const newValue )
{
#ifndef NDEBUG
    #if LE_SW_SEPARATED_DSP_GUI
    /// \note Skip the check in case the module GUI has not yet been attached to
    /// its parent/editor (e.g. when setting values during preset loading right
    /// after the module has been created).
    /// See the related note for the ModuleUI::baseParameters_ data member.
    ///                                       (20.10.2014.) (Domagoj Saric)
    if ( getParentComponent()->getParentComponent() != nullptr )
    #endif // LE_SW_SEPARATED_DSP_GUI
    {
        // Implementation note:
        //   A simple
        // BOOST_ASSERT( Math::isValueInRange( static_cast<value_type>( newValue ), getMinimum(), getMaximum() ) );
        // assertion would sometimes falsely fail for knobs with
        // quantization-adjusted ranges.
        //                                    (05.05.2011.) (Domagoj Saric)
        Engine::Setup const & engineSetup( SpectrumWorxEditor::fromChild( *this ).engineSetup() );
        Knob::value_type const maxQuantizationAdjustment
        (
            std::max
            (
                engineSetup.frequencyRangePerBin<Knob::param_type>(),
                engineSetup.stepTime() * 1000
            )
        );
        auto const minimum( getMinimum() );
        auto const maximum( getMaximum() );
        BOOST_ASSERT_MSG
        (
            Math::isValueInRange
            (
                static_cast<value_type>( newValue ),
                minimum - maxQuantizationAdjustment,
                maximum + maxQuantizationAdjustment
            ),
            "Knob value out of range"
        );
    }
#endif // NDEBUG
    juce::Slider::setValue( static_cast<value_type>( newValue ), juce::dontSendNotification );
}


void Knob::paint
(
    juce::Image    const &       filmStrip,
    unsigned int           const xMargin,
    unsigned int           const yMargin,
    juce::Graphics       &       graphics
)
{
    BOOST_ASSERT( filmStrip.getWidth() == filmStrip.getHeight() / signed( numberOfKnobSubbitmaps ) );

    unsigned int const imageWidth   ( filmStrip.getWidth() );
    unsigned int const imageHeight  ( imageWidth           );
    unsigned int const pictureIndex ( Math::convert<unsigned int>( ( numberOfKnobSubbitmaps - 1 ) * juce::Slider::valueToProportionOfLength( getValue() ) ) );
    unsigned int const pictureOffset( pictureIndex * imageHeight );
    BOOST_ASSERT( pictureIndex  < numberOfKnobSubbitmaps                             );
    BOOST_ASSERT( pictureOffset < static_cast<unsigned int>( filmStrip.getHeight() ) );

    graphics.drawImage
    (
        filmStrip,
        xMargin, yMargin      , imageWidth, imageHeight,
        0      , pictureOffset, imageWidth, imageHeight
    );
}


Knob::param_type Knob::getNormalisedValue() const
{
    Knob::param_type const fullRangeValue( static_cast<param_type>( getValue  () ) );
    Knob::param_type const minimumValue  ( static_cast<param_type>( getMinimum() ) );
    Knob::param_type const maximumValue  ( static_cast<param_type>( getMaximum() ) );

    return Math::convertLinearRange<Knob::param_type, 0, 1, 1, Knob::param_type>( fullRangeValue, minimumValue, maximumValue );
}


EditorKnob::EditorKnob
(
    SpectrumWorxEditor &       parent,
    unsigned int         const x     ,
    unsigned int         const y
)
    :
    Knob           ( parent, x, y, 0, 0 ),
    parameterIndex_( 0                  )
{
    setScrollWheelEnabled          ( true  );
    setWantsKeyboardFocus          ( false );
    setMouseClickGrabsKeyboardFocus( false );
}


void EditorKnob::setupForParameter
(
    std::uint8_t const parameterIndex,
    param_type   const minimumValue  ,
    param_type   const maximumValue  ,
    param_type   const defaultValue
)
{
    Knob::setupForParameter
    (
        nullptr,
        resourceBitmap<EditorKnobStrip>(),
        defaultValue
    );

    parameterIndex_ = parameterIndex;

    setRange( minimumValue, maximumValue, 0 );
}


namespace
{
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    struct ParameterPrinter
    {
        typedef char const * result_type;
        template <class Parameter> result_type operator()() const
        {
            return LE::Parameters::print<Parameter>( value, engineSetup, buffer );
        }
        Engine::Setup               const &       engineSetup;
        float                               const value      ;
        LE::Parameters::PrintBuffer         const buffer     ;
    }; // struct ParameterPrinter
    #pragma warning( pop )
}

void EditorKnob::paint( juce::Graphics & graphics )
{
    Knob::paint( resourceBitmap<EditorKnobStrip>(), 0, 0, graphics );

    // For main knobs we display the value within the knob itself.
    BOOST_ASSERT( resourceBitmap<EditorKnobStrip>().getWidth() == 55 );
    graphics.setColour( juce::Colours::white );
    {
        juce::Font font( Theme::singleton().whiteFont() );
        font.setHeight( 11 );
        graphics.setFont( font );
    }

    //...mrmlj...ugh...
    std::array<char, 20> valueString;
    ParameterPrinter const printer = { editor().engineSetup(), static_cast<float>( getValue() ), boost::make_iterator_range_n( &valueString[ 0 ], valueString.size() ) };
    using LE::Parameters::IndexOf;
    using namespace GlobalParameters;
    typedef GlobalParameters::Parameters GlobalParams;
    switch ( parameterIndex_ )
    {
        case IndexOf<GlobalParams, InputGain    >::value: printer.operator()<InputGain    >(); break;
        case IndexOf<GlobalParams, OutputGain   >::value: printer.operator()<OutputGain   >(); break;
        case IndexOf<GlobalParams, MixPercentage>::value: printer.operator()<MixPercentage>(); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    //...mrmlj...assumes global parameters are static...
    ParameterID::Global parameterID; parameterID.index = parameterIndex_;
    char const * const pUnit( Plugin2HostPassiveInteropController::ParameterLabelGetter()( parameterID, nullptr ) );

    graphics.drawFittedText
    (
        juce::String( &valueString[ 0 ] ) + pUnit,
        14, 16,
        28, 24,
        juce::Justification::centred,
        1,
        0.1f
    );
}


LE_NOTHROW void EditorKnob::valueChanged() noexcept
{
    using LE::Parameters::IndexOf;
    using namespace GlobalParameters;
    typedef GlobalParameters::Parameters GlobalParams;
    auto       & editor( this->editor  () );
    auto const & value ( this->getValue() );
    switch ( parameterIndex_ )
    {
        case IndexOf<GlobalParams, InputGain    >::value: BOOST_VERIFY( editor.globalParameterChanged<InputGain    >( value, false ) ); break;
        case IndexOf<GlobalParams, OutputGain   >::value: BOOST_VERIFY( editor.globalParameterChanged<OutputGain   >( value, false ) ); break;
        case IndexOf<GlobalParams, MixPercentage>::value: BOOST_VERIFY( editor.globalParameterChanged<MixPercentage>( value, false ) ); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

void EditorKnob::startedDragging() noexcept
{
    Knob::startedDragging();
    editor().mainKnobDragStarted( parameterIndex_ );
}

void EditorKnob::stoppedDragging() noexcept
{
    editor().mainKnobDragStopped( parameterIndex_ );
    Knob::stoppedDragging();
}


SpectrumWorxEditor & EditorKnob::editor() { return *boost::polymorphic_downcast<SpectrumWorxEditor *>( this->getParentComponent() ); }


TitledComboBox::TitledComboBox
(
    juce::Component       &       parent,
    unsigned int            const x     ,
    unsigned int            const y     ,
    char            const * const title
)
    :
    ComboBox( parent, resourceBitmap<SettingsCombo>(), resourceBitmap<SettingsComboOn>() ),
    title_  ( title, 4, 0, getWidth() - 8, 13, juce::Justification::left )
{
    TitledComboBox::setBounds
    (
        x, y,
        getWidth(), getHeight() + 15
    );
}


void TitledComboBox::paint( juce::Graphics & graphics )
{
    if ( !hasValidSelection() )
        return;
    graphics.setOrigin( 0, +12 );
    ComboBox::paint( graphics );
    graphics.setOrigin( 0, -12 );
    title_.draw( graphics );
}


void TitledComboBox::mouseDown( juce::MouseEvent const & )
{
    bool const valueChanged( ComboBox::showMenu() );
    if ( valueChanged )
        //...mrmlj...move...editor/settings specific...
        SpectrumWorxEditor::Settings::comboBoxValueChanged( *this );
}


namespace Detail
{
    void addPowerOfTwoValueStringsToComboBox
    (
        unsigned int const firstValue,
        unsigned int const lastValue,
        ComboBox & comboBox
    )
    {
        BOOST_ASSERT_MSG( comboBox.numberOfItems() == 0, "ComboBox already filled." );
        std::array<char, 20> buffer;
        unsigned int value( firstValue );
        while ( value <= lastValue )
        {
            Utility::lexical_cast( value, &buffer[ 0 ] );
            comboBox.addItem
            (
                value,
                &buffer[ 0 ]
            );
            value *= 2;
        }
        comboBox.setValue( firstValue );
    }

    void addEnumeratedParameterValueStringsToComboBox
    (
        boost::iterator_range<char const * LE_RESTRICT const *>   strings,
        ComboBox                                                & comboBox
    )
    {
        BOOST_ASSERT_MSG( comboBox.numberOfItems() == 0, "ComboBox already filled." );
        ComboBox::value_type parameterValue( 0 );
        while ( strings )
        {
            comboBox.addItem
            (
                parameterValue,
                strings.front()
            );
            ++parameterValue;
            strings.advance_begin( 1 );
        }
        comboBox.setValue( 0 );
    }
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
// Theme
////////////////////////////////////////////////////////////////////////////////

namespace
{
    void registerFonts( bool const doRegister )
    {
        /// \todo JUCE does not provide a way to load fonts from files/not
        /// registered in the system so we load and register them manually here.
        /// Find a better cross-platform solution.
        /// Check the following discussions:
        /// http://www.rawmaterialsoftware.com/viewtopic.php?f=2&t=2678&hilit=ttf
        /// http://www.rawmaterialsoftware.com/viewtopic.php?f=6&t=54&start=30
        ///                                       (09.02.2010.) (Domagoj Saric)

        static char const * const fonts[] = { "Vera.ttf", "VeraBd.ttf" };

        juce::File const resources( resourcesPath() );

        for ( auto const font : fonts )
        {
            juce::File const fontPath( resources.getChildFile( font ) );
        #ifdef _WIN32
            if ( doRegister ) BOOST_VERIFY( ::AddFontResourceEx   ( fontPath.getFullPathName().getCharPointer(), FR_PRIVATE, 0 ) > 0 );
            else              BOOST_VERIFY( ::RemoveFontResourceEx( fontPath.getFullPathName().getCharPointer(), FR_PRIVATE, 0 ) > 0 );
        #else // OSX
            // http://developer.apple.com/legacy/mac/library/#documentation/Carbon/Reference/ATS/Reference/reference.html
            // http://blogs.msdn.com/b/michkap/archive/2006/06/25/646701.aspx
            ::CFURLRef const fontURL( makeCFURLFromPath( fontPath ) );
            if ( doRegister ) BOOST_VERIFY( ::CTFontManagerRegisterFontsForURL  ( fontURL, kCTFontManagerScopeProcess, nullptr ) );
            else              BOOST_VERIFY( ::CTFontManagerUnregisterFontsForURL( fontURL, kCTFontManagerScopeProcess, nullptr ) );
            if ( fontURL ) ::CFRelease( fontURL );
        #endif // OS
        }
    }
} // anonymous namespace

Theme::Settings Theme::settings_;

Theme::Theme()
    :
    blueFont_ ( "Bitstream Vera Sans"      , 14, juce::Font::bold ),
    whiteFont_( blueFont_.getTypefaceName(), 12, juce::Font::bold )
{
#ifdef LE_SW_FMOD //...mrmlj...
    BOOST_VERIFY( initializePaths() );
#endif // LE_SW_FMOD
    juce::LookAndFeel::setDefaultSansSerifTypefaceName( blueFont_.getTypefaceName() );

    registerFonts( true );

    setColour( juce::PopupMenu::backgroundColourId           , juce::Colours::black                                       );
    setColour( juce::PopupMenu::textColourId                 , juce::Colours::lightgrey                                   );
    setColour( juce::PopupMenu::headerTextColourId           , juce::Colours::white                                       );
    setColour( juce::PopupMenu::highlightedBackgroundColourId, juce::Colours::transparentWhite                            );
    setColour( juce::PopupMenu::highlightedTextColourId      , juce::Colours::white                                       );
    setColour( juce::PopupMenu::backgroundColourId           , juce::Colours::black.withAlpha( settings().globalOpacity ) );

  //setColour( juce::Slider::trackColourId              , juce::Colours::aliceblue );
  //setColour( juce::Slider::rotarySliderFillColourId   , juce::Colours::aqua      );
  //setColour( juce::Slider::rotarySliderOutlineColourId, juce::Colours::white     );
  //setColour( juce::Slider::thumbColourId, blueColour()         );
  //setColour( juce::Slider::trackColourId, juce::Colours::white );

    setColour( juce::TextButton::buttonColourId  , juce::Colours::black );
    setColour( juce::TextButton::buttonOnColourId, blueColour()         );
    setColour( juce::TextButton::textColourOnId  , juce::Colours::white );
    setColour( juce::TextButton::textColourOffId , juce::Colours::white );

  //setColour( juce::ToggleButton::textColourId, juce::Colours::white );

    setColour( juce::AlertWindow::backgroundColourId, juce::Colours::darkgrey );
    setColour( juce::AlertWindow::textColourId      , juce::Colours::white    );

    setColour( juce::ComboBox::backgroundColourId, juce::Colours::black     );
    setColour( juce::ComboBox::buttonColourId    , juce::Colours::lightgrey );
    setColour( juce::ComboBox::textColourId      , juce::Colours::white     );

    setColour( juce::Label::backgroundColourId, juce::Colours::transparentWhite );
    setColour( juce::Label::outlineColourId   , juce::Colours::transparentWhite );
    setColour( juce::Label::textColourId      , juce::Colours::white            );

    setColour( juce::TextEditor::backgroundColourId     , juce::Colour( 0x88000000 )      );
    setColour( juce::TextEditor::focusedOutlineColourId , juce::Colours::transparentBlack );
    setColour( juce::TextEditor::outlineColourId        , juce::Colours::transparentBlack );
    setColour( juce::TextEditor::highlightColourId      , blueColour()                    );
    setColour( juce::TextEditor::highlightedTextColourId, juce::Colours::white            );
    setColour( juce::TextEditor::textColourId           , juce::Colours::white            );
    setColour( juce::CaretComponent::caretColourId      , blueColour()                    );

    setColour( juce::ListBox::backgroundColourId, juce::Colour( 0x11000000 ) );
    setColour( juce::ListBox::outlineColourId   , juce::Colour( 0xAA000000 ) );
    setColour( juce::ListBox::textColourId      , juce::Colours::white       );

    setColour( juce::DirectoryContentsDisplayComponent::highlightColourId, juce::Colour( 0x88ffffff ) );
    setColour( juce::DirectoryContentsDisplayComponent::textColourId     , juce::Colours::white       );

  //setColour( juce::GroupComponent::textColourId   , juce::Colours::white     );
  //setColour( juce::GroupComponent::outlineColourId, juce::Colours::lightgrey );

    setColour( juce::TabbedButtonBar::tabOutlineColourId  , juce::Colours::transparentBlack );
    setColour( juce::TabbedButtonBar::tabTextColourId     , juce::Colours::transparentBlack );
    setColour( juce::TabbedButtonBar::frontOutlineColourId, juce::Colours::transparentBlack );
    setColour( juce::TabbedButtonBar::frontTextColourId   , juce::Colours::transparentBlack );

    setColour( juce::TabbedComponent::backgroundColourId, juce::Colours::transparentBlack );
    setColour( juce::TabbedComponent::outlineColourId   , juce::Colours::transparentBlack );

    setColour( juce::ScrollBar::trackColourId, juce::Colours::lightgrey );
}


Theme::~Theme()
{
    registerFonts( false );
}


bool Theme::shouldUpdateLFOControl( ModuleControlBase const & control )
{
    Theme::LFOUpdateBehaviour const lfoUpdateBehaviour( singleton().settings().lfoUpdateBehaviour );
    return
        ( lfoUpdateBehaviour == Always                                                            ) ||
        ( lfoUpdateBehaviour == WhenControlActive   && control.isActive()                         ) ||
        ( lfoUpdateBehaviour == WhenControlSelected && Detail::hasDirectFocus( control.widget() ) );
}


bool Theme::aModuleControlNeedsLFOUpdate( ModuleUI const & moduleUI )
{
    Theme::LFOUpdateBehaviour const lfoUpdateBehaviour( singleton().settings().lfoUpdateBehaviour );
    ModuleControlBase const * const pActiveControl( ModuleControlBase::activeControl() );
    return
        ( lfoUpdateBehaviour == Always ) ||
        (
            ( pActiveControl && ( &pActiveControl->moduleUI() == &moduleUI ) ) &&
            (
                ( lfoUpdateBehaviour == WhenControlActive                                                         ) ||
                ( lfoUpdateBehaviour == WhenControlSelected && Detail::hasDirectFocus( pActiveControl->widget() ) )
            )
        );
}


void Theme::drawPopupMenuBackground( juce::Graphics & g, int const width, int const height )
{
    g.setOpacity( std::pow( Theme::singleton().settings().globalOpacity, 3 ) );
    g.fillRoundedRectangle( 0, 0, Math::convert<float>( width ), Math::convert<float>( height ), 12 );
}


void Theme::drawTabAreaBehindFrontButton( juce::TabbedButtonBar &, juce::Graphics &, int /*w*/, int /*h*/ )
{
    // Implementation note:
    //   This is necessary to prevent a line being drawn between the tab buttons
    // and the page(s) in our Settings windows which causes an ugly dot to
    // appear.
    //                                        (16.06.2010.) (Domagoj Saric)
}


juce::Image Theme::getDefaultFolderImage()
{
    if ( Theme::singleton().folderIcon_.isNull() )
    {
        Theme::singleton().folderIcon_ = juce::LookAndFeel::getDefaultFolderImage();
        Theme::singleton().folderIcon_.juce::Image::desaturate();
    }
    return Theme::singleton().folderIcon_;
}


int Theme::getMenuWindowFlags()
{
    // Implementation note:
    //   The original enabled a drop shadow.
    //                                        (11.05.2010.) (Domagoj Saric)
    return 0;
}


juce::Font Theme::getPopupMenuFont()
{
    juce::Font font( whiteFont() );
    font.setBold( false );
    return font;
}


void Theme::drawLinearSliderBackground( juce::Graphics & g, int const x, int const y, int const width, int const height, float /*sliderPos*/, float /*minSliderPos*/, float /*maxSliderPos*/, juce::Slider::SliderStyle const style, juce::Slider & /*slider*/ )
{
    BOOST_VERIFY( ( style == juce::Slider::LinearHorizontal ) || ( style == juce::Slider::TwoValueHorizontal ) );
    g.setColour( juce::Colours::white );
    // Implementation note:
    //   MSVC 9.0 SP1 inserts unnecessary cdq and sub instructions without the
    // unsigned int static_cast.
    //                                        (27.05.2010.) (Domagoj Saric)
    g.drawHorizontalLine( y + ( static_cast<unsigned int>( height ) / 2 ), Math::convert<float>( x ), Math::convert<float>( x + width ) );
}

namespace
{
    void paintSliderThumb
    (
        juce::Graphics       & graphics,
        juce::Image    const & image,
        float          const & position,
        unsigned int   const   sliderVerticalPosition,
        unsigned int   const   sliderHeight,
        bool           const   enlarge
    )
    {
        unsigned int const thumbWidth ( image.getWidth () );
        unsigned int const thumbHeight( image.getHeight() );

        unsigned int const scaleNumerator  ( enlarge ? 5 : 1 );
        unsigned int const scaleDenominator( enlarge ? 3 : 1 );

        unsigned int const scaledThumbWidth ( thumbWidth  * scaleNumerator / scaleDenominator );
        unsigned int const scaledThumbHeight( thumbHeight * scaleNumerator / scaleDenominator );

        unsigned int const horizontalPosition( Math::convert<unsigned int>( position )       - ( scaledThumbWidth  / 2 ) );
        unsigned int const verticalPosition  ( sliderVerticalPosition + ( sliderHeight / 2 ) - ( scaledThumbHeight / 2 ) );

        graphics.drawImage
        (
            image,
            horizontalPosition, verticalPosition, scaledThumbWidth, scaledThumbHeight,
            0                 , 0               , thumbWidth      , thumbHeight
        );
    }
} // anonymous namespace

void Theme::drawLinearSliderThumb
(
    juce::Graphics            &       g,
    int                         const /*x*/,
    int                         const y,
    int                         const /*width*/,
    int                         const height,
    float                       const sliderPos,
    float                       const minSliderPos,
    float                       const maxSliderPos,
    juce::Slider::SliderStyle   const style,
    juce::Slider              &       slider
)
{
    juce::Image const & thumb( resourceBitmap<LFOSliderThumb>() );

    switch ( style )
    {
        case juce::Slider::LinearHorizontal:
            paintSliderThumb( g, thumb, sliderPos, y, height, slider.getThumbBeingDragged() == 0 );
            break;
        case juce::Slider::TwoValueHorizontal:
        {
            paintSliderThumb( g, thumb, minSliderPos, y, height, slider.getThumbBeingDragged() == 1 );
            paintSliderThumb( g, thumb, maxSliderPos, y, height, slider.getThumbBeingDragged() == 2 );
            break;
        }

        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


int Theme::getSliderThumbRadius( juce::Slider & )
{
    return resourceBitmap<LFOSliderThumb>().getWidth() / 2;
}


Theme::Settings::Settings()
    :
    globalOpacity            ( 0.9f   ),
    moduleUIMouseOverReaction( Never  ),
    lfoUpdateBehaviour       ( Always ),
    hideCursorOnKnobDrag     ( true   )
{
}


static boost::optional<Theme> singleton_;

void Theme:: createSingleton() { singleton_ = boost::in_place(); }
void Theme::destroySingleton() { singleton_ = boost::none      ; }

Theme & Theme::singleton() { return *singleton_; }


namespace
{
    void onGUIInitialization()
    {
        Theme::createSingleton();
    }

    void onGUIShutdown()
    {
        Theme::destroySingleton();
    }
}

//------------------------------------------------------------------------------
} // namespace GUI
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

/// \note OSX 10.6 does not provide a std::strnlen implementation so we have to
/// provide one on our own.
///                                           (27.09.2013.) (Domagoj Saric)
#if defined( __APPLE__ ) /*&& !defined( __LP64__ )*/
extern "C"
{
size_t __attribute__(( weak )) __cdecl strnlen( char const * str, size_t const maxsize_param )
{
    BOOST_ASSERT( str );
    unsigned int const maxsize( static_cast<unsigned int>( maxsize_param ) );
    BOOST_ASSERT( maxsize == maxsize_param );
    unsigned int n;
    for ( n = 0; n < maxsize && *str; n++, str++ ) {}
    return n;
}

size_t __attribute__(( weak )) __cdecl wcsnlen( wchar_t const * wcs, size_t const maxsize_param )
{
    BOOST_ASSERT( wcs );
    unsigned int const maxsize( static_cast<unsigned int>( maxsize_param ) );
    BOOST_ASSERT( maxsize == maxsize_param );
    unsigned int n;
    for ( n = 0; n < maxsize && *wcs; n++, wcs++ ) {}
    return n;
}
};
#endif // __APPLE__ /*&& !__LP64__*/
