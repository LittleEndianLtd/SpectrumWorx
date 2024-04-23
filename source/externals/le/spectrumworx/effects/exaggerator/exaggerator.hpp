////////////////////////////////////////////////////////////////////////////////
///
/// \file exaggerator.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef exaggerator_hpp__FF94E183_1CB9_4A44_8A1A_47BF21E50306
#define exaggerator_hpp__FF94E183_1CB9_4A44_8A1A_47BF21E50306
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/parameters.hpp"
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
/// \class Exaggerator
///
/// \ingroup Effects
///
/// \brief Spectral exaggerator.
///  
/// Applies an exponential function on magnitudes, emphasizing or suppressing 
/// peaks. If a positive setting is dialled in, the higher the amplitude of the 
/// incoming signal, the more it will be amplified. If a negative setting is 
/// selected, the higher the amplitude of the incoming signal, the more it will 
/// be suppressed.
///
////////////////////////////////////////////////////////////////////////////////

struct Exaggerator
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Exaggerate )( SymmetricFloat )( MaximumOffset<100> )( Unit<' %'> ) )
    );

    /// \typedef Exaggerate
    /// \brief Controls the amount and direction of peak exaggeration. 

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

#endif // exaggerator_hpp
