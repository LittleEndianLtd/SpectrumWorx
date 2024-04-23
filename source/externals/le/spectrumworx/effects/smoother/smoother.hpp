////////////////////////////////////////////////////////////////////////////////
///
/// \file smoother.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef smoother_hpp__17DDFA09_71A5_4CDD_9A50_B60BB89DD53B
#define smoother_hpp__17DDFA09_71A5_4CDD_9A50_B60BB89DD53B
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/linear/parameter.hpp"

#include "boost/config/abi_prefix.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Smoother
///
/// \ingroup Effects
///
/// \brief Smooth spectrum with a lowpass filter. 
///
/// "Smoothes" the frequency spectrum of the incoming signal, causing it to 
/// lose sharpness and detail for a blurred image.
/// 
////////////////////////////////////////////////////////////////////////////////

struct Smoother
{
    LE_DEFINE_PARAMETERS
    (    
        ( ( AveragingWidth )( LinearUnsignedInteger )( Minimum<0> )( Maximum<2000> )( Default<500> )( Unit<' Hz'> ) )
    );

    /// \typedef AveragingWidth
    /// \brief Width of the region to be smoothed.

    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // smoother_hpp
