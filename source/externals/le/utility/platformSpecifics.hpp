////////////////////////////////////////////////////////////////////////////////
///
/// \file platformSpecifics.hpp
/// ---------------------------
///
///   An internal collection of macros that wrap platform specific details/non
/// standard extensions (expands the public parts exposed in abi.hpp).
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef platformSpecifics_hpp__B98C0700_01F9_4B49_AADC_C1AF5BB3EB9B
#define platformSpecifics_hpp__B98C0700_01F9_4B49_AADC_C1AF5BB3EB9B
#pragma once
//------------------------------------------------------------------------------
#include "abi.hpp"

#include <boost/assert.hpp>
//------------------------------------------------------------------------------

/// \note Use a more elaborate version of assume for internal code.
///                                           (31.10.2013.) (Domagoj Saric)
#undef LE_ASSUME

#if defined( _MSC_VER ) && !defined( __clang__ )

    #define LE_NOVTABLE __declspec( novtable )

    #if ( _MSC_VER >= 1800 ) && ( defined( _M_X64 ) || ( _M_IX86_FP >= 2 ) )
        #define LE_FASTCALL __vectorcall
    #else
        #define LE_FASTCALL LE_FASTCALL_ABI
    #endif

    #define LE_ALIGN( alignment ) __declspec( align( alignment ) )

    #define LE_FORCEINLINE __forceinline
    #define LE_NOINLINE    __declspec( noinline )
    #define LE_HOT
    #define LE_COLD

    #define LE_WEAK_SYMBOL       __declspec( selectany )
    #define LE_WEAK_SYMBOL_CONST __declspec( selectany ) extern
    // http://blogs.msdn.com/b/freik/archive/2005/10/26/485276.aspx
    #define LE_WEAK_FUNCTION     extern __declspec( noinline ) inline

    #define LE_UNREACHABLE_CODE()  BOOST_ASSERT_MSG( false    , "This code should not be reached." ); __assume( false     )
    #define LE_ASSUME( condition ) BOOST_ASSERT_MSG( condition, "Assumption broken."               ); __assume( condition )

    #define LE_MSVC_SPECIFIC(  expression ) expression
    #define LE_GNU_SPECIFIC(   expression )
    #define LE_CLANG_SPECIFIC( expression )

    #define LE_OPTIMIZE_FOR_SPEED_BEGIN()        \
        __pragma( optimize        ( "t", on ) )  \
        __pragma( auto_inline     ( on      ) )  \
        __pragma( inline_recursion( on      ) )  \
        __pragma( inline_depth    ( 255     ) )  \

    /// \note This one seems to interact badly with LTCG (as if Ot was never
    /// turned on) with MSVC10.
    ///                                       (31.10.2013.) (Domagoj Saric)
    #ifdef LE_UNITY_BUILD
        #define LE_OPTIMIZE_FOR_SPEED_END() \
            __pragma( optimize    ( "", on ) )
    #else
        #define LE_OPTIMIZE_FOR_SPEED_END()
    #endif // LE_UNITY_BUILD

    #define LE_OPTIMIZE_FOR_SIZE_BEGIN() \
        __pragma( optimize        ( "s", on ) )

    #define LE_OPTIMIZE_FOR_SIZE_END LE_OPTIMIZE_FOR_SPEED_END


    // msdn.microsoft.com/en-us/library/45ec64h6.aspx
    #define LE_FAST_MATH_ON()                      \
        __pragma( float_control( except , off ) )  \
        __pragma( fenv_access  ( off          ) )  \
        __pragma( float_control( precise, off ) )  \
        __pragma( fp_contract  ( on           ) )

    #define LE_FAST_MATH_OFF()                     \
        __pragma( float_control( precise, on  ) )  \
        __pragma( fenv_access  ( on           ) )  \
        __pragma( float_control( except , on  ) )  \
        __pragma( fp_contract  ( off          ) )

    #define LE_FAST_MATH_ON_BEGIN()                \
        __pragma( float_control( push ) )          \
        LE_FAST_MATH_ON()

    #define LE_FAST_MATH_ON_END()                  \
        LE_FAST_MATH_OFF()                         \
        __pragma( float_control( pop ) )

    #define LE_FAST_MATH_OFF_BEGIN()               \
        __pragma( float_control( push ) )          \
        LE_FAST_MATH_OFF()

    #define LE_FAST_MATH_OFF_END()                 \
        LE_FAST_MATH_ON()                          \
        __pragma( float_control( pop ) )

    // https://msdn.microsoft.com/en-us/library/hh923901.aspx
    #define LE_DISABLE_LOOP_VECTORIZATION() __pragma( loop( no_vector ) )
    #define LE_DISABLE_LOOP_UNROLLING()

    #ifdef _CPPUNWIND
        #define LE_EXCEPTION_ON
    #endif // _CPPUNWIND

    #if _MSC_VER < 1900
        #define LE_ARR_SZ( x ) ( sizeof( x ) / sizeof( x.front() ) )
    #endif

