////////////////////////////////////////////////////////////////////////////////
///
/// lfoImpl.cpp
/// -----------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// http://en.wikipedia.org/wiki/Time_signature
// http://en.wikipedia.org/wiki/Mensural_notation
// http://www.u-he.com/zebra/manual/?page_id=15
// http://www.kvraudio.com/forum/viewtopic.php?t=280178
// http://www.kvraudio.com/forum/viewtopic.php?t=196528
// http://www.kvraudio.com/forum/viewtopic.php?t=212337
// http://www.kvraudio.com/forum/viewtopic.php?t=214140
// http://www.kvraudio.com/forum/viewtopic.php?t=257719
// http://www.kvraudio.com/forum/viewtopic.php?t=270213
// http://www.kvraudio.com/forum/viewtopic.php?t=170968
// http://web.forret.com/tools/bpm_tempo.asp
// http://testtone.com/calculators/lfo-speed-calculator
// http://mp3.deepsound.net/eng/samples_calculs.php
//------------------------------------------------------------------------------
#include "lfoImpl.hpp"

#include "le/math/constants.hpp"
#include "le/math/math.hpp"
#include "le/parameters/conversion.hpp"
#include "le/parameters/fusionAdaptors.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/parentFromMember.hpp"

#include "boost/assert.hpp"
#include "boost/range/algorithm/find_if.hpp"
#include "boost/range/algorithm/min_element.hpp"
#include "boost/range/counting_range.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

UI_NAME( LFOImpl::Enabled     ) = "on"  ;
UI_NAME( LFOImpl::PeriodScale ) = "T"   ;
UI_NAME( LFOImpl::Phase       ) = "ph"  ;
UI_NAME( LFOImpl::LowerBound  ) = "lbnd";
UI_NAME( LFOImpl::UpperBound  ) = "ubnd";
UI_NAME( LFOImpl::SyncTypes   ) = "sync";
UI_NAME( LFOImpl::Waveform    ) = "wfrm";

//...mrmlj...this does not work yet because the Window enum is not a member
//...of the WindowFunction parameter class...fix this...
//ENUMERATED_PARAMETER_STRINGS
//(
//    LFOImpl, Waveform,
//    (( Sine           , "Sine"       ))
//    (( Triangle       , "Triangle"   ))
//    (( Sawtooth       , "Sawtooth"   ))
//    (( ReverseSawtooth, "htootwaS"   ))
//    (( Square         , "Square"     ))
//    (( Exponent       , "Exponent"   ))
//    (( RandomHold     , "Hrandom"    ))
//    (( RandomSlide    , "Grandom"    ))
//    (( Whacko         , "Whacko"     ))
//    (( Dirac          , "Dirac up"   ))
//    (( dIRAC          , "Dirac down" ))
//)

template <>
char const * LE_RESTRICT const DiscreteValues<Parameters::LFOImpl::Waveform>::strings[] =
{
    "Sine"      ,
    "Triangle"  ,
    "Sawtooth"  ,
    "htootwaS"  ,
    "Square"    ,
    "Exponent"  ,
    "Hrandom"   ,
    "Grandom"   ,
    "Whacko"    ,
    "Dirac up"  ,
    "Dirac down"
};

using Enabled     = LFOImpl::Enabled    ;
using PeriodScale = LFOImpl::PeriodScale;
using Phase       = LFOImpl::Phase      ;
using LowerBound  = LFOImpl::LowerBound ;
using UpperBound  = LFOImpl::UpperBound ;
using SyncTypes   = LFOImpl::SyncTypes  ;


void LFO::setEnabled( bool const value )
{
    static_cast<LFOImpl &>( *this ).parameters().set<Enabled>( value );
}


void LFO::setPhase( LFOImpl::value_type const newPhase )
{
    BOOST_ASSERT( newPhase >= -0.5 );
    BOOST_ASSERT( newPhase <= +0.5 );
    static_cast<LFOImpl &>( *this ).parameters().set<Phase>( newPhase );
}


void LFO::setWaveform( Waveform const waveform )
{
    static_cast<LFOImpl &>( *this ).parameters().set<LFOImpl::Waveform>( waveform );
}


