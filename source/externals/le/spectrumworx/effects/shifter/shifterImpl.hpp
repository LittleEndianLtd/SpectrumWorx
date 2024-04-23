////////////////////////////////////////////////////////////////////////////////
///
/// \file shifterImpl.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef shifterImpl_hpp__8FBF160E_8886_4672_8CEC_90EA6370B72F
#define shifterImpl_hpp__8FBF160E_8886_4672_8CEC_90EA6370B72F
#pragma once
//------------------------------------------------------------------------------
#include "shifter.hpp"

#include "le/spectrumworx/effects/effects.hpp"
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

class ShifterImpl : public EffectImpl<Shifter>
{
public: // LE::Effect interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;

private:
  //////////////////////////////////////////////////////////////////////////////
  //
  // Shifter::shift()
  // ----------------
  //
  //////////////////////////////////////////////////////////////////////////////
  ///
  /// \brief Shifts data to left or right.
  ///
  /// \param data - spectrum data (in-place modification)
  ///
  /// \throws nothing
  ///
  //////////////////////////////////////////////////////////////////////////////
  
  void shift( DataRange const & data ) const;

private:
    unsigned int shiftLength_   ;
    bool         positiveOffset_;

    bool magnitudes_;
    bool phases_    ;
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // shifterImpl_hpp