////////////////////////////////////////////////////////////////////////////////
///
/// \file bandpass.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef bandpass_hpp__3E2335FA_F97E_47A9_9BED_977505A90353
#define bandpass_hpp__3E2335FA_F97E_47A9_9BED_977505A90353
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
    struct BandGain
    {
        LE_DEFINE_PARAMETERS
        (
            ( ( Attenuation )( LinearFloat )( Minimum<0> )( Maximum<60> )( Default<0> )( Unit<' dB'> ) )
        );

        /// \typedef Attenuation
        /// \brief Amount of signal attenuation (within the selected frequency
        /// band).

        static bool const usesSideChannel = false;
    };
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class Bandpass
///
/// \ingroup Effects
///
/// \brief Band pass filter.
///
/// A simple bandpass filter with a variable pass band. The pass band is defined
/// using the standard StartFrequency and StopFrequency parameters. This effect
/// somewhat deviates from the usual behaviour because it affects (attenuates)
/// the parts of the signal _outside_ the specified frequency range.
///
////////////////////////////////////////////////////////////////////////////////

struct Bandpass : Detail::BandGain
{
    static char const title      [];
    static char const description[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class Bandstop
///
/// \ingroup Effects
///
/// \brief Band-stop filter.
///
/// A simple bandstop filter with a variable stop band. The stop band is defined
/// using the standard StartFrequency and StopFrequency parameters.
///
////////////////////////////////////////////////////////////////////////////////

struct Bandstop : Detail::BandGain
{
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

#endif // bandpass_hpp
