////////////////////////////////////////////////////////////////////////////////
///
/// \file blender.hpp
/// -----------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef blender_hpp__F2596834_8722_40A2_B5AB_B2E91D94182A
#define blender_hpp__F2596834_8722_40A2_B5AB_B2E91D94182A
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
/// \class Blender
///
/// \ingroup Effects
///
/// \brief Linear blend of two signals.
///
/// Input and side channel frequencies are combined.
///
////////////////////////////////////////////////////////////////////////////////

struct Blender
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Amount )( LinearUnsignedInteger )( Minimum<0> )( Maximum<100> )( Default<30> )( Unit<' %'> ) )
    );

    /// \typedef Amount
    /// \brief Controls how much of each channel is sent to the output.

    static bool const usesSideChannel = true;

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

#endif // blender_hpp
