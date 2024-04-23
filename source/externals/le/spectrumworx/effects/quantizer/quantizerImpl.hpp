////////////////////////////////////////////////////////////////////////////////
///
/// \file quantizer.hpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef quantizerImpl_hpp__3DC36D0D_02D4_4EBE_A133_5CAA7A852469
#define quantizerImpl_hpp__3DC36D0D_02D4_4EBE_A133_5CAA7A852469
#pragma once
//------------------------------------------------------------------------------
#include "quantizer.hpp"

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

class QuantizerImpl : public EffectImpl<Quantizer>
{
public: // LE::Effect interface.

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( IndexRange const &, Engine::Setup const & );
    void process( Engine::ChannelData_AmPh, Engine::Setup const & ) const;

  static bool const canSwapChannels = false;
  
private:
  
  //////////////////////////////////////////////////////////////////////////////
  //
  // quantize()
  // ----------
  //
  //////////////////////////////////////////////////////////////////////////////
  ///
  /// \brief Quantizes a block of spectrum values to a starting bin value. 
  ///
  /// \param data - input/output spectrum data
  ///
  /// \throws nothing
  ///
  //////////////////////////////////////////////////////////////////////////////
  
  void quantize( DataRange amps ) const;

private:  
    IndexRange::value_type chunkSize_; ///< number of bins to quantize
    float                  origami_  ; ///< linear interpolation intensity
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // quantizerImpl_hpp
