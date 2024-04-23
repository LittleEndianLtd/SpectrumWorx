////////////////////////////////////////////////////////////////////////////////
///
/// plugin.cpp
/// ----------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin.hpp"

#include "le/utility/buffers.hpp"
#include "le/utility/clear.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

AUHostProxy::AUHostProxy( ::AudioUnit const auInstance )
    :
    auInstance_       ( auInstance ),
    pCurrentTimeStamp_( nullptr    )
{
    Utility::clear( callbacks() );
    BOOST_ASSERT( propertyListeners_.begin() == propertyListeners_.end() );

    //...mrmlj...SW HARDCODE...
    /// \note Side channels have to be enabled for AU validation format tests to
    /// pass.
    ///                                       (21.03.2013.) (Domagoj Saric)
    inputChannels_  = 2;
    outputChannels_ = 2;
    sideChannels_   = 2;

    canTryDynamicParameterList_ = false;
}


bool LE_NOTHROW LE_COLD AUHostProxy::propertyChanged( ::AudioUnitPropertyID const propertyID, ::AudioUnitScope const scope, ::AudioUnitElement const element ) const
{
    // http://list-archives.org/2013/02/07/coreaudio-api-lists-apple-com/kaudiounitproperty_latency-not-working-in-logic/f/2245281181

    LE_ASSUME( element == 0 || element == 1 );

    // http://lists.apple.com/archives/coreaudio-api/2009/Dec/msg00224.html
    // http://www.kvraudio.com/forum/viewtopic.php?p=2832575 (AULab AudioUnit UI weirdness?)
    //firePropertyEvent( propertyID );

    PropertyListeners const * LE_RESTRICT const pListeners( findListenersFor( propertyID ) );
    if ( !pListeners )
        return false;

    // https://groups.google.com/forum/#!topic/coreaudio-api/1rZp94kNkh4 Logic registers a ParameterInfo listener for each parameter
    // http://lists.apple.com/archives/coreaudio-api/2013/Mar/msg00034.html [AU] Dynamic parameters
    for ( auto const & listener : *pListeners )
    {
        listener.callback
        (
            listener.pUserData,
            auInstance(),
            propertyID,
            scope,
            element
        );
    }
    return true;
}


bool LE_NOTHROW LE_COLD AUHostProxy::globalPropertyChanged( ::AudioUnitPropertyID const propertyID ) const
{
    return propertyChanged( propertyID, kAudioUnitScope_Global, 0 );
}


void AUHostProxy::automatedParameterChanged( ParameterID const parameterID, float ) const
{
    // http://web.archiveorange.com/archive/v/q7bubUmr1HZyr7sn7htp
    // http://lists.apple.com/archives/coreaudio-api/2007/Feb/msg00107.html
    // https://developer.apple.com/library/mac/#documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitDevelopmentFundamentals/AudioUnitDevelopmentFundamentals.html#//apple_ref/doc/uid/TP40003278-CH7-SW1
    fireParameterEvent( kAudioUnitEvent_ParameterValueChange, parameterID.value );
}

// https://developer.apple.com/library/mac/technotes/tn2104/_index.html#//apple_ref/doc/uid/DTS10003198-CH1-GESTURES
void AUHostProxy::automatedParameterBeginEdit( ParameterID const parameterID ) const
{
    fireParameterEvent( kAudioUnitEvent_BeginParameterChangeGesture, parameterID.value );
}

void AUHostProxy::automatedParameterEndEdit( ParameterID const parameterID ) const
{
    fireParameterEvent( kAudioUnitEvent_EndParameterChangeGesture, parameterID.value );
}


bool AUHostProxy::parameterListChanged() const
{
    bool const parameterListChangeAcknowledged( globalPropertyChanged( kAudioUnitProperty_ParameterList ) );
    canTryDynamicParameterList_ =
        parameterListChangeAcknowledged
            &
        static_cast<AUPluginBase const &>( *this ).staticParameterListReported_; //...ugh...mrmlj...
    if ( parameterListChangeAcknowledged )
        return true;
    LE_TRACE( "Parameter list change ignored" );
    return globalPropertyChanged( kAudioUnitProperty_ParameterInfo );
}


