////////////////////////////////////////////////////////////////////////////////
///
/// \file runtimeInformation.hpp
/// ----------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef runtimeInformation_hpp__3FF98324_ED63_409C_8624_1DC2E69AF3CE
#define runtimeInformation_hpp__3FF98324_ED63_409C_8624_1DC2E69AF3CE
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/abi.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

/// \addtogroup Parameters
/// @{

////////////////////////////////////////////////////////////////////////////////
///
/// \class RuntimeInformation
///
/// \brief Runtime/dynamic parameter metadata
///
/// \details All instances of this struct (as well as the data its members point
/// to) are statically allocated and it is therefore safe to hold references to
/// them.
///
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    #if _MSC_VER < 1800
        #pragma warning( disable : 4480 ) // Specifying underlying type for enum
    #endif
#endif // _MSC_VER

struct RuntimeInformation
{
    typedef float value_type; ///< Uniform type used for marshaling values of all parameter types

    /// \brief Possible parameter types (does not cover power-of-two parameters yet)
    enum Type
    #ifndef DOXYGEN_ONLY
        : std::uint8_t
    #endif // DOXYGEN_ONLY
        { Boolean, Enumerated, FloatingPoint, Integer, Trigger };

    Type const type; ///< parameter's type

    value_type const minimum ; ///< parameter's minimum possible/allowed value
    value_type const maximum ; ///< parameter's maximum possible/allowed value
    value_type const default_; ///< parameter's default value

    char const * LE_RESTRICT const name; ///< parameter's name
    char const * LE_RESTRICT const unit; ///< parameter's unit (e.g. "Hz" or "%", may be an empty string but never null)

    /// \brief For <VAR>Enumerated</VAR> parameters, an array of C strings (of
    /// length <VAR>maximum</VAR>) representing the individual values of the
    /// enumerated parameter. A nullptr for all other types of parameters.
    char const * LE_RESTRICT const * LE_RESTRICT const enumeratedValueStrings;
}; // struct RuntimeInformation

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

/// @} // group Parameters

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // runtimeInformation_hpp
