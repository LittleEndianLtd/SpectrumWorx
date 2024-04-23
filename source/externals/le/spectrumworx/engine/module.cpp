////////////////////////////////////////////////////////////////////////////////
///
/// module.cpp
/// ----------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "module.hpp"

#include "moduleNode.hpp" //...mrmlj...for lack of moduleNode.cpp

#include "channelData.hpp"
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

LE_COLD ModuleDSP::~ModuleDSP() {}

LE_NOTHROW LE_COLD
void ModuleDSP::preProcess( LFO::Timer const & timer, Setup const & engineSetup )
{
    if ( bypass() )
        return;
#ifndef LE_NO_LFOs
    ModuleParameters::updateBaseParametersFromLFOs  ( timer );
    ModuleParameters::updateEffectParametersFromLFOs( timer );
#endif // LE_NO_LFOs
    setup( engineSetup );
}


void LE_COLD ModuleDSP::setup( Setup const & engineSetup )
{
    using namespace Effects::BaseParameters;

    float const  leftFrequency( baseParameters().get<StartFrequency>() );
    float const rightFrequency( baseParameters().get<StopFrequency >() );
    //...mrmlj...for now we will just fix the range until all other code (LFO) ensures a valid range...
    //BOOST_ASSERT( leftFrequency <= rightFrequency );
    workingRange_.setNewRange
    (
        engineSetup.normalisedFrequencyToBin( std::min( leftFrequency, rightFrequency ) ),
        engineSetup.normalisedFrequencyToBin(                          rightFrequency   )
    );
    doPreProcess( engineSetup );
}


bool LE_COLD ModuleDSP::allocateStorage
(
    StorageFactors const & storageFactors,
    std::uint16_t const channelStateSize,
    std::uint32_t const channelStateRequiredStorage // HistoryBuffer requires uint32_t
)
{
    using Utility::align;

    auto const numberOfChannels( storageFactors.numberOfChannels );

    auto const baseNumberOfBytes
    (
        numberOfChannels * channelStateSize
    );

    auto const alignmentPadding
    (
        channelStateRequiredStorage
            ? align( baseNumberOfBytes ) - baseNumberOfBytes
            : 0
    );

    auto const bufferNumberOfBytes
    (
        numberOfChannels * align( channelStateRequiredStorage )
    );

    auto const totalBytes
    (
        baseNumberOfBytes
            +
        alignmentPadding
            +
        bufferNumberOfBytes
    );

    return storage_.resize( totalBytes );
}

LE_OPTIMIZE_FOR_SIZE_END()

LE_OPTIMIZE_FOR_SPEED_BEGIN()
LE_NOTHROW
void ModuleDSP::process( std::uint8_t const channel, ChannelData & channelData, Setup const & engineSetup ) const
{
    if ( !bypass() )
    {
        using namespace Math;
        using namespace Effects::BaseParameters;

        float const & wet ( baseParameters().get<Wet >() );
        float const & gain( baseParameters().get<Gain>() );

        bool const blend  ( !is<100>( wet  ) );
        bool const amplify( !isZero ( gain ) );

        bool amPh2ReIm;//...mrmlj...quick-fix for blending bug with amPh2ReIm effects...

        doProcess( channel, ChannelDataProxy( channelData, *this, blend, amPh2ReIm ), engineSetup );

        if ( blend   ) { channelData.blendWithPreviousData( wet / 100, amPh2ReIm        ); }
        if ( amplify ) { channelData.amplifyCurrentData   ( dB2NormalisedLinear( gain ) ); }
    }
}
LE_OPTIMIZE_FOR_SPEED_END()


ModuleDSP::ChannelDataProxy::ChannelDataProxy( ChannelData & data, ModuleDSP const & module, bool const doBlend, bool & amPh2ReIm )
    :
    module_       ( module    ),
    data_         ( data      ),
    amPh2ReIm_    ( amPh2ReIm ),
    blendRequired_( doBlend   )
{
    amPh2ReIm = false;
}

LE_NOINLINE LE_NOTHROWNOALIAS
ModuleDSP::ChannelDataProxy::operator MainSideChannelData_AmPh () const
{
    return MainSideChannelData_AmPh( data_.freshAmPhData( blendRequired_ ), module_.workingRange() );
}

LE_NOTHROWNOALIAS
ModuleDSP::ChannelDataProxy::operator MainSideChannelData_ReIm () const
{
    return MainSideChannelData_ReIm( data_.freshReImData( blendRequired_ ), module_.workingRange() );
}

LE_NOINLINE LE_NOTHROWNOALIAS
ModuleDSP::ChannelDataProxy::operator ChannelData_AmPh () const
{
    return ChannelData_AmPh( data_.freshAmPhData( blendRequired_ ).main(), module_.workingRange() );
}

LE_NOINLINE LE_NOTHROWNOALIAS
ModuleDSP::ChannelDataProxy::operator ChannelData_ReIm () const
{
    return ChannelData_ReIm( data_.freshReImData( blendRequired_ ).main(), module_.workingRange() );
}