bool LFO::setLowerBound( LFOImpl::value_type const newLowerBound )
{
    static_cast<LFOImpl &>( *this ).setLowerBound( newLowerBound );
    auto const upperBound( this->upperBound() );
    if ( upperBound < newLowerBound )
    {
        setUpperBound( newLowerBound );
        return true;
    }
    return false;
}


bool LFO::setUpperBound( LFOImpl::value_type const newUpperBound )
{
    BOOST_ASSERT( LFOImpl::isValueInRange( newUpperBound ) );
    static_cast<LFOImpl &>( *this ).setUpperBound( newUpperBound );
    auto const lowerBound( this->lowerBound() );
    if ( lowerBound > newUpperBound )
    {
        setLowerBound( newUpperBound );
        return true;
    }
    return false;
}


std::uint16_t LFO::setPeriodInMilliseconds( std::uint16_t const periodInMilisecons )
{
    return Math::convert<std::uint16_t>( setPeriodInSeconds( periodInMilisecons / 1000.0f ) * 1000.0f );
}

float LFO::setPeriodInSeconds( float const periodInSeconds )
{
    auto & impl( static_cast<LFOImpl &>( *this ) );
    auto const periodScale( periodInSeconds / LFOImpl::Timer::basePeriod() );
    auto const clampedPeriodScale
    (
        ( impl.syncTypes() == LFO::Free )
            ? impl.clampFreePeriod ( periodScale                   )
            : impl.snapSyncedPeriod( periodScale, impl.syncTypes() ).first
    );
    impl.setPeriodScale( clampedPeriodScale );
    return clampedPeriodScale * LFOImpl::Timer::basePeriod();
}


/// \todo These synchronization type altering functions do not automatically
/// cause period scale resnapping. Reconsider this.
///                                       (23.02.2011.) (Domagoj Saric)
void LFO::addSyncType( SyncType const syncType )
{
    auto & parameters( static_cast<LFOImpl &>( *this ).parameters() );
    BOOST_ASSERT( syncType && "No sync type specified." );
    BOOST_ASSERT( !hasEnabledSync( syncType ) && "Sync type already enabled." );
    parameters.set<SyncTypes>( parameters.get<SyncTypes>().getValue() | syncType );
}


void LFO::removeSyncType( SyncType const syncType )
{
    auto & parameters( static_cast<LFOImpl &>( *this ).parameters() );
    BOOST_ASSERT( syncType && "No sync type specified." );
    BOOST_ASSERT( hasEnabledSync( syncType ) && "Sync type not enabled." );
    parameters.set<SyncTypes>( parameters.get<SyncTypes>().getValue() & ~syncType );
}

LE_NOTHROWNOALIAS
bool LFO::hasEnabledSync( SyncType const syncType ) const
{
    return ( syncTypes() & syncType ) != 0;
}


LE_NOTHROWNOALIAS          bool       LFO::enabled   () const { return                             static_cast<LFOImpl const &>( *this ).parameters().get<         Enabled   >()             ; }
LE_NOTHROWNOALIAS std    ::uint8_t    LFO::syncTypes () const { return                             static_cast<LFOImpl const &>( *this ).parameters().get<         SyncTypes >()             ; }
LE_NOTHROWNOALIAS LFOImpl::value_type LFO::phase     () const { return                             static_cast<LFOImpl const &>( *this ).parameters().get<         Phase     >()             ; }
LE_NOTHROWNOALIAS LFOImpl::value_type LFO::lowerBound() const { return                             static_cast<LFOImpl const &>( *this ).parameters().get<         LowerBound>()             ; }
LE_NOTHROWNOALIAS LFOImpl::value_type LFO::upperBound() const { return                             static_cast<LFOImpl const &>( *this ).parameters().get<         UpperBound>()             ; }
LE_NOTHROWNOALIAS LFO    ::Waveform   LFO::waveForm  () const { return static_cast<LFO::Waveform>( static_cast<LFOImpl const &>( *this ).parameters().get<LFOImpl::Waveform  >().getValue() ); }

LFOImpl::value_type LFOImpl::periodScale() const { return parameters().get<PeriodScale>(); }


void LFOImpl::setLowerBound( value_type const newLowerBound )
{
    BOOST_ASSERT( isValueInRange( newLowerBound ) );
    parameters().set<LowerBound>( newLowerBound );
}


