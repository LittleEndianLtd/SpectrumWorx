////////////////////////////////////////////////////////////////////////////////
///
/// plugin.inl
/// ----------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin.hpp"
#include "properties.hpp"

#include "le/plugins/entryPoint.hpp"

#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"
#include "le/utility/typeTraits.hpp"

#include "boost/assert.hpp"
#include "boost/range/iterator_range_core.hpp"

#include <cmath>
#include <cstddef>
//------------------------------------------------------------------------------
#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
typedef OSStatus (*AudioComponentMethod) (void *self,...);
struct AudioComponentPlugInInterface
{
    OSStatus             (*Open)(void *self, AudioComponentInstance);
    OSStatus             (*Close)(void *self);
    AudioComponentMethod (*Lookup) (SInt16 selector);
    void *				 reserved;
};
#endif // MAC_OS_X_VERSION_10_6
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// Audio Component Plugin API
///
////////////////////////////////////////////////////////////////////////////////


template <class Impl>
struct Plugin<Impl, Protocol::AU>::AudioComponentPlugInInstance : ::AudioComponentPlugInInterface
{
public:
    AudioComponentPlugInInstance()
    {
        ::AudioComponentPlugInInterface	& interface( *this );

        interface.Open     = AudioComponentPlugInInstance::open  ;
        interface.Close    = AudioComponentPlugInInstance::close ;
        interface.Lookup   = AudioComponentPlugInInstance::lookup;
        interface.reserved = nullptr;
    }

    typename std::aligned_storage<sizeof( Impl ), std::alignment_of<Impl>::value>::type implPlaceholder;


public:
    using This = AudioComponentPlugInInstance;

    static This & LE_RESTRICT instance( void * const pSelf )
    {
        LE_ASSUME( pSelf );
        AudioComponentPlugInInstance * LE_RESTRICT const pInstance( static_cast<AudioComponentPlugInInstance *>( pSelf ) );
        return *pInstance;
    }

    static Impl & LE_RESTRICT impl( This & instance )
    {
        Impl & LE_RESTRICT impl( reinterpret_cast<Impl &>( instance.implPlaceholder ) );
        return impl;
    }

    Impl       & LE_RESTRICT impl()       { return                                                     impl( *this ); }
    Impl const & LE_RESTRICT impl() const { return const_cast<AudioComponentPlugInInstance &>( *this ).impl(       ); }

public: // Callbacks

    ////////////////////////////////////////////////////////////////////////////
    // Creation and initialisation
    ////////////////////////////////////////////////////////////////////////////

    static OSStatus LE_NOTHROW open( void * const pSelf, ::AudioUnit const componentInstance )
    {
        LE_ASSUME( componentInstance );
        Impl * LE_RESTRICT const pImpl( new ( &instance( pSelf ).implPlaceholder ) Impl( componentInstance ) );
        LE_ASSUME( pImpl );
        return noErr;
    }


    static OSStatus LE_NOTHROW close( void * const pSelf )
    {
        AudioComponentPlugInInstance & inst( instance( pSelf ) );
        Impl                         & impl( inst.impl()       );
        impl.~Impl();
        delete &inst;
        return noErr;
    }


    static AudioComponentMethod LE_NOTHROW lookup( ::SInt16 const selector )
    {
        switch ( selector )
        {
            case kAudioUnitInitializeSelect                        : return reinterpret_cast<AudioComponentMethod>( AUMethodInitialize                         );
            case kAudioUnitUninitializeSelect                      : return reinterpret_cast<AudioComponentMethod>( AUMethodUninitialize                       );
            case kAudioUnitGetPropertyInfoSelect                   : return reinterpret_cast<AudioComponentMethod>( AUMethodGetPropertyInfo                    );
            case kAudioUnitGetPropertySelect                       : return reinterpret_cast<AudioComponentMethod>( AUMethodGetProperty                        );
            case kAudioUnitSetPropertySelect                       : return reinterpret_cast<AudioComponentMethod>( AUMethodSetProperty                        );
            case kAudioUnitAddPropertyListenerSelect               : return reinterpret_cast<AudioComponentMethod>( AUMethodAddPropertyListener                );
        #if (!__LP64__)
            case kAudioUnitRemovePropertyListenerSelect            : return reinterpret_cast<AudioComponentMethod>( AUMethodRemovePropertyListener             );
        #endif
            case kAudioUnitRemovePropertyListenerWithUserDataSelect: return reinterpret_cast<AudioComponentMethod>( AUMethodRemovePropertyListenerWithUserData );
            case kAudioUnitAddRenderNotifySelect                   : return reinterpret_cast<AudioComponentMethod>( AUMethodAddRenderNotify                    );
            case kAudioUnitRemoveRenderNotifySelect                : return reinterpret_cast<AudioComponentMethod>( AUMethodRemoveRenderNotify                 );
            case kAudioUnitGetParameterSelect                      : return reinterpret_cast<AudioComponentMethod>( AUMethodGetParameter                       );
            case kAudioUnitSetParameterSelect                      : return reinterpret_cast<AudioComponentMethod>( AUMethodSetParameter                       );
            case kAudioUnitScheduleParametersSelect                : return reinterpret_cast<AudioComponentMethod>( AUMethodScheduleParameters                 );
            case kAudioUnitRenderSelect                            : return reinterpret_cast<AudioComponentMethod>( AUMethodRender                             );
            case kAudioUnitResetSelect                             : return reinterpret_cast<AudioComponentMethod>( AUMethodReset                              );
          //case kAudioUnitProcessSelect                           : return reinterpret_cast<AudioComponentMethod>( AUMethodProcess                            );
          //case kAudioUnitComplexRenderSelect                     : return reinterpret_cast<AudioComponentMethod>( AUMethodComplexRender                      );
        }

        LE_TRACE( "\tSW AU: cannot do selector %hu.", selector );
	    return nullptr;
    }


