////////////////////////////////////////////////////////////////////////////////
///
/// \file centroidExtractor.hpp
/// ---------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef centroidExtractor_hpp__91D2BE08_ACCE_4C50_B172_CCB947A246DC
#define centroidExtractor_hpp__91D2BE08_ACCE_4C50_B172_CCB947A246DC
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
/// \class CentroidExtractor
///
/// \ingroup Effects
///
/// \brief Adaptively finds the spectral centre and preserves the spectrum
/// around it.
///
/// Centroid is an adaptive, self-tuning band pass filter which follows the 
/// centre frequency with a controllable symmetric band curve (slope). 
/// The module determines the time variable centre frequency with different 
/// processing modes. Frequencies outside the region around the centroid 
/// frequency are removed, the rest inside the band is smoothed.
///
////////////////////////////////////////////////////////////////////////////////

struct CentroidExtractor
{
    LE_ENUMERATED_PARAMETER( Mode, ( Centroid )( Peak )( Dominant ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Mode        ) )
        ( ( Bandwidth   )( LinearUnsignedInteger )( Minimum<0> )( Maximum<6000> )( Default<1000> )( Unit<' Hz'> ) )
        ( ( Attenuation )( LinearSignedInteger   )( Minimum<0> )( Maximum< 100> )( Default<  10> )( Unit<' dB'> ) )
    );

    /// \typedef Mode
    /// \brief
    ///   - Centroid: finds the weighted centre frequency.
    ///   - Peak: finds the highest peak in the spectrum.
    ///   - Dominant: estimates the dominant pitch.
    /// \typedef Bandwidth
    /// \brief Frequencies around the centroid frequency outside the Bandwidth
    /// region are removed.
    /// \typedef Attenuation
    /// \brief Intensity of attenuation.


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

#endif // centroidExtractor_hpp
