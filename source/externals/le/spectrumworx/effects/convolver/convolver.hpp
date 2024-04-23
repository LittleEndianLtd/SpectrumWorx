////////////////////////////////////////////////////////////////////////////////
///
/// \file convolver.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef convolver_hpp__A7F10DAF_A6F4_44E2_B6B4_460805ACC405
#define convolver_hpp__A7F10DAF_A6F4_44E2_B6B4_460805ACC405
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/enumerated/parameter.hpp"
#include "le/parameters/trigger/parameter.hpp"

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
/// \class Convolver
///
/// \ingroup Effects
///
/// \brief Performs multiplications in frequency domain i.e. convolution in 
/// time domain.
///
////////////////////////////////////////////////////////////////////////////////

struct Convolver
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    LE_ENUMERATED_PARAMETER( ConvolutionType, ( Triggered )( Continuous ) );
    LE_ENUMERATED_PARAMETER( Phase          , ( Sum )( Side )( Main )     );

    LE_DEFINE_PARAMETERS
    (
        ( ( ConvolutionType )                     )
        ( ( GrabIR          )( TriggerParameter ) )
        ( ( Phase           )                     )
    );

    /// \typedef ConvolutionType
    /// \brief Defines the type of convolution.
    /// \details
    ///   - Continuous: continuous convolution with the Side channel.
    ///   - Triggered: new Impulse Response is taken from Side channel
    ///                after trigger button is activated.
    /// \typedef GrabIR
    /// \brief Grabs the impulse response from the side channel.
    /// \typedef Phase
    /// \brief Defines which phase will the output signal take.
    /// \details
    ///   - Sum: sum main and side channel phases
    ///   - Side: forward side channel phases output
    ///   - Main: forward main channel phases output

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

#endif // convolver_hpp
