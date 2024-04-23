////////////////////////////////////////////////////////////////////////////////
///
/// parameters.cpp
/// --------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "parameters.hpp"

#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Global parameters UI elements definitions.
//
////////////////////////////////////////////////////////////////////////////////

namespace Parameters
{
    UI_NAME( SW::GlobalParameters::InputGain     ) = "In";
    UI_NAME( SW::GlobalParameters::OutputGain    ) = "Out";
    UI_NAME( SW::GlobalParameters::MixPercentage ) = "Mix";
#if LE_SW_ENGINE_INPUT_MODE >= 1
    UI_NAME( SW::GlobalParameters::InputMode     ) = "Input mode";
#endif //LE_SW_ENGINE_INPUT_MODE >= 1
  //UI_NAME( SW::GlobalParameters::StreamMode    ) = "Sampler streaming mode"; // ...MIDI not supported yet

#if LE_SW_ENGINE_INPUT_MODE >= 1
    ENUMERATED_PARAMETER_STRINGS
    (
        SW::GlobalParameters, InputMode,
        (( Stereo         , "2in / 2out (Stereo)"                 ))
        (( StereoSideChain, "4in / 2out (Stereo with side chain)" ))
        (( Mono           , "1in / 1out (Mono)"                   ))
        (( MonoSideChain  , "2in / 1out (Mono with side chain)"   ))
    )
#endif // LE_SW_ENGINE_INPUT_MODE >= 1
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
