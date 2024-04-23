////////////////////////////////////////////////////////////////////////////////
///
/// \file precompiledHeaders.hpp
/// ----------------------------
///
/// Little Endian global precompiled headers file.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef precompiledHeader_hpp__9E869819_C6CF_4497_8151_E469F29AEd78
#define precompiledHeader_hpp__9E869819_C6CF_4497_8151_E469F29AEd78
#ifdef _MSC_VER
    #pragma once
#endif // _MSC_VER
//------------------------------------------------------------------------------

#include "leConfigurationAndODRHeader.h"

// Implementation note:
//   Disable precompiled headers for Objective-C(++) sources as we usually only
// have a few simple such sources and it does not pay off to build the entire
// PCH because of those.
//                                            (12.05.2011.) (Domagoj Saric)
#ifndef __OBJC__


#if defined( __APPLE__ ) && !defined( __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES )
    #define __ASSERT_MACROS_DEFINE_VERSIONS_WITHOUT_UNDERSCORES 0
#endif


#ifdef LEB_PRECOMPILE_OS_HEADERS

////////////////////////////////////////////////////////////////////////////////
// Apple
////////////////////////////////////////////////////////////////////////////////

#ifdef __APPLE__
    // http://stackoverflow.com/questions/1083541/built-in-preprocessor-token-to-detect-iphone-platform
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE
        #include "Accelerate/Accelerate.h"
        #include "CFNetwork/CFNetwork.h"
    #else
        #include "AudioToolbox/AudioToolbox.h"
        #include "Carbon/Carbon.h"
        #include "CoreFoundation/CoreFoundation.h"
        #include "CoreServices/CoreServices.h"
    #endif
    #undef nil
#endif // __APPLE__


////////////////////////////////////////////////////////////////////////////////
// Windows
////////////////////////////////////////////////////////////////////////////////

// Implementation note:
//   Windows headers are not included/precompiled by default because this
// actually slows down compilation unless windows.h is actually included in a
// lot of headers.
//                                            (28.06.2011.) (Domagoj Saric)
#ifdef _WIN32
    #include "windows.h"
    #include "commdlg.h"
    #include "shellapi.h"
    #include "shlobj.h"
    #include "shlwapi.h"
#endif // _WIN32

#endif // LEB_PRECOMPILE_OS_HEADERS


/// \todo To avoid duplication we include these two utility headers of our own
/// here. This requires every project that uses the PCH file to also contain
/// these files at this location. Think of a cleaner solution.
///                                           (13.02.2012.) (Domagoj Saric)
#include "le/utility/intrinsics.hpp"
#include "le/utility/tchar.hpp"


////////////////////////////////////////////////////////////////////////////////
// CRT
////////////////////////////////////////////////////////////////////////////////

#include <cassert>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <type_traits>
#include <utility>

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4100 ) // Unreferenced formal parameter.
    #include "tchar.h"
    #pragma warning( pop )
#endif // _MSC_VER


////////////////////////////////////////////////////////////////////////////////
// STL
////////////////////////////////////////////////////////////////////////////////

#if defined( _MSC_VER ) && !defined( _CPPUNWIND ) && !defined( NDEBUG )
    #pragma warning( push )
    #pragma warning( disable : 4530 ) // C++ exception handler used, but unwind semantics are not enabled.
    #include <istream>
    #include <xlocale>
    #include <xstring>
    #pragma warning( pop )
#endif

#include <algorithm>
#include <array>
#include <limits>
#include <numeric>
#include <utility>


////////////////////////////////////////////////////////////////////////////////
// Boost
////////////////////////////////////////////////////////////////////////////////

