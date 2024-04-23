////////////////////////////////////////////////////////////////////////////////
///
/// tuneWorxImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "tuneWorxImpl.hpp"

#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/parameters/uiElements.hpp"
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

// Papers/lectures:

// Discussions:

// 3rd party SDKs:
// - free:
// - commercial:


// 3rd party end products:
// - free:
// http://www.gvst.co.uk/gsnap.htm
// - commercial:
// http://www.antarestech.com/products/auto-tune_live.shtml


////////////////////////////////////////////////////////////////////////////////
//
// TuneWorx static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const TuneWorx   ::title[] = "TuneWorx";
char const TuneWorxPVD::title[] = "TuneWorx (pvd)";

char const Detail::TuneWorxBase::description[] = "Chromatic scale pitch discretization.";

EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Key          , "Key" )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi01       , "1"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi02       , "2"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi03       , "3"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi04       , "4"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi05       , "5"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi06       , "6"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi07       , "7"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi08       , "8"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi09       , "9"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi10       , "10"  )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi11       , "11"  )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Semi12       , "12"  )
#ifdef LE_SW_SDK_BUILD
/// \note Actual names commented out because of the new DoxyHelper process which
/// needs to report "GUI" names of parameters.
///                                           (22.10.2013.) (Domagoj Saric)
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi01 , "N/A" ) //"Bypass 1"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi02 , "N/A" ) //"Bypass 2"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi03 , "N/A" ) //"Bypass 3"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi04 , "N/A" ) //"Bypass 4"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi05 , "N/A" ) //"Bypass 5"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi06 , "N/A" ) //"Bypass 6"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi07 , "N/A" ) //"Bypass 7"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi08 , "N/A" ) //"Bypass 8"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi09 , "N/A" ) //"Bypass 9"                )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi10 , "N/A" ) //"Bypass 10"               )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi11 , "N/A" ) //"Bypass 11"               )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::BypassSemi12 , "N/A" ) //"Bypass 12"               )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::PitchMinFreq , "N/A" ) //"Pitch range lower bound" )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::PitchMaxFreq , "N/A" ) //"Pitch range upper bound" )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::TuneTolerance, "N/A" ) //"Out of tune tolerance"   )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::RetuneTime   , "N/A" ) //"Retune time"             )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::PitchShift   , "N/A" ) //"Pitch shift"             )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::Vibrato      , "N/A" ) //"Vibrato"                 )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::VibratoDepth , "N/A" ) //"Vibrato extent"          )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::VibratoPeriod, "N/A" ) //"Vibrato rate"            )
EFFECT_PARAMETER_NAME( Detail::TuneWorxBase::VibratoDelay , "N/A" ) //"Vibrato delay"           )
#endif // LE_SW_SDK_BUILD

EFFECT_ENUMERATED_PARAMETER_STRINGS
(
    Detail::TuneWorxBase, Key,
    (( A  , "A"  ))
    (( Ais, "A#" ))
    (( B  , "B"  ))
    (( C  , "C"  ))
    (( Cis, "C#" ))
    (( D  , "D"  ))
    (( Dis, "D#" ))
    (( E  , "E"  ))
    (( F  , "F"  ))
    (( Fis, "F#" ))
    (( G  , "G"  ))
    (( Gis, "G#" ))
)


namespace Detail
{
    LE_OPTIMIZE_FOR_SIZE_BEGIN()

