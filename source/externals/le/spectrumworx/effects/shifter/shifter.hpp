////////////////////////////////////////////////////////////////////////////////
///
/// \file shifter.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef shifter_hpp__8FBF160E_8886_4672_8CEC_90EA6370B72F
#define shifter_hpp__8FBF160E_8886_4672_8CEC_90EA6370B72F
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
/// \class Shifter 
///
/// \ingroup Effects
/// 
/// \brief Shifts the spectrum along the frequency axis.
///
/// Shifts frequency "bins" along the frequency axis to produce unusual 
/// clangorous and metallic effects. 
/// 
/// \note In some cases Shifter will produce silent output due to sinusoids 
/// canceling out at some shift ranges. This happens only when both magnitudes 
/// and phases are shifted and depends very much on the engine settings 
/// (Overlap and Window type). 
///
////////////////////////////////////////////////////////////////////////////////

struct Shifter
{
    LE_ENUMERATED_PARAMETER( ShiftTarget, ( Magnitudes )( Phases )( Both     ) );
    LE_ENUMERATED_PARAMETER( Tail       , ( Leave      )( Clear  )( Circular ) );

    LE_DEFINE_PARAMETERS
    (
        ( ( ShiftTarget ) )
        ( ( Offset      )( SymmetricFloat )( MaximumOffset<10> )( Unit<' bw%'> ) )
        ( ( Tail        ) )
    );
    
    /// \typedef ShiftTarget
    /// \brief Specifies what is to be shifted.
    /// \details
    ///   - Magnitudes: shift only magnitudes.
    ///   - Phases: shift only phases.
    ///   - Both: shift both magnitudes and phases.
    /// \typedef Offset
    /// \brief Amount of shift.
    /// \details (in \%bw, percentage of bandwidth i.e. total
    /// frequency range)
    /// \typedef Tail
    /// \brief Determines what shall be done with the tail data.
    /// \details
    ///   - Leave: shifted tail samples are left unchanged.
    ///   - Clear: shifted tail samples are cleared.
    ///   - Circular: shifted samples are circularly fed to the beginning.


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

#endif // shifter_hpp
