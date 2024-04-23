////////////////////////////////////////////////////////////////////////////////
///
/// \file armonizer.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef armonizer_hpp__D2DC61B5_FEEC_4FD5_B4DA_E89976A55507
#define armonizer_hpp__D2DC61B5_FEEC_4FD5_B4DA_E89976A55507
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/symmetric/parameter.hpp"

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
/// \class Armonizer
///
/// \ingroup Effects
///
/// \brief Pitch harmonizer.
///
/// Adds a pitch-shifted copy of an incoming signal to the original. Blending 
/// of the original and shifted signals can be controlled with the
/// BaseParameters::Wet parameter. In the special case of the Armonizer
/// effect, this (Wet) parameter defaults to the value of 50% (as opposed to
/// 100%).
///
////////////////////////////////////////////////////////////////////////////////

struct Armonizer
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Interval )( SymmetricFloat )( MaximumOffset<24> )( Unit<'\''> ) )
    );

    /// \typedef Interval
    /// \brief Controls the amount of pitch shifting.

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

#endif // armonizer_hpp
