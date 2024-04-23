////////////////////////////////////////////////////////////////////////////////
///
/// filesystemAndroid.cpp
/// ---------------------
///
/// Target platform specific boilerplate code.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "filesystem.hpp"

#include "jni.hpp"
#include "trace.hpp"

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/smart_ptr/scoped_array.hpp>
#include <boost/utility/string_ref.hpp>

#include <../../../../../sources/android/ndk_helper/JNIHelper.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <android/native_activity.h>

#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib> // getenv
#include <cstring>
#include <ctime>
#include <new>
#include <string>
//------------------------------------------------------------------------------
namespace ndk_helper { LE_WEAK_FUNCTION std::string JNIHelper::GetExternalFilesDir() { LE_UNREACHABLE_CODE(); return "You have to compile in 'sources/android/ndk_helper/JNIHelper.cpp'."; } }
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

using CString = boost::scoped_array<char>;

namespace
{
    CString                     internalDataPath;
    CString                     externalDataPath;
    CString                     cacheDir        ( /*std*/::strdup( "" ) );
    AAssetManager * LE_RESTRICT pAssetManager   ;

    JNI::GlobalRef<> globalAssetManager;
} // anonymous namespace

bool LE_FASTCALL_ABI setAppContext( ::ANativeActivity const & nativeActivity )
{
    JNI::setVM( *nativeActivity.vm );

#if defined( __arm__ ) && !( defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7__ ) || defined( __aarch64__ ) )
    /// \note Gingerbread bug: internalDataPath and externalDataPath are null.
    // https://groups.google.com/forum/#!topic/android-ndk/4lD8OS-w4UU
    // http://stackoverflow.com/questions/10683119/android-app-activity-internaldatapath-still-null-in-2-3-8-ndk-r8
    /// If we don't have ARMv7 we certainly don't have Android 4+.
    /// http://android.stackexchange.com/questions/34958/what-are-the-minimum-hardware-specifications-for-android
    ///                                       (07.05.2014.) (Domagoj Saric)
    if ( !nativeActivity.internalDataPath )
        if ( BOOST_UNLIKELY( !setAppContext( *JNI::env(), nativeActivity.clazz ) ) )
            return false;
#endif // Gingerbread/pre ARMv7

    BOOST_ASSERT( nativeActivity.internalDataPath );
    BOOST_ASSERT( nativeActivity.externalDataPath );

#ifndef NDEBUG
    BOOST_VERIFY( setAppContext( *JNI::env(), nativeActivity.clazz ) );
    BOOST_ASSERT( std::strcmp( internalDataPath.get(), nativeActivity.internalDataPath ) == 0 );
    BOOST_ASSERT( std::strcmp( externalDataPath.get(), nativeActivity.externalDataPath ) == 0 || *nativeActivity.externalDataPath == 0 || std::strstr( nativeActivity.externalDataPath, externalDataPath.get() ) == nativeActivity.externalDataPath );
#endif // NDEBUG

    pAssetManager = nativeActivity.assetManager;
    BOOST_ASSERT( pAssetManager );

    internalDataPath.reset( /*std*/::strdup( nativeActivity.internalDataPath ) );
    externalDataPath.reset( /*std*/::strdup( nativeActivity.externalDataPath ) );

    if ( BOOST_UNLIKELY( !internalDataPath || !externalDataPath ) )
        return false;

    switch ( *nativeActivity.externalDataPath )
    {
        case '\0':
        case '/':
        {
            char const * const sdCardPath( std::getenv( "EXTERNAL_STORAGE" ) );
            if ( sdCardPath )
            {
                // http://thesai.org/Downloads/Volume4No7/Paper_15-POSIX.1_conformance_for_Android_Applications.pdf
                if ( ::access( sdCardPath, F_OK ) == 0 )
                {
                #ifndef NDEBUG
                    int const result( ::mkdir( sdCardPath, accessFlags ) );
                    BOOST_ASSERT( ( result == 0 ) || ( errno == EEXIST ) );
                #endif // NDEBUG
                    externalDataPath.reset( /*std*/::strdup( sdCardPath ) );
                    if ( BOOST_UNLIKELY( !externalDataPath ) )
                        return false;
                }
            }
            break;
        }

        default:
            break;
    }
    return true;
}

