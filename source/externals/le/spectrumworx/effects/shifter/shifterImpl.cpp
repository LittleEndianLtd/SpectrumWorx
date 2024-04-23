////////////////////////////////////////////////////////////////////////////////
///
/// shifterImpl.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "shifterImpl.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/utility/buffers.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"
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
// Shifter static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Shifter::title      [] = "Shifter";
char const Shifter::description[] = "Shifts spectrum along the frequency axis.";


////////////////////////////////////////////////////////////////////////////////
//
// Shifter UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Shifter::ShiftTarget, "Target" )
EFFECT_PARAMETER_NAME( Shifter::Offset     , "Offset" )
EFFECT_PARAMETER_NAME( Shifter::Tail       , "Tail"   )

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Shifter, Tail,
    (( Leave    , "Leave"    ))
    (( Clear    , "Clear"    ))
    (( Circular , "Circular" ))
)

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Shifter, ShiftTarget,
    (( Magnitudes, "Magnitudes"  ))
    (( Phases    , "Phases"      ))
    (( Both      , "Mags&Phases" ))
)


////////////////////////////////////////////////////////////////////////////////
//
// ShifterImpl::setup()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void ShifterImpl::setup( IndexRange const & workingRange, Engine::Setup const & )
{
    using namespace Math;

    int const offset( convert<int>( percentage2NormalisedLinear( parameters().get<Offset>() ) * convert<float>( workingRange.size() ) ) );

    shiftLength_    = abs( offset );
    positiveOffset_ = offset > 0;

    ShiftTarget::value_type const mode( parameters().get<ShiftTarget>() );
    magnitudes_ = ( mode == ShiftTarget::Both ) | ( mode == ShiftTarget::Magnitudes );
    phases_     = ( mode == ShiftTarget::Both ) | ( mode == ShiftTarget::Phases     );
}


////////////////////////////////////////////////////////////////////////////////
//
// ShifterImpl::process()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

void ShifterImpl::process( Engine::ChannelData_AmPh data, Engine::Setup const & ) const
{
    if ( shiftLength_ == 0 )
        return;

    if ( magnitudes_ ) shift( data.amps  () );
    if ( phases_     ) shift( data.phases() );
}


////////////////////////////////////////////////////////////////////////////////
//
// ShifterImpl::shift()
// --------------------
//
////////////////////////////////////////////////////////////////////////////////

void ShifterImpl::shift( DataRange const & data ) const
{
    Tail::value_type const type( parameters().get<Tail>() );

    unsigned int const numBins       ( static_cast<unsigned int>( data.size() ) );
    unsigned int const shiftLength   ( shiftLength_                             );
    unsigned int const positiveOffset( positiveOffset_                          );

    unsigned int sourceBin;
    unsigned int targetBin;

    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( circularTailBuffer, Engine::real_t, shiftLength );
    if ( type == Tail::Circular )
    {
        if ( positiveOffset ) { sourceBin = numBins - shiftLength; }
        else                  { sourceBin = 0                    ; }
        Math::copy( &data[ sourceBin ], circularTailBuffer.begin(), shiftLength );
    }

    // In any case:
    {
        unsigned int const dataSize( numBins - shiftLength );
        if ( positiveOffset ) { sourceBin = 0          ; targetBin = shiftLength; }
        else                  { sourceBin = shiftLength; targetBin = 0          ; }
        Math::move( data.begin() + sourceBin, data.begin() + targetBin, dataSize );
    }

    switch ( type )
    {
        case Tail::Leave:
            break;

        case Tail::Clear:
            sourceBin = positiveOffset ? 0 : numBins - shiftLength;
            Math::clear( data.begin() + sourceBin, shiftLength );
            break;

        case Tail::Circular:
            if ( positiveOffset ) { targetBin = 0                    ; }
            else                  { targetBin = numBins - shiftLength; }
            Math::copy( circularTailBuffer.begin(), data.begin() + targetBin, shiftLength );
            break;
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