void LFOImpl::setUpperBound( value_type const newUpperBound )
{
    BOOST_ASSERT( isValueInRange( newUpperBound ) );
    parameters().set<UpperBound>( newUpperBound );
}


void LFOImpl::setPeriodScale( value_type const newPeriodScale )
{
    BOOST_ASSERT( newPeriodScale >= LFOImpl::currentPeriodScaleMinimum() );
    BOOST_ASSERT( newPeriodScale <= LFOImpl::currentPeriodScaleMaximum() );
    parameters().set<PeriodScale>( newPeriodScale );
}


LFOImpl::value_type LE_NOTHROWNOALIAS LFOImpl::currentPeriodScaleMinimum() { return ( 2.0f / 3.0f /*for triplets    */ ) / LFOImpl::minimumPeriodAsMaximumBeatDenominator / LFOImpl::Timer::measureNumeratorFloat(); }
LFOImpl::value_type LE_NOTHROWNOALIAS LFOImpl::currentPeriodScaleMaximum() { return ( 3.0f / 2.0f /*for dotted notes*/ ) * LFOImpl::maximumPeriodInNumberOfBars                                                    ; }

namespace
{
    using lfo_value_t = LFOImpl::value_type;

    template <int inputMinimum, int inputMaximum, typename Input>
    lfo_value_t convertToLFORange( Input const inputvalue )
    {
        return Math::convertLinearRange
        <
            lfo_value_t, LFOImpl::minimumValue, LFOImpl::maximumValue - LFOImpl::minimumValue, 1,
            Input      , inputMinimum         , inputMaximum          - inputMinimum         , 1
        >( inputvalue );
    }


    using LFOState = lfo_value_t[ 2 ];

    lfo_value_t LE_FASTCALL sine( lfo_value_t const position, LFOState &, bool /*newPeriodBegun*/ )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );
        lfo_value_t const offsetSine( - std::cos( Math::Constants::twoPi * position ) );
        return convertToLFORange<-1, +1>( offsetSine );
    }


    lfo_value_t LE_FASTCALL sawtooth( lfo_value_t const position, LFOState &, bool /*newPeriodBegun*/ )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );
        return convertToLFORange<0, 1>( position );
    }


    lfo_value_t LE_FASTCALL reverseSawtooth( lfo_value_t const position, LFOState &, bool /*newPeriodBegun*/ )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );
        return convertToLFORange<0, 1>( 1 - position );
    }


    lfo_value_t LE_FASTCALL triangle( lfo_value_t const position, LFOState & state, bool const newPeriodBegun )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );
        return ( position < 0.5f )
                ? sawtooth       (   position          * 2.0f, state, newPeriodBegun )
                : reverseSawtooth( ( position - 0.5f ) * 2.0f, state, newPeriodBegun );
    }


    lfo_value_t LE_FASTCALL square( lfo_value_t const position, LFOState &, bool /*newPeriodBegun*/ )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );
        return position > 0.5f;
    }


    lfo_value_t LE_FASTCALL exponent( lfo_value_t const position, LFOState &, bool /*newPeriodBegun*/ )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );

        lfo_value_t const e( 2.71828182845904523536f );

        lfo_value_t exponent;
             if ( position < 0.5f ) exponent = (     position ) * 2;
        else if ( position > 0.5f ) exponent = ( 1 - position ) * 2;
        else                        exponent = 1;

        lfo_value_t const result( Math::exp( exponent ) );
        return Math::convertLinearRange
               <
                 lfo_value_t, LFOImpl::minimumValue, LFOImpl::maximumValue - LFOImpl::minimumValue, 1,
                 lfo_value_t
               >
               ( result, 1.0f, e );
    }


    lfo_value_t LE_FASTCALL randomWhacko( lfo_value_t /*position*/, LFOState &, bool /*newPeriodBegun*/ )
    {
        return Math::normalisedRand();
    }


    lfo_value_t LE_FASTCALL randomHold( lfo_value_t const position, LFOState & state, bool const newPeriodBegun )
    {
        if ( newPeriodBegun )
        {
            state[ 0 ] = randomWhacko( position, state, newPeriodBegun );
        }
        return state[ 0 ];
    }


    lfo_value_t LE_FASTCALL randomSlide( lfo_value_t const position, LFOState & state, bool const newPeriodBegun )
    {
        LE_ASSUME( position >= 0 );
        LE_ASSUME( position <= 1 );
        if ( newPeriodBegun )
        {
            lfo_value_t const oldTarget( state[ 0 ] + state[ 1 ]                         );
            lfo_value_t const newTarget( randomWhacko( position, state, newPeriodBegun ) );
            state[ 0 ] = newTarget - oldTarget;
            state[ 1 ] = oldTarget;
        }

        return position * state[ 0 ] + state[ 1 ];
    }


    lfo_value_t LE_FASTCALL dirac( lfo_value_t /*position*/, LFOState &, bool const newPeriodBegun )
    {
        if ( newPeriodBegun )
            return static_cast<lfo_value_t>( LFOImpl::maximumValue );
        else
            return static_cast<lfo_value_t>( LFOImpl::minimumValue );
    }


    lfo_value_t LE_FASTCALL diracUpsideDown( lfo_value_t /*position*/, LFOState &, bool const newPeriodBegun )
    {
        if ( newPeriodBegun )
            return static_cast<lfo_value_t>( LFOImpl::minimumValue );
        else
            return static_cast<lfo_value_t>( LFOImpl::maximumValue );
    }

    /// \note Workaround for a Clang 3.5 crash.
    ///                                       (02.01.2015.) (Domagoj Saric)
