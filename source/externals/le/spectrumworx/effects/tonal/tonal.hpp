////////////////////////////////////////////////////////////////////////////////
///
/// \file tonal.hpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef tonal_hpp__71A9A670_AA87_4755_A67A_A61833B57203
#define tonal_hpp__71A9A670_AA87_4755_A67A_A61833B57203
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

namespace Detail
{
    struct TonalBase ///<
    {
        LE_DEFINE_PARAMETER( ( Strength        )( LinearFloat )( Minimum< 0> )( Maximum< 90> )( Default<15> )( Unit<' dB'> ) );
        LE_DEFINE_PARAMETER( ( GlobalThreshold )( LinearFloat )( Minimum<10> )( Maximum<120> )( Default<30> )( Unit<' dB'> ) );
        LE_DEFINE_PARAMETER( ( LocalThreshold  )( LinearFloat )( Minimum< 0> )( Maximum<120> )( Default<10> )( Unit<' dB'> ) );
        LE_DEFINE_PARAMETER( ( Attenuation     )( LinearFloat )( Minimum< 0> )( Maximum< 60> )( Default<20> )( Unit<' dB'> ) );

        /// \typedef Strength
        /// \brief How strong the peak must be to be considered tonal.
        /// \typedef GlobalThreshold
        /// \brief Global (across frames) threshold under which peaks are not
        /// detected.
        /// \typedef LocalThreshold
        /// \brief Local threshold (current frame only) under which peaks are not
        /// detected.
        /// \typedef Attenuation
        /// \brief Intensity of attenuation to apply to non-tonal parts.

        static bool const usesSideChannel = false;
    };
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
///
/// \class Tonal
///
/// \ingroup Effects
///
/// \brief Suppresses non-tonal parts of the signal. 
/// 
/// Lets through only highly-tonal frequencies. Finds peaks in the spectrum, 
/// then it estimates the peak strength (how strong the peak is when compared 
/// to its neighbours), and if the strength is above the value determined by 
/// the strength parameter, and if the peak is stronger than that determined 
/// by the global threshold or local threshold, the peak will be passed through. 
/// The rest of the signal is attenuated.
///
////////////////////////////////////////////////////////////////////////////////

struct Tonal : Detail::TonalBase
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Strength        ) )
        ( ( GlobalThreshold ) )
        ( ( LocalThreshold  ) )
        ( ( Attenuation     ) )
    );

    static char const title      [];
    static char const description[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class Atonal
///
/// \ingroup Effects
///
/// \brief Suppresses tonal parts of the signal. 
/// 
/// Attenuates peaks and allows the non-peak components to pass through.
/// 
////////////////////////////////////////////////////////////////////////////////

struct Atonal : Detail::TonalBase
{
    /// \name Parameters
    /// @{
    typedef Detail::TonalBase::LocalThreshold LocalThreshold;
    /// @}

    LE_DEFINE_PARAMETERS
    (
        ( ( Strength        )( Detail::TonalBase::Strength        )( Default< 0> )( ValuesDenominator<1> ) )
        ( ( GlobalThreshold )( Detail::TonalBase::GlobalThreshold )( Default<60> )( ValuesDenominator<1> ) )
        ( ( LocalThreshold  ) )
        ( ( Attenuation     )( Detail::TonalBase::Attenuation     )( Default<30> )( ValuesDenominator<1> ) )
    );

    /// \typedef Strength
    /// \brief How strong the peak must be to be considered tonal.
    /// \typedef GlobalThreshold
    /// \brief Global (across frames) threshold under which peaks are not
    /// detected.
    /// \typedef LocalThreshold
    /// \brief Local threshold (current frame only) under which peaks are not
    /// detected.
    /// \typedef Attenuation
    /// \brief Intensity of attenuation to apply to non-tonal parts.

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

#endif // tonal_hpp
