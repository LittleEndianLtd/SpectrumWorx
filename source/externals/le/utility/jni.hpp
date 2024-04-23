////////////////////////////////////////////////////////////////////////////////
///
/// \file jni.hpp
/// -------------
///
/// JNI helpers&wrappers.
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef jni_hpp__386FB370_4077_483D_98B3_C8A07DCE236C
#define jni_hpp__386FB370_4077_483D_98B3_C8A07DCE236C
#pragma once
//------------------------------------------------------------------------------
#include "assert.hpp"

#include <jni.h>

#include <memory>
#include <type_traits>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------
/// \addtogroup Android Android specific
/// @{
//------------------------------------------------------------------------------
/// \addtogroup JNI Java<->Native interop helpers
/// @{
//------------------------------------------------------------------------------
namespace JNI /// \brief Java<->Native interop helpers
{
//------------------------------------------------------------------------------

namespace Detail
{
    extern ::JavaVM * __restrict pJVM;

    struct EnvDeleter       { LE_NOTHROW void LE_FASTCALL_ABI operator()( JNIEnv * ) const; bool const detach; };
    struct GlobalRefDeleter { LE_NOTHROW void LE_FASTCALL_ABI operator()( jobject  ) const;                    };
    struct LocalRefDeleter  { LE_NOTHROW void LE_FASTCALL_ABI operator()( jobject  ) const;                    };

    struct CStrDeleter
    {
        LE_NOTHROW void LE_FASTCALL_ABI operator()( char const * const cString ) const
        {
            cachedEnvironment.ReleaseStringUTFChars( sourceString, cString );
        }
        JNIEnv  &       cachedEnvironment;
        jstring   const sourceString;
    }; // struct CStrDeleter
    
    jobject LE_FASTCALL_ABI newGlobalReference( JNIEnv & cachedEnv, jobject );
    jobject LE_FASTCALL_ABI newLocalReference ( JNIEnv & cachedEnv, jobject );

    LE_NOTHROW LE_PURE_FUNCTION std::pair<JNIEnv *, EnvDeleter> LE_FASTCALL_ABI env();
} // namespace Detail

/// \addtogroup JNI Java<->Native interop helpers
/// @{

                                       using EnvPtr    = std::unique_ptr<                             JNIEnv         , Detail::EnvDeleter      >; ///< Thread-local JNI environment smart ptr
template <typename JavaType = jobject> using GlobalRef = std::unique_ptr<typename std::remove_pointer<JavaType>::type, Detail::GlobalRefDeleter>; ///< Global JNI reference to a <VAR>JavaType</VAR> Java object
template <typename JavaType = jobject> using LocalRef  = std::unique_ptr<typename std::remove_pointer<JavaType>::type, Detail::LocalRefDeleter >; ///< Local JNI reference to a <VAR>JavaType</VAR> Java object

                                       using CStr      = std::unique_ptr<                             char const[]   , Detail::CStrDeleter     >; ///< C-string representation of a Java String smart ptr

/// \brief Get a smart ptr to a UTF8 C string from a Java String
inline LE_NOTHROW
CStr LE_FASTCALL_ABI c_str( JNIEnv & env, jstring const javaString )
{
    return { env.GetStringUTFChars( javaString, nullptr ), { env, javaString } };
}

/// \brief Retrieve the (previously saved) singleton JavaVM instance.<BR>
/// \details <B>Preconditions:</B> a successful call to Utility::JNI::setVM() or
/// Utility::setAppContext().
inline
JavaVM & vm() { LE_ASSUME( Detail::pJVM ); return *Detail::pJVM; }

/// \brief Set the singleton JavaVM instance
inline     void                 setVM( ::JavaVM & vm ) { LE_ASSERT_MSG( !Detail::pJVM, "JVM singleton already set" ); Detail::pJVM = &vm; }
LE_NOTHROW void LE_FASTCALL_ABI setVM( ::JNIEnv &    ); ///< \brief \overload


/// \brief Retrieve the JNIEnv instance for the current thread.
/// \details Tries to attach the thread to the JVM if it is not already
/// attached (and in that case it may fail, if the system is very low on
/// resources, and return a nullptr). The returned EnvPtr remembers whether the
/// function had to attach the thread to the JVM and, if so, will detach it in
/// its destructor.
LE_NOTHROW LE_PURE_FUNCTION inline
EnvPtr LE_FASTCALL_ABI env() { auto const abi_env( Detail::env() ); return { abi_env.first, abi_env.second }; }

/// \brief Retrieve the JNIEnv instance for the current thread. The function
/// assumes the calling thread is already attached to the JVM.
LE_NOTHROW LE_PURE_FUNCTION
JNIEnv & LE_FASTCALL_ABI preAttachedEnv();

namespace Detail
{
    inline jobject LE_FASTCALL_ABI newGlobalReference( jobject object ) { return newGlobalReference( preAttachedEnv(), object ); }
    inline jobject LE_FASTCALL_ABI newLocalReference ( jobject object ) { return newLocalReference ( preAttachedEnv(), object ); }
} // namespace Detail

template <typename JavaType> GlobalRef<JavaType> globalReference( JavaType const javaObject ) { return { static_cast<JavaType>( Detail::newGlobalReference( javaObject ) ), Detail::GlobalRefDeleter() }; } ///< Create a global reference to a Java object.
template <typename JavaType> LocalRef <JavaType>  localReference( JavaType const javaObject ) { return { static_cast<JavaType>( Detail::newLocalReference ( javaObject ) ), Detail::LocalRefDeleter () }; } ///< Create a local reference to a Java object.


template <typename T> jlong   marshalPointer( T *   const cPtr   ) { return                        reinterpret_cast<std::intptr_t>(   cPtr   ); } ///< Store a native pointer into an opaque integer data type for passing to Java code.
template <typename T> T *   unmarshalPointer( jlong const jniPtr ) { return reinterpret_cast<T *>(      static_cast<std::intptr_t>( jniPtr ) ); } ///< Extract a native pointer from the opaque integer data type in which it was stored on the Java side.

/// @}  // group JNI

//------------------------------------------------------------------------------
} // namespace JNI
//------------------------------------------------------------------------------
/// @}  // group JNI
//------------------------------------------------------------------------------
/// @}  // group Android
//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // jni_hpp
