////////////////////////////////////////////////////////////////////////////////
///
/// \file mirror.hpp
/// ----------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef mirror_hpp__072Bf8BB_15CE_4F0C_8D64_ABB39CF24A4E
#define mirror_hpp__072Bf8BB_15CE_4F0C_8D64_ABB39CF24A4E
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Mirror
///
/// \ingroup Algorithms RC
///
/// \brief Mirrors bands across the spectrum.
/// 
/// Mirrors magnitudes, phases, or both across the spectrum. 
/// For example (width is two):
/// Input  spectrum: abcdefghijklmnop...
/// Output spectrum: abbaeffeijjimnnm...
///
////////////////////////////////////////////////////////////////////////////////

class Mirror
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DEFINE_PARAMETERS
    (
        ( ( Width          )( unsigned int )( MinimumValue<0> )( MaximumValue< 10> )( DefaultValue<  2> )( DisplayValueSuffix<' %bw'> ) )
        ( ( Mode           ) )
        ( ( StartFrequency ) )
        ( ( StopFrequency  ) )
    );


    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup  ( EngineSetup const &, Parameters const & );
    void process( ChannelData_AmPh & ) const;

public: // Algorithm traits.
    static bool const canSwapChannels = false;
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];

private: 

    void mirror( float * data, unsigned int start ) const;

private:
    /// chunk width
    unsigned int width_;    
    /// what to mirror, mag, phase, or both
    Mode::Value mode_ ;
    /// freq range
    InclusiveIndexRange bins_;
  
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // mirror_hpp

