////////////////////////////////////////////////////////////////////////////////
///
/// \file ahAh.hpp
/// --------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef ahah_hpp__D12349D97_2DB9_4249_87EF_851D727FAF71
#define ahah_hpp__D12349D97_2DB9_4249_87EF_851D727FAF71
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/linear/parameter.hpp"
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
/// \class AhAh
///
/// \ingroup Effects
///
/// \brief Wah wah.
///
/// A kind of a wah-wah. It filters a region with a variable centre frequency.
/// An LFO must be used to modulate the Center frequency knob for the intended
/// effect. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct AhAh
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Center   )( LinearUnsignedInteger )( Minimum< 0> )( Maximum<6000> )( Default<2000> )( Unit<' Hz'> ) )
        ( ( Width    )( LinearUnsignedInteger )( Minimum<50> )( Maximum<2000> )( Default<1000> )( Unit<' Hz'> ) )
        ( ( Strength )( SymmetricFloat        )( MaximumOffset<24> )                            ( Unit<' dB'> ) )
    );

    /// \typedef Center
    /// \brief Filter center frequency.
    /// \typedef Width
    /// \brief Bandwidth of the filter.
    /// \typedef Strength
    /// \brief Amplification of the moving region.

    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
}; // struct AhAh

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // ahAh_hpp