    static OSStatus LE_NOTHROW AUMethodInitialize( This & LE_RESTRICT instance )
    {
        using namespace Detail;

        // http://lists.apple.com/archives/coreaudio-api/2004/Jun/msg00238.html

        auto & LE_RESTRICT impl     ( instance.impl() );
        auto & LE_RESTRICT hostProxy( impl    .host() );

        /// \note AU Lab 2.2.2 (Mountain Lion) calls this more than once and it
        /// gets confused if we return kAudioUnitErr_Initialized.
        ///                                   (21.02.2013.) (Domagoj Saric)
        if ( BOOST_UNLIKELY( impl.initialised_ ) )
        {
            BOOST_ASSERT_MSG( hostProxy.inputChannels_  == impl.numberOfInputChannels (), "IO setup mismatch on extra initialisation." );
            BOOST_ASSERT_MSG( hostProxy.sideChannels_   == impl.numberOfSideChannels  (), "IO setup mismatch on extra initialisation." );
            BOOST_ASSERT_MSG( hostProxy.outputChannels_ == impl.numberOfOutputChannels(), "IO setup mismatch on extra initialisation." );
            return LE_TRACE_RETURN( noErr, "\tSW AU: multiple initialisation." );
        }

        /// \note Logic 10.2 and OSX10.11 auval set bus connections after
        /// initialisation.
        ///                                   (12.11.2015.) (Domagoj Saric)
        //...mrmlj...SW HARDCODE...
        std::uint8_t const sideChannels( impl.inputConnections_[ 1 ] ? hostProxy.sideChannels_ : 0 );
        LE_TRACE_IF
        (
            sideChannels != hostProxy.sideChannels_,
            "\tSW AU: side channels enabled but no connection given before initialisation."
        );

        /// \note See the related note in
        /// PropertyHandler<kAudioUnitProperty_StreamFormat>::set().
        ///                                   (27.08.2014.) (Domagoj Saric)
        OSStatus const channelSetupResult( makeErrorCode( impl.setNumberOfChannels( hostProxy.inputChannels_ + sideChannels, hostProxy.outputChannels_ ) ) );
        if ( BOOST_UNLIKELY( channelSetupResult != noErr ) )
        {
            LE_TRACE( "\tSW AU: initialisation failure: unsupported IO mode (%d+%d:%d).", hostProxy.inputChannels_, hostProxy.sideChannels_, hostProxy.outputChannels_ );
            hostProxy.inputChannels_  = impl.numberOfInputChannels ();
            hostProxy.sideChannels_   = impl.numberOfSideChannels  ();
            hostProxy.outputChannels_ = impl.numberOfOutputChannels();
            return channelSetupResult; //kAudioUnitErr_FormatNotSupported
        }
        if ( !impl.processBlockSize() )
        {
            std::uint32_t const defaultAUProcessingBlockSize( 1156 );
            OSStatus const blockSizeResult( makeErrorCode( impl.setBlockSize( defaultAUProcessingBlockSize ) ) );
            if ( blockSizeResult != noErr )
                return LE_TRACE_RETURN( blockSizeResult, "Initialisation failure: process block allocation failure." );
        }
        OSStatus const initResult( makeErrorCode( impl.initialise() ) );
        if ( BOOST_UNLIKELY( initResult != noErr ) )
            return LE_TRACE_RETURN( initResult, "Initialisation failure: plugin init failure (" LE_OSX_INT_FORMAT( d ) ").", initResult );
        impl.resume();
      //updateInitialDelayAndTailTimes();
        impl.initialised_ = true;
        return noErr;
    }