#ifdef __clang__
    typedef decltype( &sine ) GetWaveformAmplitudeForPosition;
#else
    typedef lfo_value_t (LE_FASTCALL * GetWaveformAmplitudeForPosition)( lfo_value_t position, LFOState &, bool newPeriodBegun )  LE_MSVC_SPECIFIC( throw() );
#endif // __clang__

    LE_MSVC_SPECIFIC( LE_WEAK_SYMBOL_CONST )
    GetWaveformAmplitudeForPosition const lfoFunctions[] =
    {
        &sine           ,
        &triangle       ,
        &sawtooth       ,
        &reverseSawtooth,
        &square         ,
        &exponent       ,
        &randomHold     ,
        &randomSlide    ,
        &randomWhacko   ,
        &dirac          ,
        &diracUpsideDown
    };
} // anonymous namespace

LE_NOTHROWNOALIAS
LFOImpl::LFOImpl()
{
    state_[ 0 ] = minimumValue;
    state_[ 1 ] =            0;
}


LFOImpl::value_type LE_NOTHROWNOALIAS LFOImpl::getWaveformAmplitudeForPosition( value_type const position, bool const newPeriodBegun ) const
{
    return lfoFunctions[ waveForm() ]( position, state_, newPeriodBegun );
}


LFOImpl::value_type LE_NOTHROWNOALIAS LFOImpl::getValue( Timer const & timer ) const
{
    value_type const periodScale( this->periodScale() );

    //...mrmlj...
#ifndef NDEBUG
    value_type const periodOffset( Math::abs( periodScale * phase() ) );
#else
    value_type const periodOffset(            periodScale * phase()   );
#endif // _DEBUG

    value_type const currentPeriodNormalisedPosition( Math::splitFloat( ( periodOffset + timer.currentTimeInBars() ) / periodScale ).fractional );
    BOOST_ASSERT( currentPeriodNormalisedPosition >= 0 );
    BOOST_ASSERT( currentPeriodNormalisedPosition <= 1 );

    value_type const previousPeriodPosition                ( Math::PositiveFloats::modulo( ( periodOffset + timer.previousTimeInBars() ), periodScale ) );
    value_type const previousTimeDifferenceToPeriodBoundary( periodScale - previousPeriodPosition                                                       );
    value_type const periodEndForPreviousTime              ( timer.previousTimeInBars() + previousTimeDifferenceToPeriodBoundary                        );
    bool       const newPeriod                             ( timer.currentTimeInBars () > periodEndForPreviousTime                                      );

    value_type const newValue( getWaveformAmplitudeForPosition( currentPeriodNormalisedPosition, newPeriod ) );

    BOOST_ASSERT( isValueInRange( newValue ) );
    value_type const result
    (
        Math::convertLinearRange
        <
            value_type,
            value_type, LFOImpl::minimumValue, LFOImpl::maximumValue - LFOImpl::minimumValue, 1
        >
        (
           newValue    ,
           lowerBound(),
           upperBound()
        )
    );
    BOOST_ASSERT( isValueInBounds( result ) );

    return result;
}


