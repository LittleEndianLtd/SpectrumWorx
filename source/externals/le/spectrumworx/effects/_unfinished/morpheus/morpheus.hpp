////////////////////////////////////////////////////////////////////////////////
///
/// \file morpheus.hpp
/// ------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#pragma once
#ifndef morpheus_hpp__50690992_6F20_47F3_805E_C0D854E63F48
#define morpheus_hpp__50690992_6F20_47F3_805E_C0D854E63F48
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
/// \class Morpheus
///
/// \ingroup Algorithms RC
///
/// \brief Adaptive spectral blend.
///
////////////////////////////////////////////////////////////////////////////////

class Morpheus
{
public: // LE::Algorithm required interface.

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////
    
    DISCRETE_VALUES_PARAMETER( Mode1, ( AbsDiff )( MainDiff )( SideDiff )( Sum )( Side )( Main ) );
    DISCRETE_VALUES_PARAMETER( Mode2, ( Blend   )( BlendInv ) ( Replace )                        );

    DEFINE_PARAMETERS
    (
        ( ( Mode1  )                                                                                                             )
        ( ( Mode2  )                                                                                                             )
        ( ( Amount )( float        )( MinimumValue<0> )( MaximumValue<100> )( DefaultValue< 30> )( DisplayValueSuffix<' %'  >  ) )
        ( ( Range  )( unsigned int )( MinimumValue<0> )( MaximumValue<100> )( DefaultValue< 50> )( DisplayValueSuffix<' bw%'>  ) )
    );

    ////////////////////////////////////////////////////////////////////////////
    // setup() and process()
    ////////////////////////////////////////////////////////////////////////////

    void setup( EngineSetup const &, Parameters const & );
    void process( ChannelData_AmPh & ) const;

public: // Algorithm traits.
    static bool const canUseTwoInputs = false;

public: 
    static char const title      [];
    static char const description[];


private:
    unsigned int num_bins_;
    unsigned int range_   ;    
    float        amount_  ;
    Mode1::Value mode1_   ;
    Mode2::Value mode2_   ;
    
    struct PeakR
    {
        float        val;
        unsigned int pos;
    };

    friend bool operator<( PeakR const & left, PeakR const & right ) { return left.val < right.val; }

    // Implementation note:
    //   Preallocated work buffers to avoid the hidden call to _chkstk() in the
    // process() function.
    //                                        (25.02.2010.) (Domagoj Saric)
    mutable  boost::array<PeakR, (LE_CONFIGURATION_MAX_FFT_SIZE / 2) + 1> peaks_;
};

//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // morpheus
