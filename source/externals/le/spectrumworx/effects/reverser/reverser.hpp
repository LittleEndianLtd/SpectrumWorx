////////////////////////////////////////////////////////////////////////////////
///
/// \file reverser.hpp
/// ------------------
///
/// Copyright (c) 2009 - 2016. Little Endian. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef reverser_hpp__C3A377D7_0B2D_4B86_B9F9_5609C8675D7E
#define reverser_hpp__C3A377D7_0B2D_4B86_B9F9_5609C8675D7E
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

////////////////////////////////////////////////////////////////////////////////
///
/// \class Reverser
///
/// \ingroup Effects
///
/// \brief Reverses sound chunks.
///
/// Breaks the input down into chunks and reverses them. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct Reverser
{
    LE_DEFINE_PARAMETERS
    (
        ( ( Length )( LinearUnsignedInteger )( Minimum<1> )( Maximum<5000> )( Default<1000> )( Unit<' ms'> ) )
    );

    /// \typedef Length
    /// \brief The length of the chunks to be reversed.

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

#endif // reverser_hpp
