////////////////////////////////////////////////////////////////////////////////
///
/// \file lfo.hpp
/// -------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef lfoImpl_hpp__506C13FB_1220_44C6_BFC0_C6C9844F02B0
#define lfoImpl_hpp__506C13FB_1220_44C6_BFC0_C6C9844F02B0
#pragma once
//------------------------------------------------------------------------------
#include "lfo.hpp"

#include "le/math/conversion.hpp"
#include "le/parameters/boolean/parameter.hpp"
#include "le/parameters/dynamic/tag.hpp"
#include "le/parameters/factoryMacro.hpp"
#include "le/parameters/linear/parameter.hpp"
#include "le/parameters/symmetric/parameter.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins //...mrmlj...
{
    using AutomatedParameterValue = float;
}
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class LFO
///
////////////////////////////////////////////////////////////////////////////////

class LFOImpl : public LFO
{
public:
    static unsigned int const minimumValue = 0;
    static unsigned int const maximumValue = 1;

    static unsigned int const maximumPeriodInNumberOfBars           = 16;
    static unsigned int const minimumPeriodAsMaximumBeatDenominator = 8 ;

private:
    struct PeriodScaleParameterTraits;

public:
    using value_type    = float;
    using SnappedPeriod = std::pair<float, SyncType>;
    using PeriodScale   = Parameters::Parameter<PeriodScaleParameterTraits>;

public: // Parameters
    // Implementation note:
    //   To enable LFO automation, settings are stored as
    // LE::Parameters::Parameters<>.
    //                                        (18.02.2011.) (Domagoj Saric)

    ////////////////////////////////////////////////////////////////////////////
    /// \class SyncTypes
    /// \brief Implements the Parameter<> concept for the SyncTypes parameter.
    ////////////////////////////////////////////////////////////////////////////

    class SyncTypes
        :
        public Parameters::Parameter
        <
            Parameters::LinearUnsignedInteger::Modify
            <
                Parameters::Traits::Minimum<Free   >,
                Parameters::Traits::Maximum<All    >,
                Parameters::Traits::Default<Quarter>
            >::type
        >
    {
    public:
        SyncTypes() : type( default_() ) {}
        static LE_NOTHROWNOALIAS value_type LE_FASTCALL_ABI default_();
        using type::operator=;
    }; // class SyncTypes


    // Implementation note:
    //   Unlike with other enumerate/"discrete values parameters", we do not use the
    // LE_ENUMERATED_PARAMETER macro to define the window function parameter
    // because its possible values are already defined with a plain enum in the
    // SW SDK which we simply reuse here.
    //                                            (01.04.2011.) (Domagoj Saric)
    // Implementation note:
    //   As a first choice, based on the comment at the bottom of this page
    // http://www.katjaas.nl/FFTwindow/FFTwindow&filtering.html, the Hann
    // window was chosen as the default/"overall best" window.
    //                                            (20.01.2010.) (Domagoj Saric)
    /// \todo Further investigate the Hann <-> Hanning debate/confusion. In this
    /// http://www.hydrogenaudio.org/forums/lofiversion/index.php/t29439.html
    /// discussion it is claimed that both Hann and Hanning windows exist.
    ///                                           (25.01.2010.) (Domagoj Saric)

    using Waveform = Parameters::EnumeratedParameter<LFO::Waveform::NumberOfWaveforms>;

    ////////////////////////////////////////////////////////////////////////////
    /// \class PeriodScale
    /// \brief Implements the Parameter<> concept for the PeriodScale parameter.
    ////////////////////////////////////////////////////////////////////////////
    //...mrmlj...a 'dynamic' (bounds) parameter...
private:
    struct PeriodScaleParameterTraits
        :
        Parameters::Detail::LinearFloatParameterTraits
        <
            Parameters::TraitPack //...mrmlj...
            <
                Parameters::Traits::Minimum          <1>,
                Parameters::Traits::Maximum          <1>,
                Parameters::Traits::Default          <1>,
                Parameters::Traits::ValuesDenominator<1>,
                Parameters::Traits::Unit             <0>
            >
        >
    {
        using Tag = Parameters::DynamicRangeParameterTag;

        static LE_NOTHROWNOALIAS value_type LE_FASTCALL_ABI minimum();
        static LE_NOTHROWNOALIAS value_type LE_FASTCALL_ABI maximum();

        static LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI isValidValue( param_type value );
    }; // struct PeriodScaleParameterTraits

public:
    LE_DEFINE_PARAMETERS
    (
        ( ( Enabled     ) ( LE::Parameters::Boolean ) )
        ( ( PeriodScale ) )
        ( ( Phase       ) ( Parameters::SymmetricFloat )( MaximumOffset<50> )( ValuesDenominator<100> ) )
        ( ( LowerBound  ) ( Parameters::LinearFloat    )( Minimum<minimumValue> )( Maximum<maximumValue> )( Default<minimumValue> ) )
        ( ( UpperBound  ) ( LowerBound                 )( Default<maximumValue> )/*...mrmlj...*/( ValuesDenominator<1> )( Unit<0> ) )
        ( ( SyncTypes   ) )
        ( ( Waveform    ) )
    );

