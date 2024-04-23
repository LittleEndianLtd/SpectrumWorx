////////////////////////////////////////////////////////////////////////////////
///
/// baseParametersUIElements.cpp
/// ----------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "baseParametersUIElements.hpp"

#include "le/spectrumworx/engine/setup.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
    using namespace SW::Effects;
    UI_NAME( BaseParameters::Bypass         ) = "Bypass"         ;
    UI_NAME( BaseParameters::Gain           ) = "Gain"           ;
    UI_NAME( BaseParameters::Wet            ) = "Wet"            ;
    UI_NAME( BaseParameters::StartFrequency ) = "Start frequency";
    UI_NAME( BaseParameters::StopFrequency  ) = "Stop frequency" ;

    float DisplayValueTransformer<BaseParameters::StartFrequency>::transform( float const & value, SW::Engine::Setup const & engineSetup )
    {
        return engineSetup.normalisedFrequencyToHz( value );
    }
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
