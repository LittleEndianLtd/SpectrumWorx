////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin.hpp
/// ----------------
///
///   Provides the Audio Unit backend for the LE Plugin framework.
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// Audio Unit Programming Guide(s):
//  - https://developer.apple.com/library/mac/#documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/Introduction/Introduction.html
//  - https://developer.apple.com/library/mac/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/AudioUnitProgrammingGuide.pdf
//  - http://developer.apple.com/library/mac/#documentation/MusicAudio/Conceptual/CoreAudioOverview/Introduction/Introduction.html
//  - http://www.ele.uri.edu/courses/ele470/AudioUnitHostingGuideForiOS.pdf
//  - http://developer.apple.com/library/ios/#documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/Introduction/Introduction.html
//  - http://www.slideshare.net/invalidname/core-audioios6portland
//  - "Optimizing Audio Unit User Experience in Logic Studio" http://developer.apple.com/library/mac/#technotes/tn2207/_index.html
// CoreAudio SDK download location:
//  - https://developer.apple.com/library/mac/#samplecode/CoreAudioUtilityClasses/Introduction/Intro.html
//  - https://developer.apple.com/downloads/index.action
//  - http://developer.apple.com/library/mac/#samplecode/PlaySequence/Introduction/Intro.html
//  - http://www.rawmaterialsoftware.com/viewtopic.php?f=8&t=9546
//  - http://www.mojolama.com/restore-apples-audio-unit-templates
// Tutorials/examples:
//  - http://developer.apple.com/library/mac/#documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/Tutorial-BuildingASimpleEffectUnitWithAGenericView/Tutorial-BuildingASimpleEffectUnitWithAGenericView.html#//apple_ref/doc/uid/TP40003278-CH5-SW4
//  - http://developer.apple.com/library/mac/#samplecode/CocoaAUHost
//  - http://developer.apple.com/library/mac/#samplecode/TremoloUnit/Introduction/Intro.html
//  - http://developer.apple.com/library/mac/#samplecode/SampleAUs/Introduction/Intro.html
//  - http://developer.apple.com/library/mac/#samplecode/FilterDemo/Introduction/Intro.html
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin_hpp__9709D1EE_B393_4DEF_914A_F1741E4FE09D
#define plugin_hpp__9709D1EE_B393_4DEF_914A_F1741E4FE09D
#pragma once
//------------------------------------------------------------------------------
#include "le/plugins/plugin.hpp"
#include "tag.hpp"

#include "le/parameters/enumerated/tag.hpp"
#include "le/parameters/powerOfTwo/tag.hpp"
#include "le/parameters/runtimeInformation.hpp"
#include "le/utility/buffers.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/objcfwdhelpers.hpp"
#include "le/utility/rvalueReferences.hpp"
#include "le/utility/trace.hpp"

#include "boost/integer/static_log2.hpp"
#include "boost/mpl/string.hpp"

#include "AvailabilityMacros.h"
#import  "AudioUnit/AudioUnit.h"
#import  "AudioToolbox/AudioToolbox.h" // #include "AudioToolbox/AudioUnitUtilities.h"
#include "CoreServices/../Frameworks/CarbonCore.framework/Headers/Components.h"

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace Parameters { template <class Parameter> struct DisplayValueTransformer; }

namespace Plugins
{
//------------------------------------------------------------------------------

#if MAC_OS_X_VERSION_MAX_ALLOWED <= MAC_OS_X_VERSION_10_6
    // http://developer.apple.com/library/mac/#releasenotes/General/MacOSXLionAPIDiffs/CoreAudio.html
    // http://www.welovemacs.com/whdoalltherc.html
    // MacErrors.h
    ::OSStatus const kAudio_MemFullError      ( ::memFullErr     );
    ::OSStatus const kAudio_FileNotFoundError ( ::wfFileNotFound );
    ::OSStatus const kAudio_ParamError        ( ::paramErr       );
    ::OSStatus const kAudio_UnimplementedError( ::unimpErr       );
#endif // MAC_OS_X_VERSION_10_6


#if __LP64__
    #define LE_OSX_INT_FORMAT( format ) "%" #format
#else
    #define LE_OSX_INT_FORMAT( format ) "%l" #format
#endif // __LP64__


// Forward declaration for the Detail::AEffectWrapper class.
class AUHostProxy;

////////////////////////////////////////////////////////////////////////////////
//
// Private implementation details for this module.
// -----------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    template <class Impl, AudioUnitPropertyID> struct PropertyHandler       ;
    template <            AudioUnitPropertyID> struct PropertyChangeDetector;
    template <            AudioUnitPropertyID> struct PropertyValueCount    ;

    ::AudioUnitPropertyID const auViewPropertyID = 'LEVW';

    ::CFURLRef    auBundlePath               ();
    ::CFStringRef auCocoaViewFactoryClassName();

    class AudioBuffers
    {
    public:
         AudioBuffers();
        ~AudioBuffers();

        AudioBuffers   ( AudioBuffers const & ) = delete;
        void operator =( AudioBuffers const & ) = delete;

        bool LE_FASTCALL resize( std::uint8_t numberOfChannels, std::uint16_t numberOfSamples, bool interleaved );

