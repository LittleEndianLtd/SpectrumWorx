////////////////////////////////////////////////////////////////////////////////
///
/// plugin.inl
/// ----------
///
/// Copyright (c) 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/utility/countof.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tchar.hpp"
#include "le/utility/trace.hpp"

#include <boost/assert.hpp>
#include <boost/core/ignore_unused.hpp>

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

#ifndef NDEBUG
    template <class Impl>
    unsigned int Plugin<Impl, Protocol::Unity>::pluginInstanceCount_( 0 );
#endif


template <class Impl>
Plugin<Impl, Protocol::Unity>::Plugin( ConstructionParameter const constructionParameter )
    :
    UnityPluginBase( constructionParameter )
#ifndef NDEBUG
    ,pluginInstanceID_( pluginInstanceCount_++ )
#endif
{
#ifndef NDEBUG
    std::printf( "---- %s Unity instance %d created ----\n", Impl::name, pluginInstanceID_ );
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
// Plugin<Unity>::create()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK Plugin<Impl, Protocol::Unity>::create( UnityAudioEffectState * LE_RESTRICT const pState )
{
#ifndef NDEBUG
    bool const usesSideChain( Impl::maxNumberOfInputs == ( Impl::maxNumberOfOutputs * 2 ) );

    BOOST_ASSERT( pState                                                                                    );
  //BOOST_ASSERT( pState->currdsptick     == 0                                                              );
    BOOST_ASSERT( pState->prevdsptick     == 0                                                              );
    BOOST_ASSERT( pState->sidechainbuffer == nullptr                                     ||  usesSideChain  );
    BOOST_ASSERT( pState->effectdata      == nullptr                                                        );
    BOOST_ASSERT( pState->flags           & UnityAudioEffectStateFlags_IsSideChainTarget || !usesSideChain  );
#endif // NDEBUG

  //std::uint16_t const defaultBlockSize( 2048 );
  //std::uint16_t const blockSize       ( pState->structsize >= sizeof( *pState ) ? pState->dspbuffersize : defaultBlockSize );

    std::auto_ptr<Impl> pImpl( new (std::nothrow) Impl() );
    if
    (
        pImpl.get() &&
        Impl::makeErrorCode( pImpl->setSampleRate( pState->samplerate ) ) == Success &&
        Impl::makeErrorCode( pImpl->initialise   (                    ) ) == Success &&
      /*Impl::makeErrorCode( pImpl->setBlockSize ( blockSize          ) ) == Success*/ true
    )
    {
        pState->effectdata = pImpl.release();
        return Success;
    }
    else
    {
        pState->effectdata = nullptr;
        return OutOfMemory;
    }
}


template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK Plugin<Impl, Protocol::Unity>::release( UnityAudioEffectState * const pState )
{
    delete &impl( pState );
    return Success;
}


template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK Plugin<Impl, Protocol::Unity>::reset( UnityAudioEffectState * const pState )
{
    impl( pState ).reset();
    return Success;
}


template <class Impl>
LE_NOTHROW
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK Plugin<Impl, Protocol::Unity>::process
(
    UnityAudioEffectState * LE_RESTRICT const pState    ,
    float                 * LE_RESTRICT const inBuffer  ,
    float                 * LE_RESTRICT const outBuffer ,
    unsigned int                        const length    ,
             int                        const inChannels,
             int                        const outChannels
)
{
    Impl & impl( Plugin::impl( pState ) );

    if ( BOOST_UNLIKELY( inChannels != outChannels ) )
        return Unsupported;

  //if ( BOOST_UNLIKELY( pState->structsize < sizeof( *pState ) ) ) // pre Unity 5.2
  //{
  //    if ( !impl.setBlockSize( length ) )
  //        return OutOfMemory;
  //}

  //BOOST_ASSERT_MSG( length <= impl.processBlockSize(), "Unity changed maximum processing block size in the middle of processing." );

    auto const numberOfMainChannels( static_cast<std::uint8_t>( inChannels                                         ) );
    auto const numberOfSideChannels( static_cast<std::uint8_t>( pState->sidechainbuffer ? numberOfMainChannels : 0 ) );

    LE_TRACE_IF
    (
        ( numberOfMainChannels != impl.numberOfInputChannels() ) || ( numberOfSideChannels != impl.numberOfSideChannels() ),
        "Unity changed the channel configuration in the middle of processing (%u+%u -> %u+%u).",
        impl.numberOfInputChannels(), impl.numberOfSideChannels(), numberOfMainChannels, numberOfSideChannels
    );

    if ( BOOST_UNLIKELY( !impl.setNumberOfChannels( numberOfMainChannels + numberOfSideChannels, numberOfMainChannels ) ) )
        return OutOfMemory;

    impl.process( inBuffer, pState->sidechainbuffer, outBuffer, length );
    return Success;
}


template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
Plugin<Impl, Protocol::Unity>::setPosition( UnityAudioEffectState * const pState, unsigned int const positionInSamples )
{
    impl( pState ).setPosition( positionInSamples );
    return Success;
}


////////////////////////////////////////////////////////////////////////////////
//
// Plugin<Unity>::setParameter()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
Plugin<Impl, Protocol::Unity>::setParameter( UnityAudioEffectState * const pState, int const index, float const value )
{
    return Impl::makeErrorCode( impl( pState ).setParameter( ParameterIndex{ static_cast<ParameterIndex::value_type>( index ) }, value ) );
}


////////////////////////////////////////////////////////////////////////////////
//
// Plugin<Unity>::getParameter()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
Plugin<Impl, Protocol::Unity>::getParameter( UnityAudioEffectState * const pState, int const index, float * const pValue, char * const pValueString )
{
    BOOST_ASSERT( pValue );
    Impl & effect( impl( pState ) );
    ParameterIndex const parameterIndex{ static_cast<ParameterIndex::value_type>( index ) };
    *pValue = effect.getParameter( parameterIndex );
    if ( pValueString )
    {
    #if !LE_NO_PARAMETER_STRINGS
        pValueString[  0 ] = 0;
        pValueString[ 15 ] = 0; //...mrmlj...
        effect.getParameterDisplay( parameterIndex, boost::make_iterator_range_n( pValueString, 16 ), nullptr );
        BOOST_ASSERT( std::strlen( pValueString ) < 16 );
    #endif // !LE_NO_PARAMETER_STRINGS
    }
    return Success;
}


template <class Impl>
LE_NOTHROW LE_COLD
UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK
Plugin<Impl, Protocol::Unity>::getSignal
(
    UnityAudioEffectState       * LE_RESTRICT const pState,
    char                  const * LE_RESTRICT const pName,
    float                       * LE_RESTRICT const pBuffer,
    int                                       const numberOfSamples
)
{
    boost::ignore_unused( pState, pName, pBuffer, numberOfSamples );
    return Unsupported;
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