void AUHostProxy::fireParameterEvent( ::AudioUnitEventType const eventType, ::AudioUnitParameterID const parameterID ) const
{
    //::AudioUnitParameter parameter;
    //parameter.mAudioUnit   = auInstance()          ;
    //parameter.mParameterID = parameterID           ;
    //parameter.mScope       = kAudioUnitScope_Global;
    //parameter.mElement     = 0                     ;
    //BOOST_VERIFY( ::AUParameterListenerNotify( nullptr, nullptr, &parameter ) == noErr );
    fireEvent( eventType, parameterID );
}

void AUHostProxy::firePropertyEvent( ::AudioUnitPropertyID const propertID ) const
{
    fireEvent( kAudioUnitEvent_PropertyChange, propertID );
}

void AUHostProxy::fireEvent( ::AudioUnitEventType const eventType, ::UInt32 const itemID ) const
{
    // http://developer.apple.com/library/mac/#technotes/tn2104/_index.html (Handling Audio Unit Events)
    // http://www.kvraudio.com/forum/viewtopic.php?t=292071 ("View >> AU communication")
    // http://developer.apple.com/library/mac/#documentation/Cocoa/Conceptual/EventOverview/EventArchitecture/EventArchitecture.html

    ::AudioUnitEvent event;
    event.mEventType = eventType;
    event.mArgument.mParameter.mAudioUnit   = auInstance()          ;
    event.mArgument.mParameter.mParameterID = itemID                ;
    event.mArgument.mParameter.mScope       = kAudioUnitScope_Global;
    event.mArgument.mParameter.mElement     = 0                     ;
    BOOST_STATIC_ASSERT( sizeof( event.mArgument.mParameter ) == sizeof( event.mArgument.mProperty ) );
    BOOST_ASSERT( event.mArgument.mParameter.mAudioUnit   == event.mArgument.mProperty.mAudioUnit   );
    BOOST_ASSERT( event.mArgument.mParameter.mParameterID == event.mArgument.mProperty.mPropertyID  );
    BOOST_ASSERT( event.mArgument.mParameter.mScope       == event.mArgument.mProperty.mScope       );
    BOOST_ASSERT( event.mArgument.mParameter.mElement     == event.mArgument.mProperty.mElement     );
    BOOST_VERIFY( ::AUEventListenerNotify( 0, 0, &event ) == noErr );
}


OSStatus AUHostProxy::addPropertyListener( ::AudioUnitPropertyID const propertyID, ::AudioUnitPropertyListenerProc const callback, void * const pUserData )
{
    PropertyListeners * LE_RESTRICT pListeners( findListenersFor( propertyID ) );
    if ( !pListeners )
    {
        if ( propertyListeners_.emplace_back( BoundPropertyListeners{ propertyID } ) )
        {
            BOOST_ASSERT( propertyListeners_.back().propertyID == propertyID );
            pListeners = &propertyListeners_.back().listeners;
        }
    }
    if ( pListeners )
    {
    #ifdef _DEBUG
        for ( auto const & listener : *pListeners )
        {
            LE_TRACE_IF
            (
                ( listener.callback == callback ) && ( listener.pUserData == pUserData ),
                "\tSW AU: adding a duplicate property (" LE_OSX_INT_FORMAT( u ) ") listener.", propertyID
            );
        }
    #endif // _DEBUG
        if ( !pListeners->resize( pListeners->size() + 1 ) )
            return Detail::makeErrorCode( false );
        PropertyChangeListener & listener( pListeners->back() );
        listener.callback  = callback ;
        listener.pUserData = pUserData;
    }
    OSStatus const result( Detail::makeErrorCode( pListeners != nullptr ) );
    LE_TRACE_IF( result == noErr, "\tSW AU: added listener (%X, %X) number %u for property ("         LE_OSX_INT_FORMAT( u ) ").", callback, pUserData, pListeners->size() - 1, propertyID );
    LE_TRACE_IF( result != noErr, "\tSW AU: failed to add listener (%X, %X) number %u for property (" LE_OSX_INT_FORMAT( u ) ").", callback, pUserData, pListeners->size()    , propertyID );
    return result;
}


