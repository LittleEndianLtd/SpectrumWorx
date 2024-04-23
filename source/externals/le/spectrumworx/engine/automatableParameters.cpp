////////////////////////////////////////////////////////////////////////////////
///
/// automatableParameters.cpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "automatableParameters.hpp"

#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
    UI_NAME( SW::Engine::FFTSize          ) = "FFT size"          ;
    UI_NAME( SW::Engine::OverlapFactor    ) = "Overlap factor"    ;
    UI_NAME( SW::Engine::WindowFunction   ) = "Window type"       ;
#if LE_SW_ENGINE_WINDOW_PRESUM
    UI_NAME( SW::Engine::WindowSizeFactor ) = "Window size factor";
#endif // LE_SW_ENGINE_WINDOW_PRESUM

    //...mrmlj...this does not work yet because the Window enum is not a member
    //...of the WindowFunction parameter class...fix this...
    //ENUMERATED_PARAMETER_STRINGS
    //(
    //    SW::Engine, WindowFunction,
    //    (( Hamming       , "Hamming"         ))
    //    (( Hann          , "Hann"            ))
    //    (( Rectangle     , "Rectangle"       ))
    //    (( Triangle      , "Triangle"        ))
    //    (( Blackman      , "Blackman"        ))
    //    (( BlackmanHarris, "Blackman Harris" ))
    //    (( Welch         , "Welch"           ))
    //    (( FlatTop       , "Flat top"        ))
    //    (( Gaussian      , "Gaussian"        ))
    //)

    template <>
    char const * LE_RESTRICT const DiscreteValues<SW::Engine::WindowFunction>::strings[] =
    {
        "Hann"          ,
        "Hamming"       ,
        "Blackman"      ,
        "BlackmanHarris",
        "Gaussian"      ,
        "FlatTop"       ,
        "Welch"         ,
        "Triangle"      ,
        "Rectangle"
    };
} // namespace Parameters
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
} // namespace Engine
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
