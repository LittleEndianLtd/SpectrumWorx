////////////////////////////////////////////////////////////////////////////////
///
/// \file ethereal.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef ethereal_hpp__A78CE259_D3CA_4556_8B65_AAF88162B342
#define ethereal_hpp__A78CE259_D3CA_4556_8B65_AAF88162B342
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/commonParameters.hpp"
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
/// \class Ethereal
///
/// \ingroup Effects
///
/// \brief Replaces main with side channel based on magnitude comparisons.
///
/// Compares the side-channel signal with that of the input and it replaces the 
/// input with the side signal if certain conditions are met:  
/// (input - side > or < threshold). Can replace magnitudes or phases, or both.
///
////////////////////////////////////////////////////////////////////////////////

struct Ethereal
{
    /// \name Parameters
    /// @{
    typedef CommonParameters::Mode Mode;
    /// @}

    LE_ENUMERATED_PARAMETER( Condition, ( DiffHigher )( DiffLower ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( Condition ) )
        ( ( Threshold )( SymmetricFloat )( MaximumOffset<30> )( Unit<' dB'> ) )        
        ( ( Mode      ) )
    );

    /// \typedef Condition
    /// \brief Condition to meet in order to replace input with side channel.
    /// \details
    ///   - DiffHigher: replace if (input - side) > threshold
    ///   - DiffLower: replace if (input - side) < threshold
    /// \typedef Threshold
    /// \brief Comparison threshold.
    /// \typedef Mode
    /// \brief Specifies what is to be replaced.
    /// \details
    ///   - Both      : replace both magnitudes and phases. 
    ///   - Magnitudes: replace only magnitudes.
    ///   - Phases    : replace only phases.


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

#endif // ethereal_hpp

