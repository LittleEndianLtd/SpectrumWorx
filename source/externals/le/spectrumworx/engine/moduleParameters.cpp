////////////////////////////////////////////////////////////////////////////////
///
/// moduleParameters.cpp
/// --------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "moduleParameters.hpp"

#include "le/parameters/conversion.hpp"
#include "le/parameters/parametersUtilities.hpp"
#include "le/parameters/runtimeInformation.hpp"

#ifndef LE_NO_PRESETS
#include "le/spectrumworx/effects/baseParametersUIElements.hpp" // Bypass@presets
#include "le/spectrumworx/presets.hpp"
#endif // !LE_NO_PRESETS
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

using LFO = Parameters::LFOImpl;

LE_OPTIMIZE_FOR_SIZE_BEGIN()

LE_COLD LE_NOTHROW
ModuleParameters::ModuleParameters
(
  //std::uint8_t           const moduleSlotIndex,
    EffectMetaData const &       metadata
#ifndef LE_NO_LFOs
   ,LFOPlaceholder       * const pLFOStorage
#endif // LE_NO_LFOs
)
    :
  //moduleSlotIndex_( moduleSlotIndex ),
    metaData_( metadata )
#ifdef LE_NO_LFOs
    {}
#else
   ,pLFOs_( reinterpret_cast<LFO *>( pLFOStorage ) )
{
    LE_DISABLE_LOOP_UNROLLING()
    for ( auto & lfoPlaceholder : lfos() )
    {
        new ( &lfoPlaceholder ) LFO;
    }
}
#endif // LE_NO_LFOs

LE_COLD LE_NOTHROW
bool ModuleParameters::bypass() const { return baseParameters().get<Effects::BaseParameters::Bypass>(); }


namespace
{
    struct ValueGetter
    {
        typedef float result_type;
        template <class Parameter>
        result_type operator()( Parameter const & parameter ) const { return Math::convert<float>( parameter.getValue() ); }
    }; // struct ValueGetter
}
LE_COLD LE_NOTHROW
float ModuleParameters::getBaseParameter( std::uint8_t const baseParameterIndex ) const
{
    return LE::Parameters::invokeFunctorOnIndexedParameter
    (
        baseParameters(),
        baseParameterIndex,
        ValueGetter()
    );
}


namespace
{
    struct ValueSetter
    {
        ValueSetter( float const value ) : value_( value ) {}
        typedef float result_type;
        template <class Parameter>
        result_type operator()( Parameter & parameter ) const
        {
            static_assert
            (
                !std::is_same<typename Parameter::Tag, LE::Parameters::PowerOfTwoParameterTag>::value,
                "Automation-to-parameter-value conversion using Plugins::AutomatedParameter::Info is correct only for linear parameters." //...mrmlj...
            );
            parameter.setValue( Math::convert<typename Parameter::value_type>( value_ ) );
            return Math::convert<float>( parameter.getValue() );
        }
        float const value_;
    }; // struct ValueSetter
} // anonymous namespace
LE_COLD LE_NOTHROW
float ModuleParameters::setBaseParameter( std::uint8_t const baseParameterIndex, float const parameterValue )
{
    return LE::Parameters::invokeFunctorOnIndexedParameter
    (
        baseParameters(),
        baseParameterIndex,
        ValueSetter( parameterValue )
    );
}

#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wassume"
#endif // __clang__
LE_COLD LE_NOTHROW
ParameterInfo const & ModuleParameters::parameterInfo( std::uint8_t const parameterIndex ) const
{
    LE_ASSUME( parameterIndex < numberOfParameters() );

    ParameterInfo const * LE_RESTRICT pParameterInfos;
    std::uint8_t                      index( parameterIndex );
    if ( parameterIndex < BaseParameters::static_size )
    {
        pParameterInfos = &parameterInfos()[ 0 ];
    }
    else
    {
        pParameterInfos = metaData_.pParameterInfos;
        index           = effectSpecificParameterIndex( index );
        LE_ASSUME( index < numberOfEffectSpecificParameters() );
    }
    return pParameterInfos[ index ];
}

LE_COLD LE_NOTHROW
ParameterInfo const & ModuleParameters::effectSpecificParameterInfo( std::uint8_t const parameterIndex ) const
{
    LE_ASSUME( parameterIndex < numberOfEffectSpecificParameters() );
    return metaData_.pParameterInfos[ parameterIndex ];
}
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

LE_COLD LE_NOTHROW
std::uint8_t ModuleParameters::effectSpecificParameterIndex( std::uint8_t const parameterIndex )
{
    LE_ASSUME( parameterIndex >= numberOfBaseParameters );
    return parameterIndex - numberOfBaseParameters;
}