    Parameters       & parameters()       { return parameters_; }
    Parameters const & parameters() const { return const_cast<LFOImpl &>( *this ).parameters(); }

    value_type periodScale() const;

    void setLowerBound( value_type newLowerBound );
    void setUpperBound( value_type newUpperBound );

    void setPeriodScale( value_type periodScale );

public:
#ifdef LE_NO_LFOs
    struct Timer { struct TimingInformationChange; void reset() {} };
#else
    class Timer
    {
    public:
    #ifdef _MSC_VER
        #pragma warning( push )
        #pragma warning( disable : 4510 ) // Default constructor could not be generated.
        #pragma warning( disable : 4512 ) // Assignment operator could not be generated.
        #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    #endif // _MSC_VER
        struct TimingInformationChange
        {
            bool timingInfoChanged() const { return barDurationChanged() || measureNumeratorChanged(); }

            bool barDurationChanged     () const;
            bool measureNumeratorChanged() const { return measureNumeratorChanged_; }

            value_type const barDurationChangeRatio_ ;
            bool       const measureNumeratorChanged_;
        }; // struct TimingInformationChange
    #ifdef _MSC_VER
        #pragma warning( pop )
    #endif // _MSC_VER

    public:
        Timer();

        value_type const & currentTimeInBars () const { return currentTimeInBars_ ; }
        value_type const & previousTimeInBars() const { return previousTimeInBars_; }

        TimingInformationChange LE_FASTCALL updatePositionAndTimingInformation( float positionInBars, float barDuration, std::uint8_t measureNumerator );
        TimingInformationChange LE_FASTCALL updatePositionAndTimingInformation( unsigned int deltaNumberOfSamples, float sampleRate                    );

        void setPosition( unsigned int numberOfSamples, float sampleRate );
        void setPosition( float        numberOfSeconds                   );

        void reset();

        static bool               hasTempoInformation  () { return hasTempoInformation_; }
        static value_type const & basePeriod           () { return barDuration_        ; }
        static std::uint8_t       measureNumerator     () { return measureNumerator_   ; }
        static value_type         measureNumeratorFloat();

    private:
        value_type currentTimeInBars_ ;
        value_type previousTimeInBars_;

        static value_type   barDuration_        ;
        static std::uint8_t measureNumerator_   ;
        static bool         hasTempoInformation_;
    }; // class Timer
#endif // LE_NO_LFOs
public:
    LE_NOTHROWNOALIAS
	LFOImpl();

    value_type LE_NOTHROWNOALIAS getValue( Timer const & ) const;

    /// \todo These synchronization type altering functions do not automatically
    /// cause period scale resnapping. Reconsider this.
    ///                                       (23.02.2011.) (Domagoj Saric)
    //void addSyncType   ( SyncType syncType );
    //void removeSyncType( SyncType syncType );

    void updateForNewTimingInformation( Timer::TimingInformationChange const & );

    static value_type LE_NOTHROWNOALIAS currentPeriodScaleMinimum();
    static value_type LE_NOTHROWNOALIAS currentPeriodScaleMaximum();

    static SnappedPeriod snapPeriodScale( value_type periodScale, std::uint8_t syncTypes );

    //...mrmlj...cleanup with a new 'logarithmic' parameter/control...
    static void snapPeriodScaleFromAutomation( PeriodScale & );

public: // Preset saving/loading section

    template <typename T>
    typename T::value_type adjustValueForPreset( T const & value ) const { return value; }

    template <typename T>
    typename T::value_type adjustValueFromPreset( typename T::value_type const value ) const { return value; }

public: // Parameters
    // Implementation note:
    //   To enable LFO automation, settings are stored as
    // LE::Parameters::Parameters<>.
    //                                        (18.02.2011.) (Domagoj Saric)

public:
    static Plugins::AutomatedParameterValue internal2AutomatedValue( std::uint8_t parameterIndex, float                             internalValue, bool normalised );
    static float                            automated2InternalValue( std::uint8_t parameterIndex, Plugins::AutomatedParameterValue automatedValue, bool normalised );

    static Plugins::AutomatedParameterValue unlinearisePeriodScale( Plugins::AutomatedParameterValue linearisedNormalisedPeriodScale );
    static Plugins::AutomatedParameterValue   linearisePeriodScale( Plugins::AutomatedParameterValue nonlinearNormalisedPeriodScale  );

private: friend class LFO;
    static value_type    clampFreePeriod ( value_type absolutePeriod                      );
    static SnappedPeriod snapSyncedPeriod( value_type periodScale, std::uint8_t syncTypes );

    value_type LE_NOTHROWNOALIAS getWaveformAmplitudeForPosition( value_type position, bool newPeriodBegun ) const;

           bool isValueInBounds( value_type ) const;
    static bool isValueInRange ( value_type )      ;

private: friend class LFO;
            Parameters parameters_;
    mutable value_type state_[ 2 ];
}; // class LFOImpl

template <>
LFOImpl::PeriodScale::value_type LFOImpl::adjustValueForPreset( PeriodScale const & ) const;

template <>
LFOImpl::PeriodScale::value_type LFOImpl::adjustValueFromPreset<LFOImpl::PeriodScale>( LFOImpl::PeriodScale::value_type ) const;

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // lfoImpl_hpp