#elif defined( __GNUC__ )

    #if defined( __clang__ )
        // http://clang.llvm.org/docs/LanguageExtensions.html
        #ifndef __has_extension
            #define __has_extension __has_feature // Compatibility with pre-3.0 compilers.
        #endif
        #define LE_HAS_CLANG_BUILTIN(   builtin   ) __has_builtin  ( builtin   )
        #define LE_HAS_CLANG_EXTENSION( extension ) __has_extension( extension )
    #else
        #define LE_HAS_CLANG_BUILTIN(   builtin   ) 0
        #define LE_HAS_CLANG_EXTENSION( extension ) 0
    #endif // __clang__

    //#define LE_NOVTABLE __declspec( novtable )...-fms-extensions
    #define LE_NOVTABLE

    #ifdef __i386__
        #ifdef __clang__
            // http://llvm.org/bugs/show_bug.cgi?id=12990
            // http://code.google.com/p/nativeclient/issues/detail?id=2346
            //...mrmlj...pre Xcode 4.6 Clang generates wrong/"non fastcall" code
            //...mrmlj...for (functions with) reference parameters...
            #define LE_FASTCALL __vectorcall
        #else
            #define LE_FASTCALL __attribute__(( regparm( 3 ), stdcall, sseregparm ))
        #endif // __clang__
    /// \note Clang crashes when trying to use aapcs-vfp with Thumb1 ("Thumb1
    /// can only use GPR registers") so for armeabi targets we disable it
    /// completely for the ABI attribute and enable it only for non-thumb VFP2
    /// builds.
    ///                                       (12.06.2015.) (Domagoj Saric)
    #elif defined( __arm__ ) && ( defined( __TARGET_FPU_VFP ) || defined( __VFP_FP__ ) ) && ( !defined( __thumb__ ) || defined( __ARM_ARCH_7A__ ) || defined( __ARM_ARCH_7__ ) ) && !defined( __aarch64__ )
        #define LE_FASTCALL __attribute__(( pcs( "aapcs-vfp" ) ))
    #else
        #define LE_FASTCALL LE_FASTCALL_ABI
    #endif // __i386__

    #define LE_ALIGN( alignment ) __attribute__(( aligned( alignment ) ))

    #ifdef _DEBUG
        #define LE_FORCEINLINE inline
    #else
        #define LE_FORCEINLINE __attribute__(( always_inline )) inline
    #endif
    #define LE_NOINLINE __attribute__(( noinline ))

    // http://lists.cs.uiuc.edu/pipermail/cfe-commits/Week-of-Mon-20120507/057599.html
    // http://clang-developers.42468.n3.nabble.com/PROPOSAL-per-function-optimization-level-control-td4031670.html
    // https://gcc.gnu.org/onlinedocs/gcc/ARM-Function-Attributes.html
    #define LE_HOT  __attribute__(( hot  ))
    //...mrmlj...disable 'cold' until we split cold and minsize (as cold can slow down calling code)...
    #if ( defined( __arm__ ) && !defined( __aarch64__ ) ) && !defined( __clang__ )
        //...mrmlj...Clang claims support for 'target' but does not work with
        //...3.6 or Xcode7
        //...http://clang.llvm.org/docs/AttributeReference.html#target-gnu-target
        #define LE_COLD __attribute__(( /*cold,*/ minsize, target( "thumb" ) ))
    #else
        #define LE_COLD __attribute__(( /*cold,*/ minsize ))
    #endif

    #define LE_WEAK_SYMBOL       __attribute__(( weak ))
    #define LE_WEAK_SYMBOL_CONST LE_WEAK_SYMBOL extern
    #define LE_WEAK_FUNCTION     LE_WEAK_SYMBOL extern

    #if LE_HAS_CLANG_BUILTIN( __builtin_assume )
        #define LE_ASSUME(           condition ) BOOST_ASSERT_MSG( condition, "Assumption broken."               ); __builtin_assume( condition )
        #define LE_UNREACHABLE_CODE(           ) BOOST_ASSERT_MSG( false    , "This code should not be reached." ); __builtin_unreachable()
    #elif LE_HAS_CLANG_BUILTIN( __builtin_unreachable ) || ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) >= 45 )
        // http://en.chys.info/2010/07/counterpart-of-assume-in-gcc
        // http://nondot.org/sabre/LLVMNotes/BuiltinUnreachable.txt
        // http://llvm-reviews.chandlerc.com/file/data/3qqhjnypd5j4vaxwuogy/PHID-FILE-q4turx3ss4xgvxyl2zht/D149.diff
        // http://gcc.gnu.org/ml/gcc-patches/2008-04/msg00059.html
        #define LE_UNREACHABLE_CODE() BOOST_ASSERT_MSG( false, "This code should not be reached." ); __builtin_unreachable()
        #if !defined( __clang__ ) && ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) > 45 ) && ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) < 48 )
            // Broken/pessimization in GCC 4.6:
            //  https://bugs.launchpad.net/gcc-linaro/+bug/1020601
            //  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=50385
            //  http://gcc.gnu.org/ml/gcc-patches/2012-07/msg00254.html
            //  http://gcc.gnu.org/bugzilla/show_bug.cgi?id=49054
            #define LE_ASSUME( condition ) BOOST_ASSERT_MSG( condition, "Assumption broken." )
            #pragma GCC diagnostic ignored "-Wunused-value"
        #else
            #define LE_ASSUME( condition ) BOOST_ASSERT_MSG( condition, "Assumption broken." ); do { if ( !( condition ) ) __builtin_unreachable(); } while ( 0 )
        #endif
    #else
        #define LE_UNREACHABLE_CODE()  BOOST_ASSERT_MSG( false    , "This code should not be reached." )
        #define LE_ASSUME( condition ) BOOST_ASSERT_MSG( condition, "Assumption broken."               )
    #endif

    #define LE_MSVC_SPECIFIC( expression )
    #define LE_GNU_SPECIFIC(  expression ) expression
    #ifdef __clang__
        #define LE_CLANG_SPECIFIC( expression ) expression
    #else // Clang
        #define LE_CLANG_SPECIFIC( expression )
    #endif // Clang

    // No support in Clang yet...
    // http://lists.cs.uiuc.edu/pipermail/llvmdev/2013-April/061527.html
    // http://comments.gmane.org/gmane.comp.compilers.clang.devel/28958
    #if ( ( ( __GNUC__ * 10 ) + __GNUC_MINOR__ ) >= 44 )
        // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=41201
        // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=52144
        #define LE_OPTIMIZE_FOR_SPEED_BEGIN()     \
            _Pragma( "push" )                     \
            _Pragma( "GCC push_options"        )  \
            _Pragma( "GCC optimize ( \"O3\" )" )  \
            _Pragma( "arm" )

        #define LE_OPTIMIZE_FOR_SPEED_END() \
            _Pragma( "GCC pop_options" )    \
            _Pragma( "pop" )

        #define LE_OPTIMIZE_FOR_SIZE_BEGIN()     \
            _Pragma( "push" )                    \
            _Pragma( "GCC push_options"        ) \
            _Pragma( "GCC optimize ( \"Os\" )" ) \
            _Pragma( "thumb" )

        #define LE_OPTIMIZE_FOR_SIZE_END LE_OPTIMIZE_FOR_SPEED_END


        // http://lists.cs.uiuc.edu/pipermail/llvmdev/2013-April/061527.html
        // http://gcc.gnu.org/bugzilla/show_bug.cgi?id=50782
        #define LE_FAST_MATH_ON()  _Pragma( "GCC optimize ( \"associative-math\"    )" )
        #define LE_FAST_MATH_OFF() _Pragma( "GCC optimize ( \"no-associative-math\" )" )

        #define LE_FAST_MATH_ON_BEGIN()   \
            _Pragma( "GCC push_options" ) \
            LE_FAST_MATH_ON()

        #define LE_FAST_MATH_ON_END() _Pragma( "GCC pop_options" )

        #define LE_FAST_MATH_OFF_BEGIN()  \
            _Pragma( "GCC push_options" ) \
            LE_FAST_MATH_OFF()

        #define LE_FAST_MATH_OFF_END() _Pragma( "GCC pop_options" )
    #else
        #define LE_OPTIMIZE_FOR_SPEED_BEGIN()
        #define LE_OPTIMIZE_FOR_SPEED_END()
        #define LE_OPTIMIZE_FOR_SIZE_BEGIN()
        #define LE_OPTIMIZE_FOR_SIZE_END()
        #define LE_FAST_MATH_ON()
        #define LE_FAST_MATH_OFF()
        #define LE_FAST_MATH_ON_BEGIN()
        #define LE_FAST_MATH_ON_END()
        #define LE_FAST_MATH_OFF_BEGIN()
        #define LE_FAST_MATH_OFF_END()
    #endif // GCC 4.4+

    // http://llvm.org/docs/Vectorizers.html#pragma-loop-hint-directives
    #define LE_DISABLE_LOOP_VECTORIZATION() _Pragma( "clang loop vectorize( disable ) interleave( disable )" )
    #define LE_DISABLE_LOOP_UNROLLING()     _Pragma( "clang loop unroll( disable )" )


    // http://llvm.org/releases/3.6.0/tools/clang/docs/ReleaseNotes.html#the-exceptions-macro
    #if defined( __EXCEPTIONS ) && !( defined( __clang__ ) && !__has_feature( cxx_exceptions ) )
        #define LE_EXCEPTION_ON
    #endif // __EXCEPTIONS

#else

    #error Unkown compiler

#endif

#ifndef LE_ARR_SZ //...ugh vs2013 quick-hack...
    #define LE_ARR_SZ( x ) x.size()
#endif // LE_ARR_SZ

#define LE_DEFAULT_CASE_UNREACHABLE() default: LE_UNREACHABLE_CODE(); break

#ifdef NDEBUG
    #define LE_NDEBUG_NOVTABLE LE_NOVTABLE
#else
    #define LE_NDEBUG_NOVTABLE
#endif

#endif // platformSpecifics_hpp
