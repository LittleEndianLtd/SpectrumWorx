////////////////////////////////////////////////////////////////////////////////
///
/// commonParametersUIElements.cpp
/// ------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "commonParametersUIElements.hpp"

#include "commonParameters.hpp"

#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace Parameters
{
    UI_NAME( SW::Effects::CommonParameters::Mode       ) = "Target"   ;
    UI_NAME( SW::Effects::CommonParameters::SpringType ) = "Direction";

    ENUMERATED_PARAMETER_STRINGS
    (
        SW::Effects::CommonParameters, Mode,
        (( Both      , "Mags&Phases" ))
        (( Magnitudes, "Magnitudes"  ))
        (( Phases    , "Phases"      ))
    )

    ENUMERATED_PARAMETER_STRINGS
    (
        SW::Effects::CommonParameters, SpringType,
        (( Symmetric, "Symmetric" ))
        (( Up       , "Up"        ))
        (( Down     , "Down"      ))
    )
} // namespace Parameters

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