        bool          LE_FASTCALL interleaved     () const;
        std::uint8_t  LE_FASTCALL numberOfBuffers () const;
        std::uint8_t  LE_FASTCALL numberOfChannels() const;
        std::uint16_t LE_FASTCALL numberOfSamples () const;

        static void LE_FASTCALL alias( ::AudioBufferList const &, ::AudioBufferList & );
        static void LE_FASTCALL copy ( ::AudioBufferList const &, ::AudioBufferList & );

        void LE_FASTCALL aliasTo  ( ::AudioBufferList       & ) const;
        void LE_FASTCALL aliasFrom( ::AudioBufferList const & )      ;
        void LE_FASTCALL copyTo   ( ::AudioBufferList       & ) const;

        ::AudioBufferList       & bufferList()       LE_NOTHROW LE_PURE_FUNCTION { return *static_cast<::AudioBufferList *>( __builtin_assume_aligned( pIOBuffers_, 16 ) ); }
        ::AudioBufferList const & bufferList() const LE_NOTHROW LE_PURE_FUNCTION { return const_cast<AudioBuffers &>( *this ).bufferList(); }

                 ::AudioBufferList * __restrict  operator -> () const LE_NOTHROW LE_PURE_FUNCTION { return  pIOBuffers_; }
        operator ::AudioBufferList * __restrict              () const LE_NOTHROW LE_PURE_FUNCTION { return  pIOBuffers_; }
        operator ::AudioBufferList & __restrict              () const LE_NOTHROW LE_PURE_FUNCTION { return *pIOBuffers_; }

        explicit operator bool() const { return pIOBuffers_; }

    private:
        using AudioBufferListPtr /*alignas( 16 )*/ = ::AudioBufferList * __restrict;
        AudioBufferListPtr pIOBuffers_;
    }; // class AudioBuffers

    struct MemoryComparator
    {
        void const * const pMemory;
        unsigned int const size   ;
        template <typename T>
        bool operator()( T const & other ) const
        {
            LE_ASSUME( size <= sizeof( other ) );
            return std::memcmp( &other, pMemory, size ) == 0;
        }
    }; // struct MemoryComparator

    template <typename T, unsigned int N>
    class VariableLengthArray : public std::array<T, N>
    {
    public:
        VariableLengthArray() : pEnd_( this->begin() ) {}

        T       * end()       { return pEnd_; }
        T const * end() const { return pEnd_; }

        T       & back()       { return *( end() - 1 ); }
        T const & back() const { return *( end() - 1 ); }

        unsigned int size() const { return end() - this->begin(); }

        bool push_back( T const & element )
        {
            if ( pEnd_ != std::array<T, N>::end() )
            {
                *pEnd_++ = element;
                return true;
            }
            return false;
        }

        bool emplace_back( T && element )
        {
            if ( pEnd_ != std::array<T, N>::end() )
            {
                *pEnd_++ = std::forward<T>( element );
                return true;
            }
            return false;
        }

        bool remove
        (
            T const & element
        #if !__LP64__ // for remove property listener w/o user data
            ,unsigned int const trailBytesToSkip = 0
        #endif // __LP64__
        )
        {
        #if __LP64__
            unsigned int const trailBytesToSkip( 0 );
        #endif // __LP64__
            //T const * const pNewEnd( std::remove( begin(), end(), element ) );
            //BOOST_ASSERT( pNewEnd == pEnd_ - 1 );
            //pEnd_ = pNewEnd;
            T * const pElement( std::find_if( this->begin(), end(), [=,&element]( T const & other ){ return std::memcmp( &other, &element, sizeof( element ) - trailBytesToSkip ) == 0; } ) );
            if ( pElement == end() )
                return false;
            std::memmove( pElement, pElement + 1, ( --pEnd_ - pElement ) * sizeof( *pElement ) );
            return true;
        }

    private:
        T * pEnd_;
    }; // class VariableLengthArray


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class AUTimingInformation
    ///
    ////////////////////////////////////////////////////////////////////////////

    class AUTimingInformation
    {
    public:
        enum FieldFlags
        {
            SampleTime      = kAudioTimeStampSampleTimeValid    ,
            SampleRateScale = kAudioTimeStampRateScalarValid    ,
            SystemTime      = kAudioTimeStampWordClockTimeValid ,
            NumberOfBeats   = kAudioTimeStampHostTimeValid      , // http://developer.apple.com/library/ios/#qa/qa1643/_index.html, http://www.macresearch.org/tutorial_performance_and_time
            SMTPE           = kAudioTimeStampSMPTETimeValid     ,
            BPM             = kAudioTimeStampSMPTETimeValid << 1,
            LastBarPosition = kAudioTimeStampSMPTETimeValid << 2,
            CyclePosition   = kAudioTimeStampSMPTETimeValid << 3,
            TimeSignature   = kAudioTimeStampSMPTETimeValid << 4,
            MIDIClock       = kAudioTimeStampSMPTETimeValid << 5,
            SampleRate      = kAudioTimeStampSMPTETimeValid << 6,
            TransportState  = kAudioTimeStampSMPTETimeValid << 7
        };