OSStatus AUHostProxy::removePropertyListener( ::AudioUnitPropertyID const propertyID, ::AudioUnitPropertyListenerProc const callback, void const * const pUserData )
{
    PropertyListeners * LE_RESTRICT const pListeners( findListenersFor( propertyID ) );
    if ( !pListeners )
    {
        LE_TRACE( "\tSW AU: failed to remove listener (%X, %X) for property (" LE_OSX_INT_FORMAT( u ) ") - no listeners for that property.", callback, pUserData, propertyID );
        return kAudioUnitErr_InvalidProperty;
    }

#if __LP64__
    unsigned int const trailBytesToSkip(                                              0 );
#else
    unsigned int const trailBytesToSkip( pUserData == nullptr ? sizeof( pUserData ) : 0 );
#endif

    PropertyChangeListener const propertyListener = { callback, const_cast<void *>( pUserData ) };
    BOOST_STATIC_ASSERT( sizeof( PropertyChangeListener ) == 2 * sizeof( pUserData ) );

    PropertyChangeListener * LE_RESTRICT const pElement
    (
        std::find_if
        (
            pListeners->begin(),
            pListeners->end  (),
            [&, trailBytesToSkip]( PropertyChangeListener const & other )
            {
                return std::memcmp( &other, &propertyListener, sizeof( propertyListener ) - trailBytesToSkip ) == 0;
            }
        )
    );
    if ( pElement == pListeners->end() )
    {
        LE_TRACE( "\tSW AU: failed to remove listener (%X, %X) for property (" LE_OSX_INT_FORMAT( u ) ") - listener not found.", callback, pUserData, propertyID );
        return kAudio_ParamError;
    }
    unsigned int const elementsToMove   ( static_cast<unsigned int>( pListeners->end() - pElement ) - 1 );
    unsigned int const remainingElements( pListeners->size()                                        - 1 );
    std::memmove( pElement, pElement + 1, elementsToMove * sizeof( *pElement ) );
    pListeners->resize( remainingElements );

    //OSStatus const result( Detail::makeErrorCode( propertyListeners_.remove( propertyListener,  ) ) );
    OSStatus const result( Detail::makeErrorCode( true ) );
    LE_TRACE( "\tSW AU: removed listener (%X, %X) for property (" LE_OSX_INT_FORMAT( u ) ").", callback, pUserData, propertyID );
    return result;
}


AUHostProxy::PropertyListeners * __restrict__ AUHostProxy::findListenersFor( ::AudioUnitPropertyID const propertyID )
{
    for ( auto & listener : propertyListeners_ )
    {
        if ( listener.propertyID == propertyID )
            return &listener.listeners;
    }
    return nullptr;
}

AUHostProxy::PropertyListeners const * LE_RESTRICT AUHostProxy::findListenersFor( ::AudioUnitPropertyID const propertyID ) const
{
    return const_cast<AUHostProxy &>( *this ).findListenersFor( propertyID );
}


bool AUHostProxy::reportNewNumberOfInputChannels( std::uint8_t /*const inputs*/ ) const
{
    return propertyChanged( kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input , 0 );
}

bool AUHostProxy::reportNewNumberOfOutputChannels( std::uint8_t /*const outputs*/ ) const
{
    return propertyChanged( kAudioUnitProperty_StreamFormat, kAudioUnitScope_Output, 1 );
}


bool AUHostProxy::reportNewNumberOfSideChannels( std::uint8_t /*const sideInputs*/ ) const
{
    return propertyChanged( kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input , 1 );
  //propertyChanged( kAudioUnitProperty_ElementCount, kAudioUnitScope_Output, 1 );
}