/// \todo See if there is a cleaner way to silence this warning.
///                                           (03.12.2010.) (Domagoj Saric)
#pragma warning( disable : 4512 ) // assignment operator could not be generated

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4324 ) // structure was padded due to __declspec(align()).
#include "boost/aligned_storage.hpp"
#pragma warning( pop )
#endif // MSVC
#if !( defined( _MSC_VER ) || defined( _LIBCPP_VERSION ) || !defined( BOOST_NO_CXX11_HDR_ATOMIC ) ) //...mrmlj...Boost 1.59 still defines BOOST_NO_CXX11_HDR_ATOMIC for libc++ although 'it works for us'...
#define BOOST_ATOMIC_NO_LIB
#include "boost/atomic/atomic.hpp"
#endif // BOOST_NO_CXX11_HDR_ATOMIC
#if !defined( BOOST_NO_RTTI ) && ( !defined( BOOST_NO_EXCEPTIONS ) || defined( _MSC_VER ) )
    #include "boost/polymorphic_cast.hpp"
#endif // BOOST_NO_EXCEPTIONS
#include "boost/checked_delete.hpp"
#include "boost/concept_check.hpp"
#include "boost/detail/endian.hpp"
#include "boost/fusion/algorithm/iteration/for_each.hpp"
#include "boost/fusion/iterator/value_of.hpp"
#include "boost/fusion/sequence/intrinsic/at.hpp"
#include "boost/fusion/sequence/intrinsic/begin.hpp"
#include "boost/fusion/sequence/intrinsic/end.hpp"
#include "boost/fusion/sequence/intrinsic/value_at.hpp"
#include "boost/fusion/support/category_of.hpp"
#include "boost/integer/static_log2.hpp"
#include "boost/mpl/at.hpp"
#include "boost/mpl/bool.hpp"
#include "boost/mpl/integral_c.hpp"
#include "boost/mpl/pair.hpp"
#include "boost/mpl/range_c.hpp"
#include "boost/mpl/size.hpp"
#include "boost/mpl/string.hpp"
#include "boost/noncopyable.hpp"
#include "boost/preprocessor/cat.hpp"
#include "boost/preprocessor/comparison/greater.hpp"
#include "boost/preprocessor/control/iif.hpp"
#include "boost/preprocessor/facilities/empty.hpp"
#include "boost/preprocessor/punctuation/comma_if.hpp"
#include "boost/preprocessor/seq/seq.hpp"
#include "boost/preprocessor/seq/for_each.hpp"
#include "boost/preprocessor/seq/for_each_i.hpp"
#include "boost/preprocessor/seq/enum.hpp"
#include "boost/preprocessor/seq/transform.hpp"
#include "boost/range/iterator_range_core.hpp"
#include "boost/smart_ptr/intrusive_ptr.hpp"
#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.
#include "boost/utility/in_place_factory.hpp"
#pragma warning( pop )
#include "boost/utility/string_ref.hpp"


////////////////////////////////////////////////////////////////////////////////
// NT2/Boost.SIMD
////////////////////////////////////////////////////////////////////////////////

#ifdef LEB_PRECOMPILE_NT2
    #include "boost/simd/sdk/simd/extensions.hpp"
#endif // LEB_PRECOMPILE_NT2


////////////////////////////////////////////////////////////////////////////////
// RapidXML
////////////////////////////////////////////////////////////////////////////////

#ifdef LEB_PRECOMPILE_RapidXML
    //...
#endif


////////////////////////////////////////////////////////////////////////////////
// JUCE
////////////////////////////////////////////////////////////////////////////////

#ifdef LEB_PRECOMPILE_JUCE
    #ifdef __APPLE__ //...mrmlj...compilation errors quick-fix...
        #include "CoreServices/CoreServices.h"
    #endif // __APPLE__
    #include "juce/AppConfig.h"
    #include "juce/juce_core/juce_core.h"
    #include "juce/juce_data_structures/juce_data_structures.h"
    #include "juce/juce_events/juce_events.h"
    #include "juce/juce_graphics/juce_graphics.h"
    #include "juce/juce_gui_basics/juce_gui_basics.h"
#endif

#endif // __OBJC__

//------------------------------------------------------------------------------
#endif // precompiledHeader_hpp