    public:
        double const & position                () const { return currentSampleInTimeLine_          ; }
        double         sampleRate              () const { BOOST_ASSERT( false ); return 0          ; }
        double const & sampleRateScale         () const { return pCurrentTimeStamp_->mRateScalar   ; }
        UInt64 const & systemTime              () const { return pCurrentTimeStamp_->mWordClockTime; }
        double const & numberOfBeats           () const { return currentBeat_                      ; }
        double const & bpm                     () const { return currentTempo_                     ; }
        double const & lastBarStartPosition    () const { return currentMeasureDownBeat_           ; }
        double const & cycleStartPosition      () const { return cycleStartBeat_                   ; }
        double const & cycleEndPosition        () const { return cycleEndBeat_                     ; }
        float  const & timeSignatureNumerator  () const { return timeSigNumerator_                 ; }
        unsigned int   timeSignatureDenominator() const { return timeSigDenominator_               ; }
        unsigned int   smpteOffset             () const { return smpteTime().mSubframes            ; }
        unsigned int   smpteFrameRate          () const { return smpteTime().mType                 ; }
    //           int   samplesToNextClock      () const { return ; }

        ::SMPTETime const & smpteTime() const { return pCurrentTimeStamp_->mSMPTETime; }

        template <FieldFlags desiredField>
        bool hasField() const { return fields_ & desiredField; }

        template <unsigned int desiredFields>
        bool hasFields() const { return ( fields_ & desiredFields ) == desiredFields; }

    private: friend class Plugins::AUHostProxy;
        template <unsigned int desiredDataFlags>
        static AUTimingInformation create( ::HostCallbackInfo const &, ::AudioTimeStamp const * );

        AUTimingInformation( ::AudioTimeStamp const * );

        /// \note Cannot be defined within the create() member function.
        /// http://llvm-reviews.chandlerc.com/D1866
        ///                                   (18.03.2014.) (Domagoj Saric)
        template <unsigned int filterMask>
        struct ConditionalData
        {
            ConditionalData() : flags( 0 ) {}

            template <unsigned int fields>
            static bool wants() { return filterMask & fields; }

            template <FieldFlags field, typename T>
            static T * get( T & data ) { return ( filterMask & field ) ? &data : nullptr; }

            template <unsigned int fields>
            void setFlags( OSStatus const result )
            {
                if ( result == noErr ) { flags |= fields & filterMask; }
                else                   { BOOST_ASSERT_MSG( result == kAudioUnitErr_CannotDoInCurrentContext, "Unexpected result." ); }
            }

            unsigned int flags;
        }; // struct ConditionalData

    private:
        ::Float64 currentBeat_                ;
        ::Float64 currentTempo_               ;
        ::UInt32  deltaSampleOffsetToNextBeat_;
        ::Float32 timeSigNumerator_           ;
        ::UInt32  timeSigDenominator_         ;
        ::Float64 currentMeasureDownBeat_     ;
        ::Boolean isPlaying_                  ;
        ::Boolean transportStateChanged_      ;
        ::Float64 currentSampleInTimeLine_    ;
        ::Boolean isCycling_                  ;
        ::Float64 cycleStartBeat_             ;
        ::Float64 cycleEndBeat_               ;

        unsigned int fields_;

        ::AudioTimeStamp const * LE_RESTRICT const pCurrentTimeStamp_;
    }; // class AUPluginBase::TimingInformation