bool AUHostProxy::reportNewNumberOfIOChannels( std::uint8_t const inputs, std::uint8_t const sideInputs, std::uint8_t const outputs ) const
{
    BOOST_ASSERT_MSG( findListenersFor( kAudioUnitProperty_StreamFormat ) == nullptr, "Unexpected: host listening to stream format changes." );
    BOOST_ASSERT_MSG( findListenersFor( kAudioUnitProperty_ElementCount ) == nullptr, "Unexpected: host listening to bus count changes."     );
    //return false;
    bool const allowed //...mrmlj...
    (
        ( inputs     == inputChannels_  ) &
        ( outputs    == outputChannels_ ) &
        ( sideInputs == sideChannels_   )
    );
    //...mrmlj...OSX10.6 auval failure...if ( allowed ) const_cast<AUHostProxy &>( *this ).sideChannels_ = sideInputs;
    return allowed;
    //return
    //    ( ( inputs     == inputChannels_  ) || ( reportNewNumberOfInputChannels ( inputs     ) ) ) &
    //    ( ( outputs    == outputChannels_ ) || ( reportNewNumberOfSideChannels  ( sideInputs ) ) ) &
    //    ( ( sideInputs == sideChannels_   ) || ( reportNewNumberOfOutputChannels( outputs    ) ) );
}

bool AUHostProxy::reportNewLatencyInSamples( std::uint16_t /*samples*/ ) const { return globalPropertyChanged( kAudioUnitProperty_Latency ); }
bool AUHostProxy::currentPresetNameChanged (                           ) const { return globalPropertyChanged( kAudioUnitProperty_PresentPreset ); }


void AUHostProxy::presetChangeEnd() const
{
    if ( !globalPropertyChanged( kAudioUnitProperty_ClassInfo ) && !parameterListChanged() )
    {
        currentPresetNameChanged();
        // http://nathanmock.com/files/com.apple.adc.documentation.AppleiOS6.0.iOSLibrary.docset/Contents/Resources/Documents/#samplecode/CoreAudioUtilityClasses/Listings/CoreAudio_AudioUnits_AUPublic_AUCarbonViewBase_AUCarbonViewControl_cpp.html
        // https://github.com/Ardour/ardour/blob/master/libs/ardour/audio_unit.cc
        // http://uri-labs.com/macosx_headers/AudioUnitUtilities_h/index.html
        // http://web.archiveorange.com/archive/v/q7bub4vngCovqXKm4XcH
        ParameterID const anyParameterID{ kAUParameterListener_AnyParameter };
        automatedParameterChanged( anyParameterID, 0 );
    }
}


template <>
bool AUHostProxy::canDo<AcceptIOChanges>() const
{
    /// \note See the "[VST2.4 -> AU] host and property notifications" thread on
    /// the CoreAudio ML.
    ///                                       (19.03.2013.) (Domagoj Saric)
    BOOST_ASSERT_MSG( findListenersFor( kAudioUnitProperty_StreamFormat ) == nullptr, "Unexpected: host listening to stream format changes." );
    return false;
    //for ( auto const & listener : propertyListeners_ )
    //{
    //    if ( listener.propertyID == kAudioUnitProperty_StreamFormat )
    //        return true;
    //}
    //return false;
}

template <>
bool AUHostProxy::canDo<SendTimeInfo>() const
{
    return callbacks().hostUserData != nullptr;
}


namespace Detail
{
    OSStatus makeErrorCode( AUPluginBase::ErrorCode const errorCode ) { return static_cast<OSStatus>( errorCode )   ; }
    OSStatus makeErrorCode( OSStatus                const errorCode ) { return errorCode                            ; }
    OSStatus makeErrorCode( bool                    const success   ) { return success ? noErr : kAudio_MemFullError; }

    AudioBuffers:: AudioBuffers() : pIOBuffers_( nullptr ) {}
    AudioBuffers::~AudioBuffers() { std::free( pIOBuffers_ ); }

