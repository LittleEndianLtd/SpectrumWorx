////////////////////////////////////////////////////////////////////////////////
///
/// \file denoiser.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef denoiser_hpp__8FEB141B_4DF4_4E5B_ABDE_D546693193E0
#define denoiser_hpp__8FEB141B_4DF4_4E5B_ABDE_D546693193E0
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
/// \class Denoiser
///
/// \ingroup Effects
///
/// \brief Performs denoising optionally using an external noise footprint. 
///
/// Applies the standard de-noising formula. Uses the side-channel input as a 
/// noise footprint or a sum of Main and Side-channel. The main-channel will be 
/// attenuated when the side-channel contains noise (in the latter case). 
///
////////////////////////////////////////////////////////////////////////////////

struct Denoiser
{
    LE_ENUMERATED_PARAMETER( Mode, ( Main )( Side )( Sum ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Mode      ) )
        ( ( Intensity ) ( LinearUnsignedInteger )( Minimum<1> )( Maximum<100> )( Default<5> )( Unit<' %'> ) )
    );

    /// \typedef Mode
    /// \brief Controls the noise footprint.
    /// \details
    ///   - Main: uses main-channel as the noise footprint.
    ///   - Side: uses side-channel as the noise footprint.
    ///   - Sum: uses sum of main and side channels as noise footprint.
    /// \typedef Intensity
    /// \brief Controls the intensity of the de-noising algorithm.


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

#endif // denoiser_hpp
