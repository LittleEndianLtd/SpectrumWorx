////////////////////////////////////////////////////////////////////////////////
///
/// \internal
/// \file parameters.hpp
/// --------------------
///
/// Shared LE parameter forward declarations required by all effects.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameters_hpp__E052A5AB_F608_48B9_9C09_DC2497B4086D
#define parameters_hpp__E052A5AB_F608_48B9_9C09_DC2497B4086D
#pragma once
//------------------------------------------------------------------------------
#include "le/parameters/factoryMacro.hpp"

#if defined( _MSC_VER ) && !defined( __clang__ ) && ( _MSC_VER < 1800 )
#include "le/parameters/boolean/parameter.hpp"
#endif // old MSVC
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace Parameters
{
    template <class Traits> class Parameter; ///< \internal

#if !( defined( _MSC_VER ) && !defined( __clang__ ) && ( _MSC_VER < 1800 ) )
    struct Boolean;
#endif // !old MSVC

    struct LinearUnsignedInteger;
    struct LinearSignedInteger  ;
    struct LinearFloat          ;

    struct SymmetricInteger;
    struct SymmetricFloat  ;

    class TriggerParameter;
} // namespace Parameters

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

// Implementation note:
//   Import shared parameter types into the Effects namespace for less
// verbose effect parameters definitions with the LE_DEFINE_PARAMETERS macro.
//                                            (15.04.2011.) (Domagoj Saric)

using Parameters::Boolean;

using Parameters::LinearUnsignedInteger;
using Parameters::LinearSignedInteger  ;
using Parameters::LinearFloat          ;

using Parameters::SymmetricInteger;
using Parameters::SymmetricFloat  ;

using Parameters::TriggerParameter;

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameters_hpp
