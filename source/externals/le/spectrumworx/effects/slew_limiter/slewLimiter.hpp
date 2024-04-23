////////////////////////////////////////////////////////////////////////////////
///
/// \file slewLimiter.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef slewLimiter_hpp__3E24E3B5_8DED_4323_8369_F01D41240B98
#define slewLimiter_hpp__3E24E3B5_8DED_4323_8369_F01D41240B98
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/enumerated/parameter.hpp"
#include "le/parameters/linear/parameter.hpp"

#include "boost/mpl/string.hpp"

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
/// \class SlewLimiter
///
/// \ingroup Effects
///
/// \brief Limits maximum per-bin magnitude change speed.
/// 
/// Limits the speed of change of the magnitudes. The direction and the rate 
/// of the slew are selectable. 
///
////////////////////////////////////////////////////////////////////////////////

struct SlewLimiter
{
private:
    typedef boost::mpl::string<' dB/', 's'> DecibelsPerSecond;

public:
    LE_ENUMERATED_PARAMETER( Direction, ( RiseFall )( Rise )( Fall ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Direction ) ) 
        ( ( SlewRate  ) ( LinearFloat )( Minimum<0> )( Maximum<300> )( Default<50> )( Unit2<DecibelsPerSecond> ) )
    );
    
    /// \typedef Direction
    /// \brief Determines the slew direction.
    /// \details
    ///   - RiseFall: limits both rise and fall.
    ///   - Rise: limits rise of the amplitudes.
    ///   - Fall: limits fall of the amplitudes.
    /// \typedef SlewRate
    /// \brief Limit in decibels per second.

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

#endif // slewLimiter_hpp
