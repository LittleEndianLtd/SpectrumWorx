////////////////////////////////////////////////////////////////////////////////
///
/// \file burrito.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef burrito_hpp__C27A00BA_023C_4D1F_9742_F431F45C6018
#define burrito_hpp__C27A00BA_023C_4D1F_9742_F431F45C6018
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/enumerated/parameter.hpp"
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
/// \class Burrito
///
/// \ingroup Effects 
///
/// \brief Random spectrum combination.
///
/// Copies side-channel data to the input at random locations. 
///
////////////////////////////////////////////////////////////////////////////////

struct Burrito
{
    LE_ENUMERATED_PARAMETER( Mode, ( Replace )( Sum ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Mode     )                                                                                          )
        ( ( Range    )( LinearUnsignedInteger )( Minimum< 0> )( Maximum< 100> )( Default< 50> )( Unit<' bw%'> ) )
        ( ( Period   )( LinearUnsignedInteger )( Minimum<10> )( Maximum<2000> )( Default<250> )( Unit<' ms' > ) )
        ( ( SideGain )( SymmetricInteger      )( MaximumOffset<24> )                           ( Unit<' dB' > ) )
    );

    /// \typedef Mode
    /// \brief Combination mode.
    /// \details
    ///   - Replace: input is replaced by the side-channel at random locations.
    ///   - Sum: input is summed with side-channel at random locations.
    /// \typedef Range
    /// \brief Maximum amount of spectrum to randomize.
    /// \details (in \%bw, percentage of bandwidth i.e. total
    /// frequency range)
    /// \typedef Period
    /// \brief Period in which the random locations are kept constant.
    /// \typedef SideGain
    /// \brief Gain applied to side-channel.


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

#endif // burrito_hpp