bool LE_FASTCALL_ABI setAppContext( ::ndk_helper::JNIHelper const & jniHelper, ::ANativeActivity const & nativeActivity )
{
    if ( BOOST_UNLIKELY( !setAppContext( nativeActivity ) ) ) return false;
    auto & jni( const_cast<::ndk_helper::JNIHelper &>( jniHelper ) );
    externalDataPath.reset( /*std*/::strdup( jni.GetExternalFilesDir().c_str() ) );
    return !!externalDataPath;
}

bool LE_FASTCALL_ABI setAppContext( ::JNIEnv & __restrict jni, ::jobject const __restrict activity, ::jobject const __restrict assetManager )
{
    BOOST_ASSERT_MSG( !globalAssetManager, "App context already set" );

    JNI::setVM( jni );

    BOOST_ASSERT_MSG( &jni == &JNI::preAttachedEnv(), "Invalid JNIEnv reference" );

    globalAssetManager = JNI::globalReference( assetManager );
    pAssetManager      = ::AAssetManager_fromJava( &jni, assetManager );
    if ( BOOST_UNLIKELY( !globalAssetManager || !pAssetManager ) )
        return false;

    // SD card access:
    // http://android.stackexchange.com/questions/39542/confused-by-the-many-locations-of-the-virtual-sdcard
    // "SDcard mount point in portable way?" https://groups.google.com/forum/#!topic/android-developers/LUcuw06JNe0
    // http://stackoverflow.com/questions/10166638/access-android-apk-asset-data-directly-in-c-without-asset-manager-and-copying
    // http://stackoverflow.com/questions/6276933/getfilesdir-from-ndk
    // http://stackoverflow.com/questions/19568354/accessing-the-sdcard-location-or-getexternalstoragedirectory-in-native-code
    // http://stackoverflow.com/questions/11281010/how-can-i-get-external-sd-card-path-for-android-4-0
    // http://stackoverflow.com/questions/22406061/howto-avoid-the-eacces-permission-denied-on-sdcard-with-kitkat-4-4-2-version
    // http://developer.android.com/guide/topics/data/data-storage.html
    // http://developer.android.com/reference/android/os/Environment.html#getExternalStorageDirectory()
    // No standardised way to access the >external< SD card (if any)
    // https://groups.google.com/forum/#!topic/android-developers/LUcuw06JNe0
    // http://forum.unity3d.com/threads/61198-Data-Storage-How-to-read-write-files
    // http://books.google.hr/books?id=b6lAhtOVtF0C&pg=PT569&lpg=PT569&dq=android+ndk+jni+getExternalStorageDirectory&source=bl&ots=_Zarr89vAg&sig=ptxr2CPNHKldP_xSvPq0y7cINTw&hl=en&sa=X&ei=EExGU-KSApTT4QT2p4HQBg&redir_esc=y#v=onepage&q=android%20ndk%20jni%20getExternalStorageDirectory&f=false
    // known hardcoded paths (/proc/mounts):
    // /sdcard
    // /mnt/sdcard
    // /system/media/sdcard
    // /storage/sdcardx
    // /mnt/sdcardx

    // ANativeActivity::internalDataPath = getFilesDir()
    // ANativeActivity::externalDataPath = Environment.getExternalStorageAppFilesDirectory( ai.packageName )
    // http://grepcode.com/file/repository.grepcode.com/java/ext/com.google.android/android/2.3.1_r1/android/app/NativeActivity.java

    /// \note We don't explicitly manage local references (i.e. use LocalRefs)
    /// because JNI automatically cleans them up upon exiting the function and
    /// it guarantees slots for at least 16 coexisting local references.
    ///                                       (13.01.2016.) (Domagoj Saric)

    jclass    const activityClass  ( jni.GetObjectClass  ( activity                                                 ) ); BOOST_ASSERT( activityClass   );
    jmethodID const getFilesDir    ( jni.GetMethodID     ( activityClass, "getFilesDir"    , "()Ljava/io/File;"     ) ); BOOST_ASSERT( getFilesDir     );
    jobject   const fileObject     ( jni.CallObjectMethod( activity     ,  getFilesDir                              ) ); BOOST_ASSERT( fileObject      );
    jclass    const fileClass      ( jni.GetObjectClass  ( fileObject                                               ) ); BOOST_ASSERT( fileClass       );
    jmethodID const getAbsolutePath( jni.GetMethodID     ( fileClass    , "getAbsolutePath", "()Ljava/lang/String;" ) ); BOOST_ASSERT( getAbsolutePath );

    if ( BOOST_UNLIKELY( jni.ExceptionOccurred() != nullptr ) )
        return false;

    {
        // http://stackoverflow.com/questions/5859673/should-you-call-releasestringutfchars-if-getstringutfchars-returned-a-copy
        jstring const jpath( static_cast<jstring>( jni.CallObjectMethod( fileObject, getAbsolutePath ) ) );
        char const * const path( jni.GetStringUTFChars( jpath, nullptr ) ); if ( BOOST_UNLIKELY( !path ) ) return false;
        internalDataPath.reset( /*std*/::strdup( path ) );
        jni.ReleaseStringUTFChars( jpath, path );
        if ( BOOST_UNLIKELY( !internalDataPath ) ) return false;
    }

    // Get File object for the external storage directory.
    {
        jclass    const classEnvironment                   ( jni.FindClass             ( "android/os/Environment" ) ); BOOST_ASSERT( classEnvironment );
        jmethodID const methodIDgetExternalStorageDirectory( jni.GetStaticMethodID     ( classEnvironment, "getExternalStorageDirectory", "()Ljava/io/File;" ) ); BOOST_ASSERT( methodIDgetExternalStorageDirectory ); // public static File getExternalStorageDirectory()
        jobject   const objectFile                         ( jni.CallStaticObjectMethod( classEnvironment, methodIDgetExternalStorageDirectory ) );

        // Call method on File object to retrieve String object.
        jstring const jpath( static_cast<jstring>( jni.CallObjectMethod( objectFile, getAbsolutePath ) ) );

        if ( BOOST_UNLIKELY( jni.ExceptionOccurred() != nullptr ) )
            return false;
        {
            // http://stackoverflow.com/questions/5859673/should-you-call-releasestringutfchars-if-getstringutfchars-returned-a-copy
            char const * const path( jni.GetStringUTFChars( jpath, nullptr ) ); if ( BOOST_UNLIKELY( !path ) ) return false;
            externalDataPath.reset( /*std*/::strdup( path ) );
            jni.ReleaseStringUTFChars( jpath, path );
            if ( BOOST_UNLIKELY( !externalDataPath ) ) return false;
        }
    }

#if 0
    // getCacheDir() - java
    mid_getExtStorage = jni_env->GetMethodID( cls_Env, "getCacheDir", "()Ljava/io/File;" );
    obj_File = jni_env->CallObjectMethod( gstate->activity->clazz, mid_getExtStorage, NULL );
    cls_File = jni_env->FindClass( "java/io/File" );
    mid_getPath = jni_env->GetMethodID( cls_File, "getAbsolutePath", "()Ljava/lang/String;" );
    obj_Path = (jstring)jni_env->CallObjectMethod( obj_File, mid_getPath );
    path = jni_env->GetStringUTFChars( obj_Path, NULL );
    FHZ_PRINTF( "CACHE DIR = %s\n", path );
    jni_env->ReleaseStringUTFChars( obj_Path, path );
#endif // 0

    return true;
}

