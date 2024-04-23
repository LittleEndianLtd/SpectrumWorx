////////////////////////////////////////////////////////////////////////////////
///
/// \file musicalScales.hpp
/// -----------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef musicalScales_hpp__95B29F4C_895F_43C1_9919_6770BC2FED62
#define musicalScales_hpp__95B29F4C_895F_43C1_9919_6770BC2FED62
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include <array>
#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Music
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Scale
///
////////////////////////////////////////////////////////////////////////////////

class Scale
{
public:
    using ToneOffsets = std::array<std::uint8_t, 12>;

    Scale();

    float LE_FASTCALL snap2Scale( float freq, std::uint8_t keyIndex ) const;

    void LE_FASTCALL tonesUpdated( std::uint8_t snappeTo, std::uint8_t bypassed );

    ToneOffsets       & toneOffsets()       { return toneOffsets_; }
    ToneOffsets const & toneOffsets() const { return toneOffsets_; }

    std::uint8_t numberOfTones   () const { return numberOfTones_   ; }
#ifdef LE_SW_SDK_BUILD
    std::uint8_t numberOfBypassed() const { return numberOfBypassed_; }
#else
    std::uint8_t numberOfBypassed() const { return 0                ; }
#endif // LE_SW_SDK_BUILD

private:
    ToneOffsets::value_type LE_FASTCALL toneOffset( std::uint8_t index ) const;

private:
    std::uint8_t  numberOfTones_;
#ifdef LE_SW_SDK_BUILD
	std::uint8_t  numberOfBypassed_;
#endif // LE_SW_SDK_BUILD
#ifndef LE_MELODIFY_SDK_BUILD
    std::int8_t   targetPitchChangeDirection_;
            float centerTone_;
    mutable float lastPitchScale_;
#endif // LE_MELODIFY_SDK_BUILD
    ToneOffsets toneOffsets_;
}; // class Scale

//------------------------------------------------------------------------------
} // namespace Music
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // musicalScales_hpp