// Implementation note:
//   'Free' LFO absolute<->relative value conversion. See the implementation
// note for the static barDuration member for more details.
//                                            (07.01.2011.) (Domagoj Saric)
template <>
LFOImpl::PeriodScale::value_type LFOImpl::adjustValueForPreset( PeriodScale const & periodScale ) const
{
    BOOST_ASSERT( periodScale == this->periodScale() );
    if ( syncTypes() == LFO::Free )
    {
        return periodScale * LFOImpl::Timer::basePeriod() * 1000;
    }
    return periodScale;
}


template <>
LFOImpl::PeriodScale::value_type LFOImpl::adjustValueFromPreset<LFOImpl::PeriodScale>( LFOImpl::PeriodScale::value_type const periodScale ) const
{
    if ( syncTypes() == LFO::Free )
        return clampFreePeriod ( periodScale / LFOImpl::Timer::basePeriod() / 1000 );
    else
        return snapSyncedPeriod( periodScale, syncTypes() ).first;
}


void LFOImpl::snapPeriodScaleFromAutomation( PeriodScale & periodScale )
{
    auto const & parameters( Utility::FusionContainerFromMember<Parameters, 1                             >()( periodScale ) );
    auto const & lfo       ( Utility::ParentFromMember         <LFOImpl, Parameters, &LFOImpl::parameters_>()( parameters  ) );

    if ( lfo.syncTypes() != LFO::Free )
        periodScale = snapSyncedPeriod( periodScale, lfo.syncTypes() ).first;
    else
    {
        BOOST_ASSERT( clampFreePeriod( periodScale ) == periodScale );
    }
}


bool LFOImpl::isValueInBounds( value_type const value ) const
{
    return Math::isValueInRange<value_type>( value, lowerBound(), upperBound() );
}


bool LFOImpl::isValueInRange( value_type const value )
{
    return Math::isValueInRange<value_type>( value, static_cast<value_type>( minimumValue ), static_cast<value_type>( maximumValue ) );
}


LFOImpl::value_type LFOImpl::clampFreePeriod( value_type const absolutePeriod )
{
    return Math::clamp( absolutePeriod, currentPeriodScaleMinimum(), currentPeriodScaleMaximum() );
}


namespace
{
    lfo_value_t snapSyncedPeriodScale( lfo_value_t const periodScale )
    {
        using namespace Math;

        float        const measureNumerator ( LFOImpl::Timer::measureNumeratorFloat() );
        std::uint8_t const numberOfBeats    ( round( periodScale * measureNumerator ) );
        if ( numberOfBeats > LFOImpl::Timer::measureNumerator() )
        {
            float const numberOfBeatsClampedToPowerOfTwo( convert<float>( PowerOfTwo::round( numberOfBeats ) ) );

            float const lowerBound( 1.0f                                 );
            float const upperBound( LFOImpl::maximumPeriodInNumberOfBars );

            return clamp( numberOfBeatsClampedToPowerOfTwo / measureNumerator, lowerBound, upperBound );
        }
        else
        if ( numberOfBeats > 0 )
        {
            BOOST_ASSERT
            (
                ( numberOfBeats >= 1                                  ) &&
                ( numberOfBeats <= LFOImpl::Timer::measureNumerator() )
            );

            auto const wholeDivisorFinder( []( std::uint8_t const value ) { return LFOImpl::Timer::measureNumerator() % value == 0; } );

            auto const upperClosest( *boost::find_if( boost::counting_range( numberOfBeats    , LFOImpl::Timer::measureNumerator() ), wholeDivisorFinder ) );
            auto const lowerClosest( *boost::find_if( boost::counting_range( std::uint8_t( 1 ), numberOfBeats                      ), wholeDivisorFinder ) );
            auto const closest
            (
                ( ( upperClosest - numberOfBeats ) < ( numberOfBeats - lowerClosest ) )
                    ? upperClosest
                    : lowerClosest
            );
            return convert<float>( closest ) / measureNumerator;
        }
        else
        {
            BOOST_ASSERT( numberOfBeats < 1 );

            float const upperBound( 1          / measureNumerator                               );
            float const lowerBound( upperBound / LFOImpl::minimumPeriodAsMaximumBeatDenominator );

            float const clamped( clamp( periodScale, lowerBound, upperBound ) );

            float const scaledNumerator( LFOImpl::minimumPeriodAsMaximumBeatDenominator * measureNumerator );

            std::uint8_t const clampedToPowerOfTwo( PowerOfTwo::round( round( clamped * scaledNumerator ) ) );

            return convert<float>( clampedToPowerOfTwo ) / ( scaledNumerator );
        }
    }