bool LE_FASTCALL_ABI setAppContext( ::JNIEnv & __restrict jni, ::jobject const __restrict activity )
{
    //BOOST_ASSERT_MSG( &jni == &JNI::preAttachedEnv(), "Invalid JNIEnv reference" );

    // https://github.com/ddlee/AndroidLuaActivity/blob/master/jni/src/jnicontext.cpp

    jmethodID const methodGetAssets  ( jni.GetMethodID     ( jni.GetObjectClass( activity ), "getAssets", "()Landroid/content/res/AssetManager;" ) ); BOOST_ASSERT( methodGetAssets );
    jobject   const localAssetManager( jni.CallObjectMethod( activity, methodGetAssets ) ); BOOST_ASSERT( localAssetManager );

    if ( BOOST_UNLIKELY( jni.ExceptionOccurred() != nullptr ) )
        return false;

    return setAppContext( jni, activity, localAssetManager );
}


LE_CONST_FUNCTION ::AAssetManager & LE_FASTCALL_ABI resourceManager()
{
    BOOST_ASSERT_MSG( pAssetManager, "Android app context not set" );
    return *pAssetManager;
}


namespace
{
    template <SpecialLocations> char const * pathFor();

    template <> char const * pathFor<AppData        >() { return internalDataPath.get(); }
    template <> char const * pathFor<Documents      >() { return pathFor<AppData>()    ; }
    template <> char const * pathFor<Library        >() { return pathFor<AppData>()    ; }
    template <> char const * pathFor<Resources      >() { return nullptr               ; } // http://stackoverflow.com/questions/7701801/obtaining-the-name-of-an-android-apk-using-c-and-the-nativeactivity-class
    template <> char const * pathFor<ExternalStorage>() { return externalDataPath.get(); }
    template <> char const * pathFor<Temporaries    >() { return cacheDir        .get(); }
    template <> char const * pathFor<ToolOutput     >()
    {
    #ifndef NDEBUG
        static bool sdCardWarningIssued( false );
    #endif // NDEBUG
        if ( !accessible<ExternalStorage, true>() )
        {
        #ifndef NDEBUG
            if ( !sdCardWarningIssued )
            {
                Tracer::message( "Unable to find or access an SD card, ToolOutput will redirect to AppData (app's private storage)." );
                sdCardWarningIssued = true;
            }
        #endif // NDEBUG
            return pathFor<AppData>();
        }
    #ifndef NDEBUG
        sdCardWarningIssued = false;
    #endif // NDEBUG
        return pathFor<ExternalStorage>();
    }
} // anonymous namespace

