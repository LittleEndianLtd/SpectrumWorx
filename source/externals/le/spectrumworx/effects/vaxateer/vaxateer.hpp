////////////////////////////////////////////////////////////////////////////////
///
/// \file vaxateer.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef vaxateer_hpp__BE1E8F69_E3AC_444F_91A4_4906B7C669E4
#define vaxateer_hpp__BE1E8F69_E3AC_444F_91A4_4906B7C669E4
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/enumerated/parameter.hpp"
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
/// \class Vaxateer
///
/// \ingroup Effects
///
/// \brief Spectral composition based on RMS.
/// 
/// Conditional replacement of the Main signal with the Side-channel based on 
/// the RMS value (Root Mean Square) of the target signal (can be Main or 
/// Side-channel). If the "Swap condition" is satisfied, Main signal is replaced
/// with the Side-channel content. There are eight available Swap conditions. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct Vaxateer
{
    LE_ENUMERATED_PARAMETER( RMSTarget, ( MainRMS )( SideRMS )                           );
    LE_ENUMERATED_PARAMETER( Mode     , ( M1 )( M2 )( M3 )( M4 )( M5 )( M6 )( M7 )( M8 ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( RMSTarget ) )
        ( ( RMSGain   ) ( SymmetricInteger )( MaximumOffset<24> )( Unit<' dB'> ) )  
        ( ( Mode      ) )
    );

    /// \typedef RMSTarget
    /// \brief Target channel for RMS calculation, can be Main or Side.
    /// \typedef RMSGain
    /// \brief Gain applied to the calculated RMS, to be used as threshold for
    /// comparison.
    /// \typedef Mode
    /// \brief Specifies the swap condition.
    /// \details
    ///   - M1 (Main: >Thr >Side): Main should be higher than threshold and higher than Side.
    ///   - M2 (Main: >Thr <Side): Main should be higher than threshold and lower than Side.
    ///   - M3 (Main: <Thr >Side): Main should be lower than threshold and higher than Side.
    ///   - M4 (Main: <Thr <Side): Main should be lower than threshold and lower than Side.
    ///   - M5 (Side: >Thr >Main): Side should be higher than threshold and higher than Main.
    ///   - M6 (Side: >Thr <Main): Side should be higher than threshold and lower than Main.
    ///   - M7 (Side: <Thr >Main): Side should be lower than threshold and higher than Main.
    ///   - M8 (Side: <Thr <Main): Side should be lower than threshold and lower than Main.

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

#endif // vaxateer_hpp
