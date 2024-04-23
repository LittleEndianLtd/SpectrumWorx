////////////////////////////////////////////////////////////////////////////////
///
/// \file octaver.hpp
/// -----------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef octaver_hpp__9579DBDB_C9A8_486D_985B_DB6955708192
#define octaver_hpp__9579DBDB_C9A8_486D_985B_DB6955708192
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
#include "le/parameters/linear/parameter.hpp"
#include "le/parameters/symmetric/parameter.hpp"
#include "le/parameters/enumerated/parameter.hpp"

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
/// \class Octaver
///
/// \ingroup Effects
///
/// \brief Adds two octaves.
///
/// This module adds two (or one) octaves to the input signal. Range is up or
/// down two octaves. Each octave has Gain control and cut-off frequency is
/// shared between both octaves. 
///
////////////////////////////////////////////////////////////////////////////////

struct Octaver
{
    LE_ENUMERATED_PARAMETER( Octave1, ( Down2 )( Down1 )( Off ) ( Up1 ) ( Up2 ) );
    LE_ENUMERATED_PARAMETER( Octave2, ( Down2 )( Down1 )( Off ) ( Up1 ) ( Up2 ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Octave1 ) )
        ( ( GainOctave1     )( LinearFloat           )( Minimum<-48> )( Maximum<  +24> )( Default<  0>)( Unit<' dB'> ) )
        ( ( Octave2 ) )
        ( ( GainOctave2     )( LinearFloat           )( Minimum<-48> )( Maximum<  +24> )( Default<  0>)( Unit<' dB'> ) )
        ( ( CutoffFrequency )( LinearUnsignedInteger )( Minimum<  0> )( Maximum<16000> )( Default<350>)( Unit<' Hz'> ) )
    );

    /// \typedef Octave1
    /// \brief Controls the first octave to be added.
    /// \details
    ///   - Down 2: adds an octave which is 2 octaves down.
    ///   - Down 1: adds an octave which is 1 octaves down.
    ///   - Off   : no octave added.
    ///   - Up   1: adds an octave which is 1 octaves up.
    ///   - Up   2: adds an octave which is 2 octaves up.
    /// \typedef GainOctave1
    /// \brief Controls the first octave's gain.
    /// \typedef Octave2
    /// \brief Controls the second octave to be added.
    /// \details
    ///   - Down 2: adds an octave which is 2 octaves down.
    ///   - Down 1: adds an octave which is 1 octaves down.
    ///   - Off   : no octave added.
    ///   - Up   1: adds an octave which is 1 octaves up.
    ///   - Up   2: adds an octave which is 2 octaves up.
    /// \typedef GainOctave2
    /// \brief Controls the second octave's gain.
    /// \typedef CutoffFrequency
    /// \brief Controls the cut-off frequency for both octaves.


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

#endif // octaver_hpp
