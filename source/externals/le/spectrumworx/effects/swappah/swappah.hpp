////////////////////////////////////////////////////////////////////////////////
///
/// \file swappah.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef swappah_hpp__8B69F4EC_B747_4300_B33F_701BABBE80F9
#define swappah_hpp__8B69F4EC_B747_4300_B33F_701BABBE80F9
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/spectrumworx/effects/commonParameters.hpp"
#include "le/parameters/enumerated/parameter.hpp"
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
/// \class Swappah
///
/// \ingroup Effects
///
/// \brief Swaps three spectral bands, low, mid and high.
///
/// Swaps the three frequency bands determined by the band borders. Swap order 
/// is determined by the Swap order parameter. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct Swappah
{
    /// \name Parameters
    /// @{
    typedef CommonParameters::Mode Mode;
    /// @}

    LE_ENUMERATED_PARAMETER( BandOrder, ( LowHighMid )( MidLowHigh )( MidHighLow )( HighLowMid )( HighMidLow ) );
    
    LE_DEFINE_PARAMETERS
    (
        ( ( Mode        ) )
        ( ( BandOrder   ) )
        ( ( BandLowMid  )( LinearUnsignedInteger )( Minimum<0> )( Maximum<100> )( Default<33> )( Unit<' bw%'> ) )
        ( ( BandMidHigh )( LinearUnsignedInteger )( Minimum<0> )( Maximum<100> )( Default<66> )( Unit<' bw%'> ) )
    );

    /// \typedef Mode
    /// \brief Specifies what is to be swapped.
    /// \details
    ///   - Magnitudes: swap only magnitudes.
    ///   - Phases: swap only phases.
    ///   - Both: swap both magnitudes and phases.
    /// \typedef BandOrder
    /// \brief Determines the swapping order.
    /// \details
    ///    - LowHighMid: low-high-mid is output.
    ///    - MidLowHigh: mid-low-high is output.
    ///    - MidHighLow: mid-high-low is output.
    ///    - HighLowMid: high-low-mid is output.
    ///    - HighMidLow: high-mid-low is output.     
    /// \typedef BandLowMid
    /// \brief Determines the border between low and mid band.
    /// \typedef BandMidHigh
    /// \brief Determines the border between mid and high band.

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

#endif // swappah_hpp
