////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \file entryPoint.hpp
/// --------------------
///
/// Helper macros for declaring and defining exported entry point/factory
/// functions.
///
/// Copyright (c) 2013 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef entryPoint_hpp__FE480627_652A_4BB6_9059_D3CAEBF9D4C1
#define entryPoint_hpp__FE480627_652A_4BB6_9059_D3CAEBF9D4C1
#pragma once
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Implementation note:
//   The Barton-Nackman friend-injection trick is used to force the creation/ of
// injection of the entry point function in the global namespace from a template
// class (and thus save the user from having to do it manually).
//   The injector class template has to be declared in the global namespace so
// that the injected entry point function would also be injected into the global
// namespace (otherwise it would end up C++ name mangled in the exported
// functions list).
//   Unfortunately, the "original"/Barton-Nackman style friend injection has
// been removed from the C++ standard and so Clang does not support it and the
// version from Xcode 4.6 even crashes on code that attempts to use the new ADL
// based apporach (N0777) and thus requires other tricks and/or workarounds.
// http://en.wikipedia.org/wiki/Barton-Nackman_trick
// http://objectmix.com/c/39865-how-fix-program-depends-friend-injection.html
// http://llvm.org/bugs/show_bug.cgi?id=8007
// http://www.open-std.org/jtc1/sc22/wg21/docs/papers/1995/N0777.pdf
//                                            (20.08.2009.) (Domagoj Saric)
//
////////////////////////////////////////////////////////////////////////////////


#if defined( _MSC_VER ) || ( defined( __GNUC__ ) && !defined( __clang__ ) )
    #define LE_HAS_FRIEND_INJECTION
#endif

#if defined( _MSC_VER )
    #define LE_ENTRY_POINT_ATTRIBUTES __declspec( dllexport nothrow noinline restrict )
#elif defined( __GNUC__ )
    #define LE_ENTRY_POINT_ATTRIBUTES __attribute__(( visibility( "default" ), nothrow, cdecl ))
#else
    #define LE_ENTRY_POINT_ATTRIBUTES
#endif


#ifdef LE_HAS_FRIEND_INJECTION
    #define LE_ENTRY_POINT_BEGIN( returnType, name, ... )                             \
        extern "C" LE_ENTRY_POINT_ATTRIBUTES returnType __cdecl name( __VA_ARGS__ );  \
        template <class Impl> struct LEPlugin_ ## name ## _EntryPointFriendInjector { \
        friend LE_ENTRY_POINT_ATTRIBUTES returnType __cdecl name( __VA_ARGS__ )

    #define LE_ENTRY_POINT_END() };
#else
    #define LE_ENTRY_POINT_BEGIN( returnType, name, ... )                         \
    extern "C" LE_ENTRY_POINT_ATTRIBUTES returnType __cdecl name( __VA_ARGS__ );  \
    template <class Impl> returnType name ## Impl( __VA_ARGS__ )

    #define LE_ENTRY_POINT_END()
#endif // LE_HAS_FRIEND_INJECTION

//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // entryPoint_hpp
