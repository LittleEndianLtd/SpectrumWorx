////////////////////////////////////////////////////////////////////////////////
///
/// jni.cpp
/// -------
///
/// Target platform specific boilerplate code.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "jni.hpp"

#include "platformSpecifics.hpp"
#include "tracePrivate.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------
namespace JNI
{
//------------------------------------------------------------------------------

// JNI / Java interop:
//  http://en.wikipedia.org/wiki/Java_Native_Interface
//  http://docs.oracle.com/javase/6/docs/technotes/guides/jni
//  http://web.archive.org/web/20070101174413/http://java.sun.com/docs/books/jni Java Native Interface: Programmer's Guide and Specification
//  http://developer.android.com/training/articles/perf-jni.html
//  https://www.ibm.com/developerworks/library/j-jni "Best practices"
//  http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/types.html#wp16432 signature strings
//  http://android.wooyd.org/JNIExample/#NWD1sCYeT-J
//  https://code.google.com/p/android-cpp-sdk
//  http://stackoverflow.com/questions/9304185/how-to-call-java-function-from-c
//  http://stackoverflow.com/questions/12872742/sending-jstring-from-jni-c-code-to-a-java-function-that-receives-string-as-an-ar
//  https://groups.google.com/forum/#!topic/android-ndk/-vN0maMKtOs Calling Java Functions From Within C++ Code
//  http://sbcgamesdev.blogspot.com/2012/12/using-jnionload-in-adroid-ndk.html
//  http://audioprograming.wordpress.com/2012/10/30/on-wrapping-android-ndk-code-with-swig
//  https://groups.google.com/forum/#!topic/android-ndk/_jDkSkLgPvs NewGlobalRef/DeleteGlobalRef/DeleteLocalRef
//  http://android-developers.blogspot.com.es/2011/11/jni-local-reference-changes-in-ics.html
//  http://docs.oracle.com/javase/6/docs/technotes/guides/jni/spec/design.html#wp16785 (references)
// JNI_OnLoad
//  https://code.google.com/p/awesomeguy/wiki/JNITutorial
//  http://adndevblog.typepad.com/cloud_and_mobile/2013/08/android-ndk-passing-complex-data-to-jni.html
//  http://bleaklow.com/2006/02/18/jni_onunload_mostly_useless.html

#if 0 // unused
////////////////////////////////////////////////////////////////////////////////
// Private functionality not (yet) available to SDK clients
////////////////////////////////////////////////////////////////////////////////

template <typename JavaType>
LE_NOTHROW bool LE_FASTCALL operator==( GlobalRef<JavaType> const & right, GlobalRef<JavaType> const & left ) { env()->IsSameObject( right.get(), left.get() ); }
#endif

namespace Detail
{
    JavaVM * __restrict pJVM;

    jobject LE_FASTCALL_ABI newGlobalReference( JNIEnv & cachedEnv, jobject const pObject ) { return cachedEnv.NewGlobalRef( pObject ); }
    jobject LE_FASTCALL_ABI newLocalReference ( JNIEnv & cachedEnv, jobject const pObject ) { return cachedEnv.NewLocalRef ( pObject ); }

    void GlobalRefDeleter::operator()( ::jobject const pObject ) const { preAttachedEnv().DeleteGlobalRef( pObject ); }
    void LocalRefDeleter ::operator()( ::jobject const pObject ) const { preAttachedEnv().DeleteLocalRef ( pObject ); }

    LE_NOTHROW LE_COLD
    void EnvDeleter::operator()( ::JNIEnv * const pJNI ) const
    {
        BOOST_ASSERT( pJNI );
        if ( BOOST_UNLIKELY( detach ) )
        {
            LE_TRACE_LOGONLY( "Detaching a native thread from the JVM." );
            BOOST_VERIFY( pJVM->DetachCurrentThread() == JNI_OK );
        }
    }
} // namespace Detail

LE_NOTHROW LE_COLD
void LE_FASTCALL_ABI setVM( ::JNIEnv & jni )
{
    BOOST_ASSERT_MSG( !Detail::pJVM, "JVM singleton already set" );
    BOOST_VERIFY( jni.GetJavaVM( const_cast<JavaVM * *>( &Detail::pJVM ) ) == JNI_OK );
}


LE_NOTHROW LE_PURE_FUNCTION LE_COLD
JNIEnv & LE_FASTCALL_ABI preAttachedEnv()
{
    JNIEnv * pJNI;
    BOOST_VERIFY_MSG( vm().GetEnv( reinterpret_cast<void **>( &pJNI ), JNI_VERSION_1_6 ) == JNI_OK, "Calling thread not attached to the JVM" );
    return *pJNI;
}

namespace Detail
{
LE_NOTHROW LE_PURE_FUNCTION LE_COLD /*EnvPtr...mrmlj...libc++ vs libstdc++ std::unique_ptr incompatibility*/
std::pair<JNIEnv *, EnvDeleter> LE_FASTCALL_ABI env()
{
    JNIEnv * pJNI;
    auto const result     ( vm().GetEnv( reinterpret_cast<void **>( &pJNI ), JNI_VERSION_1_6 ) );
    auto const preAttached( result == JNI_OK );
    if ( BOOST_UNLIKELY( !preAttached ) )
    {
        LE_ASSUME( result == JNI_EDETACHED );
        LE_ASSUME( pJNI   == nullptr       );
        LE_TRACE_LOGONLY( "Attaching a native thread to the JVM." ); //...mrmlj...avoid infinite loops and showing this info in the example app gui...
        BOOST_VERIFY( vm().AttachCurrentThread( &pJNI, nullptr ) == JNI_OK );
    }
    return { pJNI, Detail::EnvDeleter{ !preAttached } };
}
} // namespace Detail ...mrmlj...

//------------------------------------------------------------------------------
} // namespace JNI
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