template <SpecialLocations rootDirectory>
template <class Result, class Functor>
Result PathResolver<rootDirectory>::apply( char const * const relativePathParam, Functor const f )
{
    boost::string_ref const rootPath    ( pathFor<rootDirectory>() );
    boost::string_ref const relativePath( relativePathParam        );

    char absolutePath[ rootPath.size() + 1 + relativePath.size() ];
    char * position( std::copy( rootPath.begin(), rootPath.end(), absolutePath ) );
    *position++ = '/';
    std::copy( relativePath.begin(), relativePath.end() + 1, position );
    return f( absolutePath );
}


namespace
{
    void closeAsset( void * const pAsset )
    {
        if ( pAsset )
            ::AAsset_close( static_cast<::AAsset *>( pAsset ) );
    }

    void moveAsset( void * & pTargetAsset, void * & pSourceAsset )
    {
        closeAsset( pTargetAsset );
        pTargetAsset = pSourceAsset;
        pSourceAsset = nullptr;
    }
} // anonymous namespace

ResourceFile::MemoryMapping:: MemoryMapping(                               ) : handle_( nullptr       ) {}
ResourceFile::MemoryMapping:: MemoryMapping( void          *  const handle ) : handle_( handle        ) {}
ResourceFile::MemoryMapping:: MemoryMapping( MemoryMapping &&       other  ) : handle_( other.handle_ ) { other.handle_ = nullptr; }
ResourceFile::MemoryMapping::~MemoryMapping() { closeAsset( handle_ ); }

ResourceFile::MemoryMapping & ResourceFile::MemoryMapping::operator=( ResourceFile::MemoryMapping && other )
{
    moveAsset( this->handle_, other.handle_ );
    return *this;
}