LE_NOTHROWNOALIAS
ModuleDSP::ChannelDataProxy::operator ChannelData_AmPh2ReIm () const
{
    amPh2ReIm_ = true;
    ChannelData::AmPhReImData const bothDomainData( data_.freshAmPh2ReImData( blendRequired_ ) );
    ChannelData_AmPh2ReIm const result =
    {
        MainSideChannelData_AmPh( bothDomainData.first        , module_.workingRange() ),
                ChannelData_ReIm( bothDomainData.second.main(), module_.workingRange() )
    };
    BOOST_ASSERT( result.input.main().beginBin() == result.output.beginBin() );
    BOOST_ASSERT( result.input.main().endBin  () == result.output.endBin  () );
    return result;
}

LE_NOTHROWNOALIAS
ModuleDSP::ChannelDataProxy::operator ChannelData_ReIm2AmPh () const
{
    ChannelData::AmPhReImData const bothDomainData( data_.freshReIm2AmPhData( blendRequired_ ) );
    ChannelData_ReIm2AmPh const result =
    {
        MainSideChannelData_ReIm( bothDomainData.second, module_.workingRange() ),
        MainSideChannelData_AmPh( bothDomainData.first , module_.workingRange() ),
    };
    BOOST_ASSERT( result.input.beginBin() == result.output.beginBin() );
    BOOST_ASSERT( result.input.endBin  () == result.output.endBin  () );
    return result;
}


LE_NOTHROW LE_CONST_FUNCTION
void * ModuleDSP::getEffectParameterPtr( std::uint8_t const parameterIndex )
{
    LE_ASSUME( pParameterOffsets_[ 0 ] == 0 );
    std::uint16_t const parameterOffset( parametersBaseOffset_ + pParameterOffsets_[ parameterIndex ] );
    return reinterpret_cast<char *>( this ) + parameterOffset;
}

LE_NOTHROW LE_CONST_FUNCTION
void const * ModuleDSP::getEffectParameterPtr( std::uint8_t const parameterIndex ) const
{
    return const_cast<ModuleDSP &>( *this ).getEffectParameterPtr( parameterIndex );
}

