////////////////////////////////////////////////////////////////////////////////
///
/// blenderImpl.cpp
/// ---------------
///
/// Copyright (C) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "blenderImpl.hpp"

#include "le/spectrumworx/engine/channelDataReIm.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
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
//
// Blender static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Blender::title      [] = "Blender";
char const Blender::description[] = "Linear blend.";


////////////////////////////////////////////////////////////////////////////////
//
// Blender UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Blender::Amount, "Amount" )


////////////////////////////////////////////////////////////////////////////////
//
// BlenderImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void BlenderImpl::setup( IndexRange const &, Engine::Setup const & )
{
    amount_ = 1.0f - Math::percentage2NormalisedLinear( parameters().get<Amount>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// BlenderImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////
/// \note Alex says: "Blender performs a linear blend or mix (sometimes wrongly 
/// referred to as a «morph») in the frequency domain of magnitudes and phases 
/// of Source to Target. There are two controls: Magn blends the Source 
/// Magnitudes to the Target, and Phase blends the Source phase to the Target 
/// Phase.  
/// If both Magn and Phase set to 0 values, you will hear only the Source input.
/// If both Magn and Phase are at 1 – you will hear only the Target. 
/// Blending creates a smooth, but perceptible transition from one sound to the 
/// other. It is highly recommended that you use similar spectra sounds to avoid 
/// any unwanted spectral artifacts."
///                                           (21.01.2010.) (Danijel Domazet)
////////////////////////////////////////////////////////////////////////////////

void BlenderImpl::process( Engine::MainSideChannelData_ReIm data, Engine::Setup const & ) const
{
    auto const main( data.main().jointView() );
    auto const side( data.side().jointView() );
    Math::mix
    (
        main.begin(),
        side.begin(),
        main.begin(),
        amount_,
        static_cast<unsigned int>( main.size() )
    );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