    bool AudioBuffers::resize( std::uint8_t const numberOfChannels, std::uint16_t const numberOfSamples, bool const interleaved )
    {
        LE_ASSUME( numberOfChannels );

        std::uint8_t const numberOfBuffers          ( interleaved ? 1 : numberOfChannels );
        std::uint8_t const numberOfChannelsPerBuffer( interleaved ? numberOfChannels : 1 );

        /// \note Assume 'interleavedness' does not change once it is set.
        ///                                   (12.11.2015.) (Domagoj Saric)
        BOOST_ASSERT( !pIOBuffers_ || interleaved == this->interleaved() );
        /// \note Even though realloc() does not actually reallocate if we
        /// request the same/old/previous allocation size it still does a
        /// non-trivial amount of work (examined @ OSX10.11) so we try to avoid
        /// it.
        ///                                   (12.11.2015.) (Domagoj Saric)
        if
        (
            pIOBuffers_                                  &&
            numberOfChannels == this->numberOfChannels() &&
            numberOfSamples  == this->numberOfSamples ()
        )
            return true;

        auto const structureRequiredStorage ( sizeof( *pIOBuffers_ ) + ( ( numberOfBuffers - 1 ) * sizeof( *pIOBuffers_->mBuffers ) ) );
        auto const structureAlignmentPadding( 16 - ( structureRequiredStorage % 16 )                                                  );
        auto const perBufferRequiredStorage ( numberOfSamples * sizeof( float ) * numberOfChannelsPerBuffer                           );
        auto const perBufferAlignedStorage  ( Utility::align( perBufferRequiredStorage )                                              );
        auto const requiredStorage
        (
            structureRequiredStorage
                +
            ( structureAlignmentPadding + numberOfChannels * perBufferAlignedStorage )
        );

        BOOST_ASSERT( requiredStorage != 0                        );
        BOOST_ASSERT( numberOfSamples || !perBufferAlignedStorage );

        auto const pBufferList( static_cast<AudioBufferListPtr>( std::realloc( pIOBuffers_, requiredStorage ) ) );
        if ( BOOST_UNLIKELY( !pBufferList ) )
            return false;
        BOOST_ASSERT_MSG
        (
            ( pBufferList == pIOBuffers_ ) ||
            (
                ( pIOBuffers_      == nullptr                  ) ||
                ( numberOfChannels != this->numberOfChannels() ) ||
                ( numberOfSamples  != this->numberOfSamples () ) ||
                ( interleaved      != this->interleaved     () )
            ),
            "Unexpected reallocation"
        );

        pIOBuffers_                 = pBufferList    ;
        pIOBuffers_->mNumberBuffers = numberOfBuffers;

        auto              const buffers       ( boost::make_iterator_range_n( &pIOBuffers_->mBuffers[ 0 ], numberOfBuffers ) );
        char * __restrict       pBufferStorage( numberOfSamples ? reinterpret_cast<char *>( buffers.end() ) + structureAlignmentPadding : nullptr );

        for ( auto & __restrict buffer : buffers )
        {
            buffer.mNumberChannels = numberOfChannelsPerBuffer;
            buffer.mDataByteSize   = perBufferRequiredStorage ;
            buffer.mData           = pBufferStorage           ;
            LE_ASSUME( reinterpret_cast<std::size_t>( buffer.mData ) % 16 == 0 );
            pBufferStorage += perBufferAlignedStorage;
        }

        BOOST_ASSERT_MSG( ( pBufferStorage - reinterpret_cast<char const *>( pIOBuffers_ ) == requiredStorage ) || !numberOfSamples, "Buffer overrun." );

        return true;
    }

    bool AudioBuffers::interleaved() const
    {
        return bufferList().mBuffers[ 0 ].mNumberChannels != 1;
    }

    std::uint8_t LE_FASTCALL AudioBuffers::numberOfBuffers() const
    {
        return static_cast<std::uint8_t>( bufferList().mNumberBuffers );
    }

    std::uint8_t LE_FASTCALL AudioBuffers::numberOfChannels() const
    {
        return std::max<std::uint8_t>( numberOfBuffers(), static_cast<std::uint8_t>( bufferList().mBuffers[ 0 ].mNumberChannels ) );
    }

    std::uint16_t LE_FASTCALL AudioBuffers::numberOfSamples() const
    {
        auto const & buffer( bufferList().mBuffers[ 0 ] );
        return static_cast<std::uint16_t>( buffer.mDataByteSize / sizeof( float ) / buffer.mNumberChannels );
    }

    void AudioBuffers::alias( ::AudioBufferList const & __restrict source, ::AudioBufferList & __restrict target )
    {
        BOOST_ASSERT_MSG( source.mNumberBuffers == target.mNumberBuffers, "Mismatched buffers." );
        std::memcpy
        (
            &target,
            &source,
            reinterpret_cast<char const *>( &source.mBuffers[ source.mNumberBuffers ] ) - reinterpret_cast<char const *>( &source )
        );
    }