    template <unsigned int filterMask>
    AUTimingInformation AUTimingInformation::create
    (
        ::HostCallbackInfo const & LE_RESTRICT       callbacks,
        ::AudioTimeStamp   const * LE_RESTRICT const pCurrentTimeStamp
    )
    {
        /// Timing information:
        /// AudioTimeStamp: https://developer.apple.com/library/mac/documentation/musicaudio/reference/CoreAudioDataTypesRef/Reference/reference.html#//apple_ref/doc/uid/TP40004488-CH3-SW14
        /// http://lists.apple.com/archives/Coreaudio-api/2003/Apr/msg00099.html (clear description of AudioTimeStamp)
        /// http://lists.apple.com/archives/coreaudio-api/2004/Dec/msg00010.html (Converting AU transport info to VST time info)
        /// http://lists.apple.com/archives/coreaudio-api/2008/Dec/msg00129.html (Recognize signature change with CallHostMusicalTimeLocation)
        /// http://lists.apple.com/archives/coreaudio-api/2004/Aug/msg00006.html (CallHostTransportState)
        /// http://lists.apple.com/archives/coreaudio-api/2009/Mar/msg00167.html (Synchronize Audio Queue start times in iPhone)

        AUTimingInformation info( pCurrentTimeStamp );

        ConditionalData<filterMask> data;
	    if ( data. template wants<NumberOfBeats | BPM>() && callbacks.beatAndTempoProc )
        {
		    ::OSStatus const result
            (
                (*callbacks.beatAndTempoProc)
                (
                    callbacks.hostUserData,
                    data. template get<NumberOfBeats>( info.currentBeat_  ),
                    data. template get<BPM          >( info.currentTempo_ )
                )
            );
            LE_TRACE_IF( result != noErr, "\tSW AU: beatAndTempo host callback failed (" LE_OSX_INT_FORMAT( d ) ")", result );
            data. template setFlags<NumberOfBeats | BPM>( result );
            if ( info.currentBeat_ < 0 )
            {
                LE_TRACE( "\tSW AU: host returned negative position in beats (%f)", info.currentBeat_ );
                info.currentBeat_ = 0;
            }
            if ( data. template wants<BPM>() && info.currentTempo_ == 0 ) //...mrmlj...Ableton Live 8.4
            {
                data.flags &= ~BPM;
            }
	    }
	    if ( data. template wants<LastBarPosition | TimeSignature>() && callbacks.musicalTimeLocationProc )
        {
		    ::OSStatus const result
            (
                (*callbacks.musicalTimeLocationProc)
                (
                    callbacks.hostUserData,
                    nullptr,//data. template get</**/>( info.deltaSampleOffsetToNextBeat_ ),
                    data. template get<TimeSignature  >( info.timeSigNumerator_       ),
                    data. template get<TimeSignature  >( info.timeSigDenominator_     ),
                    // http://lists.apple.com/archives/coreaudio-api/2004/Mar/msg00207.html
                    data. template get<LastBarPosition>( info.currentMeasureDownBeat_ )
                )
            );
            LE_TRACE_IF( result != noErr, "\tSW AU: musicalTimeLocation host callback failed (" LE_OSX_INT_FORMAT( d ) ")", result );
            data. template setFlags<LastBarPosition | TimeSignature>( result );
	    }
	    if ( data. template wants<SampleTime | TransportState | CyclePosition>() && callbacks.transportStateProc )
        {
		    ::OSStatus const result
            (
                (*callbacks.transportStateProc)
                (
                    callbacks.hostUserData,
                    data. template get<TransportState>( info.isPlaying_               ),
                    data. template get<TransportState>( info.transportStateChanged_   ),
                    data. template get<SampleTime    >( info.currentSampleInTimeLine_ ),
                    data. template get<TransportState>( info.isCycling_               ),
                    data. template get<CyclePosition >( info.cycleStartBeat_          ),
                    data. template get<CyclePosition >( info.cycleEndBeat_            )
                )
            );
            LE_TRACE_IF( result != noErr, "\tSW AU: transportState host callback failed (" LE_OSX_INT_FORMAT( d ) ")", result );
            data. template setFlags<SampleTime | TransportState | CyclePosition>( result );
		    if
            (
                data. template wants<SampleTime>() &&
                !( data.flags & SampleTime ) &&
                pCurrentTimeStamp
            )
            {
                info.currentSampleInTimeLine_ = pCurrentTimeStamp->mSampleTime;
                data.flags |= SampleTime;
            }
	    }

        info.fields_ = data.flags;

        return info;
    }
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
///
/// \class AUHostProxy
///
/// \brief Implements the host proxy class for the AU platform.
///
////////////////////////////////////////////////////////////////////////////////

class AUHostProxy
{
public:
    AUHostProxy( ::AudioUnit );

public: // Host capabilities.
    static bool const threadSafeEventSystem          = true ;
    static bool const pluginCanChangeIOConfiguration = false;

    template <HostCapability capability>
    bool canDo() const { return false; }

public: // Host information/state.
    template <unsigned int filterMask>
    Detail::AUTimingInformation getTimeInfo() const { return Detail::AUTimingInformation::create<filterMask>( callbacks(), pCurrentTimeStamp_ ); }

public: // Plugin information/state reported/visible to host.
    bool reportNewNumberOfInputChannels ( std::uint8_t inputs                                                ) const;
    bool reportNewNumberOfOutputChannels( std::uint8_t outputs                                               ) const;
    bool reportNewNumberOfSideChannels  ( std::uint8_t sideInputs                                            ) const;
    bool reportNewNumberOfIOChannels    ( std::uint8_t inputs, std::uint8_t sideInputs, std::uint8_t outputs ) const;

    bool reportNewLatencyInSamples( std::uint16_t samples ) const;
    bool reportNewLatencyInSeconds( float         seconds ) const;

public: // Parameter automation.
    bool LE_NOTHROW LE_FASTCALL       propertyChanged( ::AudioUnitPropertyID, ::AudioUnitScope, ::AudioUnitElement ) const;
    bool LE_NOTHROW LE_FASTCALL globalPropertyChanged( ::AudioUnitPropertyID                                       ) const;

    //http://www.rawmaterialsoftware.com/viewtopic.php?f=8&t=2705
    void LE_NOTHROW LE_FASTCALL automatedParameterChanged  ( ParameterID, float newValue ) const;
    void LE_NOTHROW LE_FASTCALL automatedParameterBeginEdit( ParameterID                 ) const;
    void LE_NOTHROW LE_FASTCALL automatedParameterEndEdit  ( ParameterID                 ) const;

    bool LE_NOTHROW LE_FASTCALL parameterListChanged() const;

    bool canTryDynamicParameterList() const { return canTryDynamicParameterList_; }

    static bool wantsManualDependentParameterNotifications() { return false; }

    static void gestureBegin( char const * /*description*/ ) {}
    static void gestureEnd  (                              ) {}

public: // GUI.
    void presetChangeBegin       () const {}
    void presetChangeEnd         () const;
    bool currentPresetNameChanged() const;

