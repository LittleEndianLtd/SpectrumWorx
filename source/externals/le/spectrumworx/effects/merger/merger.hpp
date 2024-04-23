////////////////////////////////////////////////////////////////////////////////
///
/// \file merger.hpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef merger_hpp__6BCD6590_5CD7_40EE_9566_2D273B6A9B1C
#define merger_hpp__6BCD6590_5CD7_40EE_9566_2D273B6A9B1C
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
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
/// \class Merger
///
/// \ingroup Effects
///
/// \brief Combines two samples.
///
/// This effect provides conditional combinations of the input signal with that 
/// of the side channel. Copies the side channel to the input channel if the 
/// selected conditions are met, and depending on the threshold setting. 
///
////////////////////////////////////////////////////////////////////////////////

struct Merger
{
    LE_ENUMERATED_PARAMETER( Operation, ( MainLargerThanSide )( SideLargerThanMain )( MainAboveThreshold )( SideAboveThreshold )( MainBelowThreshold )( SideBelowThreshold ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Operation ) )
        ( ( Threshold )( LinearFloat )( Minimum<-120> )( Maximum<0> )( Default<-20> )( Unit<' dB'> ) )
    );

    /// \typedef Operation
    /// \brief Defines the operation to be executed.
    /// \details
    ///   - MainLargerThanSide: replaces main channel with side where main larger than side.
    ///   - SideLargerThanMain: replaces main channel with side where side larger than main.
    ///   - MainAboveThreshold: replaces main channel with side where main above threshold.
    ///   - SideAboveThreshold: replaces main channel with side where side above threshold.
    ///   - MainBelowThreshold: replaces main channel with side where main below threshold.
    ///   - SideBelowThreshold: replaces main channel with side where side above threshold.
    /// \typedef Threshold
    /// \brief Threshold used for comparisons.


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

#endif // merger_hpp