    lfo_value_t snapSyncedPeriodScale( lfo_value_t const periodScale, float const tempoScale )
    {
        return snapSyncedPeriodScale( periodScale * tempoScale ) / tempoScale;
    }
} // anonymous namespace

LFOImpl::SnappedPeriod LFOImpl::snapSyncedPeriod( value_type const periodScale, std::uint8_t const syncTypes )
{
    BOOST_ASSERT( syncTypes != Free );

    float const quarterPeriod( snapSyncedPeriodScale( periodScale, 1 / 1.0f ) );
    float const tripletPeriod( snapSyncedPeriodScale( periodScale, 3 / 2.0f ) );
    float const dottedPeriod ( snapSyncedPeriodScale( periodScale, 2 / 3.0f ) );

    SnappedPeriod const nearestPeriods[] =
    {
        SnappedPeriod( ( syncTypes & Quarter ) ? quarterPeriod : std::numeric_limits<float>::max(), Quarter ),
        SnappedPeriod( ( syncTypes & Triplet ) ? tripletPeriod : std::numeric_limits<float>::max(), Triplet ),
        SnappedPeriod( ( syncTypes & Dotted  ) ? dottedPeriod  : std::numeric_limits<float>::max(), Dotted  )
    };

    return *boost::min_element
    (
        nearestPeriods,
        [=]( SnappedPeriod const & left, SnappedPeriod const & right )
        {
            return
            (
                Math::abs( right.first - periodScale ) >
                Math::abs( left .first - periodScale )
            );
        }
    );
}


LFOImpl::SnappedPeriod LFOImpl::snapPeriodScale( value_type const periodScale, std::uint8_t const syncTypes )
{
    return ( syncTypes == Free )
                ? SnappedPeriod( clampFreePeriod( periodScale ), Free )
                : snapSyncedPeriod( periodScale, syncTypes );
}


void LFOImpl::updateForNewTimingInformation( Timer::TimingInformationChange const & timingInformationChage )
{
    if ( syncTypes() == Free )
    {
        if ( timingInformationChage.barDurationChanged() )
            setPeriodScale( clampFreePeriod( periodScale() * timingInformationChage.barDurationChangeRatio_ ) );
    }
    else
    {
        BOOST_ASSERT( syncTypes() != Free );
        if ( timingInformationChage.measureNumeratorChanged() )
            setPeriodScale( snapSyncedPeriod( periodScale(), syncTypes() ).first );
    }
}


namespace
{
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    struct LFOParameterGetter
    {
        typedef Plugins::AutomatedParameterValue result_type;

        #pragma warning( push )
        #pragma warning( disable : 4127 ) // Conditional expression is constant.
        template <class Parameter>
        result_type operator()() const
        {
            BOOST_ASSERT_MSG( Parameter::isValidValue( Math::convert<typename Parameter::value_type>( value_ ) ), "Invalid LFO parameter value." );
            result_type result( LE::Parameters::convertParameterValueToLinearValue<result_type, 0, 1, 1, Parameter>( value_ ) );
            if ( std::is_same<Parameter, LFOImpl::PeriodScale>::value )
                result = LFOImpl::linearisePeriodScale( result );
            return result;
        }
        #pragma warning( pop )

        float const value_;
    }; // class LFOParameterGetter
    #pragma warning( pop )
}
Plugins::AutomatedParameterValue LFOImpl::internal2AutomatedValue( std::uint8_t const parameterIndex, float const internalValue, bool const normalised )
{
    if ( !normalised )
        return internalValue;

    LFOParameterGetter const getter = { internalValue };
    return LE::Parameters::invokeFunctorOnIndexedParameter<Parameters>( parameterIndex, getter );
}


//float LFOImpl::automated2InternalValue( std::uint8_t const parameterIndex, Plugins::AutomatedParameterValue const automatedValue, bool const normalised )
//{
//    if ( !normalised )
//        return automatedValue;
//    return 0;
//}