    bool sizeWindow( unsigned int width, unsigned int height ) const;

public: // Offline functionality.

public:
    OSStatus addPropertyListener   ( ::AudioUnitPropertyID, ::AudioUnitPropertyListenerProc, void *       pUserData );
    OSStatus removePropertyListener( ::AudioUnitPropertyID, ::AudioUnitPropertyListenerProc, void const * pUserData );

protected:
    template <class Impl, AudioUnitPropertyID> friend struct Detail::PropertyHandler;
    template <class Impl, class Protocol>      friend class  Plugin;

    ::AudioUnit        const & auInstance() const { return auInstance_; }
    ::HostCallbackInfo       & callbacks ()       { return callbacks_ ; }
    ::HostCallbackInfo const & callbacks () const { return callbacks_ ; }

    void setCurrentTimeStamp( ::AudioTimeStamp const * LE_RESTRICT pCurrentTimeStamp ) { pCurrentTimeStamp_ = pCurrentTimeStamp; }

private:
    void fireParameterEvent( ::AudioUnitEventType, ::AudioUnitParameterID ) const;
    void firePropertyEvent (                       ::AudioUnitPropertyID  ) const;
    void fireEvent         ( ::AudioUnitEventType, ::UInt32 itemID        ) const;

    struct PropertyChangeListener;
    using PropertyListeners = Utility::AlignedHeapBuffer<PropertyChangeListener>;

    PropertyListeners       * LE_RESTRICT findListenersFor( ::AudioUnitPropertyID )      ;
    PropertyListeners const * LE_RESTRICT findListenersFor( ::AudioUnitPropertyID ) const;

private:
    ::AudioUnit const auInstance_;

    /// \note Logic 9 registers a separate kAudioUnitProperty_ParameterInfo
    /// listener for every parameter so we use a separate dynamically allocated
    /// array of listeners for each property in a fixed capacity flat-map like
    /// container (PropertyListenersMap) in order to most efficiently handle a
    /// large number of possible listeners while taking into account the much
    /// smaller number of properties that might actually be listened to.
    ///                                       (21.03.2013.) (Domagoj Saric)
    struct PropertyChangeListener
    {
        ::AudioUnitPropertyListenerProc   callback ;
        void                            * pUserData;
    }; // struct PropertyChangeListener

    struct BoundPropertyListeners
    {
        ::AudioUnitPropertyID propertyID;
        PropertyListeners     listeners ;
    }; // struct BoundPropertyListeners
    using PropertyListenersMap = Detail::VariableLengthArray<BoundPropertyListeners, 12>;

    PropertyListenersMap propertyListeners_;

    ::HostCallbackInfo callbacks_;

    ::AudioTimeStamp const * LE_RESTRICT pCurrentTimeStamp_;

    std::uint8_t inputChannels_ ;
    std::uint8_t outputChannels_;
    std::uint8_t sideChannels_  ;