ResourceFile::MemoryMapping::value_type * ResourceFile::MemoryMapping::begin() const { return static_cast<char const * >( ::AAsset_getBuffer( static_cast<::AAsset *>( handle_ ) ) ); }
ResourceFile::MemoryMapping::value_type * ResourceFile::MemoryMapping::end  () const { return begin() + size(); }
std::uint32_t                             ResourceFile::MemoryMapping::size () const { return static_cast<std::uint32_t>( ::AAsset_getLength( static_cast<::AAsset *>( handle_ ) ) ); }

     ResourceFile::MemoryMapping::operator bool () const { BOOST_ASSERT( !handle_ || begin() ); return handle_ != nullptr; }
bool ResourceFile::MemoryMapping::operator     !() const { BOOST_ASSERT( !handle_ || begin() ); return handle_ == nullptr; }


ResourceFile::MemoryMapping ResourceFile::map( char const * const relativeFilePath )
{
    // MP3 from memory N/A https://groups.google.com/forum/#!searchin/android-ndk/opensl|sort:date/android-ndk/cMHlkyQkFU0/vMkyO2201yYJ
    // http://stackoverflow.com/questions/18862715/how-to-generate-the-aac-adts-elementary-stream-with-android-mediacodec
    AAsset * const pInputFileAsset( ::AAssetManager_open( &resourceManager(), relativeFilePath, AASSET_MODE_STREAMING ) );
    if ( BOOST_UNLIKELY( !pInputFileAsset ) )
        return LE_TRACE_RETURN( MemoryMapping(), "Failed to open resource %s", relativeFilePath );

    if ( BOOST_UNLIKELY( !::AAsset_getBuffer( pInputFileAsset ) ) )
    {
        ::AAsset_close( pInputFileAsset );
        return LE_TRACE_RETURN( MemoryMapping(), "Failed to load resource %s", relativeFilePath );
    }

    return MemoryMapping( pInputFileAsset );
}


ResourceFile::Stream:: Stream() : handle_( nullptr ) {}
ResourceFile::Stream:: Stream( void * const handle ) : handle_( handle ) {}
ResourceFile::Stream:: Stream( Stream && other ) : handle_( other.handle_ ) { other.handle_ = nullptr; }
ResourceFile::Stream::~Stream() { closeAsset( handle_ ); }

ResourceFile::Stream & ResourceFile::Stream::operator=( ResourceFile::Stream && other )
{
    moveAsset( this->handle_, other.handle_ );
    return *this;
}

// http://stackoverflow.com/questions/9871516/opensl-es-crashes-randomly-on-samsung-galaxy-sii-gt-i9100
int           ResourceFile::Stream::asPOSIXFile( ::off_t & startOffset, std::size_t & size                     ) const { return          ::AAsset_openFileDescriptor( static_cast<::AAsset *>( handle_ ), &startOffset, &reinterpret_cast<::off_t &>( size ) )     ; }
std::uint32_t ResourceFile::Stream::read       ( void * const pBuffer, std::uint32_t const numberOfBytesToRead )       { return          ::AAsset_read              ( static_cast<::AAsset *>( handle_ ), pBuffer, numberOfBytesToRead                       )     ; }
std::uint32_t ResourceFile::Stream::size       (                                                               ) const { return          ::AAsset_getLength         ( static_cast<::AAsset *>( handle_ )                                                     )     ; }
std::uint32_t ResourceFile::Stream::position   (                                                               ) const { return size() - ::AAsset_getRemainingLength( static_cast<::AAsset *>( handle_ )                                                     )     ; }
bool          ResourceFile::Stream::seek       ( std::int32_t const offset, std::uint8_t const whence          )       { return          ::AAsset_seek              ( static_cast<::AAsset *>( handle_ ), offset, whence                                     ) >= 0; }

     ResourceFile::Stream::operator bool () const { BOOST_ASSERT( !handle_ == !size() ); return handle_ != nullptr; }
bool ResourceFile::Stream::operator     !() const { BOOST_ASSERT( !handle_ == !size() ); return handle_ == nullptr; }

ResourceFile::Stream ResourceFile::open( char const * const relativeFilePath )
{
    return Stream( ::AAssetManager_open( &resourceManager(), relativeFilePath, AASSET_MODE_STREAMING ) );
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "filesystemImpl.inl"