// LFOImpl::PeriodScale "linearization" section.
/// \todo Cleanup with a new 'logarithmic' parameter/control.
///                                           (25.01.2012.) (Domagoj Saric)
namespace
{
    static Plugins::AutomatedParameterValue normalisedPeriodScaleSkewFactor()
    {
        unsigned int const middleValue( 1                               );
        auto         const minimum    ( LFOImpl::PeriodScale::minimum() );
        auto         const maximum    ( LFOImpl::PeriodScale::maximum() );

        auto const normalisedMiddleValue( ( middleValue - minimum ) / ( maximum - minimum ) );

        Plugins::AutomatedParameterValue const skewFactor( - 1 / Math::log2( normalisedMiddleValue ) );

        return skewFactor;
    }
}

Plugins::AutomatedParameterValue LFOImpl::unlinearisePeriodScale( Plugins::AutomatedParameterValue const linearisedNormalisedPeriodScale )
{
    using value_type = Plugins::AutomatedParameterValue;
    value_type const normalisedSkewedAutomationValue  ( linearisedNormalisedPeriodScale );
    value_type const normalisedUnskewedAutomationValue( Math::exp( Math::ln( normalisedSkewedAutomationValue ) / normalisedPeriodScaleSkewFactor() ) );
    BOOST_ASSERT( Math::isNormalisedValue( normalisedUnskewedAutomationValue ) );
    return normalisedUnskewedAutomationValue;
}


Plugins::AutomatedParameterValue LFOImpl::linearisePeriodScale( Plugins::AutomatedParameterValue const nonlinearNormalisedPeriodScale )
{
    return std::pow( nonlinearNormalisedPeriodScale, normalisedPeriodScaleSkewFactor() );
}


// Implementation note:
//   'Free' LFO periods are saved to presets with absolute millisecond values so
// that their value would be restored correctly independent of the current BPM.
// The conversion between an absolute period value and a period scale (used for
// synced LFOs) requires the current bar duration, to avoid requiring the active
// LFOImpl::Timer instance to be passed to preset loading/saving code a global
// variable is used. This should not create problems if the assumption that no
// host uses more than one tempo value at any given time is correct.
//                                            (07.01.2011.) (Domagoj Saric)
// Assume 120 BPM 4/4
LFOImpl::value_type LFOImpl::Timer::barDuration_        ( 60.0f / 120 * 4 );
std::uint8_t        LFOImpl::Timer::measureNumerator_   ( 4               );
bool                LFOImpl::Timer::hasTempoInformation_( false           );


LFOImpl::Timer::Timer() { reset(); }


LFOImpl::Timer::TimingInformationChange LE_FASTCALL
LFOImpl::Timer::updatePositionAndTimingInformation
(
    float        const positionInBars,
    float        const barDuration,
    std::uint8_t const measureNumerator
)
{
    BOOST_ASSERT( std::isfinite( positionInBars ) );
    BOOST_ASSERT( std::isfinite( barDuration    ) );

    BOOST_ASSERT( positionInBars >= 0 );
    BOOST_ASSERT( barDuration    >= 0 );

    // Position
    previousTimeInBars_ = currentTimeInBars_;
    currentTimeInBars_  = positionInBars    ;

    // Timing info
    // Implementation note:
    //   Bar duration change alone (e.g. when only the tempo changes) would be
    // implicitly handled (i.e. no update would be required because we remember
    // LFO durations relative to bar durations anyway) if it weren't for free
    // LFOs which require that their (absolute) value remains constant
    // (therefore their relative duration must be updated when the bar
    // duration changes).
    //                                        (02.02.2011.) (Domagoj Saric)
    TimingInformationChange const changeInfo =
    {
        barDuration_      / barDuration,      // barDurationChangeRatio_
        measureNumerator_ != measureNumerator // measureNumeratorChanged_
    };

    hasTempoInformation_ = true;

    barDuration_      = barDuration;
    measureNumerator_ = measureNumerator;

    BOOST_ASSERT( std::isfinite( currentTimeInBars_  ) );
    BOOST_ASSERT( std::isfinite( previousTimeInBars_ ) );
    BOOST_ASSERT( std::isfinite( barDuration_        ) );

    return changeInfo;
}