    mutable bool canTryDynamicParameterList_;
}; // class AUHostProxy

template <> bool AUHostProxy::canDo<AcceptIOChanges>() const;
template <> bool AUHostProxy::canDo<SendTimeInfo   >() const;


////////////////////////////////////////////////////////////////////////////////
///
/// \class AUPluginBase
///
/// \brief Base class for the AU plugin classes. Contains members
/// independent of a particular concrete plugin implementation class.
///
////////////////////////////////////////////////////////////////////////////////

template <class Protocol> class  ParameterInformation ;
template <class Protocol> struct ErrorCode            ;
template <class Protocol> struct AutomatedParameterFor;

template <>
struct ErrorCode<Protocol::AU>
{
    enum value_type
    {
        Success                  = noErr                                 ,
        OutOfMemory              = kAudio_MemFullError                   ,
        OutOfRange               = kAudio_ParamError                     ,
        CannotDoInCurrentContext = kAudioUnitErr_CannotDoInCurrentContext,
        Unauthorized             = kAudioUnitErr_Unauthorized            ,
        Unsupported              = kAudio_UnimplementedError             ,
        FileNotFound             = kAudio_FileNotFoundError
    };
}; // struct ErrorCode<Protocol::AU>


template <> struct AutomatedParameterFor<Protocol::AU> { typedef FullRangeAutomatedParameter type; };

class AUPluginBase
    :
    /*private...mrmlj...*/ public AUHostProxy
{
public: // Type definitions.
    using ConstructionParameter = ::AudioUnit                  ;
    using AutomatedParameter    =   FullRangeAutomatedParameter;
    using MIDIEvent             =   void                       ;
    using ParameterSelector     =   ParameterID                ;

    using HostProxy = AUHostProxy;

    class Editor;

    using TimingInformation = Detail::AUTimingInformation;

    using Protocol             = Protocol::AU;
    using ErrorCode            = ErrorCode           <Protocol>::value_type;
    using ParameterInformation = ParameterInformation<Protocol>;

public: // AU string buffer definitions.
    //...mrmlj...
    typedef char ProgramNameBuf    [ 16 ];
    typedef char ParameterStringBuf[ 52 ];

public:
    AUHostProxy       & host()       { return *this; }
    AUHostProxy const & host() const { return *this; }

protected:
     AUPluginBase( ::AudioUnit );
    ~AUPluginBase() = default;

    void LE_FASTCALL sendRenderNotification( ::AudioUnitRenderActionFlags extraFlags, ::AudioUnitRenderActionFlags &, ::AudioTimeStamp const &, ::UInt32 busNumber, ::UInt32 numberFrames, ::AudioBufferList & ) const;

    void LE_FASTCALL setRenderResult( OSStatus renderResult );

protected:
    template <class Impl, AudioUnitPropertyID> friend struct Detail::PropertyHandler       ;
    template <            AudioUnitPropertyID> friend struct Detail::PropertyChangeDetector;
    template <            AudioUnitPropertyID> friend struct Detail::PropertyValueCount    ;

    class RenderDelegate
    {
    public:
        RenderDelegate() { renderCallback_.inputProc = nullptr; }

        enum Type { None, Connection, Callback };

        Type type() const;

        OSStatus operator() ( ::AudioUnitRenderActionFlags &, ::AudioTimeStamp const &, ::UInt32 numberFrames, ::AudioBufferList & ) const;

        void operator=( std::pair<::AURenderCallbackStruct const &, ::AudioUnitElement>         renderCallback );
        void operator=( ::AudioUnitConnection                                           const & connection     );

        explicit operator bool() const;

    private:
        ::AURenderCallbackStruct renderCallback_;
        ::AudioUnitElement       bus_           ;
    }; // class RenderDelegate

    class InputConnection : public RenderDelegate
    {
    public:
        InputConnection() : shouldAllocateBuffer_( true ) {}

        OSStatus LE_FASTCALL operator() ( ::AudioUnitRenderActionFlags &, ::AudioTimeStamp const *, ::UInt32 numberFrames, ::AudioBufferList *, ::UInt32 numberOfChannels ) const;

        LE_ALIGN( 16 ) ::AudioBufferList       & LE_RESTRICT buffers()       { return *static_cast<::AudioBufferList *>( buffers_ ); }
        LE_ALIGN( 16 ) ::AudioBufferList const & LE_RESTRICT buffers() const { return const_cast<InputConnection &>( *this ).buffers(); }

        void setShouldAllocateBuffer( bool const value )       { shouldAllocateBuffer_ = value; }
        bool getShouldAllocateBuffer(                  ) const { return shouldAllocateBuffer_ ; }

    private:
        bool mustAllocateBuffer() const { return ( type() != Connection ) && shouldAllocateBuffer_; }

    private:
        mutable Detail::AudioBuffers buffers_             ;
                bool                 shouldAllocateBuffer_;
    }; // class InputConnection
    std::array<InputConnection, 2> inputConnections_;

protected:
    bool inPlaceProcessing_;
    bool initialised_      ;

    Detail::AudioBuffers outputBuffers_             ;
    bool                 shouldAllocateOutputBuffer_;

    Detail::VariableLengthArray<::AURenderCallbackStruct, 2> renderNotificationCallbacks_;

    ::OSStatus lastRenderError_;
    friend class AUHostProxy; //...mrmlj...
    mutable bool staticParameterListReported_;
}; // class AUPluginBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class AUPluginBase::Editor
///
/// \brief Provides the bare minimum for an AU plugin editor.
///
////////////////////////////////////////////////////////////////////////////////

class AUPluginBase::Editor
{
public: // Plugin::Editor type definition requirements.

    using WindowHandle = ObjC::NSView *;
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class AUPluginBase::ParameterInformation
///
////////////////////////////////////////////////////////////////////////////////
// http://ygrabit.steinberg.de/~ygrabit/public_html/vstsdk/OnlineDoc/vstsdk2.3/html/plug/2.0/aeffectx.html#VstParameterProperties
// http://www.kvraudio.com/forum/viewtopic.php?t=273783&postdays=0&postorder=asc&start=0
// http://lists.apple.com/archives/coreaudio-api/2003/Jul/msg00304.html
// http://www.cockos.com/reaper/sdk/vst/vst_ext.php
// http://forum.cockos.com/showthread.php?t=42246
// http://www.oifii.org/ns-org/nsd/ar/cp/vst/mda-vst_source/mda-vst/vst/vstsdk2.4/doc/html/vstparamstruct.html
////////////////////////////////////////////////////////////////////////////////

template <class Protocol> class ParameterInformation;

template <>
class ParameterInformation<Protocol::AU> : public ::AudioUnitParameterInfo
{
public:
    using result_type = void;

