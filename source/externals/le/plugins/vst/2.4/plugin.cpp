////////////////////////////////////////////////////////////////////////////////
///
/// plugin.cpp
/// ----------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin.hpp"

#include "le/utility/clear.hpp"

#include "boost/assert.hpp"
#include "boost/type_traits/function_traits.hpp"

#include <memory>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

namespace Detail
{

////////////////////////////////////////////////////////////////////////////////
//
// AEffectWrapper::AEffectWrapper()
// --------------------------------
//
////////////////////////////////////////////////////////////////////////////////

AEffectWrapper::AEffectWrapper()
{
    Utility::clear( aEffect() );

    aEffect().magic                             = kEffectMagic;
    aEffect().DECLARE_VST_DEPRECATED( ioRatio ) = 1.0f; // so set in the original VST SDK
#ifndef NDEBUG
    aEffect().object                            = this;
    aEffect().user                              = nullptr;
#endif // NDEBUG
}


////////////////////////////////////////////////////////////////////////////////
//
// LE::Plugins::Detail::AEffectWrapper::fromAEffect()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief See the class comment for details on this function's implementation
/// and purpose.
///
////////////////////////////////////////////////////////////////////////////////

AEffectWrapper * AEffectWrapper::fromAEffect( AEffect * const pEffect )
{
    static_assert( sizeof( AEffectWrapper ) == sizeof( AEffect ), "" );
#ifdef _MSC_VER
    static_assert( &static_cast<AEffectWrapper *>( 0 )->aEffect_ == 0, "" );
#else
    BOOST_ASSERT(  &static_cast<AEffectWrapper *>( 0 )->aEffect_ == 0 );
#endif // _MSC_VER
    BOOST_ASSERT( pEffect == pEffect->object );
    LE_ASSUME(    pEffect != 0               );
    return static_cast<AEffectWrapper *>( static_cast<void *>( pEffect ) );
}

} // namespace Detail


bool VSTHost24Proxy::assumeIOChangedAlwaysSucceedes( false );
bool VSTHost24Proxy::supportsDynamicParameterLists ( false );


////////////////////////////////////////////////////////////////////////////////
//
// VSTHost24Proxy::VSTHost24Proxy()
// --------------------------------
//
////////////////////////////////////////////////////////////////////////////////

VSTHost24Proxy::VSTHost24Proxy( audioMasterCallback const audioMaster )
    :
    audioMaster_( audioMaster )
{
    BOOST_ASSERT_MSG( audioMaster, "Null VST host callback!?" );

    /// \todo This assertion fails with FL Studio 9 but everything works as
    /// expected (it does not call processAccumulate()). Try to think of a
    /// different test.
    ///                                       (25.05.2010.) (Domagoj Saric)
    //BOOST_ASSERT( call( DECLARE_VST_DEPRECATED( audioMasterWillReplaceOrAccumulate ) ) <= 1 );

    // Implementation note:
    //   Because Reaper is such a popular host and it actually does support IO
    // mode changes, even if it does not report so, we handle it as a special
    // case.
    // http://forum.cockos.com/showthread.php?t=34722&highlight=iochanged
    // http://forum.cockos.com/showthread.php?t=19012&highlight=iochanged
    //                                        (14.07.2010.) (Domagoj Saric)

    VSTPluginBase::ProductStringBuf buffer; buffer[ 0 ] = 0;
    /*BOOST_VERIFY Cantabile 2*/( getHostProductString( buffer ) );
    bool const hostIsReaper( std::strcmp( buffer, "REAPER" ) == 0 );
    bool const hostIsLive  ( std::strcmp( buffer, "Live"   ) == 0 );
    assumeIOChangedAlwaysSucceedes = hostIsReaper;
    supportsDynamicParameterLists  = hostIsReaper || hostIsLive;
}


void VSTHost24Proxy::automatedParameterChanged( ParameterIndex const parameterIndex, AutomatedParameterValue const newValue ) const
{
    BOOST_ASSERT_MSG( aEffect().getParameter( const_cast<::AEffect *>( &aEffect() ), parameterIndex.value ) == newValue, "Wrong parameter value reported?" );
    /*Live8&9 BOOST_VERIFY*/( call( audioMasterAutomate , parameterIndex.value, 0, 0, newValue ) /*!= 0*/ );
}

