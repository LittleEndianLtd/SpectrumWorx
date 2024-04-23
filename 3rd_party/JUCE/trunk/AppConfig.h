////////////////////////////////////////////////////////////////////////////////
///
/// \file AppConfig.h
/// -----------------
///
/// JUCE library configuration file for LE projects.
///
/// Copyright (c) 2013 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef AppConfig_h__BD7E2B65_D22B_4AB8_8791_E417B08BCB47
#define AppConfig_h__BD7E2B65_D22B_4AB8_8791_E417B08BCB47
#pragma once
//------------------------------------------------------------------------------
#ifdef _WIN32
#include "windows.h"
#endif // _WIN32

#include <cstdlib>
#include <cstring>
//------------------------------------------------------------------------------

/// \note This has to be defined to stop JUCE from thinking that we are building
/// a standalone application (and thus using unsupported features like
/// __declspec( thread ) in a DLL under WinXP).
///                                           (07.10.2013.) (Domagoj Saric)
#define JucePlugin_PluginCode

#define JUCE_MODULE_AVAILABLE_juce_core            1
#define JUCE_MODULE_AVAILABLE_juce_data_structures 1
#define JUCE_MODULE_AVAILABLE_juce_events          1
#define JUCE_MODULE_AVAILABLE_juce_graphics        1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics      1

#define JUCE_MODAL_LOOPS_PERMITTED 1 //...mrmlj...AlertWindow::showNativeDialogBox()...
#define JUCE_OPENGL 0

#define JUCE_USE_FREETYPE 0
#define JUCE_USE_FLAC 0
#define JUCE_USE_OGGVORBIS 0
#define JUCE_USE_CDBURNER 0
#define JUCE_USE_CDREADER 0
#define JUCE_USE_CAMERA 0
#define JUCE_USE_XINERAMA 0

#define JUCE_PLUGINHOST_VST 0
#define JUCE_PLUGINHOST_AU 0
#define JUCE_WEB_BROWSER 0

#define JUCE_INCLUDE_FLAC_CODE        0
#define JUCE_INCLUDE_OGGVORBIS_CODE   0
#define JUCE_INCLUDE_JPEGLIB_CODE     0


//#define JUCE_HANDLE_MULTIPLE_INSTANCES 0


#ifdef NDEBUG
    #define JUCE_FORCE_DEBUG 0
    #define JUCE_LOG_ASSERTIONS 0
    #define JUCE_CHECK_MEMORY_LEAKS 0
    #define JUCE_CATCH_UNHANDLED_EXCEPTIONS 0
    #define JUCE_CATCH_DEPRECATED_CODE_MISUSE 0
    #define JUCE_LOG_COREMIDI_ERRORS 0
#else
    #define JUCE_FORCE_DEBUG 1
    #define JUCE_LOG_ASSERTIONS 1
    #define JUCE_CHECK_MEMORY_LEAKS 1
    //#define JUCE_ENABLE_REPAINT_DEBUGGING 1
    #define JUCE_CATCH_UNHANDLED_EXCEPTIONS 0 //...mrmlj...1 requires the JUCEApplication class...
    #define JUCE_CATCH_DEPRECATED_CODE_MISUSE 1
    #define JUCE_LOG_COREMIDI_ERRORS 1
#endif // NDEBUG


#ifdef __APPLE__
    #define USE_COREGRAPHICS_RENDERING 1
    #define JUCE_USING_COREIMAGE_LOADER 1
    #define JUCE_USE_COREIMAGE_LOADER 1
    #define PNG_NO_WRITE_SUPPORTED    1
    #define JUCE_INCLUDE_PNGLIB_CODE  0
    #define JUCE_INCLUDE_ZLIB_CODE    0
    #define JUCE_USE_VDSP_FRAMEWORK   1
    #define JUCE_QUICKTIME 0

    #if defined( __LP64__ )
        #define JUCE_SUPPORT_CARBON 0
    #else
        //#define JUCE_SUPPORT_CARBON 1
    #endif
#endif // __APPLE__

#ifdef _WIN32
    #define JUCE_ASIO 0
    #define JUCE_WASAPI 0
    #define JUCE_DIRECTSOUND 0
    #define JUCE_DIRECTSHOW 0
    #define JUCE_MEDIAFOUNDATION 0
    #define JUCE_DIRECT2D 0

    #define PNG_NO_WRITE_SUPPORTED    1
    #define JUCE_INCLUDE_PNGLIB_CODE  1
    #define JUCE_INCLUDE_ZLIB_CODE    1

    #define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES 1
    #define JUCE_USE_DIRECTWRITE 0
#endif // _WIN32

#ifdef _linux
    #define JUCE_ALSA 0
    #define JUCE_JACK 0
    #define JUCE_USE_XSHM 1
    #define JUCE_USE_XRENDER 1
    #define JUCE_USE_XCURSOR 1
#endif // _linux



#define LE_PATCHED_JUCE


#ifdef LE_PATCHED_JUCE
    #define LE_PATCH(      ... ) __VA_ARGS__
    #define JUCE_ORIGINAL( ... )

    #ifdef _MSC_VER
        #define LE_WEAK_SYMBOL       __declspec( selectany )
        #define LE_WEAK_SYMBOL_CONST __declspec( selectany ) extern
        #define LE_PATCH_UNREACHABLE_CODE    jassertfalse; __assume( false );
        #define LE_PATCH_ASSUME_IMPL( condition ) __assume( condition )
        #define LE_PATCH_NOVTABLE __declspec( novtable )
        #define LE_PATCH_WINDOWS( ... ) __VA_ARGS__
    #else
        #define LE_WEAK_SYMBOL       __attribute__(( weak ))
        #define LE_WEAK_SYMBOL_CONST __attribute__(( weak )) extern
        #define LE_PATCH_UNREACHABLE_CODE    jassertfalse; __builtin_unreachable();
        #define LE_PATCH_ASSUME_IMPL( condition ) do { if ( !(condition) ) __builtin_unreachable(); } while ( 0 )
        #define LE_PATCH_NOVTABLE
        #define LE_PATCH_WINDOWS( ... )
    #endif
#else
    #define LE_PATCH(      ... )
    #define JUCE_ORIGINAL( ... ) __VA_ARGS__
    #define LE_WEAK_SYMBOL
    #define LE_WEAK_SYMBOL_CONST
    #define LE_PATCH_UNREACHABLE_CODE
    #define LE_PATCH_ASSUME_IMPL( condition )
    #define LE_PATCH_NOVTABLE
    #define LE_PATCH_WINDOWS( ... )
#endif
#define LE_PATCH_ASSUME( condition ) { jassert( condition ); LE_PATCH_ASSUME_IMPL( condition ); }


#ifdef LE_PATCHED_JUCE
    #if defined( _WIN32 ) && defined( _UNICODE )
        #define JUCE_STRING_UTF_TYPE 16
    #endif // Win32 unicode
#endif // LE_PATCHED_JUCE

//------------------------------------------------------------------------------
#endif // AppConfig_h
