////////////////////////////////////////////////////////////////////////////////
///
/// \file intrinsics.hpp
/// --------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef intrinsics_hpp__B8B5626D_0778_4184_BA27_3999746AEA17
#define intrinsics_hpp__B8B5626D_0778_4184_BA27_3999746AEA17
#pragma once
//------------------------------------------------------------------------------
#if defined( _XBOX )
    #include "ppcintrinsics.h"
    #include "vectorintrinsics.h"
#elif defined( __ARM_NEON__ ) || defined( __aarch64__ )
    #include "arm_neon.h"
#else
    #include "platformSpecifics.hpp"
    #if defined( _MSC_VER )
        #include "intrin.h"
    #endif // _MSC_VER
    #ifdef LE_HAS_SSE1
        #include "xmmintrin.h"
    #endif // LE_HAS_SSE1
    #ifdef LE_HAS_SSE2
        #include "emmintrin.h"
    #endif // LE_HAS_SSE1
#endif
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

#if defined( _XBOX )
    typedef __vector4 SIMDVector;
#elif defined( __ARM_NEON__ )
    typedef float32x4_t SIMDVector;
#elif defined( LE_HAS_SSE1 )
    typedef __m128 SIMDVector;
#elif defined( __GNUC__ )
    typedef float SIMDVector __attribute__(( vector_size( 16 ) ));
#else
    typedef LE_ALIGN( 16 ) float SIMDVector[ 4 ];
#endif

namespace Constants
{
    unsigned int const vectorAlignment = sizeof( SIMDVector );
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // intrinsics_hpp
