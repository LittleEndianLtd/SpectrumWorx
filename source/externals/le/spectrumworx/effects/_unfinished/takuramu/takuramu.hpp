////////////////////////////////////////////////////////////////////////////////
///
/// \file takuramu.hpp
/// ---------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef takuramu_hpp__98055B9A_6CDA_4627_B71A_DA318D536B68
#define takuramu_hpp__98055B9A_6CDA_4627_B71A_DA318D536B68
//------------------------------------------------------------------------------
#include "../algorithms.hpp"
#include "../../parameters/parameters.hpp"
#include "common/buffers.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Takuramu
///
/// \ingroup Algorithms Alpha
///
/// \brief Pitch effects. Created by Lars Hamre. 
///
////////////////////////////////////////////////////////////////////////////////

class Takuramu
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    DISCRETE_VALUES_PARAMETER( Formant, ( Yes )( No ) );

    DEFINE_PARAMETERS
    (
        ( ( Pitch   )( int )( MinimumValue<- 48> )( MaximumValue<+ 48> )( DefaultValue<0> )( DisplayValueSuffix<'\''> ) )
        ( ( Warp    )( int )( MinimumValue<-100> )( MaximumValue<+100> )( DefaultValue<0> )( DisplayValueSuffix<'\''> ) )
        ( ( Formant ) )
    );


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
    unsigned int halfFFTSize_;

    int            pitch_         ;
    int            warp_          ;
    Formant::Value formant_       ;
    float          basePitchScale_;

    // Implementation note:
    //   Preallocated work buffers to avoid the hidden call to _chkstk() in the
    // process() function.
    //                                        (25.02.2010.) (Domagoj Saric)
    mutable Common::SSEAlignedHalfFFTBuffer synthesizedMagns_;
    mutable Common::SSEAlignedHalfFFTBuffer synthesizedFreqs_;
    mutable Common::SSEAlignedHalfFFTBuffer peaks_           ;
    mutable Common::SSEAlignedHalfFFTBuffer bandDiff_        ;
    mutable Common::SSEAlignedHalfFFTBuffer indices_         ;
    mutable Common::SSEAlignedHalfFFTBuffer pitches_         ;
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // takuramu