    void AudioBuffers::aliasFrom( ::AudioBufferList const & source )
    {
        BOOST_ASSERT_MSG( pIOBuffers_, "Uninitialised buffer." );
        alias( source, *pIOBuffers_ );
    }

    void AudioBuffers::aliasTo( ::AudioBufferList & target ) const
    {
        BOOST_ASSERT_MSG( pIOBuffers_, "Uninitialised buffer." );
        alias( *pIOBuffers_, target );
    }

    void AudioBuffers::copyTo( ::AudioBufferList & target ) const
    {
        BOOST_ASSERT_MSG( pIOBuffers_, "Uninitialised buffer." );
        copy( *pIOBuffers_, target );
    }

    void AudioBuffers::copy( ::AudioBufferList const & __restrict source, ::AudioBufferList & __restrict target )
    {
        BOOST_ASSERT_MSG( source.mNumberBuffers == target.mNumberBuffers, "Mismatched buffers." );

        ::AudioBuffer const * __restrict pSource     ( source.mBuffers );
        ::AudioBuffer       * __restrict pDestination( target.mBuffers );

        unsigned int const channelSize     ( pSource->mDataByteSize );
        unsigned int       numberOfChannels( source.mNumberBuffers  );
        while ( numberOfChannels-- )
        {
            BOOST_ASSERT( pSource        != pDestination        );
            BOOST_ASSERT( pSource->mData != pDestination->mData );
            BOOST_ASSERT( pSource     ->mDataByteSize == channelSize );
            BOOST_ASSERT( pDestination->mDataByteSize == channelSize );
            BOOST_ASSERT( reinterpret_cast<std::size_t>( pDestination->mData ) % 16 == 0 );
            BOOST_ASSERT( reinterpret_cast<std::size_t>( pSource     ->mData ) % 16 == 0 );
            std::memcpy
            (
                static_cast<void       *>( __builtin_assume_aligned( pDestination++->mData, 16 ) ),
                static_cast<void const *>( __builtin_assume_aligned( pSource     ++->mData, 16 ) ),
                channelSize
            );
        }
    }


    AUTimingInformation::AUTimingInformation( ::AudioTimeStamp const * const pTimeStamp )
        :
    #ifndef NDEBUG
        fields_( 0 ),
    #endif // NDEBUG
        pCurrentTimeStamp_( pTimeStamp )
    {
    #ifndef NDEBUG
        std::memset( this, 0, reinterpret_cast<char const *>( &this->fields_ ) - reinterpret_cast<char const *>( this ) );
    #endif // NDEBUG
    }
} // namespace Detail


AUPluginBase::AUPluginBase( ::AudioUnit const auInstance )
    :
    AUHostProxy( auInstance )
{
    //unsigned int const thisSize( sizeof( *this       ) );
    //unsigned int const baseSize( sizeof( AUHostProxy ) );
    //char * pZeroArea( reinterpret_cast<char *>( this ) );
    //pZeroArea += baseSize;
    //std::memset( pZeroArea, 0, thisSize - baseSize );

    //for( auto const & connection : inputConnections_ ) connection.setShouldAllocateBuffer( true );
    shouldAllocateOutputBuffer_ = true ;
    inPlaceProcessing_          = true ;
    initialised_                = false;

    lastRenderError_ = noErr;

    staticParameterListReported_ = false;
}


void AUPluginBase::sendRenderNotification
(
    ::AudioUnitRenderActionFlags         const extraFlags,
    ::AudioUnitRenderActionFlags       &       ioActionFlags,
    ::AudioTimeStamp             const &       timeStamp,
    ::UInt32                             const busNumber,
    ::UInt32                             const numberFrames,
    ::AudioBufferList                  &       ioData
) const
{
    BOOST_ASSERT_MSG
    (
        ( ioActionFlags & extraFlags ) == 0,
        "Invalid ioActionFlags."
    );

    ioActionFlags |= extraFlags;

    for ( auto const & delegate : renderNotificationCallbacks_ )
    {
        // https://developer.apple.com/library/mac/documentation/AudioUnit/Reference/AUComponentServicesReference/Reference/reference.html#//apple_ref/c/func/AudioUnitAddRenderNotify
        BOOST_VERIFY
        (
            delegate.inputProc
            (
                delegate.inputProcRefCon,
                &ioActionFlags,
                &timeStamp,
                busNumber,
                numberFrames,
                &ioData
            ) == noErr
        );
    }

    ioActionFlags &= ~extraFlags;
}


