////////////////////////////////////////////////////////////////////////////////
///
/// transientsExtractor.cpp
/// -----------------------
///
/// Copyright (c) 2009. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "transientsExtractor.hpp"

#include "../../parameters/uiElements.hpp"
#include "../../math/math.hpp"
#include "../ydsp.h"
#include "../../../fastmath.h"

//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Algorithms 
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// TransientsExtractor static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const TransientsExtractor::title      [] = "Extrans";
char const TransientsExtractor::description[] = "Transient extractor.";


////////////////////////////////////////////////////////////////////////////////
//
// transientsExtractor UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const UIElements<TransientsExtractor::THD >::name_[] = "Threshold";
char const UIElements<TransientsExtractor::Sign>::name_[] = "Sign"     ;


/// \note Alex says: "This module tries to separate the pitch (steady) and noise
///       (unsteady, transient) parts of sound. The thd is a threshold level; 
///       sign is an extraction mode +1 is pitchy part, -1 noisy (transient) 
///       part. Effect is very audible with low threshold values. Lband and 
///       rband defines effect range. Very interesting effects if you use low 
///       threshold pitch or transient as modulation for a PitchFollower module,
///       which actually boosts pitchy part of original sound. 
///       I can't remember where I took it from, perhaps from emails with Loris 
///       author http://www.cerlsoundgroup.org/Loris/. This code work with 
///       imagery numbers, algorithm is really tricky, so we have to return to 
///       the imagery domain, convolve with delayed buffer values, calculate 
///       weighted difference using reversed power spectrum value and multiply 
///       it with our input 1. Generally Transients and Non-harmonic sounds 
///       extraction is quite hard, but what we have here is basically 
///       Sound = Harmonic + Non harmonic, so we detect harmonic data and 
///       substract from original signal."
///                                           (23.12.2009.) (Danijel Domazet)

////////////////////////////////////////////////////////////////////////////////
//
// TransientsExtractor::setup()
// ----------------------------
//
////////////////////////////////////////////////////////////////////////////////

void TransientsExtractor::setup( EngineSetup const & engineSetup, Parameters const & myParameters )
{  
    sign_      = myParameters.get<Sign>();
    threshold_ = Math::percentage2NormalizedLinear( myParameters.get<THD>() );
    
    threshold_ = ( sign_ > 0.0f ) ? ( 100.0f * (1.0f - threshold_) ) : ( 100.0f * threshold_ );
    threshold_ = (threshold_ / 100.0f) * (threshold_ / 100.0f);

    workingRange_ = engineSetup.workingRange( myParameters );
}


////////////////////////////////////////////////////////////////////////////////
//
// TransientsExtractor::process()
// ------------------------------
//
////////////////////////////////////////////////////////////////////////////////

void TransientsExtractor::process( ChannelState & cs, ChannelData_ReIm & data ) const // Use reals and imags.
{
    // Locals:
    float tsik;
    float sik ;
    float tr  ;
    float ti  ;
    float processedReal_ ;
    float processedImag_ ;
    float tmagn;


    for( InclusiveIndexRange workingRange( workingRange_ ); workingRange; ++workingRange )
    {
        ///  "...return to the imagery domain, convolve with delayed buffer 
        ///   values, calculate weighted difference using reversed power
        ///   spectrum value and multiply it with input..."

        // Product of two complex numbers: 
        //       (a+ib)(c+id) = ( ac - bd ) + i( ad + bc )
        // Here: (a+ib)(c-id) = ( ac + bd ) + i(-ad + bc )
        // Why?
        tr =    data.reals()[ *workingRange ] * cs.prevReals_[ *workingRange ] + data.imags()[ *workingRange ] * cs.prevImags_[ *workingRange ];
        ti =  - data.reals()[ *workingRange ] * cs.prevImags_[ *workingRange ] + data.imags()[ *workingRange ] * cs.prevReals_[ *workingRange ];

        /// \todo Find out how "fastMath::magnitude3 works.
        ///       http://groups.google.com/group/comp.dsp/browse_frm/thread/981e0ff3e58e078c/e55848cb1cce09a5?lnk=st&q=%2Bclay+%2Bturn
        ///                                   (13.01.2010.) (Danijel Domazet)
        tmagn = fastMath::magnitude3(tr, ti); 

        if (tmagn == 0) 
            tmagn = 1.0f;

        // 
        processedReal_ = tr / tmagn;
        processedImag_ = ti / tmagn;

        // 
        tsik = (cs.processedPrevReals_[ *workingRange ] * processedImag_) - (cs.processedPrevImags_[ *workingRange ] * processedReal_);
        sik  = (tsik * tsik);
        sik -= threshold_;
        sik *= (sign_ * (float)(1E+020)); // noise / pitched  
        sik  = LIMIT0(sik, 1.f);

        // Store for next frame:
        cs.prevReals_[ *workingRange ] = data.reals()[ *workingRange ];
        cs.prevImags_[ *workingRange ] = data.imags()[ *workingRange ];

        // New spectrum:
        data.reals()[ *workingRange ] *= sik;
        data.imags()[ *workingRange ] *= sik;

        // Store last processed data:
        cs.processedPrevReals_[ *workingRange ] = processedReal_;
        cs.processedPrevImags_[ *workingRange ] = processedImag_;
    }
}


//------------------------------------------------------------------------------
} // namespace Algorithms
//------------------------------------------------------------------------------
} // namespace LE