    template <class Parameter>
    result_type operator()() LE_COLD
    {
        using Label = typename Parameters::DisplayValueTransformer<Parameter>::Suffix;
        setLabel( boost::mpl::c_str<Label>::value, boost::mpl::size<Label>::value );

        using Tag        = typename Parameter::Tag       ;
        using value_type = typename Parameter::value_type;
        //...mrmlj...
        setValues<Parameter>( std::is_same<Tag, Parameters::PowerOfTwoParameterTag>() );

        if
        (
            !std::is_same<value_type, bool                              >::value &&
            !std::is_same<Tag       , Parameters::PowerOfTwoParameterTag>::value &&
            !std::is_same<Tag       , Parameters::EnumeratedParameterTag>::value
        )
        {
            flags |= kAudioUnitParameterFlag_CanRamp;
            if
            (
                !std::is_integral<value_type>::value ||
                ( ( Parameter::maximum() - Parameter::minimum() ) > 128 )
            )
                flags |= kAudioUnitParameterFlag_IsHighResolution;
            unit = kAudioUnitParameterUnit_Generic;
        }
        else
        {
            unit = std::is_same<value_type, bool>::value ? kAudioUnitParameterUnit_Boolean : kAudioUnitParameterUnit_Indexed;
        }

        if
        ( //...mrmlj...for N/A parameters/'dynamic' interface...
            ( Parameter::unscaledMinimum != Parameter::unscaledMaximum ) ||
            ( Parameter::unscaledMinimum != 0                          ) //...mrmlj...LFO::PeriodScale...
        )
            flags |= kAudioUnitParameterFlag_IsReadable | kAudioUnitParameterFlag_IsWritable;

        //...meta...
        // http://www.rawmaterialsoftware.com/viewtopic.php?t=&f=&p=24084
        // http://forum.cockos.com/showthread.php?t=65639

        // http://www.mailinglistarchive.com/html/coreaudio-api@lists.apple.com/2007-01/msg00172.html
    }

    void clear() { flags = 0; }

    AUPluginBase::ParameterStringBuf * nameBuffer() { return &name; }
    void                               nameSet   ()
    {
        cfNameString = ::CFStringCreateWithCString( nullptr, name, kCFStringEncodingASCII );
        flags |= kAudioUnitParameterFlag_HasCFNameString | kAudioUnitParameterFlag_CFNameRelease;
    }


    void set( AutomatedParameter::Info const & info ) LE_COLD
    {
        LE_ASSUME( flags == 0 );

        minValue     = info.minimum ;
        maxValue     = info.maximum ;
        defaultValue = info.default_;

        switch ( info.type )
        {
            case AutomatedParameter::Info::Boolean:
            case AutomatedParameter::Info::Trigger:
                unit = kAudioUnitParameterUnit_Boolean;
                break;
            case AutomatedParameter::Info::Integer:
                // http://www.juce.com/forum/topic/fix-quantization-automation-data-logic
                // http://lists.apple.com/archives/coreaudio-api/2007/Nov/msg00006.html
                if ( ( maxValue - minValue ) > 128 )
                    flags |= kAudioUnitParameterFlag_IsHighResolution;
            case AutomatedParameter::Info::Enumerated:
                unit = kAudioUnitParameterUnit_Indexed;
            case AutomatedParameter::Info::FloatingPoint:
                flags |= kAudioUnitParameterFlag_CanRamp;
                break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }

        flags |= kAudioUnitParameterFlag_IsReadable | kAudioUnitParameterFlag_IsWritable;

        if ( info.unit[ 0 ] != '\0' )
        {
            ::CFStringRef const unitName
            (
                ::CFStringCreateWithCStringNoCopy
                (
                    nullptr, info.unit, kCFStringEncodingASCII, kCFAllocatorNull
                )
            );
            BOOST_ASSERT_MSG( unitName, "CFString creation failed." );
            ::AudioUnitParameterInfo::unitName = unitName;
            ::AudioUnitParameterInfo::unit     = kAudioUnitParameterUnit_CustomUnit;
            flags |= kAudioUnitParameterFlag_ValuesHaveStrings;
        }
        else
        {
            ::AudioUnitParameterInfo::unit = kAudioUnitParameterUnit_Generic;
        }
    }

    void markAsMeta()
    {
        flags |= kAudioUnitParameterFlag_IsElementMeta;
    }

private:
    ParameterInformation( ParameterInformation const & );

    template <class Parameter>
    void setValues( std::true_type /*is power-of-two*/ )
    {
        //...mrmlj...workaround for having no way to specify discrete/valid values...

        std::uint8_t const minimumExponent( boost::static_log2<Parameter::unscaledMinimum>::value );
        std::uint8_t const maximumExponent( boost::static_log2<Parameter::unscaledMaximum>::value );
        std::uint8_t const defaultExponent( boost::static_log2<Parameter::unscaledDefault>::value );
        minValue     = static_cast<::AudioUnitParameterValue>( 0                                 );
        maxValue     = static_cast<::AudioUnitParameterValue>( maximumExponent - minimumExponent );
        defaultValue = static_cast<::AudioUnitParameterValue>( defaultExponent - minimumExponent );
    }

    template <class Parameter>
    void setValues( std::false_type /*is linear*/ )
    {
        minValue     = static_cast<::AudioUnitParameterValue>( Parameter::minimum () );
        maxValue     = static_cast<::AudioUnitParameterValue>( Parameter::maximum () );
        defaultValue = static_cast<::AudioUnitParameterValue>( Parameter::default_() );
    }

