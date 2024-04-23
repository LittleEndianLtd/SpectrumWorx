////////////////////////////////////////////////////////////////////////////////
///
/// \file entryPoint.hpp
/// --------------------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef entryPoint_hpp__DA756505_6151_4AE5_A469_AB88CEB5C1AF
#define entryPoint_hpp__DA756505_6151_4AE5_A469_AB88CEB5C1AF
#pragma once
//------------------------------------------------------------------------------
#include "trace.hpp"

#ifdef __ANDROID__
#include "assert.hpp"
#include "filesystem.hpp"

#include "android/native_activity.h"
#include "android_native_app_glue.h"

#include "unistd.h"
#else
#include <cstdlib>
#endif // __ANDROID__
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
    #define LE_UTILITY_ENTRY_POINT_AUX() ::pause()
#else
    #define LE_UTILITY_ENTRY_POINT_AUX()
#endif

#ifdef __ANDROID__
#define LE_UTILITY_ENTRY_POINT( userFunction )                                  \
    extern "C" void android_main( android_app * const state )                   \
    {                                                                           \
        /* Give the debugger time to attach:*/                                  \
        sleep( 3 );                                                             \
                                                                                \
        app_dummy();                                                            \
                                                                                \
        LE_ASSERT( state->savedState == nullptr );                              \
                                                                                \
        LE::Utility::setAppContext( *state->activity );                         \
                                                                                \
        userFunction();                                                         \
                                                                                \
        ::ANativeActivity_finish( state->activity );                            \
                                                                                \
        sleep( 5 );                                                             \
    }
#else
#define LE_UTILITY_ENTRY_POINT( userFunction )                                  \
    extern "C" int main( int const argc, char const * const /*argv*/[] )        \
    {                                                                           \
        if ( argc > 1 )                                                         \
        {                                                                       \
            LE::Utility::Tracer::error                                          \
            (                                                                   \
                "The example app does not currently support any "               \
                "command line arguments."                                       \
            );                                                                  \
            return EXIT_FAILURE;                                                \
        }                                                                       \
        bool const success( userFunction() );                                   \
        LE_UTILITY_ENTRY_POINT_AUX();                                           \
        return success ? EXIT_SUCCESS : EXIT_FAILURE;                           \
    }
#endif // OS

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // entryPoint_hpp