//...multi gesture...http://www.kvraudio.com/forum/viewtopic.php?t=303406
void VSTHost24Proxy::automatedParameterBeginEdit( ParameterIndex const parameterIndex ) const
{
    /*FL11    BOOST_VERIFY*/( call( audioMasterBeginEdit, parameterIndex.value                 ) /*!= 0*/ );
}

void VSTHost24Proxy::automatedParameterEndEdit( ParameterIndex const parameterIndex ) const
{
    /*FL11    BOOST_VERIFY*/( call( audioMasterEndEdit  , parameterIndex.value                 ) /*!= 0*/ );
}


////////////////////////////////////////////////////////////////////////////////
//
// VSTHost24Proxy::call()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////

VstIntPtr LE_NOTHROW LE_COLD VSTHost24Proxy::call
(
    VstInt32    const opCode,
    VstInt32    const index ,
    VstIntPtr   const value ,
    void      * const ptr   ,
    float       const opt
) const
{
    #ifdef _DEBUG
        // Properly typed opcode for better visualization to aid in debugging.
        AEffectOpcodes  const vstOpCode ( static_cast<AEffectOpcodes >( opCode ) ); boost::ignore_unused_variable_warning( vstOpCode  );
        AEffectXOpcodes const vstOpCodeX( static_cast<AEffectXOpcodes>( opCode ) ); boost::ignore_unused_variable_warning( vstOpCodeX );
    #endif // _DEBUG

    return audioMaster_( &const_cast<VSTHost24Proxy &>( *this ).aEffect(), opCode, index, value, ptr, opt );
}


////////////////////////////////////////////////////////////////////////////////
//
// VSTHost24Proxy::getPluginDirectory()
// ------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

VSTHost24Proxy::PathSpec VSTHost24Proxy::getPluginDirectory() const
{
    typedef boost::function_traits<std::remove_pointer<audioMasterCallback>::type>::result_type AudioMasterCallbackReturnType;

    static_assert
    (
        sizeof( PathSpec                      )
          ==
        sizeof( AudioMasterCallbackReturnType ),
        "Unexpected object sizes"
    );

    return reinterpret_cast<PathSpec>( call( audioMasterGetDirectory ) );
}


bool VSTHost24Proxy::reportNewNumberOfIOChannels( unsigned int const inputs, unsigned int const sideInputs, unsigned int const outputs )
{
    unsigned int const currentInputs ( aEffect().numInputs  );
    unsigned int const currentOutputs( aEffect().numOutputs );
    unsigned int const newInputs     ( inputs + sideInputs  );
    unsigned int const newOutputs    ( outputs              );
    /// \note Cubase calls effSetSpeakerArrangement with newInputs and
    /// newOutputs number of channels, repsectively, in response to ioChanged()
    /// so we avoid calling ioChanged() again (recursively) by detecting this
    /// and exiting early.
    ///                                       (24.04.2013.) (Domagoj Saric)
    if ( ( newInputs == currentInputs ) && ( newOutputs == currentOutputs ) )
    {
        LE_TRACE( "\tSW VST2.4: reporting an IO configuration change when nothing is changed." );
        return true;
    }
#ifndef NDEBUG
    auto const maxChannels( reinterpret_cast<std::pair<unsigned short, unsigned short> const &>( aEffect().user ) );
    LE_TRACE_IF( newInputs  > maxChannels.first , "\tSW VST2.4: enabling more input channels than the initially reported maximum."  );
    LE_TRACE_IF( newOutputs > maxChannels.second, "\tSW VST2.4: enabling more output channels than the initially reported maximum." );
#endif // NDEBUG
    aEffect().numInputs  = newInputs ;
    aEffect().numOutputs = newOutputs;
    if ( ioChanged() )
        return true;
    LE_TRACE_IF( canDo<AcceptIOChanges>(), "\tSW VST2.4: host rejected ioChanged()." );
    aEffect().numInputs  = currentInputs ;
    aEffect().numOutputs = currentOutputs;
    return false;
}


