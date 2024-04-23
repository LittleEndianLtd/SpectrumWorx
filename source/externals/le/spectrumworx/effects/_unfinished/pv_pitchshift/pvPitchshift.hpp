////////////////////////////////////////////////////////////////////////////////
///
/// \file pvPitchshift.hpp
/// ----------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef pvPitchshift_hpp__4B0E3425_3A1D_4605_A353_C6821373527F
#define pvPitchshift_hpp__4B0E3425_3A1D_4605_A353_C6821373527F
//------------------------------------------------------------------------------
#include "../pitch_shifter/pitchShifter.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class PVPitchshift
///
/// \ingroup Algorithms v200
///
/// \brief Pitch shift in "PV domain".
///
////////////////////////////////////////////////////////////////////////////////

class PVPitchshift
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    typedef PitchShifter::Parameters Parameters;


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( EngineSetup const &, Parameters const & );
    void process( ChannelData_AmPh & ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];

private:
    PhaseVocoderShared::PitchShiftParameters pitchShiftParameters_;
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // pvPitchshift
