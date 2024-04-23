////////////////////////////////////////////////////////////////////////////////
///
/// \file inserter.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef inserter_hpp__FBE18C67_5D60_4991_A6F1_5E5AAB7BA9B9
#define inserter_hpp__FBE18C67_5D60_4991_A6F1_5E5AAB7BA9B9
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/spectrumworx/effects/commonParameters.hpp"
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
/// \class Inserter
///
/// \ingroup Effects
///
/// \brief Insert side into main channel with selectable offsets and size.
/// 
/// Inserts a band of frequency bins from the side-channel into the input. Source
/// value is the position (given in percentages) from where the spectrum is copied.
/// The Destination is the target position in the input channel's spectrum. The
/// Size parameter controls how much of the spectrum is copied.
/// 
/// \note "bw%" refers to bandwidth percentage. 
///
////////////////////////////////////////////////////////////////////////////////

struct Inserter
{
    /// \name Parameters
    /// @{
    typedef CommonParameters::Mode Mode;
    /// @}

    LE_DEFINE_PARAMETERS
    (
        ( ( Mode        ) )
        ( ( Source      )( LinearFloat )( Minimum<0> )( Maximum<100> )( Default<0> )( Unit<' bw%'> ) )
        ( ( Destination )( LinearFloat )( Minimum<0> )( Maximum<100> )( Default<0> )( Unit<' bw%'> ) )
        ( ( InsertSize  )( LinearFloat )( Minimum<0> )( Maximum<100> )( Default<5> )( Unit<' bw%'> ) )
    );

    /// \typedef Mode
    /// \brief Specifies what is to be inserted.
    /// \details
    ///   - Magnitudes: insert only magnitudes.
    ///   - Phases    : insert only phases.
    ///   - Both      : insert both magnitudes and phases.
    /// \typedef Source
    /// \brief Position (given in percentages) from where the spectrum is copied.
    /// \typedef Destination
    /// \brief Position in the input channel's spectrum.
    /// \typedef InsertSize
    /// \brief Controls how much of the spectrum is copied.


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

#endif // inserter_hpp