bool VSTHost24Proxy::reportNewLatencyInSamples( unsigned int const samples )
{
    // http://www.kvraudio.com/forum/viewtopic.php?p=3190127
    LE_TRACE_IF( unsigned( aEffect().initialDelay ) != samples, "\tSW VST2.4: Avoid calling back the host if not necessary (latency change notification)." );
    aEffect().initialDelay = samples;
    return ioChanged();
}


bool VSTHost24Proxy::ioChanged() const
{
    return ( call( audioMasterIOChanged ) != 0 ) | assumeIOChangedAlwaysSucceedes;
}


Detail::VST24TimingInformation VSTHost24Proxy::makeTimeInfo( ::VstIntPtr const pVoidTimeInfo ) const
{
    ::VstTimeInfo const * LE_RESTRICT const pTimeInfo( reinterpret_cast<::VstTimeInfo const *>( pVoidTimeInfo ) );
    BOOST_ASSERT_MSG
    (
        ( canDo<SendTimeInfo>() == ( pTimeInfo != nullptr )      ) ||
        ( isHost( "Audition"             ) /*3.0*/               ) ||
        ( isHost( "Gold"                 ) /*Audition CS 5.5*/   ) ||
        ( isHost( "DSP-Quattr"           ) /*3.5.1*/             ) ||
        ( isHost( "Vst2Au2"              )                       ) ||
        ( isHost( "VSTMANLIB"            ) /*VST Scanner 1.048*/ ) ||
        ( isHost( "Soundminer"           )                       ) ||
        ( isHost( "Sound Forge Pro 10.0" )                       ) ||
        ( isHost( ""                     ) /*Audacity*/          ),
        "Inconsistent host replies to SendTimeInfo and audioMasterGetTime"
    );
    return Detail::VST24TimingInformation( pTimeInfo );
}


bool VSTHost24Proxy::isHost( char const * const hostName ) const
{
    VSTPluginBase::ProductStringBuf buffer;
    buffer[ 0 ] = 0;
    getHostProductString( buffer );
    return std::strstr( buffer, hostName ) != nullptr;
}


bool VSTHost24Proxy::updateDisplay() const
{
    // http://www.asseca.org/vst-24-specs/amUpdateDisplay.html
    // http://www.kvraudio.com/forum/viewtopic.php?t=242266
    // http://forum.cockos.com/showthread.php?t=100851
    bool const success( call( audioMasterUpdateDisplay ) != 0 );
    return success;
}


void LE_NOTHROW VSTHost24Proxy::allParametersChanged() const
{
    ::AEffect                 const &       effect            (                                          aEffect()             );
    ::AEffectGetParameterProc         const getParameter      (                                          effect.getParameter   );
    auto                              const numberOfParameters( static_cast<ParameterIndex::value_type>( effect.numParams    ) );

    // https://forum.ableton.com/viewtopic.php?f=2&t=121510
    // https://www.ableton.com/en/articles/configure-mode
    // https://www.ableton.com/en/articles/tags/plug-ins

    for ( ParameterIndex parameterIndex{ 0 }; parameterIndex.value < numberOfParameters; ++parameterIndex.value )
    {
        automatedParameterBeginEdit( parameterIndex );
    }

    for ( ParameterIndex parameterIndex{ 0 }; parameterIndex.value < numberOfParameters; ++parameterIndex.value )
    {
        AutomatedParameterValue const parameterValue( getParameter( const_cast<::AEffect *>( &effect ), parameterIndex.value ) );
        automatedParameterChanged( parameterIndex, parameterValue );
    }

    for ( ParameterIndex parameterIndex{ 0 }; parameterIndex.value < numberOfParameters; ++parameterIndex.value )
    {
        automatedParameterEndEdit( parameterIndex );
    }
}


template <>
LE_NOTHROWNOALIAS
bool VSTHost24Proxy::canDo<AcceptIOChanges>() const
{
    BOOST_ASSERT_MSG( !( assumeIOChangedAlwaysSucceedes && capabilityString<AcceptIOChanges>() ), "Host 'gained' ioChanged() support" );
    return assumeIOChangedAlwaysSucceedes || canDo( capabilityString<AcceptIOChanges>() );
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