LFOImpl::Timer::TimingInformationChange LE_FASTCALL
LFOImpl::Timer::updatePositionAndTimingInformation
(
    unsigned int const deltaNumberOfSamples,
    float        const sampleRate
)
{
    // Position
    float const timeToAdvanceInSeconds( Math::convert<float>( deltaNumberOfSamples  ) / sampleRate   );
    float const timeToAdvanceInBars   (                       timeToAdvanceInSeconds  / barDuration_ );

    BOOST_ASSERT( ( currentTimeInBars_ >= previousTimeInBars_ ) || ( currentTimeInBars_ == 0 ) );
    previousTimeInBars_  = currentTimeInBars_;
    currentTimeInBars_  += timeToAdvanceInBars;

    // Timing info
    // Assume 120 BPM 4/4
    std::uint8_t const measureNumerator( 4                              );
    float        const barDuration     ( 60.0f / 120 * measureNumerator );

    TimingInformationChange const changeInfo =
    {
        barDuration_      / barDuration,      // barDurationChangeRatio_
        measureNumerator_ != measureNumerator // measureNumeratorChanged_
    };

    hasTempoInformation_ = false;

    barDuration_         = barDuration;
    measureNumerator_    = measureNumerator;

    return changeInfo;
}


void LFOImpl::Timer::setPosition( unsigned int const numberOfSamples, float const sampleRate )
{
    float const timeInSeconds( Math::convert<float>( numberOfSamples ) / sampleRate );
    setPosition( timeInSeconds );
}

void LFOImpl::Timer::setPosition( float const timeInSeconds )
{
#ifndef LE_SW_SDK_BUILD
    // Assume 120 BPM 4/4
    LE_ASSUME( hasTempoInformation_ == false           );
    LE_ASSUME( barDuration_         == 4               );
    LE_ASSUME( measureNumerator_    == 60.0f / 120 * 4 );
#endif // LE_SW_SDK_BUILD

    // Position
    float const timeInBars( timeInSeconds / barDuration_ );

    BOOST_ASSERT( ( currentTimeInBars_ > previousTimeInBars_ ) || ( currentTimeInBars_ == 0 ) );
    previousTimeInBars_ = currentTimeInBars_;
    currentTimeInBars_  = timeInBars;
}


void LFOImpl::Timer::reset()
{
    currentTimeInBars_  = 0;
    previousTimeInBars_ = 0;

    // Assume 120 BPM 4/4
    /// \note The hasTempoInformation_ flag is not reset here because this
    /// causes bogus sporadic "Loaded preset uses tempo-synced LFOs but the host
    /// does not provide tempo information." popups in Live! when browsing
    /// through presets. This is most probably due to the threading logic in
    /// Live! where the preset loading hasTempoInformation() checking code gets
    /// executed between the time Live! calls resume() (which calls
    /// LFOImpl::Timer::reset()) and process() (which again fetches valid tempo
    /// information).
    ///                                       (28.05.2012.) (Domagoj Saric)
    //hasTempoInformation_ = false;
    barDuration_         = 60.0f / 120 * 4;
    measureNumerator_    = 4;
}


LFOImpl::value_type LFOImpl::Timer::measureNumeratorFloat()
{
    return Math::convert<LFOImpl::value_type>( measureNumerator() );
}


bool LFOImpl::Timer::TimingInformationChange::barDurationChanged() const
{
    return !Math::is<1>( barDurationChangeRatio_ );
}


LE_NOTHROWNOALIAS LFOImpl::SyncTypes::value_type LFOImpl::SyncTypes::default_() { return LFOImpl::Timer::hasTempoInformation() ? LFO::Quarter : LFO::Free; }
//...mrmlj...a 'dynamic' (bounds) parameter...
LE_NOTHROWNOALIAS LFOImpl::value_type LFOImpl::PeriodScaleParameterTraits::minimum() { return LFOImpl::currentPeriodScaleMinimum(); }
LE_NOTHROWNOALIAS LFOImpl::value_type LFOImpl::PeriodScaleParameterTraits::maximum() { return LFOImpl::currentPeriodScaleMaximum(); }
LE_NOTHROWNOALIAS bool                LFOImpl::PeriodScaleParameterTraits::isValidValue( param_type value ) { return Math::isValueInRange<param_type>( value, minimum(), maximum() ); }

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
