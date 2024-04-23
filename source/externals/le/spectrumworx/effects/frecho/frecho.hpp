////////////////////////////////////////////////////////////////////////////////
///
/// \file frecho.hpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef frecho_hpp__9D813892_843C_41F8_888D_1CFD79EA0E22
#define frecho_hpp__9D813892_843C_41F8_888D_1CFD79EA0E22
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/linear/parameter.hpp"
#include "le/parameters/symmetric/parameter.hpp"

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
/// \class Frecho
///
/// \ingroup Effects
///
/// \brief Frequency domain echo with pitch.
///
/// Frequency domain echo that functions with pitch. Distance parameter 
/// determines perceived distance of the reflection (minimum is set to 17 meters, 
/// resulting in 34 meters for the sound to return to the signal's perceived, 
/// any less than that and echo would become reverberation. Absorption controls 
/// how much of the sound is absorbed and how much is reflected back. Using the 
/// pitch function, it is possible to pitch-shift each echoed sound. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct Frecho
{
private:
    typedef boost::mpl::string<' \'/e', 'cho'> SemitonesPerEcho;

public:
    LE_DEFINE_PARAMETERS
    (
        ( ( Distance   )( LinearUnsignedInteger )( Minimum< 17> )( Maximum<500> )( Default<100> )( Unit <            ' m'> ) )
        ( ( Absorption )( LinearFloat           )( Minimum<  0> )( Maximum< 18> )( Default<  6> )( Unit <           ' dB'> ) )
        ( ( EchoPitch  )( SymmetricFloat        )( MaximumOffset<12> )                           ( Unit2<SemitonesPerEcho> ) )
    );

    /// \typedef Distance
    /// \brief Perceived distance of the reflection.
    /// \typedef Absorption
    /// \brief Specifies how much sound is absorbed being reflected.
    /// \typedef EchoPitch
    /// \brief Specifies how much is each echo pitch-shifted.


    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class Frevcho
///
/// \ingroup Effects
///
/// \brief Frequency domain reverse echo with pitch.
///
/// Frequency domain reverse echo that functions with pitch. Echoed signal is
/// reversed before being reflected back to the source. Distance parameter 
/// determines perceived distance of the reflection (minimum is set to 17 meters, 
/// resulting in 34 meters for the sound to return to the signal's perceived, 
/// any less than that and echo would become reverberation. Absorption controls 
/// how much of the sound is absorbed and how much is reflected back. Using the 
/// pitch function, it is possible to pitch-shift each echoed sound.
/// 
////////////////////////////////////////////////////////////////////////////////

struct Frevcho
{
    /// \name Parameters
    ///@{
    typedef Frecho::Distance   Distance  ;
    typedef Frecho::Absorption Absorption;
    typedef Frecho::EchoPitch  EchoPitch ;
    ///@}
    typedef Frecho::Parameters Parameters;

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

#endif // frecho_hpp