    void LE_COLD TuneWorxBaseImpl::setup( IndexRange const &, Engine::Setup const & engineSetup )
    {
        // Tones:
        {
            Music::Scale::ToneOffsets & userTones( userScale_.toneOffsets() );

            boost::int8_t pos( 0 );

        #ifdef LE_SW_SDK_BUILD
            if ( parameters().get<Semi01>() && !parameters().get<BypassSemi01>() ) { userTones[ pos++ ] =  0; }
            if ( parameters().get<Semi02>() && !parameters().get<BypassSemi02>() ) { userTones[ pos++ ] =  1; }
            if ( parameters().get<Semi03>() && !parameters().get<BypassSemi03>() ) { userTones[ pos++ ] =  2; }
            if ( parameters().get<Semi04>() && !parameters().get<BypassSemi04>() ) { userTones[ pos++ ] =  3; }
            if ( parameters().get<Semi05>() && !parameters().get<BypassSemi05>() ) { userTones[ pos++ ] =  4; }
            if ( parameters().get<Semi06>() && !parameters().get<BypassSemi06>() ) { userTones[ pos++ ] =  5; }
            if ( parameters().get<Semi07>() && !parameters().get<BypassSemi07>() ) { userTones[ pos++ ] =  6; }
            if ( parameters().get<Semi08>() && !parameters().get<BypassSemi08>() ) { userTones[ pos++ ] =  7; }
            if ( parameters().get<Semi09>() && !parameters().get<BypassSemi09>() ) { userTones[ pos++ ] =  8; }
            if ( parameters().get<Semi10>() && !parameters().get<BypassSemi10>() ) { userTones[ pos++ ] =  9; }
            if ( parameters().get<Semi11>() && !parameters().get<BypassSemi11>() ) { userTones[ pos++ ] = 10; }
            if ( parameters().get<Semi12>() && !parameters().get<BypassSemi12>() ) { userTones[ pos++ ] = 11; }

            boost::int8_t const userNumTones( pos );

			if ( parameters().get<BypassSemi01>() ) { userTones[ pos++ ] =  0; }
			if ( parameters().get<BypassSemi02>() ) { userTones[ pos++ ] =  1; }
			if ( parameters().get<BypassSemi03>() ) { userTones[ pos++ ] =  2; }
			if ( parameters().get<BypassSemi04>() ) { userTones[ pos++ ] =  3; }
			if ( parameters().get<BypassSemi05>() ) { userTones[ pos++ ] =  4; }
			if ( parameters().get<BypassSemi06>() ) { userTones[ pos++ ] =  5; }
			if ( parameters().get<BypassSemi07>() ) { userTones[ pos++ ] =  6; }
			if ( parameters().get<BypassSemi08>() ) { userTones[ pos++ ] =  7; }
			if ( parameters().get<BypassSemi09>() ) { userTones[ pos++ ] =  8; }
			if ( parameters().get<BypassSemi10>() ) { userTones[ pos++ ] =  9; }
			if ( parameters().get<BypassSemi11>() ) { userTones[ pos++ ] = 10; }
			if ( parameters().get<BypassSemi12>() ) { userTones[ pos++ ] = 11; }

			boost::int8_t const userBypassedTones( pos - userNumTones );
        #else
            boost::ignore_unused_variable_warning( engineSetup );
            if ( parameters().get<Semi01>() ) { userTones[ pos++ ] =  0; }
            if ( parameters().get<Semi02>() ) { userTones[ pos++ ] =  1; }
            if ( parameters().get<Semi03>() ) { userTones[ pos++ ] =  2; }
            if ( parameters().get<Semi04>() ) { userTones[ pos++ ] =  3; }
            if ( parameters().get<Semi05>() ) { userTones[ pos++ ] =  4; }
            if ( parameters().get<Semi06>() ) { userTones[ pos++ ] =  5; }
            if ( parameters().get<Semi07>() ) { userTones[ pos++ ] =  6; }
            if ( parameters().get<Semi08>() ) { userTones[ pos++ ] =  7; }
            if ( parameters().get<Semi09>() ) { userTones[ pos++ ] =  8; }
            if ( parameters().get<Semi10>() ) { userTones[ pos++ ] =  9; }
            if ( parameters().get<Semi11>() ) { userTones[ pos++ ] = 10; }
            if ( parameters().get<Semi12>() ) { userTones[ pos++ ] = 11; }
            boost::int8_t const userNumTones     ( pos );
            boost::int8_t const userBypassedTones(   0 );
        #endif // LE_SW_SDK_BUILD

            userScale_.tonesUpdated( userNumTones, userBypassedTones );
        }
    #if defined( LE_SW_SDK_BUILD )
		// Retune Time
		{
			retuneTime_ = engineSetup.milliSecondsToSteps( parameters().get<RetuneTime>() );
		}
		// Pitch Shift
		{
            // cents to scale:
			pitchShift_ = Math::cents2Interval12TET( parameters().get<PitchShift>() );
		}
		// Vibrato
		{
            VibratoEffect::setup( parameters().get<VibratoPeriod>(), engineSetup );
			vibratoDelay_ = engineSetup.milliSecondsToSteps( parameters().get<VibratoDelay>() );
			vibratoDelay_ = std::max( vibratoDelay_, retuneTime_ );
		}
    #elif defined( LE_SW_TW_RETUNE_TEST )
        retuneTime_ = engineSetup.milliSecondsToSteps( 50 );
    #endif // LE_SW_SDK_BUILD
    }


#if defined( LE_MELODIFY_SDK_BUILD )
    static float const melodifyActualFixedPitch( 164.8137784564f          );
    static float       melodifyFixedPitch      ( melodifyActualFixedPitch );
    void melodifyAdjustFixedPitch( unsigned int const sampleRate )
    {
        melodifyFixedPitch = Math::convert<float>( sampleRate ) / 44100 * melodifyActualFixedPitch;
    }
#endif

    LE_OPTIMIZE_FOR_SIZE_END()

#if defined( _MSC_VER ) && !defined( _M_X64 ) && defined( LE_SW_SDK_BUILD )
    LE_OPTIMIZE_FOR_SIZE_BEGIN()
#else
    LE_OPTIMIZE_FOR_SPEED_BEGIN()
#endif // MSVC10 x86 bad codegen workaround