void AUPluginBase::setRenderResult( OSStatus const renderResult )
{
    if ( renderResult == lastRenderError_ )
        return;
    lastRenderError_ = renderResult;
    host().propertyChanged( kAudioUnitProperty_LastRenderError, kAudioUnitScope_Output, 0 );
}


AUPluginBase::RenderDelegate::Type AUPluginBase::RenderDelegate::type() const
{
    if ( !renderCallback_.inputProc )
        return None;
    if ( renderCallback_.inputProc == reinterpret_cast<::AURenderCallback>( &::AudioUnitRender ) )
        return Connection;
    return Callback;
}


OSStatus AUPluginBase::RenderDelegate::operator()
(
    ::AudioUnitRenderActionFlags       &       actionFlags,
    ::AudioTimeStamp             const &       timeStamp,
    ::UInt32                             const numberFrames,
    ::AudioBufferList                  &       buffers
) const
{
    // http://lists.apple.com/archives/coreaudio-api/2009/Oct/msg00124.html (AudioUnitRender returning kAudioUnitErr_InvalidPropertyValue)
    return renderCallback_.inputProc
    (
        renderCallback_.inputProcRefCon,
        &actionFlags,
        &timeStamp,
        bus_,
        numberFrames,
        &buffers
    );

#ifdef _DEBUG
    for ( ::AudioBuffer const * pBuffer( buffers.mBuffers ); pBuffer < &buffers.mBuffers[ buffers.mNumberBuffers ]; ++pBuffer )
    {
        BOOST_ASSERT_MSG( pBuffer->mData                                           , "Unexpected null IO buffer."   );
        BOOST_ASSERT_MSG( pBuffer->mDataByteSize == numberFrames * sizeof( float ) , "Incorrect IO buffer size,"    );
        BOOST_ASSERT_MSG( reinterpret_cast<std::size_t>( pBuffer->mData ) % 16 == 0, "Unexpected buffer alignment." );
    }
#endif // _DEBUG
}


AUPluginBase::RenderDelegate::operator bool() const { return renderCallback_.inputProc; }


void AUPluginBase::RenderDelegate::operator=( std::pair<::AURenderCallbackStruct const &, ::AudioUnitElement> const renderCallback )
{
    renderCallback_ = renderCallback.first ;
    bus_            = renderCallback.second;
}


void AUPluginBase::RenderDelegate::operator=( ::AudioUnitConnection const & connection )
{
    renderCallback_.inputProc       = reinterpret_cast<::AURenderCallback>( &::AudioUnitRender );
    renderCallback_.inputProcRefCon = connection.sourceAudioUnit                                ;
    bus_                            = connection.sourceOutputNumber                             ;
}


OSStatus AUPluginBase::InputConnection::operator()
(
    ::AudioUnitRenderActionFlags       & LE_RESTRICT       ioActionFlags,
    ::AudioTimeStamp             const * LE_RESTRICT const pTimeStamp,
    ::UInt32                                         const numberFrames,
    ::AudioBufferList                  * LE_RESTRICT const pExternalBuffers,
    ::UInt32                                         const numberOfChannels
) const
{
    BOOST_ASSERT_MSG( !pExternalBuffers || pExternalBuffers->mNumberBuffers == numberOfChannels, "Invalid inplace buffers." );

    bool const allocateBufferStructureOnly( pExternalBuffers || !mustAllocateBuffer() );

    if ( BOOST_UNLIKELY( !buffers_.resize( numberOfChannels, allocateBufferStructureOnly ? 0 : numberFrames, false ) ) )
        return kAudio_MemFullError;
    if ( pExternalBuffers )
        buffers_.aliasFrom( *pExternalBuffers );

    OSStatus const result( RenderDelegate::operator()( ioActionFlags, *pTimeStamp, numberFrames, buffers_ ) );
    return result;
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
