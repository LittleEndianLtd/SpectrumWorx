////////////////////////////////////////////////////////////////////////////////
///
/// \file coloriferImpl.hpp
/// -----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef coloriferImpl_hpp__B6E8308A_87E9_4d87_8E9E_C537504C54DA
#define coloriferImpl_hpp__B6E8308A_87E9_4d87_8E9E_C537504C54DA
#pragma once
//------------------------------------------------------------------------------
#include "colorifer.hpp"

#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/effects/indexRange.hpp"
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

class ColoriferImpl : public EffectImpl<Colorifer>
{
public: // LE::Effect required interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( Engine::MainSideChannelData_AmPh, Engine::Setup const & ) const;

private:
    IndexRange::value_type shapeWidth_;
}; // class ColoriferImpl

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // coloriferImpl_hpp