    float LE_NOINLINE LE_HOT TuneWorxBaseImpl::findNewPitchScale( Engine::ChannelData_AmPh const & data, Engine::Setup const & engineSetup, ChannelState & cs ) const
    {
	#ifdef LE_SW_SDK_BUILD
	    // Apply vibrato if selected
		float const vibratoPitch
        (
            parameters().get<Vibrato>() && ( cs.tuneStep == vibratoDelay_ || userScale_.numberOfTones() == 0 )
                ? findVibratoPitch( cs )
                : 1
        );
    #else
        unsigned int const vibratoPitch( 1 );
        unsigned int const pitchShift_ ( 1 );
    #endif // LE_SW_SDK_BUILD

        if ( userScale_.numberOfTones() == 0 )
            return pitchShift_ * vibratoPitch;

        // Current pitch:
        #ifdef LE_MELODIFY_SDK_BUILD
            boost::ignore_unused_variable_warning( data        );
            boost::ignore_unused_variable_warning( engineSetup );
        #endif // LE_MELODIFY_SDK_BUILD
        float const pitch
        (
        #if defined( LE_MELODIFY_SDK_BUILD )
            Detail::melodifyFixedPitch
        #else
            PitchDetector::findPitch
            (
                data.full().amps(),
                cs,
            #ifdef LE_SW_SDK_BUILD
                parameters().get<PitchMinFreq>(),
                parameters().get<PitchMaxFreq>(),
            #else
                70,
                70*2*2*2*2*2, // 5 octaves
            #endif // LE_SW_SDK_BUILD
                engineSetup
            )
        #endif // LE_MELODIFY_SDK_BUILD
        );

        if ( pitch == 0 )
		{
		#if defined( LE_SW_SDK_BUILD ) || defined( LE_SW_TW_RETUNE_TEST )
            cs.tuneStep = 0;
        #endif // LE_SW_SDK_BUILD
            return pitchShift_ * vibratoPitch;
		}

        // Target tone - detected pitch snapped to user scale:
        float const freqScaletone( userScale_.snap2Scale( pitch, parameters().get<Key>() ) );

	#if defined( LE_SW_SDK_BUILD )
		// If the newly detected pitch differs from the previous targeted tone,
		// set tuneStep to 0 and update the last targeted tone:
		if ( freqScaletone != cs.lastTargetTone )
		{
            cs.tuneStep = 0;
			cs.lastTargetTone = freqScaletone;
		}

		// If the detected pitch is inside the tolerance area from a tone, do
		// not autotune:
		if ( Math::abs( freqScaletone - pitch ) < parameters().get<TuneTolerance>() )
		{
			cs.tuneStep = 0;
			return pitchShift_ * vibratoPitch;
		}

        // Target scale:
		unsigned int const currentTuneStep( cs.tuneStep < retuneTime_ ? cs.tuneStep : retuneTime_ );
        float        const pitchScale     ( 1 + Math::convert<float>( currentTuneStep + 1 ) / Math::convert<float>( retuneTime_ + 1 ) * ( freqScaletone - pitch ) / pitch );

		if ( cs.tuneStep < vibratoDelay_ )
		{
			++cs.tuneStep;
            // Reset vibrato:
			cs.frameCounter.reset();
		}

        return pitchScale * pitchShift_ * vibratoPitch;
    #elif defined( LE_SW_TW_RETUNE_TEST )
        if ( freqScaletone != cs.lastTargetTone )
        {
            cs.tuneStep       = 0;
            cs.lastTargetTone = freqScaletone;
        }
        unsigned int const currentTuneStep( cs.tuneStep < retuneTime_ ? cs.tuneStep : retuneTime_ );
        float        const pitchScale     ( 1 + Math::convert<float>( currentTuneStep + 1 ) / Math::convert<float>( retuneTime_ + 1 ) * ( freqScaletone - pitch ) / pitch );
        if ( cs.tuneStep < retuneTime_ )
            ++cs.tuneStep;
        return pitchScale;
    #else
        float const pitchScale( freqScaletone / pitch );
        return pitchScale;
    #endif // LE_SW_SDK_BUILD
    }


#ifdef LE_SW_SDK_BUILD
	float LE_HOT TuneWorxBaseImpl::findVibratoPitch( ChannelState & cs ) const
	{
        return VibratoEffect::calculateNewPitch( cs, parameters().get<SpringType>(), parameters().get<VibratoDepth>() );
	}
#endif // LE_SW_SDK_BUILD

#if defined( _MSC_VER ) && !defined( _M_X64 ) && defined( LE_SW_SDK_BUILD )
    LE_OPTIMIZE_FOR_SIZE_END()
#else
    LE_OPTIMIZE_FOR_SPEED_END()
#endif // MSVC10 x86 bad codegen workaround
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
//
// TuneWorxImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void TuneWorxImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & setup ) const
{
    float const newPitch( TuneWorxBaseImpl::findNewPitchScale( data, setup, cs ) );
    PitchShifter::process( newPitch, cs, std::forward<Engine::ChannelData_AmPh>( data ), setup );
}


////////////////////////////////////////////////////////////////////////////////
//
// TuneWorxPVDImpl::process()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

void TuneWorxPVDImpl::process( ChannelState & cs, Engine::ChannelData_AmPh data, Engine::Setup const & setup ) const
{
    float const newPitch( TuneWorxBaseImpl::findNewPitchScale( data, setup, cs ) );
    PVPitchShifter::process( newPitch, std::forward<Engine::ChannelData_AmPh>( data ), setup );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