    static OSStatus LE_NOTHROW AUMethodUninitialize( This & instance )
    {
        Impl & LE_RESTRICT impl( instance.impl() );

        /// \note auval tool 1.6.1a1 (Mountain Lion)
        ///                                   (25.02.2013.) (Domagoj Saric)
        if ( BOOST_UNLIKELY( !impl.initialised_ ) )
            return LE_TRACE_RETURN( noErr, "Uninitialising a non initialised AU." ); // kAudioUnitErr_Uninitialized

        BOOST_ASSERT( impl.host().inputChannels_  == impl.numberOfInputChannels () );
        BOOST_ASSERT( impl.host().sideChannels_   == impl.numberOfSideChannels  () );
        BOOST_ASSERT( impl.host().outputChannels_ == impl.numberOfOutputChannels() );

        impl.suspend     ();
        impl.uninitialise();
        impl.initialised_ = false;
        return noErr;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Property access
    ////////////////////////////////////////////////////////////////////////////

    static OSStatus LE_NOTHROW AUMethodGetPropertyInfo
    (
        This                const & LE_RESTRICT       instance  ,
        AudioUnitPropertyID                     const propertyID,
        AudioUnitScope                          const scope     ,
        AudioUnitElement                        const element   ,
        UInt32                    * LE_RESTRICT const pDataSize ,
        Boolean                   * LE_RESTRICT const pWritable
    )
    {
        // http://developer.apple.com/library/mac/#documentation/AudioUnit/Reference/AudioUnitPropertiesReference/Reference/reference.html
        // http://developer.apple.com/library/mac/#qa/qa1684/_index.html

        //...mrmlj...SW HARDCODE...
        if ( scope > kAudioUnitScope_Output )
            return kAudioUnitErr_InvalidScope;

        Impl const & LE_RESTRICT impl( instance.impl() );

        Detail::PropertyInfoGetter infoGetter;

        OSStatus const result( Detail::handleProperty( impl, propertyID, scope, element, infoGetter ) );

        LE_TRACE_IF
        (
            result != noErr,
            "\tSW AU: getPropertyInfo() failed with " LE_OSX_INT_FORMAT( d ) " for %s/" LE_OSX_INT_FORMAT( u ) ".",
            result, infoGetter.pPropertyName, propertyID
        );

        if ( pDataSize ) *pDataSize = infoGetter.dataSize;
        if ( pWritable ) *pWritable = infoGetter.writable;

	    return result;
    }


    static OSStatus LE_NOTHROW AUMethodGetProperty
    (
        This                const & LE_RESTRICT       instance  ,
        AudioUnitPropertyID                     const propertyID,
        AudioUnitScope                          const scope     ,
        AudioUnitElement                        const element   ,
        void                      *             const pOutData  ,
        UInt32                    * LE_RESTRICT const pDataSize
    )
    {
        if ( !pOutData )
        {
            LE_TRACE( "\tSW AU: getProperty(): null input data pointer (propertyID " LE_OSX_INT_FORMAT( u ) ").\n", propertyID );
            return kAudioUnitErr_InvalidParameter;
        }

        LE_ASSUME( pOutData  );
        LE_ASSUME( pDataSize );

        //...mrmlj...SW HARDCODE...
        if ( scope > kAudioUnitScope_Group )
        { //...mrmlj...snow leopard auval...
            if ( !( propertyID == kAudioUnitProperty_ElementCount && scope == kAudioUnitScope_Group ) )
                return kAudioUnitErr_InvalidScope;
        }

        Impl const & LE_RESTRICT impl( instance.impl() );

        Detail::PropertyGetter const getter = { pOutData, *pDataSize };

        OSStatus const result( Detail::handleProperty( impl, propertyID, scope, element, getter ) );
        *pDataSize = getter.dataSize;

        LE_TRACE_IF( result != noErr, "\tSW AU: getProperty() failed with " LE_OSX_INT_FORMAT( d ) " for %s/" LE_OSX_INT_FORMAT( u ) ".", result, getter.pPropertyName, propertyID );
        return result;
    }


    static OSStatus LE_NOTHROW AUMethodSetProperty
    (
        This                      & LE_RESTRICT       instance  ,
        AudioUnitPropertyID                     const propertyID,
        AudioUnitScope                          const scope     ,
        AudioUnitElement                        const element   ,
        void                const *             const pInData   ,
        UInt32                                  const dataSize
    )
    {
        if ( !pInData && !dataSize )
        {
            //...mrmlj...AUBase::DispatchRemovePropertyValue(inID, inScope, inElement);
            LE_UNREACHABLE_CODE();
        }

        LE_ASSUME( pInData  );
        LE_ASSUME( dataSize );

        //...mrmlj...SW HARDCODE...
        if ( element > 1 ) // Ableton Live 8.3.4 & kAudioUnitProperty_StreamFormat
            return LE_TRACE_RETURN( kAudioUnitErr_InvalidElement, "\tSW AU: host tried to set property " LE_OSX_INT_FORMAT( u ) " for non existent bus " LE_OSX_INT_FORMAT( u ) ".", propertyID, element );

        Impl & LE_RESTRICT impl( instance.impl() );

        Detail::PropertySetter const setter = { pInData, dataSize };

        OSStatus const result( Detail::handleProperty( impl, propertyID, scope, element, setter ) );

        LE_TRACE_IF( result != noErr, "\tSW AU: setProperty() failed with " LE_OSX_INT_FORMAT( d ) " for %s/" LE_OSX_INT_FORMAT( u ) ".", result, setter.pPropertyName, propertyID );

        if ( result == noErr && setter.changed )
        {
            impl.host().propertyChanged( propertyID, scope, element );
        }

	    return result;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    static OSStatus LE_NOTHROW AUMethodGetParameter( This & instance, ::AudioUnitParameterID const parameterID, ::AudioUnitScope const scope, ::AudioUnitElement const element, ::AudioUnitParameterValue & value )
    {
        //...mrmlj...SW HARDCODE...
        LE_ASSUME( scope   == kAudioUnitScope_Global );
        LE_ASSUME( element == 0                      );

        value = instance.impl().getParameter( ParameterID{ parameterID } );
        BOOST_ASSERT( std::isfinite( value ) );
	    return noErr;
    }

    static OSStatus LE_NOTHROW AUMethodSetParameter( This & instance, ::AudioUnitParameterID const parameterID, ::AudioUnitScope const scope, ::AudioUnitElement const element, ::AudioUnitParameterValue const value, ::UInt32 const bufferOffset )
    {
        //...mrmlj...SW HARDCODE...
        LE_ASSUME( scope   == kAudioUnitScope_Global );
        LE_ASSUME( element == 0                      );
        BOOST_ASSERT( std::isfinite( value ) );

        /// \todo Implement this properly.
        ///                                   (12.11.2015.) (Domagoj Saric)
        LE_TRACE_IF( bufferOffset != 0, "\tSW AU: host tried to make a scheduled set of parameter " LE_OSX_INT_FORMAT( u ) " (offset " LE_OSX_INT_FORMAT( u ) ").", parameterID, bufferOffset );

        instance.impl().setParameter( ParameterID{ parameterID }, value );
	    return noErr;
    }

    static OSStatus LE_NOTHROW AUMethodScheduleParameters( This & instance, ::AudioUnitParameterEvent const * LE_RESTRICT const pEvents, ::UInt32 const numberOfEvents )
    {
        /// \todo Implement this properly.
        ///                                   (12.11.2015.) (Domagoj Saric)
        for ( auto const & __restrict event : boost::make_iterator_range_n( pEvents, numberOfEvents ) )
        {
            ::AudioUnitParameterValue const * __restrict pValue;
            ::SInt32                                     offset;
            if ( event.eventType == kParameterEvent_Immediate )
            {
                pValue = &event.eventValues.immediate.value;
                offset =  event.eventValues.immediate.bufferOffset;
            }
            else
            {
                BOOST_ASSERT( event.eventType == kParameterEvent_Ramped );
                LE_TRACE( "\tSW AU: host tried to make a ramped set of parameter " LE_OSX_INT_FORMAT( u ) ".", event.parameter );
                pValue = &event.eventValues.ramp.endValue;
                offset =  event.eventValues.ramp.startBufferOffset + event.eventValues.ramp.durationInFrames;
            }
            BOOST_VERIFY
            (
                AUMethodSetParameter
                (
                    instance,
                    event.parameter,
                    event.scope,
                    event.element,
                    *pValue,
                    offset
                ) == noErr
            );
        }

	    return noErr;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Callbacks
    ////////////////////////////////////////////////////////////////////////////

    static OSStatus LE_NOTHROW AUMethodAddPropertyListener( This & instance, ::AudioUnitPropertyID const propertyID, ::AudioUnitPropertyListenerProc const callback, void * const pUserData )
    {
        return instance.impl().host().addPropertyListener( propertyID, callback, pUserData );
    }

#if !__LP64__
    static OSStatus LE_NOTHROW AUMethodRemovePropertyListener( This & instance, ::AudioUnitPropertyID const propertyID, ::AudioUnitPropertyListenerProc const callback )
    {
        return instance.impl().host().removePropertyListener( propertyID, callback, nullptr );
    }
#endif

    static OSStatus LE_NOTHROW AUMethodRemovePropertyListenerWithUserData( This & instance, ::AudioUnitPropertyID const propertyID, ::AudioUnitPropertyListenerProc const callback, void * const pUserData )
    {
        return instance.impl().host().removePropertyListener( propertyID, callback, pUserData );
    }

    static OSStatus LE_NOTHROW AUMethodAddRenderNotify( This & instance, AURenderCallback const callback, void * const pUserData )
    {
        BOOST_ASSERT( callback );
        ::AURenderCallbackStruct const renderDelegate = { callback, pUserData };
        return Detail::makeErrorCode( instance.impl().renderNotificationCallbacks_.push_back( renderDelegate ) );
    }

    static OSStatus LE_NOTHROW AUMethodRemoveRenderNotify( This & instance, AURenderCallback const callback, void const * const pUserData )
    {
        BOOST_ASSERT( callback );
        ::AURenderCallbackStruct const renderDelegate = { callback, const_cast<void *>( pUserData ) };
        instance.impl().renderNotificationCallbacks_.remove( renderDelegate );
        return noErr;
    }


    ////////////////////////////////////////////////////////////////////////////
    // Processing
    ////////////////////////////////////////////////////////////////////////////

    static OSStatus LE_NOTHROW AUMethodRender
    (
          This                             & LE_RESTRICT       instance       ,
        ::AudioUnitRenderActionFlags       * LE_RESTRICT const pIOActionFlags ,
        ::AudioTimeStamp             const * LE_RESTRICT const pTimeStamp     ,
        ::UInt32                                         const outputBusNumber,
        ::UInt32                                         const numberOfSamples,
        ::AudioBufferList                  * LE_RESTRICT const pIOData
    )
    {
        // AUBase::DoRender
        // SymbiosisComponent::render

        LE_ASSUME( pTimeStamp );
        LE_ASSUME( pIOData    );

        Impl & LE_RESTRICT impl( instance.impl() );

        LE_ASSUME( impl.initialised_ );

        if ( BOOST_UNLIKELY( numberOfSamples > impl.processBlockSize() ) ) //...mrmlj...auval tests this...
            return kAudioUnitErr_TooManyFramesToProcess;

        BOOST_ASSERT_MSG( outputBusNumber               == 0                      , "SW HARDCODED: single output bus."      );
        BOOST_ASSERT_MSG( impl.numberOfOutputChannels() == pIOData->mNumberBuffers, "Incorrect number of IO data channels." );

        auto const numberOfChannels( impl.numberOfOutputChannels() );

    #ifdef _DEBUG
        if ( pIOData->mBuffers[ 0 ].mData != nullptr )
        {
            for ( AudioBuffer const * pBuffer( pIOData->mBuffers ); pBuffer < &pIOData->mBuffers[ pIOData->mNumberBuffers ]; ++pBuffer )
            {
                BOOST_ASSERT_MSG( pBuffer->mData                                             , "Unexpected null IO buffer."   );
                BOOST_ASSERT_MSG( pBuffer->mDataByteSize == numberOfSamples * sizeof( float ), "Incorrect IO buffer size,"    );
                BOOST_ASSERT_MSG( reinterpret_cast<std::size_t>( pBuffer->mData ) % 16 == 0  , "Unexpected buffer alignment." );
            }
        }
    #endif // _DEBUG

        ::AudioUnitRenderActionFlags               dummyActionFlags( kAudioUnitRenderAction_DoNotCheckRenderArgs );
        ::AudioUnitRenderActionFlags & LE_RESTRICT ioActionFlags   ( pIOActionFlags ? *pIOActionFlags : dummyActionFlags );

        impl.sendRenderNotification( kAudioUnitRenderAction_PreRender, ioActionFlags, *pTimeStamp, outputBusNumber, numberOfSamples, *pIOData );

        bool const ignoreShouldAllocateOutputBufferIfProvidedBuffer( true );
        bool const callerProvidedOutputBuffer( pIOData->mBuffers[ 0 ].mData != nullptr );
        bool const useProvidedBufferForInput
        (
            ( ignoreShouldAllocateOutputBufferIfProvidedBuffer && callerProvidedOutputBuffer        ) ||
            ( impl.inPlaceProcessing_                          && !impl.shouldAllocateOutputBuffer_ )
        );

        OSStatus const mainRenderResult
        (
            impl.inputConnections_[ 0 ]
            (
                ioActionFlags,
                pTimeStamp,
                numberOfSamples,
                useProvidedBufferForInput ? pIOData : nullptr,
                numberOfChannels
            )
        );
        LE_TRACE_IF(  mainRenderResult != noErr, "\tSW AU: main input render error (%ld)", mainRenderResult );
        BOOST_ASSERT( mainRenderResult == noErr );

        bool const hasSideChannel( impl.inputConnections_[ 1 ] );
        OSStatus const sideRenderResult
        (
            hasSideChannel
                ? impl.inputConnections_[ 1 ]( ioActionFlags, pTimeStamp, numberOfSamples, nullptr, numberOfChannels )
                : noErr
        );
        LE_TRACE_IF(  sideRenderResult != noErr, "\tSW AU: side input render error (%ld)", sideRenderResult );
        BOOST_ASSERT( sideRenderResult == noErr );

        if
        ( BOOST_LIKELY(
            ( mainRenderResult == noErr ) &&
            ( sideRenderResult == noErr )
        ) )
        {
            ::AudioBufferList * pActualOutputBuffers;
            if
            (
                        callerProvidedOutputBuffer &&
                ( !impl.shouldAllocateOutputBuffer_ || ignoreShouldAllocateOutputBufferIfProvidedBuffer ) &&
                  !impl.inPlaceProcessing_
            )
            {
                pActualOutputBuffers = pIOData;
            }
            else
            {
                LE_ASSUME( impl.shouldAllocateOutputBuffer_ );
                if ( impl.inPlaceProcessing_ )
                {
                    pActualOutputBuffers = pIOData;
                    if ( !callerProvidedOutputBuffer )
                        Detail::AudioBuffers::alias( impl.inputConnections_[ 0 ].buffers(), *pIOData );
                }
                else
                {
                    if ( BOOST_UNLIKELY( !impl.outputBuffers_.resize( numberOfChannels, numberOfSamples, false ) ) )
                        return kAudio_MemFullError;
                    pActualOutputBuffers = impl.outputBuffers_;
                }
            }
            BOOST_ASSERT_MSG( pActualOutputBuffers->mBuffers[ 0 ].mData, "Null output buffer." );

            ::AudioBuffer const * LE_RESTRICT const pMainInput( impl.inputConnections_[ 0 ].buffers().mBuffers );
            ::AudioBuffer const * LE_RESTRICT const pSideInput( impl.inputConnections_[ 1 ].buffers().mBuffers );
            if ( hasSideChannel )
            {
                /// \note Logic (8, 9 and 10) always connect 'something' to the
                /// side chain bus, even if you select 'None' in the GUI (in
                /// which case it sends the main input to the side chain bus).
                ///                           (12.11.2015.) (Domagoj Saric)
                bool const silentOrFakeSideChain
                (
                    ( ioActionFlags & kAudioUnitRenderAction_OutputIsSilence ) ||
                    ( pMainInput->mData == pSideInput->mData                 )
                );
                auto const result( Detail::makeErrorCode( impl.enableSideChannelInput( !silentOrFakeSideChain ) ) );
                if ( BOOST_UNLIKELY( result != noErr ) )
                    return result;
                impl.host().sideChannels_ = silentOrFakeSideChain ? 0 : impl.host().inputChannels_;
            }

            auto const numberOfMainChannels( impl.numberOfInputChannels() );
            auto const numberOfSideChannels( impl.numberOfSideChannels () );
            LE_ASSUME( numberOfMainChannels < 16 );
            LE_ASSUME( numberOfSideChannels < 16 );

            float const * __restrict pInputChannels [ numberOfMainChannels + numberOfSideChannels ];
            float       * __restrict pOutputChannels[ numberOfMainChannels                        ];
            {
                ::AudioBuffer * LE_RESTRICT const pOutput( pActualOutputBuffers->mBuffers );
                for ( std::uint8_t channel( 0 ); channel < numberOfMainChannels; ++channel ) { pInputChannels [ channel                        ] = static_cast<float const *>( pMainInput[ channel ].mData ); }
                for ( std::uint8_t channel( 0 ); channel < numberOfSideChannels; ++channel ) { pInputChannels [ channel + numberOfMainChannels ] = static_cast<float const *>( pSideInput[ channel ].mData ); }
                for ( std::uint8_t channel( 0 ); channel < numberOfMainChannels; ++channel ) { pOutputChannels[ channel                        ] = static_cast<float       *>( pOutput   [ channel ].mData ); }
            }

            impl.host().setCurrentTimeStamp( pTimeStamp );
            impl.process( pInputChannels, pOutputChannels, numberOfSamples );
            impl.host().setCurrentTimeStamp( nullptr    );

            if ( callerProvidedOutputBuffer && ( pActualOutputBuffers != pIOData ) )
            {
                LE_ASSUME( !ignoreShouldAllocateOutputBufferIfProvidedBuffer );
                Detail::AudioBuffers::copy( *pActualOutputBuffers, *pIOData );
            }
        } // successful input renders

        impl.sendRenderNotification
        (
            kAudioUnitRenderAction_PostRender | ( mainRenderResult != noErr ? kAudioUnitRenderAction_PostRenderError : 0 ),
            ioActionFlags,
            *pTimeStamp,
            outputBusNumber,
            numberOfSamples,
            *pIOData
        );

        impl.setRenderResult( mainRenderResult );

	    return mainRenderResult;
    }

    static OSStatus LE_NOTHROW AUMethodComplexRender( This & /*instance*/, AudioUnitRenderActionFlags * /*ioActionFlags*/, AudioTimeStamp const * /*inTimeStamp*/, UInt32 /*outputBusNumber*/, UInt32 /*inNumberOfPackets*/, UInt32 * /*outNumberOfPackets*/, AudioStreamPacketDescription * /*outPacketDescriptions*/, AudioBufferList * /*ioData*/, void * /*outMetadata*/, UInt32 * /*outMetadataByteSize*/ )
    {
        LE_UNREACHABLE_CODE();
	    return kAudio_UnimplementedError;
    }

    static OSStatus LE_NOTHROW AUMethodProcess
    (
        This                             & LE_RESTRICT       /*instance      */,
        AudioUnitRenderActionFlags       * LE_RESTRICT const /*pIOActionFlags*/,
        AudioTimeStamp             const * LE_RESTRICT const /*pTimeStamp    */,
        UInt32                                         const /*numberFrames  */,
        AudioBufferList                  * LE_RESTRICT const /*ioData        */
    )
    {
        //...mrmlj...called by 10.9 auval in 32 bit mode...
        // http://lists.apple.com/archives/coreaudio-api/2003/Jun/msg00204.html
        // http://lists.apple.com/archives/coreaudio-api/2003/May/msg00395.html
        // http://stackoverflow.com/questions/1745940/how-to-determine-what-input-bus-is-active-when-aueffectbaserender-is-called
        // http://www.juce.com/forum/topic/multiple-busses-au
        LE_UNREACHABLE_CODE();
        return kAudio_UnimplementedError;
    }

    static OSStatus LE_NOTHROW AUMethodReset( This & instance, AudioUnitScope /*const scope*/, AudioUnitElement )
    {
        Impl & LE_RESTRICT impl( instance.impl() );
        //...mrmlj...n-Track calls this on all scopes (causes redundant resets)...
        //LE_ASSUME( scope == kAudioUnitScope_Global );
        if ( impl.initialised_ )
            impl.reset();
        //updateInitialDelayAndTailTimes();
        return noErr;
    }
}; // struct AudioComponentPlugInInstance



////////////////////////////////////////////////////////////////////////////////
///
/// Component Manager API
///
////////////////////////////////////////////////////////////////////////////////

namespace
{
    unsigned int actualParameterIndex
    (
        unsigned int const index,
        unsigned int const totalParameters
    )
    {
        // http://forum.cockos.com/showthread.php?p=983937
    #if defined( __APPLE__ )
	    #if __LP64__
		    // comp instance, parameters in forward order
            unsigned int const actualIndex( index + 1 );
	    #else
		    // parameters in reverse order, then comp instance
            /// \note We handle out-of-bounds indices here in order to simplify
            /// the default case generic pMethod call in the
            /// componentManagerEntry() function.
            ///                               (29.08.2013.) (Domagoj Saric)
		    unsigned int const wouldBeActualIndex( totalParameters - 1 - index                                             );
            unsigned int const actualIndex       ( ( static_cast<int>( wouldBeActualIndex ) < 0 ) ? 0 : wouldBeActualIndex );
	    #endif
    #elif defined( _WIN32 )
		    // (no comp instance), parameters in forward order
			unsigned int const actualIndex( index );
    #endif
        return actualIndex;
    }

    template <typename Parameter>
    Parameter const & parameter
    (
        ::ComponentParameters * LE_RESTRICT const pParameters,
        unsigned int                        const index,
        unsigned int                        const totalParameters
    )
    {
        LE_ASSUME( index < totalParameters );
        unsigned int const actualIndex( actualParameterIndex( index, totalParameters ) );
        Parameter const & parameter( reinterpret_cast<Parameter const &>( pParameters->params[ actualIndex ] ) );
        return parameter;
    }
} // anonymous namespace

template <class Impl>
::ComponentResult LE_NOTHROW Plugin<Impl, Protocol::AU>::componentManagerEntry
(
    ::ComponentParameters * LE_RESTRICT const pParameters,
    ::Handle                            const userDataHandle
)
{
    // https://developer.apple.com/library/mac/#documentation/Carbon/reference/Component_Manager/Reference/reference.html
    // http://destroyfx.smartelectronix.com/dfx-au-utilities.html

    /// \note The new/"AU plugin"/Lion API has a slightly different ABI than the
    /// old/Component manager based one: the object passed as the
    /// component/plugin instance is of the AudioComponentPlugInInstance type.
    /// In order to use the same code for both APIs we (re)use the
    /// new/AudioComponentPlugInInstance approach even when loaded through the
    /// ComponentManager which only incurs the size overhead of the four
    /// AudioComponentPlugInInstance data members (pointers).
    ///                                       (20.02.2013.) (Domagoj Saric)

    LE_ASSUME( pParameters );

    ::SInt16 const selector( pParameters->what );

    BOOST_ASSERT
    (
        ( pParameters->paramSize % sizeof( pParameters->params[ 0 ] ) == 0 ) ||
        selector == kComponentCanDoSelect
    );

    if ( selector == kComponentOpenSelect )
    {
        BOOST_ASSERT( userDataHandle == nullptr );
    #if __LP64__
        BOOST_ASSERT( pParameters->paramSize == 2 * sizeof( pParameters->params[ 0 ] ) );
        BOOST_ASSERT( pParameters->params[ 0 ] == pParameters->params[ 1 ] );
    #else
        BOOST_ASSERT( pParameters->paramSize == sizeof( pParameters->params[ 0 ] ) );
    #endif // __LP64__
        ::AudioUnit const auComponentInstance( reinterpret_cast<::ComponentInstance>( pParameters->params[ 0 ] ) );
        AudioComponentPlugInInstance * LE_RESTRICT const pInstance( new (std::nothrow) AudioComponentPlugInInstance );
        if ( !pInstance )
            return kAudio_MemFullError;
        BOOST_VERIFY( AudioComponentPlugInInstance::open( pInstance, auComponentInstance ) == noErr );
        ::SetComponentInstanceStorage( auComponentInstance, reinterpret_cast<::Handle>( pInstance ) );
        return noErr;
    }

    AudioComponentPlugInInstance & instance( AudioComponentPlugInInstance::instance( userDataHandle ) );

    switch ( selector )
    {
        case kComponentCanDoSelect:
        {
            //...mrmlj...auval -comp...
            //BOOST_ASSERT( userDataHandle == nullptr );
            //BOOST_ASSERT( pParameters->paramSize == sizeof( pParameters->params[ 0 ] ) );
            ::SInt16 const canDoSelector( reinterpret_cast<::SInt16 const &>( pParameters->params[ 0 ] ) );
            switch ( canDoSelector )
            {
				case kComponentOpenSelect   :
                case kComponentCloseSelect  :
                case kComponentVersionSelect:
					return true;

                case kComponentCanDoSelect       :
                case kAudioUnitInitializeSelect  :
                #if __LP64__
                    return true;
                #endif // __LP64__
                case kAudioUnitUninitializeSelect:
                    LE_UNREACHABLE_CODE();
			}
            return AudioComponentPlugInInstance::lookup( canDoSelector ) != nullptr;
        }

		case kComponentCloseSelect:
        #if __LP64__
            BOOST_ASSERT( pParameters->paramSize == 2 * sizeof( pParameters->params[ 0 ] ) );
            BOOST_ASSERT( pParameters->params[ 0 ] == pParameters->params[ 1 ] );
        #else
            BOOST_ASSERT( pParameters->paramSize == sizeof( pParameters->params[ 0 ] ) );
        #endif // __LP64__
            ::SetComponentInstanceStorage( reinterpret_cast<::ComponentInstance>( pParameters->params[ 0 ] ), nullptr );
            AudioComponentPlugInInstance::close( &instance );
			return noErr;

        case kComponentVersionSelect:
        {
            UInt32 const version( Impl::versionMajor << 16 | Impl::versionMinor << 8 | Impl::versionPatch );
            return version;
        }

        case kAudioUnitGetPropertySelect:
        {
            AudioUnitPropertyID   const propertyID( parameter<AudioUnitPropertyID  >( pParameters, 0, 5 ) );
            AudioUnitScope        const scope     ( parameter<AudioUnitScope       >( pParameters, 1, 5 ) );
            AudioUnitElement      const element   ( parameter<AudioUnitElement     >( pParameters, 2, 5 ) );
            void                * const pData     ( parameter<void                *>( pParameters, 3, 5 ) );
            UInt32              * const pDataSize ( parameter<UInt32              *>( pParameters, 4, 5 ) );

            if ( propertyID == kAudioUnitProperty_FastDispatch )
            {
                LE_ASSUME( scope      == kAudioUnitScope_Global         );
                LE_ASSUME( *pDataSize == sizeof( AudioComponentMethod ) );
                SInt16               const fastDispatchSelector( static_cast<SInt16>( element )                               );
                AudioComponentMethod const pMethod             ( AudioComponentPlugInInstance::lookup( fastDispatchSelector ) );
                if ( pMethod )
                {
                    *static_cast<AudioComponentMethod *>( pData ) = pMethod;
                    break;
                }
                else
                {
                    LE_TRACE( "FastDispatch requested for unimplemented selector (%hd)", fastDispatchSelector );
                    *pDataSize = 0;
                    return kAudio_UnimplementedError;
                }
            }
            // intentional fallthrough...
        }

        default:
        {
            AudioComponentMethod const pMethod( AudioComponentPlugInInstance::lookup( selector ) );
            if ( pMethod )
            {
                // http://thexploit.com/secdev/mac-os-x-10-6-stack-boundaries
                // http://stackoverflow.com/questions/4429398/why-does-windows64-use-a-different-calling-convention-from-all-other-oses-on-x86
                // http://software.intel.com/en-us/blogs/2010/07/01/the-reasons-why-64-bit-programs-require-more-stack-memory
                // https://groups.google.com/forum/#!msg/altdevauthors/tMZQhyeZbBk/yy34T1EL8kMJ
                // http://blogs.msdn.com/b/freik/archive/2005/03/17/398200.aspx

                BOOST_ASSERT_MSG( pParameters->paramSize % sizeof( *pParameters->params ) == 0, "Invalid ComponentParameters::paramSize." );
                unsigned int const minimumParameters( 0                                                       ); // initialise() otherwise the minimum is 2...
                unsigned int const maximumParameters( 5                                                       );
                unsigned int const actualParameters ( pParameters->paramSize / sizeof( *pParameters->params ) );
                LE_ASSUME( actualParameters >= minimumParameters );
                LE_ASSUME( actualParameters <= maximumParameters );

            #if defined( __APPLE__ ) && !__LP64__
                // AUDispatch.cpp: parameters in reverse order, then comp instance
                /// \note It seems that the instance (if there at all)
                /// is not counted in pParameters->paramSize.
                ///                   (20.02.2013.) (Domagoj Saric)
                BOOST_ASSERT_MSG
                (
                    std::find( &pParameters->params[ 0 ], &pParameters->params[ actualParameters ], reinterpret_cast<long>( &instance ) ) == &pParameters->params[ actualParameters ],
                    "Instance found within parameter after all."
                );
            #endif

                return pMethod
                (
                    &instance,
                    pParameters->params[ actualParameterIndex( 0, actualParameters ) ],
                    pParameters->params[ actualParameterIndex( 1, actualParameters ) ],
                    pParameters->params[ actualParameterIndex( 2, actualParameters ) ],
                    pParameters->params[ actualParameterIndex( 3, actualParameters ) ],
                    pParameters->params[ actualParameterIndex( 4, actualParameters ) ]
                );
            }
            else
            {
                LE_TRACE( "AU unknown or unsupported selector: %hd.", selector );
                return kAudio_UnimplementedError;
            }
        }
	}

    return noErr;
}

// Cocoa http://lists.apple.com/archives/coreaudio-api/2003/Aug/msg00224.html


////////////////////////////////////////////////////////////////////////////////
//
// Plugin<Impl, Protocol::AU>::impl()
// ----------------------------------
//
////////////////////////////////////////////////////////////////////////////////


template <class Impl>
Impl & Plugin<Impl, Protocol::AU>::impl() /// \throws nothing
{
    /// \note Clang knows that this must not ever be null (and 3.7 even warns of
    /// this if we put an assume here).
    ///                                       (12.11.2015.) (Domagoj Saric)
    return static_cast<Impl &>( *this );
}

template <class Impl>
Impl & Plugin<Impl, Protocol::AU>::impl( ::Handle const handle ) /// \throws nothing
{
    LE_ASSUME( handle );
    return *reinterpret_cast<Impl * LE_RESTRICT>( handle );
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
///
/// AU entry points
///
////////////////////////////////////////////////////////////////////////////////

LE_ENTRY_POINT_BEGIN
(
    ::ComponentResult,
    lePluginAUEntry,
    ::ComponentParameters * LE_RESTRICT const pParameters,
    ::Handle                            const userDataHandle
)
{
    return ::LE::Plugins::Plugin<Impl, ::LE::Plugins::Protocol::AU>::componentManagerEntry( pParameters, userDataHandle );
}
LE_ENTRY_POINT_END()


// OS X 10.8 changes:
// - http://developer.apple.com/library/mac/#technotes/tn2276/_index.html#//apple_ref/doc/uid/DTS40011031
// - http://developer.apple.com/library/mac/#releasenotes/General/MacOSXLionAPIDiffs/AudioUnit.html
// - http://sample-hold.com/2011/11/23/getting-started-with-audio-units-on-os-x-lion-and-xcode-4-2-1
// - http://code.google.com/p/symbiosis-au-vst/issues/detail?id=22
LE_ENTRY_POINT_BEGIN
(
    AudioComponentPlugInInterface *,
    lePluginAUFactory, // AudioComponentFactoryFunction
    ::AudioComponentDescription const * LE_RESTRICT const pDescription
)
{
    typedef typename ::LE::Plugins::Plugin<Impl, ::LE::Plugins::Protocol::AU>::AudioComponentPlugInInstance AudioComponentPlugInInstance;

    return new (std::nothrow) AudioComponentPlugInInstance;
}
LE_ENTRY_POINT_END()


#ifdef LE_HAS_FRIEND_INJECTION
    #define LE_PLUGIN_AU_ENTRY_POINT( implClass )
#else
    #define LE_PLUGIN_AU_ENTRY_POINT( implClass )                                    \
        extern "C" LE_ENTRY_POINT_ATTRIBUTES                                         \
        ::ComponentResult __cdecl lePluginAUEntry                                    \
        (                                                                            \
            ::ComponentParameters * LE_RESTRICT const pParameters,                   \
            ::Handle                            const userDataHandle                 \
        )                                                                            \
        { return ::lePluginAUEntryImpl<implClass>( pParameters, userDataHandle ); }  \
                                                                                     \
        extern "C" LE_ENTRY_POINT_ATTRIBUTES                                         \
        AudioComponentPlugInInterface * __cdecl lePluginAUFactory                    \
        (                                                                            \
            ::AudioComponentDescription const * LE_RESTRICT const pDescription       \
        )                                                                            \
        { return ::lePluginAUFactoryImpl<implClass>( pDescription ); }
#endif // LE_HAS_FRIEND_INJECTION
