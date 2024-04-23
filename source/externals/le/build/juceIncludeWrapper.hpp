////////////////////////////////////////////////////////////////////////////////
///
/// \file juceIncludeWrapper.hpp
/// ----------------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef juceIncludeWrapper_hpp__0663230B_81C4_447E_B613_6104476D8AB6
#define juceIncludeWrapper_hpp__0663230B_81C4_447E_B613_6104476D8AB6
#pragma once
//------------------------------------------------------------------------------
#define JUCE_NAMESPACE juce
#define DONT_SET_USING_JUCE_NAMESPACE
#define DONT_AUTOLINK_TO_JUCE_LIBRARY
#define JUCE_DONT_DEFINE_MACROS 1
//#define DONT_LIST_JUCE_AUTOLINKEDLIBS
//...mrmlj...this macro will probably dissapear...cleanup asap...
#ifndef JUCE_DLL
    //#pragma comment( lib, "Imm32.lib"   )
    //#pragma comment( lib, "Vfw32.lib"   )
    //#pragma comment( lib, "Wininet.lib" )
#endif


#if defined( __APPLE__ ) && !defined( __OBJC__ )
    // Implementation note:
    //   Workaround for the Apple headers<->Fusion::nil name clash by first
    // including the offending Fusion header.
    //                                        (18.11.2011.) (Domagoj Saric)
    #include "boost/fusion/support/segmented_fold_until.hpp"
#endif // __APPLE__

#ifdef __GNUC__
    #pragma GCC visibility push( hidden )
#endif // __GNUC__

#ifdef __APPLE__
    //...mrmlj...juce-osx Point class name clash workaround...
    #include "CoreServices/CoreServices.h"
#endif // __APPLE__

//#include "juce.h"

#ifdef __GNUC__
    #pragma GCC visibility pop
#endif // __GNUC__

#undef T
#undef zeromem

//...mrmlj...NEW_JUCE...UNICODE
//typedef juce::String::CharPointerType::CharType char_t;

//------------------------------------------------------------------------------
#endif // juceIncludeWrapper_hpp
