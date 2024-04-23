////////////////////////////////////////////////////////////////////////////////
///
/// \file shapeless.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef shapeless_hpp__DBB27280_19FF_4CF5_B1E6_A9CFB52749B7
#define shapeless_hpp__DBB27280_19FF_4CF5_B1E6_A9CFB52749B7
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
/// \class Shapeless
///
/// \ingroup Effects
///
/// \brief Applies frequency shape from side channel.
///
/// Transfers frequency shape from the side-channel to the input signal. Width 
/// parameter regulates the coarseness of the shape estimators.
/// 
////////////////////////////////////////////////////////////////////////////////

struct Shapeless
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Width )( LinearUnsignedInteger )( Minimum<0> )( Maximum<4000> )( Default< 200> )( Unit<' Hz'> ) )
    );

    /// \typedef Width
    /// \brief Width of the region to collect the shape from.
    
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

#endif // shapeless_hpp