    void setLabel( char const * LE_RESTRICT const unit, unsigned int const length )
    {
        if ( length )
        {
            ::CFStringRef const unitName
            (
                ::CFStringCreateWithBytesNoCopy
                (
                    nullptr,
                    reinterpret_cast<::UInt8 const *>( unit ),
                    length,
                    kCFStringEncodingASCII,
                    false,
                    kCFAllocatorNull
                )
            );
            BOOST_ASSERT_MSG( unitName, "CFString creation failed." );
            ::AudioUnitParameterInfo::unitName = unitName;
            if ( ::AudioUnitParameterInfo::unit == kAudioUnitParameterUnit_Generic )
                ::AudioUnitParameterInfo::unit = kAudioUnitParameterUnit_CustomUnit;
            flags |= kAudioUnitParameterFlag_ValuesHaveStrings;
        }
    }
}; // ParameterInformation<Protocol::AU>


namespace Detail
{
    OSStatus makeErrorCode( AUPluginBase::ErrorCode errorCode );
    OSStatus makeErrorCode( OSStatus                errorCode );
    OSStatus makeErrorCode( bool                    success   );
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin<AU>
///
/// \brief Provides the AU protocol implementation.
///
////////////////////////////////////////////////////////////////////////////////

template <class ImplParam>
class Plugin<ImplParam, Protocol::AU>
    :
    public AUPluginBase
{
protected:
    Plugin( ConstructionParameter const constructionParameter ) : AUPluginBase( constructionParameter )
    {
        auto & host( this->host() );
        host.inputChannels_  = Impl::maxNumberOfOutputs;
        host.outputChannels_ = Impl::maxNumberOfOutputs;
        host.sideChannels_   = Impl::maxNumberOfInputs - Impl::maxNumberOfOutputs;
    }
#ifndef NDEBUG
    //~Plugin();
#endif

protected:
    using Impl     = ImplParam;
  //using Protocol = Protocol::AU;

    using SupportsTimingInformation = std::true_type;

protected:
    ErrorCode initialise  () { return Plugins::ErrorCode<Protocol::AU>::Success; } ///< VSTSDK AudioEffect::open () equivalent.
    void      uninitialise() {} ///< VSTSDK AudioEffect::close() equivalent.

    void getProgramName( ProgramNameBuf & name ) const { name[ 0 ] = '\0'; }

    bool getProgramNameIndexed( unsigned int /*index*/, ProgramNameBuf & ) const { return false; }

    void getParameterLabel  ( unsigned int /*index*/, ParameterStringBuf & label ) const { label[ 0 ] = '\0'; }
    void getParameterDisplay( unsigned int /*index*/, ParameterStringBuf & text  ) const { text [ 0 ] = '\0'; }
    void getParameterName   ( unsigned int /*index*/, ParameterStringBuf & name  ) const { name [ 0 ] = '\0'; }

    bool getParameterProperties( unsigned int /*index*/, ParameterInformation & ) const { return false; }

    bool canParameterBeAutomated( unsigned int /*index*/, void const * /*pContext*/ ) const { return true; }

    std::size_t getChunk( void * * /*pData*/,                           bool /*isPreset*/ ) const { return 0; }
    std::size_t setChunk( void *   /*pData*/, std::size_t /*byteSize*/, bool /*isPreset*/ )       { return 0; }

    //bool getInputProperties ( unsigned int /*index*/, VstPinProperties & ) const { return false; }
    //bool getOutputProperties( unsigned int /*index*/, VstPinProperties & ) const { return false; }

    //bool setSpeakerArrangement( VstSpeakerArrangement const & /*input*/, VstSpeakerArrangement const & /*output*/ ) { return false; }
    //bool getSpeakerArrangement( VstSpeakerArrangement * * const input  , VstSpeakerArrangement * * const output ) const { *input = 0; *output = 0; return false; }

    bool setBypass ( bool /*onOff*/ ) { return false; }

    unsigned int getTailSize() { return 0; }

protected:
           Impl       & impl(          );
           Impl const & impl(          ) const { return const_cast<Plugin &>( *this ).impl(); }
    static Impl       & impl( ::Handle );

private:
    template <typename T, typename Pointer>
    static T & castToReference( Pointer const pointer )
    {
        T * LE_RESTRICT const pT( reinterpret_cast<T *>( pointer ) );
        LE_ASSUME( pT );
        return *pT;
    }

private: template <class Impl, AudioUnitPropertyID> friend struct Detail::PropertyHandler;

    OSStatus LE_FASTCALL checkSideChannelBusStateChange()
    {
        //...mrmlj...SW HARDCODE...
        auto & impl( this->impl() );
        if ( !impl.initialised_ )
            return noErr;
        return Detail::makeErrorCode( impl.enableSideChannelInput( impl.host().sideChannels_ && impl.inputConnections_[ 1 ] ) );
    }

private: // AudioPlugin interface (OS X Lion+)
    //friend void * ::lePluginAUFactory( ::AudioComponentDescription const * ); // entry function
    public:
    struct AudioComponentPlugInInstance;

private: // ComponentManager interface
    //friend ::ComponentResult ::lePluginAUEntry( ::ComponentParameters *, ::Handle ); // entry function
    public:
    static ::ComponentResult LE_NOTHROW componentManagerEntry( ::ComponentParameters *, ::Handle );
}; // class Plugin<Impl, Protocol::AU>

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "plugin.inl"

#endif // plugin_hpp