#ifndef LE_NO_LFOs
LE_COLD LE_NOTHROW
void ModuleParameters::updateLFOs( LFO::Timer::TimingInformationChange const timingInformationChange )
{
    BOOST_ASSERT_MSG( timingInformationChange.timingInfoChanged(), "No need to call this." );

    for ( auto & lfo : lfos() )
        lfo.updateForNewTimingInformation( timingInformationChange );
}

LE_COLD LE_NOTHROW LFO       & ModuleParameters::lfo( std::uint8_t const lfoableParameterIndex )       { return lfos()[ lfoableParameterIndex ]; }
LE_COLD LE_NOTHROW LFO const & ModuleParameters::lfo( std::uint8_t const lfoableParameterIndex ) const { return const_cast<ModuleParameters &>( *this ).lfo( lfoableParameterIndex ); }

LE_COLD LE_NOTHROW
ModuleParameters::LFOs ModuleParameters::lfos() const
{
    return LFOs( pLFOs_, pLFOs_ + numberOfLFOControledParameters() );
}

LE_COLD LE_NOTHROW
void ModuleParameters::updateBaseParametersFromLFOs( LFO::Timer const & timer )
{
    LE_DISABLE_LOOP_UNROLLING()
    for
    (
        std::uint8_t baseParameter( numberOfNonLFOBaseParameters );
        baseParameter < numberOfBaseParameters;
        ++baseParameter
    )
    {
        LFO const & parameterLFO( baseLFO( baseParameter - numberOfNonLFOBaseParameters ) );
        if ( parameterLFO.enabled() )
        {
            auto const lfoValue( parameterLFO.getValue( timer ) );
            setBaseParameterFromLFO( baseParameter, lfoValue );
        }
    }
}

/// \note Up to SVN revision 8952 many more parameter related operations were
/// lowered to the individual/specific ModuleImpl<> instantiations. This, for
/// example, enabled both the parameters and their corresponding widgets to be
/// updated from the same place/the same loop iteration with optimal value range
/// and type conversion paths (e.g. whether the widget value should be updated
/// from the LFO value or the parameter value).
/// OTOH, such an approach coupled the DSP and GUI domains too much making it
/// difficult to effectively separate them which became a requirement for FMOD
/// Studio support.
/// For the above reason the design was refactored to separate the access to
/// parameters from the access to their corresponding widgets/GUI elements. This
/// approach has both pros and cons:
/// pros:
///  + better separation of DSP and GUI domains
///  + shorter compilation times
///  + smaller codegen size (more functionality can be moved up into the
///    non-template base classes)
/// cons:
///  - slower (e.g. generic value conversion without compile-time range
///    information, many more virtual function calls...).
///                                           (07.02.2014.) (Domagoj Saric)
LE_COLD LE_NOTHROW
void ModuleParameters::updateEffectParametersFromLFOs( LFO::Timer const & timer )
{
    auto const numberOfEffectSpecificParameters( this->numberOfEffectSpecificParameters() );
    for ( std::uint8_t effectParameter( 0 ); effectParameter < numberOfEffectSpecificParameters; ++effectParameter )
    {
        LFO const & parameterLFO( effectLFO( effectParameter ) );
        if ( parameterLFO.enabled() )
        {
            auto const lfoValue( parameterLFO.getValue( timer ) );
            setEffectParameterFromLFO( effectParameter, lfoValue );
        }
    }
}

LE_COLD LE_NOTHROW
parameter_value_t ModuleParameters::setBaseParameterFromLFOAux( std::uint8_t const parameterIndex, LFO::value_type const lfoValue )
{
    BOOST_STATIC_ASSERT_MSG( LFO::minimumValue == 0 && LFO::maximumValue == 1, "LFO::value_type not normalised." );
    auto const parameterValue( normalisedToParameterValue( lfoValue, parameterInfos()[ parameterIndex ] ) );
    return setBaseParameter( parameterIndex, parameterValue );
}

LE_COLD LE_NOTHROW
parameter_value_t ModuleParameters::setEffectParameterFromLFOAux( std::uint8_t const parameterIndex, LFO::value_type const lfoValue )
{
    BOOST_STATIC_ASSERT_MSG( LFO::minimumValue == 0 && LFO::maximumValue == 1, "LFO::value_type not normalised." );
    auto const &       info          ( effectSpecificParameterInfo( parameterIndex ) );
    auto         const parameterValue( normalisedToParameterValue ( lfoValue, info ) );
    return setEffectParameter( parameterIndex, parameterValue );
}

LE_COLD LE_NOTHROW
LFO::value_type   LE_FASTCALL ModuleParameters::normalisedToParameterValue( parameter_value_t const normalisedValue, ParameterInfo const & parameterInfo )
{
    return Math::convertLinearRange<float, float, 0, 1, 1>( normalisedValue, parameterInfo.minimum, parameterInfo.maximum );
}
LE_COLD LE_NOTHROW
parameter_value_t LE_FASTCALL ModuleParameters::parameterToNormalisedValue( LFO::value_type   const parameterValue , ParameterInfo const & parameterInfo )
{
    return Math::convertLinearRange<float, 0, 1, 1, float>( parameterValue, parameterInfo.minimum, parameterInfo.maximum );
}
#endif // !LE_NO_LFOs