LE_NOTHROW
float ModuleDSP::setEffectParameter( std::uint8_t const parameterIndex, float const value, ParameterInfo const & cachedInfo )
{
    BOOST_ASSERT( &cachedInfo == &effectSpecificParameterInfo( parameterIndex ) );
    void * LE_RESTRICT const pValue( getEffectParameterPtr( parameterIndex ) );
    BOOST_ASSERT_MSG( static_cast<float>( value ) >= cachedInfo.minimum, "Parameter value out of range" );
    BOOST_ASSERT_MSG( static_cast<float>( value ) <= cachedInfo.maximum, "Parameter value out of range" );
    switch ( cachedInfo.type )
    {
         //...mrmlj...internal TriggerParameter knowledge...
        case ParameterInfo::Trigger      : return ( *static_cast<char          *>( pValue ) |= Math::convert<char         >( value ) );
        case ParameterInfo::Boolean      : return ( *static_cast<bool          *>( pValue )  = Math::convert<bool         >( value ) );
        case ParameterInfo::Enumerated   : return ( *static_cast<std::uint8_t  *>( pValue )  = Math::convert<std::uint8_t >( value ) );
        case ParameterInfo::Integer      : return ( *static_cast<std:: int16_t *>( pValue )  = Math::convert<std:: int16_t>( value ) );
        case ParameterInfo::FloatingPoint: return ( *static_cast<float         *>( pValue )  =                               value   );
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


LE_NOTHROWNOALIAS void LE_FASTCALL intrusive_ptr_add_ref( ModuleNode const * LE_RESTRICT const pModuleNode )
{
    LE_ASSUME( pModuleNode );
    ++pModuleNode->referenceCount_;
}

#ifdef LE_SW_SDK_BUILD // ambiguous overload resolution (implicitly convertible to both ModuleNode and ModuleBase)
LE_NOTHROWNOALIAS void LE_FASTCALL intrusive_ptr_add_ref( ModuleDSP const * const pModule ) { return intrusive_ptr_add_ref( static_cast<ModuleNode const *>( pModule ) ); }
LE_NOTHROW        void LE_FASTCALL intrusive_ptr_release( ModuleDSP const * const pModule ) { return intrusive_ptr_release( static_cast<ModuleNode const *>( pModule ) ); }
#endif // LE_SW_SDK_BUILD



////////////////////////////////////////////////////////////////////////////////
// ModuleParameters set/getEffectParameter default implementations
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW
float ModuleParameters::getEffectParameter( std::uint8_t const parameterIndex ) const
{
    auto & impl( static_cast<ModuleDSP const &>( *this ) );
    auto const &       info  ( impl.effectSpecificParameterInfo( parameterIndex ) );
    void const * const pValue( impl.getEffectParameterPtr      ( parameterIndex ) );
    switch ( info.type )
    {
        case ParameterInfo::Trigger      :
        case ParameterInfo::Boolean      : return Math::convert<float>( *static_cast<bool          const *>( pValue ) );
        case ParameterInfo::Enumerated   : return Math::convert<float>( *static_cast<std::uint8_t  const *>( pValue ) );
        case ParameterInfo::Integer      : return Math::convert<float>( *static_cast<std:: int16_t const *>( pValue ) );
        case ParameterInfo::FloatingPoint: return                       *static_cast<float         const *>( pValue )  ;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}

LE_NOTHROW
float ModuleParameters::setEffectParameter( std::uint8_t const parameterIndex, float const value )
{
    auto const & info( effectSpecificParameterInfo( parameterIndex ) );
    return static_cast<ModuleDSP &>( *this ).setEffectParameter( parameterIndex, value, info );
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#if defined( LE_SW_SDK_BUILD ) && !( defined( LE_SW_PURE_ANALYSIS ) || defined( LE_MELODIFY_SDK_BUILD ) )
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/configuration/effectTypeNames.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{

LE_NOTHROW LE_PURE_FUNCTION std::uint8_t ModuleBase::numberOfEffectSpecificParameters()                                               const { return static_cast<ModuleDSP const &>( *this ).ModuleParameters::numberOfEffectSpecificParameters(); }
LE_NOTHROWNOALIAS           float        ModuleBase::getBaseParameter  ( std::uint8_t const baseParameterIndex                      ) const { return static_cast<ModuleDSP const &>( *this ).ModuleParameters::getBaseParameter  ( baseParameterIndex          ); }
LE_NOTHROW                  float        ModuleBase::setBaseParameter  ( std::uint8_t const baseParameterIndex  , float const value )       { return static_cast<ModuleDSP       &>( *this ).ModuleParameters::setBaseParameter  ( baseParameterIndex  , value ); }
LE_NOTHROWNOALIAS           float        ModuleBase::getEffectParameter( std::uint8_t const effectParameterIndex                    ) const { return static_cast<ModuleDSP const &>( *this ).ModuleParameters::getEffectParameter( effectParameterIndex        ); }
LE_NOTHROW                  float        ModuleBase::setEffectParameter( std::uint8_t const effectParameterIndex, float const value )       { return static_cast<ModuleDSP       &>( *this ).ModuleParameters::setEffectParameter( effectParameterIndex, value ); }

LE_NOTHROWNOALIAS           float        ModuleBase::getParameter      ( std::uint8_t const parameterIndex                          ) const { return ( parameterIndex < numberOfBaseParameters ) ? getBaseParameter( parameterIndex        ) : getEffectParameter( static_cast<ModuleDSP const &>( *this ).effectSpecificParameterIndex( parameterIndex )        ); }
LE_NOTHROW                  float        ModuleBase::setParameter      ( std::uint8_t const parameterIndex      , float const value )       { return ( parameterIndex < numberOfBaseParameters ) ? setBaseParameter( parameterIndex, value ) : setEffectParameter( static_cast<ModuleDSP       &>( *this ).effectSpecificParameterIndex( parameterIndex ), value ); }

LE_NOTHROWNOALIAS
Parameters::LFO & ModuleBase::lfo( std::uint8_t const parameterIndex )
{
    auto & impl( static_cast<ModuleDSP &>( *this ) );
    static_assert( ModuleParameters::numberOfNonLFOBaseParameters == ModuleBase::numberOfNonLFOBaseParameters, "" );
    BOOST_ASSERT( parameterIndex > ModuleParameters::numberOfNonLFOBaseParameters );
    return impl.ModuleParameters::lfo( parameterIndex - ModuleParameters::numberOfNonLFOBaseParameters );
}

LE_NOTHROWNOALIAS
Parameters::RuntimeInformation const & ModuleBase::parameterInfo( std::uint8_t const parameterIndex ) const { return static_cast<ModuleDSP const &>( *this ).ModuleParameters::parameterInfo( parameterIndex ); }

LE_NOTHROWRESTRICTNOALIAS
char const * ModuleBase::effectName() const
{
    return Effects::effectIndex2TypeName( static_cast<ModuleDSP const &>( *this ).effectTypeIndex() );
}

LE_NOTHROWNOALIAS
ModuleBase::BaseParameters & ModuleBase::baseParameters() { return static_cast<ModuleDSP &>( *this ).baseParameters(); }

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI intrusive_ptr_add_ref( ModuleBase const * const pModuleBase ) { intrusive_ptr_add_ref( &node( *static_cast<ModuleDSP const *>( pModuleBase ) ) ); }
LE_NOTHROW        void LE_FASTCALL_ABI intrusive_ptr_release( ModuleBase const * const pModuleBase ) { intrusive_ptr_release( &node( *static_cast<ModuleDSP const *>( pModuleBase ) ) ); }
} // namespace Engine
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // LE_SW_SDK_BUILD