#ifndef LE_NO_PRESETS
namespace
{
    using LFO = Parameters::LFOImpl;
    LE_COLD LE_NOTHROW
    boost::optional<float> LE_FASTCALL getParameterValueWithoutLFO
    (
        ParametersLoader const & parameterLoader,
        ParameterInfo    const & parameterInfo,
        LFO                    & parameterLFO
    )
    {
        auto const parameterValueWithoutLFO
        (
            parameterLoader.getLFOParameterValue<float>
            (
                parameterInfo.name,
                parameterLFO
            )
        );
        if
        (
               parameterValueWithoutLFO                            &&
            ( *parameterValueWithoutLFO >= parameterInfo.minimum ) &&
            ( *parameterValueWithoutLFO <= parameterInfo.maximum )
        )
            return parameterValueWithoutLFO;
        else
            return boost::none;
    }
} // anonymous namespace
LE_COLD LE_NOTHROW
void ModuleParameters::loadPresetParameters( ParametersLoader const & parameterLoader )
{
    //BOOST_ASSERT_MSG
    //(
    //    parameterLoader.currentEffectName() == Effects::effectName( effectTypeIndex() ),
    //    "ParametersLoader and module out-of-sync"
    //);

    {
        auto const bypassValue
        (
            parameterLoader.getSimpleParameterValue<bool>( LE::Parameters::Name<Effects::BaseParameters::Bypass>::string_ )
        );
        if ( bypassValue )
            setBaseParameter( 0, *bypassValue ); //...mrmlj...assumes bypass is the first parameter/@ index 0
    }

    for ( std::uint8_t i( 1 ); i < numberOfBaseParameters; ++i )
    {
        auto const parameterValueWithoutLFO
        (
            getParameterValueWithoutLFO( parameterLoader, parameterInfos()[ i ], baseLFO( i - 1 ) )
        );
        if ( parameterValueWithoutLFO )
            setBaseParameter( i, *parameterValueWithoutLFO );
    }

    auto const effectSpecificParameters( numberOfEffectSpecificParameters() );
    for ( std::uint8_t i( 0 ); i < effectSpecificParameters; ++i )
    {
        auto const parameterValueWithoutLFO
        (
            getParameterValueWithoutLFO( parameterLoader, effectSpecificParameterInfo( i ), effectLFO( i ) )
        );
        if ( parameterValueWithoutLFO )
            setEffectParameter( i, *parameterValueWithoutLFO );
    }
}

#ifndef LE_SW_SDK_BUILD
LE_COLD LE_NOTHROW
void ModuleParameters::savePresetParameters( ParametersSaver const & parameterSaver ) const
{
    ParametersSaver & saver( const_cast<ParametersSaver &>( parameterSaver ) ); //...mrmlj...

    saver.saveParameter<bool>( LE::Parameters::Name<Effects::BaseParameters::Bypass>::string_, bypass() );

    for ( std::uint8_t i( 1 ); i < numberOfBaseParameters; ++i )
    {
        saver.saveParameter<float>
        (
            parameterInfos()[ i     ].name,
            getBaseParameter( i     ),
            baseLFO         ( i - 1 )
        );
    }

    auto const effectSpecificParameters( numberOfEffectSpecificParameters() );
    for ( std::uint8_t i( 0 ); i < effectSpecificParameters; ++i )
    {
        saver.saveParameter<float>
        (
            effectSpecificParameterInfo( i ).name,
            getEffectParameter         ( i ),
            effectLFO                  ( i )
        );
        //...mrmlj...
        //ParameterInfo const &       info  ( effectSpecificParameterInfo( i ) );
        //LFO           const &       lfo   ( effectLFO                  ( i ) );
        //void          const * const pValue( getEffectParameter         ( i ) );
        //switch ( info.type )
        //{
        //    case ParameterInfo::Boolean      : saver.saveParameter<bool        >( info.name,  static_cast<Parameters::Boolean const *>( pValue )->getValue(), lfo ); break;
        //    case ParameterInfo::Integer      : saver.saveParameter<int         >( info.name, *static_cast<         int        const *>( pValue )            , lfo ); break;
        //    case ParameterInfo::Enumerated   : saver.saveParameter<unsigned int>( info.name, *static_cast<unsigned int        const *>( pValue )            , lfo ); break;
        //    case ParameterInfo::FloatingPoint: saver.saveParameter<float       >( info.name, *static_cast<float               const *>( pValue )            , lfo ); break;
        //    LE_DEFAULT_CASE_UNREACHABLE();
        //}
    }
}
#endif // LE_SW_SDK_BUILD
#endif // LE_NO_PRESETS

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
