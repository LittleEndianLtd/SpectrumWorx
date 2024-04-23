////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin.hpp
/// ----------------
///
///   Provides the VST 2.4 implementation for the LE Plugin framework. Contrary
/// to the original VST SDK design, where you only had the AudioEffect(X) class
/// with virtual methods for both calling back to the host and receiving calls
/// from the host, plugin and host functionality is now separated into different
/// classes (as also dictated by the LE Plugin framework design).
///
///   The AudioEffect(X) classes (i.e. their interface and functionality) were
/// basically separated into two classes VST24Plugin and VSTHost24Proxy. If not
/// otherwise noted their member functions are equivalent to their original VST
/// SDK counterparts so the original VST 2.4 SDK documentation should be
/// consulted for those functions.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
///
/// \todo Periodically check up on this developer's research and update our code
/// with any new knowledge:
/// http://vstnet.codeplex.com/Wiki/View.aspx?title=VST%20SDK%20Questions
/// http://github.com/ValdemarOrn/SharpSoundDevice.
///                                           (19.08.2009.) (Domagoj Saric)
/// \todo Investigate the "vstfxstore.h" header in the original VST 2.4 SDK and
/// the "Program (fxp) and Bank (fxb) structures" and functionality found in it.
///                                           (20.08.2009.) (Domagoj Saric)
///
/// "Introduction to VST Plug-in Implementation by means of a Schroeder’s
/// Reverberator Sound Effect"
/// http://citeseerx.ist.psu.edu/viewdoc/download;jsessionid=6C97FD696FB63EE0373A56E6043947B5?doi=10.1.1.110.3110&rep=rep1&type=pdf
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin_hpp__904E63F7_19A4_4B93_8972_8789EF0A343A
#define plugin_hpp__904E63F7_19A4_4B93_8972_8789EF0A343A
#pragma once
//------------------------------------------------------------------------------
#include "le/plugins/plugin.hpp"
#include "tag.hpp"

#include "le/parameters/enumerated/tag.hpp"
#include "le/parameters/powerOfTwo/tag.hpp"
#include "le/parameters/runtimeInformation.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/clear.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/objcfwdhelpers.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"

// Implementation note:
//   Currently, we bundle the "AEffectX.h" and "AEffect.h" headers from the
// original VST SDK 2.4 with our own source as the only two required files from
// the entire VST SDK so that individual users/developers need not install the
// VST SDK.
//   This note was put here/in this file so as not to modify the original VST
// SDK files.
//                                            (19.08.2009.) (Domagoj Saric)
#define VST_2_4_EXTENSIONS   1
#ifdef _DEBUG
    #define VST_FORCE_DEPRECATED 0
#else
    #define VST_FORCE_DEPRECATED 1
#endif
#if defined( _WIN32 ) && !defined( WIN32 )
    #define WIN32
#endif
#include "aeffectx.h"

#include "boost/mpl/bool.hpp"
#include "boost/mpl/string.hpp"

#include <bitset>
//------------------------------------------------------------------------------
#if defined( __APPLE__ )
    #if !defined( __x86_64__ )
        struct OpaqueWindowPtr;
        typedef struct OpaqueWindowPtr * WindowPtr;
        typedef WindowPtr WindowRef;
    #endif
    struct FSSpec;
#endif

#if defined( _WIN32 )
    struct HWND__;
    typedef struct HWND__ * HWND;
#endif
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

namespace Parameters { template <class Parameter> struct DisplayValueTransformer; }

namespace Plugins
{
//------------------------------------------------------------------------------

// Forward declaration for the Detail::AEffectWrapper class.
class VSTHost24Proxy;

////////////////////////////////////////////////////////////////////////////////
//
// Private implementation details for this module.
// -----------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{

////////////////////////////////////////////////////////////////////////////////
///
/// \class AEffectWrapper
///
/// \brief A VSTHost24Proxy helper class.
///
///   The LE VST24Plugin implementation uses a trick to avoid an unnecessary
/// pointer dereference. In the original VST SDK the AEffect struct has a
/// special member, void * object, that served the purpose to save the this
/// pointer for an AudioEffect(X) instance and in each of the callbacks that the
/// host calls it would pass a pointer to the AEffect struct from which one
/// could read the AEffect::object pointer and cast it to the required type.
/// Considering that the host should (as it seems from current knowledge) only
/// store the pointer to the AEffect struct that we give to it and not a copy of
/// it, the previously described procedure of getting one's this pointer from
/// the AEffect::object seems unnecessary: we can just as well derive from the
/// AEffect struct making in effect the pointer to the AEffect struct the same
/// as our this pointer (this is what the fromAEffect() member function assumes
/// and uses to give an AEffectWrapper pointer from an AEffect pointer).
///
///   The problem of simply deriving from AEffect (even privately) was that it
/// caused ambiguity compile errors in expressions like Impl::version in the
/// VST24Plugin<> class (similarly to those described in the Plugin class's
/// implementation note). For this reason the AEffectWrapper class was
/// introduced that does not derive from the AEffect struct but rather holds it
/// as its first and only member (thus having the same binary layout as if it
/// derived from it) and provides the "conversion" fromAEffect() static member
/// function.
///
///   None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class AEffectWrapper : boost::noncopyable
{
private:
    friend class Plugins::VSTHost24Proxy;

     AEffectWrapper();
    ~AEffectWrapper() {}

    AEffect & aEffect() { return aEffect_; }

    static AEffectWrapper * fromAEffect( AEffect * );

private:
    AEffect aEffect_;
}; // class AEffectWrapper


////////////////////////////////////////////////////////////////////////////////
///
/// \class VSTPluginBase::TimingInformation
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Here we interpret the VstTimeInfo::ppqPos data member as the number of
// beats, in contradiction to the official documentation, because this seems
// to be more inline with how the major hosts interpret it. See this thread
// http://www.kvraudio.com/forum/viewtopic.php?t=310318 for more
// information.
//   For code that used the VstTimeInfo::ppqPos data member as the number of
// quarters see SVN revision number 3486.
//                                        (09.02.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////
// http://www.audiomulch.com/mulchnotes/mulchnote_2.htm
////////////////////////////////////////////////////////////////////////////////

class VST24TimingInformation
{
public:
/* ...mrmlj...
    kVstTransportChanged     = 1,		///< indicates that play, cycle or record state has changed
    kVstTransportPlaying     = 1 << 1,	///< set if Host sequencer is currently playing
    kVstTransportCycleActive = 1 << 2,	///< set if Host sequencer is in cycle mode
    kVstTransportRecording   = 1 << 3,	///< set if Host sequencer is in record mode
    kVstAutomationWriting    = 1 << 6,	///< set if automation write mode active (record parameter changes)
    kVstAutomationReading    = 1 << 7,	///< set if automation read mode active (play parameter changes)
*/
    enum FieldFlags
    {
        SystemTime      = kVstNanosValid   ,
        NumberOfBeats   = kVstPpqPosValid  ,
        BPM             = kVstTempoValid   ,
        LastBarPosition = kVstBarsValid    ,
        CyclePosition   = kVstCyclePosValid,
        TimeSignature   = kVstTimeSigValid ,
        SMTPE           = kVstSmpteValid   ,
        MIDIClock       = kVstClockValid   ,
        SampleTime      = MIDIClock << 1   ,
        SampleRate      = MIDIClock << 2   ,
        SampleRateScale = MIDIClock << 3   ,
        TransportState  = MIDIClock << 4
    };

public:
    double const & position                () const { return pTimeInfo_->samplePos         ; } ///< current Position in audio samples (always valid)
    double const & sampleRate              () const { return pTimeInfo_->sampleRate        ; } ///< current Sample Rate in Herz (always valid)
    double const & systemTime              () const { return pTimeInfo_->nanoSeconds       ; } ///< System Time in nanoseconds (10^-9 second)
    double const & numberOfBeats           () const { return pTimeInfo_->ppqPos            ; } ///< Musical Position, in Quarter Note (1.0 equals 1 Quarter Note)
    double const & bpm                     () const { return pTimeInfo_->tempo             ; } ///< current Tempo in BPM (Beats Per Minute)
    double const & lastBarStartPosition    () const { return pTimeInfo_->barStartPos       ; } ///< last Bar Start Position, in Quarter Note
    double const & cycleStartPosition      () const { return pTimeInfo_->cycleStartPos     ; } ///< Cycle Start (left locator), in Quarter Note
    double const & cycleEndPosition        () const { return pTimeInfo_->cycleEndPos       ; } ///< Cycle End (right locator), in Quarter Note
    unsigned int   timeSignatureNumerator  () const { return pTimeInfo_->timeSigNumerator  ; } ///< Time Signature Numerator (e.g. 3 for 3/4)
    unsigned int   timeSignatureDenominator() const { return pTimeInfo_->timeSigDenominator; } ///< Time Signature Denominator (e.g. 4 for 3/4)
    unsigned int   smpteOffset             () const { return pTimeInfo_->smpteOffset       ; } ///< SMPTE offset (in SMPTE subframes (bits; 1/80 of a frame)). The current SMPTE position can be calculated using #samplePos, #sampleRate, and #smpteFrameRate.
    unsigned int   smpteFrameRate          () const { return pTimeInfo_->smpteFrameRate    ; } ///< @see VstSmpteFrameRate
             int   samplesToNextClock      () const { return pTimeInfo_->samplesToNextClock; } ///< MIDI Clock Resolution (24 Per Quarter Note), can be negative (nearest clock)

    template <FieldFlags   desiredField > bool hasField () const { return ( desiredField & alwaysTrueFlags ) || ( pTimeInfo_ && (   pTimeInfo_->flags & desiredField                                          ) ); }
    template <unsigned int desiredFields> bool hasFields() const { return                                         pTimeInfo_ && ( ( pTimeInfo_->flags & desiredFields ) == ( desiredFields & ~alwaysTrueFlags ) ); }

    operator ::VstTimeInfo const * () const { return pTimeInfo_; }

private: friend class Plugins::VSTHost24Proxy;
    VST24TimingInformation( ::VstTimeInfo const * LE_RESTRICT const pTimeInfo ) : pTimeInfo_( pTimeInfo ) {}

    static unsigned int const alwaysTrueFlags = SampleTime | SampleRate | TransportState;

private:
    ::VstTimeInfo const * LE_RESTRICT const pTimeInfo_;
}; // class VST24TimingInformation

//inline template <> bool VST24TimingInformation::hasField<VST24TimingInformation::SampleTime>() const { return true; }
//inline template <> bool VST24TimingInformation::hasField<VST24TimingInformation::SampleRate>() const { return true; }
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
///
/// \class VSTHost24Proxy
///
/// \brief Implements the host proxy class for the VST 2.4 platform. Mostly
/// consists of plugin -> host related functionality extracted from the
/// AudioEffect(X) classes.
///
///   None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class VSTHost24Proxy
    :
    private Detail::AEffectWrapper
{
public:
    typedef
        #if   defined( _WIN32    )
            char const *
        #elif defined( __APPLE__ )
            FSSpec const *
        #endif
    PathSpec;

public:
    VSTHost24Proxy( audioMasterCallback );

public: // Host capabilities.
    static bool const indexedParameters              = true ;
    static bool const identifiedParameters           = false;
    static bool const normalisedParameters           = true ;
    static bool const dynamicParameterList           = false;
    static bool const dynamicParameterInfo           = false;
    static bool const threadSafeEventSystem          = false;
    static bool const pluginCanChangeIOConfiguration = true ;

    template <HostCapability capability>
    bool LE_NOTHROWNOALIAS canDo(                                   ) const { return canDo( capabilityString<capability>() ); }
    bool LE_NOTHROWNOALIAS canDo( char const * const hostCapability ) const { return call( audioMasterCanDo, 0, 0, const_cast<char *>( hostCapability ) ) == 1; }

public: // Host information/state.
    ::VstAutomationStates getAutomationState    () const { return static_cast<::VstAutomationStates>( call( audioMasterGetAutomationState     ) ); }
    ::VstProcessLevels    getCurrentProcessLevel() const { return static_cast<::VstProcessLevels   >( call( audioMasterGetCurrentProcessLevel ) ); }
    ::VstHostLanguage     getHostLanguage       () const { return static_cast<::VstHostLanguage    >( call( audioMasterGetLanguage            ) ); }
      unsigned int        getSupportedVSTVersion() const { return static_cast<unsigned int         >( call( audioMasterVersion                ) ); }

    template <unsigned int filterMask>
    Detail::VST24TimingInformation getTimeInfo() const { return makeTimeInfo( call( audioMasterGetTime, 0, filterMask ) ); }

    unsigned int getInputLatency () const { return static_cast<unsigned int>( call( audioMasterGetInputLatency  ) ); }
    unsigned int getOutputLatency() const { return static_cast<unsigned int>( call( audioMasterGetOutputLatency ) ); }

    unsigned int getBlockSize    () const { return static_cast<unsigned int>( call( audioMasterGetBlockSize     ) ); } ///< VSTSDK AudioEffectX::updateBlockSize () equivalent.
    unsigned int getSampleRate   () const { return static_cast<unsigned int>( call( audioMasterGetSampleRate    ) ); } ///< VSTSDK AudioEffectX::updateSampleRate() equivalent.

public: // Plugin information/state reported/visible to host.
    bool reportNewNumberOfIOChannels( unsigned int inputs, unsigned int sideInputs, unsigned int outputs );

    bool reportNewLatencyInSamples( unsigned int samples );

    bool ioChanged() const; //...mrmlj...only for effSetSpeakerArrangement...

    PathSpec getPluginDirectory() const; ///< VSTSDK AudioEffectX::getDirectory() equivalent.

    unsigned int getNumInputs () const { return aEffect().numInputs ; }
    unsigned int getNumOutputs() const { return aEffect().numOutputs; }

public: // Parameter automation.
    void automatedParameterChanged  ( ParameterIndex, AutomatedParameterValue ) const;
    void automatedParameterBeginEdit( ParameterIndex                          ) const;
    void automatedParameterEndEdit  ( ParameterIndex                          ) const;

    static void gestureBegin( char const * /*description*/ ) {}
    static void gestureEnd  (                              ) {}

    void allParametersChanged() const;

    bool knownToSupportDynamicParameterLists() const { return supportsDynamicParameterLists; }

public: // GUI.
    bool currentPresetNameChanged() const { return updateDisplay(); }

    bool sizeWindow   ( unsigned int const width, unsigned int const height ) const { return call( audioMasterSizeWindow, width, height ) != 0; }
    bool updateDisplay(                                                     ) const;

    bool openFileSelector ( ::VstFileSelect const & fileSelectInfo ) const { return call( audioMasterOpenFileSelector , 0, 0, &const_cast<::VstFileSelect &>( fileSelectInfo ) ) != 0; }
    bool closeFileSelector( ::VstFileSelect const & fileSelectInfo ) const { return call( audioMasterCloseFileSelector, 0, 0, &const_cast<::VstFileSelect &>( fileSelectInfo ) ) != 0; }

public: // Offline functionality (http://www.kvraudio.com/forum/viewtopic.php?t=369997).
    bool         offlineRead              ( VstOfflineTask const & offlineTask, VstOfflineOption const option, bool const readSource ) const { return call( audioMasterOfflineRead, readSource, option, const_cast<VstOfflineTask *>( &offlineTask ) ) != 0; }
    bool         offlineWrite             ( VstOfflineTask const & offlineTask, VstOfflineOption const option                        ) const { return call( audioMasterOfflineWrite,         0, option, const_cast<VstOfflineTask *>( &offlineTask ) ) != 0; }
    bool         offlineStart             ( VstAudioFile & audioFiles, VstInt32 const numAudioFiles, VstInt32 const numNewAudioFiles ) const { return call( audioMasterOfflineStart, numNewAudioFiles, numAudioFiles, &audioFiles ) != 0; }
    unsigned int offlineGetCurrentPass    (                                                                                          ) const { return call( audioMasterOfflineGetCurrentPass     ) != 0; }
    unsigned int offlineGetCurrentMetaPass(                                                                                          ) const { return call( audioMasterOfflineGetCurrentMetaPass ) != 0; }

public: // Vendor specific.

    template <class Range>
    bool getHostVendorString( Range & text ) const
    {
        BOOST_ASSERT( boost::size( text ) >= kVstMaxVendorStrLen );
        return call( audioMasterGetVendorString, 0, 0, &*boost::begin( text ) ) != 0 ;
    }

    template <class Range>
    bool getHostProductString( Range & text ) const
    {
        BOOST_ASSERT( boost::size( text ) >= kVstMaxProductStrLen );
        return call( audioMasterGetProductString, 0, 0, &*boost::begin( text ) ) != 0 ;
    }

    unsigned int getHostVendorVersion() const { return static_cast<unsigned int>( call( audioMasterGetVendorVersion ) ); }

    VstIntPtr hostVendorSpecific( VstInt32 const lArg1, VstIntPtr const lArg2, void * const ptrArg, float const floatArg ) const { return call( audioMasterVendorSpecific, lArg1, lArg2, ptrArg, floatArg ); }

    bool isHost( char const * hostName ) const;

protected:
    static VSTHost24Proxy * fromAEffect( AEffect * const pEffect ) { return static_cast<VSTHost24Proxy *>( AEffectWrapper::fromAEffect( pEffect ) ); }

protected:
    /// \todo This section should be made private. See how to fix this
    /// encapsulation breach/design flaw.
    ///                                       (20.08.2009.) (Domagoj Saric)
    ::AEffect       & aEffect()       { return AEffectWrapper::aEffect(); }
    ::AEffect const & aEffect() const { return const_cast<VSTHost24Proxy &>( *this ).aEffect(); }

private:
    LE_NOTHROW
    VstIntPtr LE_COLD LE_FASTCALL call( VstInt32 opCode, VstInt32 index, VstIntPtr value, void * ptr, float opt ) const;

    VstIntPtr LE_COLD LE_FASTCALL call( VstInt32 const opCode, VstInt32 const index, VstIntPtr const value, void * const ptr ) const { return call( opCode, index, value, ptr, 0 ); }
    VstIntPtr LE_COLD LE_FASTCALL call( VstInt32 const opCode, VstInt32 const index, VstIntPtr const value                   ) const { return call( opCode, index, value,   0, 0 ); }
    VstIntPtr LE_COLD LE_FASTCALL call( VstInt32 const opCode, VstInt32 const index                                          ) const { return call( opCode, index,     0,   0, 0 ); }
    VstIntPtr LE_COLD LE_FASTCALL call( VstInt32 const opCode                                                                ) const { return call( opCode,     0,     0,   0, 0 ); }

    template <HostCapability capability>
    static char const * capabilityString()
    {
        switch ( capability )
        {
            case SendVstEvents                 : return "sendVstEvents"                 ;
            case SendMidiEvent                 : return "sendVstMidiEvent"              ;
            case SendTimeInfo                  : return "sendVstTimeInfo"               ;
            case ReceiveVstEvents              : return "receiveVstEvents"              ;
            case ReceiveMidiEvent              : return "receiveVstMidiEvent"           ;
            case ReportConnectionChanges       : return "reportConnectionChanges"       ;
            case AcceptIOChanges               : return "acceptIOChanges"               ;
            case SizeWindow                    : return "sizeWindow"                    ;
            case Offline                       : return "offline"                       ;
            case OpenFileSelector              : return "openFileSelector"              ;
            case CloseFileSelector             : return "closeFileSelector"             ;
            case StartStopProcess              : return "startStopProcess"              ;
            case ShellCategory                 : return "shellCategory"                 ;
            case SendVstMidiEventFlagIsRealtime: return "sendVstMidiEventFlagIsRealtime";
            LE_DEFAULT_CASE_UNREACHABLE();
        }
    }

    Detail::VST24TimingInformation makeTimeInfo( ::VstIntPtr ) const;

private:
    audioMasterCallback const audioMaster_;

    friend class VSTPluginBase; //...mrmlj...
    static bool assumeIOChangedAlwaysSucceedes;
    static bool supportsDynamicParameterLists ;
}; // class VSTHost24Proxy

template <> LE_NOTHROWNOALIAS bool VSTHost24Proxy::canDo<AcceptIOChanges>() const;


////////////////////////////////////////////////////////////////////////////////
///
/// \class VSTPluginBase
///
/// \brief Base class for the VST 2.4 plugin classes. Contains members
/// independent of a particular concrete plugin implementation class.
///
////////////////////////////////////////////////////////////////////////////////

template <class Protocol> class  ParameterInformation;
template <class Protocol> struct ErrorCode;
template <class Protocol> struct AutomatedParameterFor;

template <>
struct ErrorCode<Protocol::VST24>
{
    enum value_type
    {
        Success                  = true ,
        OutOfMemory              = false,
        OutOfRange               = false,
        CannotDoInCurrentContext = false,
        Unauthorized             = false,
        Unsupported              = false
    };
}; // struct ErrorCode<Protocol::VST24>


template <> struct AutomatedParameterFor<Protocol::VST24> { typedef NormalisedAutomatedParameter type; };

class VSTPluginBase
    :
    protected VSTHost24Proxy,
    public    ErrorCode<Protocol::VST24> //...mrmlj...
{
public: // Type definitions.
    using ConstructionParameter = ::audioMasterCallback         ;
    using AutomatedParameter    =   NormalisedAutomatedParameter;
    using MIDIEvent             = ::VstMidiEvent                ;
    using ParameterSelector     =   ParameterIndex              ;

    using HostProxy = VSTHost24Proxy;

    class Editor;

    using TimingInformation = Detail::VST24TimingInformation;

    using Protocol             = Protocol::VST24                           ;
    using ErrorCode            = ErrorCode           <Protocol>::value_type;
    using ParameterInformation = ParameterInformation<Protocol>            ;

    static DSPGUISeparation const dspGUISeparation = NotAvailable;

public: // VST string buffer definitions.
    typedef char NameBuf           [ kVstMaxNameLen       ];
    typedef char LabelBuf          [ kVstMaxLabelLen      ];
    typedef char ShortLabelBuf     [ kVstMaxShortLabelLen ];
    typedef char CategoryLabel     [ kVstMaxCategLabelLen ];
    typedef char FileNameBuf       [ kVstMaxFileNameLen   ];
    typedef char ProgramNameBuf    [ kVstMaxProgNameLen   ];
    /// \note See the "Cubase and kVstMaxParamStrLen" discussion on the
    /// vst-plugins ML.
    ///                                       (13.03.2013.) (Domagoj Saric)
  //typedef char ParameterStringBuf[ kVstMaxParamStrLen   ];
    typedef char ParameterStringBuf[ kVstMaxEffectNameLen ];
    typedef char VendorStringBuf   [ kVstMaxVendorStrLen  ];
    typedef char ProductStringBuf  [ kVstMaxProductStrLen ];
    typedef char EffectNameBuf     [ kVstMaxEffectNameLen ];

private: // Traits (must be defined by the concrete plugin class).
    static VstPlugCategory const _category           ;
    static unsigned int    const _maxNumberOfPrograms;
    static VstInt32        const _vstUniqueID        ;
    static VstInt32        const _vstVersion         ;

public:
    VSTHost24Proxy       & host()       { return *this; }
    VSTHost24Proxy const & host() const { return const_cast<VSTPluginBase &>( *this ).host(); }

protected:
     VSTPluginBase( ConstructionParameter const constructionParameter )
         :
         VSTHost24Proxy             ( constructionParameter ),
         pChunkData_                ( nullptr               ),
         pSpeakerArrangementStorage_( nullptr               ),
         allParametersInspected_    ( false                 ),
         rect_                      ( ERect()               )
     {}
    ~VSTPluginBase() { std::free( pChunkData_ ); std::free( pSpeakerArrangementStorage_ ); }

    /// \todo @see the todo item for the functions mentioned below in the
    /// VSTHost24Proxy class.
    ///                                       (20.08.2009.) (Domagoj Saric)
    using VSTHost24Proxy::aEffect;
    using VSTHost24Proxy::call;

    static VSTPluginBase * fromAEffect( AEffect * const pEffect ) { return static_cast<VSTPluginBase *>( VSTHost24Proxy::fromAEffect( pEffect ) ); }

protected:
    enum CanDoAnswer
    {
        Can      =  1,
        Cannot   = -1,
        DontKnow =  0
    }; // enum CanDoAnswer

protected:
    void * LE_RESTRICT pChunkData_;
    void * LE_RESTRICT pSpeakerArrangementStorage_;

    mutable bool allParametersInspected_;

    //...mrmlj...gui components...quick-hack to allow a non-templated editor class...
    ERect rect_;
}; // class VSTPluginBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class VSTPluginBase::Editor
///
/// \brief Provides the bare minimum for a VST 2.4 plugin editor.
///
////////////////////////////////////////////////////////////////////////////////

class VSTPluginBase::Editor
{
public: // Plugin::Editor type definition requirements.
    using Rect = ERect;

    using WindowHandle =
        #if defined( __APPLE__ )
            #if defined( __x86_64__ )
                ObjC::NSView *;
            #else
                WindowRef;
            #endif
        #elif defined( _WIN32 )
            HWND;
        #endif // OS

public: // VST specific types.
    enum KnobMode // CKnobMode from vstgui.h
    {
        kCircularMode = 0,
        kRelativCircularMode,
        kLinearMode
    };

public: // VST specific (optional) function interface.
    static bool setKnobMode( KnobMode ) { return false; }

    static bool onKeyDown( char /*asciiCode*/, VstVirtualKey, VstModifierKey ) { return false; }
    static bool onKeyUp  ( char /*asciiCode*/, VstVirtualKey, VstModifierKey ) { return false; }

    static void idle() {}

protected:
    ERect & rect() { return rect_; }

private:
    ERect rect_;
}; // class VSTPluginBase::Editor


////////////////////////////////////////////////////////////////////////////////
///
/// \class ParameterInformation<Protocol::VST24>
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

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

template <>
class ParameterInformation<Protocol::VST24> : public ::VstParameterProperties
{
public:
    typedef void result_type;

    void set( AutomatedParameter::Info const & info ) LE_COLD
    {
        LE_ASSUME( flags == 0 );

        switch ( info.type )
        {
            case AutomatedParameter::Info::Trigger:
            case AutomatedParameter::Info::Boolean:
                flags |= kVstParameterIsSwitch | kVstParameterUsesIntegerMinMax;
                minInteger = false;
                maxInteger = true ;
                break;
            case AutomatedParameter::Info::Integer:
            case AutomatedParameter::Info::Enumerated:
            case AutomatedParameter::Info::FloatingPoint:
                flags |= kVstParameterCanRamp;
                minInteger = static_cast<int>( info.minimum );
                maxInteger = static_cast<int>( info.maximum );
                if
                (
                    ( info.type != AutomatedParameter::Info::FloatingPoint ) ||
                    (
                        ( minInteger == info.minimum ) &&
                        ( maxInteger == info.maximum ) &&
                        ( minInteger != 0 || maxInteger != 1 )
                    )
                )
                {
                    flags |= kVstParameterUsesIntegerMinMax;
                }
                break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }
        std::strcpy(      label, info.unit );
        std::strcpy( shortLabel, info.unit );
    }

    template <class Parameter>
    result_type operator()() LE_COLD
    {
        LE_ASSUME( flags == 0 );
        setValues<Parameter>( std::is_floating_point<typename Parameter::value_type>() );
        typedef typename Parameters::DisplayValueTransformer<Parameter>::Suffix Label;
        setLabel( boost::mpl::c_str<Label>::value, boost::mpl::size<Label>::value );
    }

    static VSTPluginBase::ParameterStringBuf * nameBuffer() { return nullptr; }
    static void                                nameSet   () {}

    static void markAsMeta() {}

    void clear() { /*...mrmlj...broken Live undo system quick-fix search...flags = 0;*/Utility::clear( static_cast<::VstParameterProperties &>( *this ) ); }

private:
    ParameterInformation( ParameterInformation const & );

    template <class Parameter>
    void setValues( std::true_type /*is float*/ )
    {
        static_assert( Parameter::discreteValueDistance == 0, "" );
        //...mrmlj...Audition...
        //BOOST_ASSERT( flags          == 0 );
        //BOOST_ASSERT( stepFloat      == 0 );
        //BOOST_ASSERT( smallStepFloat == 0 );
        //BOOST_ASSERT( largeStepFloat == 0 );

        flags |= kVstParameterCanRamp;
        if
        (
            ( Parameter::rangeValuesDenominator == 1 ) &&
            (
                ( Parameter::unscaledMinimum != 0 ) ||
                ( Parameter::unscaledMaximum != 1 )
            )
        )
        {
            flags |= kVstParameterUsesIntegerMinMax;
            minInteger = Parameter::unscaledMinimum;
            maxInteger = Parameter::unscaledMaximum;
        }
    }

    template <class Parameter>
    void setValues( std::false_type /*is integral*/ )
    {
        static_assert( Parameter::rangeValuesDenominator == 1, "" );

        flags |= kVstParameterUsesIntegerMinMax;
        minInteger = Parameter::unscaledMinimum;
        maxInteger = Parameter::unscaledMaximum;

        if ( Parameter::unscaledMinimum == 0 && Parameter::unscaledMaximum == 1 )
            flags |= kVstParameterIsSwitch;
        else
        if
        (
            !std::is_same<typename Parameter::Tag, Parameters::PowerOfTwoParameterTag>::value &&
            !std::is_same<typename Parameter::Tag, Parameters::EnumeratedParameterTag>::value
        )
            flags |= kVstParameterCanRamp;

        if ( Parameter::discreteValueDistance > 1 )
        {
            flags |= kVstParameterUsesIntStep;
            stepInteger = largeStepInteger = Parameter::discreteValueDistance;
        }
    }

    void setLabel( char const * LE_RESTRICT const unit, unsigned int const length )
    {
        LE_ASSUME( length         <= BOOST_MPL_LIMIT_STRING_SIZE );
        LE_ASSUME( unit[ length ] == '\0'                        );
        static_assert( BOOST_MPL_LIMIT_STRING_SIZE <= _countof( label      ), "" );
        static_assert( BOOST_MPL_LIMIT_STRING_SIZE <= _countof( shortLabel ), "" );
        std::memcpy(      label, unit, length + 1 );
        std::memcpy( shortLabel, unit, length + 1 );
    }
}; // class ParameterInformation<VST24>

#pragma warning( push )


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin<VST24>
///
/// \brief Provides the VST 2.4 protocol implementation.
///
////////////////////////////////////////////////////////////////////////////////

template <class ImplParam>
class Plugin<ImplParam, Protocol::VST24>
    :
    public VSTPluginBase
{
protected:
    Plugin( ConstructionParameter );
#ifndef NDEBUG
    ~Plugin();
#endif

protected:
    typedef ImplParam Impl;

    typedef std::true_type SupportsTimingInformation;

public:
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    class HostProxy : public VSTPluginBase::HostProxy
    {
    public:
        bool LE_NOTHROW LE_COLD parameterListChanged() const
        {
            auto & inspectedParameters( Plugin::inspectedParameters() );
            inspectedParameters.reset();
            updateDisplay();
            return inspectedParameters.all();
        }
        void LE_NOTHROW LE_COLD presetChangeBegin() const {}
        void LE_NOTHROW LE_COLD presetChangeEnd  () const
        {
            if ( !parameterListChanged() )
            {
                LE_TRACE( "\tSW VST2.4: host rejected updateDisplay() (have to manually update all parameters)." );
                allParametersChanged();
            }
        }

        bool wantsManualDependentParameterNotifications() const
        {
            return !static_cast<Plugin const &>( static_cast<VSTPluginBase::HostProxy const &>( *this ) ).impl().gui();
        }
    }; // class HostProxy
    #pragma warning( pop )

    HostProxy       & host()       { return static_cast<HostProxy &>( VSTPluginBase::host() ); }
    HostProxy const & host() const { return const_cast <Plugin    &>( *this ).host(); }

protected: // Functions (VST 2.4 host -> plugin events/callbacks) that must be
           // implemented by the user Impl class in order to support the VST 2.4
           // platform (in other words no default implementations are provided).

    // void setSampleRate( float sampleRate );
    // void setBlockSize ( unsigned int blockSize );

    // unsigned int getProgram();
    // void         setProgram( unsigned int programIndex );

protected: // Default implementations for VST 2.4 host -> plugin
           // events/callbacks (user Impl classes are free to override them and
           // properly implement them).
    void initialise() {} ///< VSTSDK AudioEffect::open () equivalent.
    void finalise  ()    ///< VSTSDK AudioEffect::close() equivalent.
    {
        /// \note
        ///   MSVC10 has problems detecting that impl() returns a non-null
        /// reference and inserts a bogus nullptr check before calling delete.
        ///                                   (11.01.2012.) (Domagoj Saric)
        Impl * LE_RESTRICT const pImpl( &impl() );
        LE_ASSUME( pImpl );
        delete pImpl;
    }

    void setProgramName( char const * ) {}
    void getProgramName(                         ProgramNameBuf & name ) const { name[ 0 ] = '\0'; }
    void getProgramName( unsigned int /*index*/, ProgramNameBuf &      ) const {}

    bool beginSetProgram() { return false; }
    bool endSetProgram  () { return false; }

    unsigned int beginLoadBank   ( VstPatchChunkInfo const & ) { return 0; }
    unsigned int beginLoadProgram( VstPatchChunkInfo const & ) { return 0; }

    static void getParameterLabel  ( ParameterIndex, ParameterStringBuf & label, Plugin const * )       { label[ 0 ] = '\0'; }
           void getParameterDisplay( ParameterIndex, ParameterStringBuf & text                  ) const { text [ 0 ] = '\0'; }
    static void getParameterName   ( ParameterIndex, ParameterStringBuf & name , Plugin const * )       { name [ 0 ] = '\0'; }

    LE_NOTHROWNOALIAS AutomatedParameterValue LE_FASTCALL getParameter( ParameterIndex                          );
    LE_NOTHROW        void                    LE_FASTCALL setParameter( ParameterIndex, AutomatedParameterValue );

    static bool getParameterProperties( ParameterIndex, ParameterInformation &, Plugin const * ) { return false; }

    static bool canParameterBeAutomated( ParameterIndex, void const * /*pContext*/ ) { return false; }

    bool         loadProgramState( unsigned int programIndex, char const * pProgramName, void const * pData, unsigned int dataSize )      ;
    unsigned int saveProgramState( unsigned int programIndex,                            void       * pData, unsigned int dataSize ) const;

    bool getInputProperties ( unsigned int /*index*/, VstPinProperties & ) const { return false; }
    bool getOutputProperties( unsigned int /*index*/, VstPinProperties & ) const { return false; }

    bool setSpeakerArrangement( ::VstSpeakerArrangement const & /*input*/, ::VstSpeakerArrangement const & /*output*/  )       { return false; }
    bool getSpeakerArrangement( ::VstSpeakerArrangement       & /*input*/, ::VstSpeakerArrangement       &  /*output*/ ) const { return false; }

    bool setPanLaw( ::VstPanLawType, float /*value*/ ) { return false; }

    bool setBypass ( bool /*onOff*/ ) { return false; }

    bool setProcessPrecision( ::VstProcessPrecision const precision ) { BOOST_VERIFY( precision == kVstProcessPrecision32 ); return false; }

    bool processVariableIo( ::VstVariableIo const * const pInfo ) { BOOST_VERIFY( !pInfo ); return false; }

    unsigned int setTotalSamplesToProcess( unsigned int const value ) { return value; }

    unsigned int getTailSize() { return Impl::maxTailSize; }

    unsigned int getNumMidiInputChannels () { return 0; }
    unsigned int getNumMidiOutputChannels() { return 0; }

             int getCurrentMidiProgram ( unsigned int /*channel*/, ::MidiProgramName     & ) const { return -1; }
    unsigned int getMidiProgramName    ( unsigned int /*channel*/, ::MidiProgramName     & ) const { return  0; }
    unsigned int getMidiProgramCategory( unsigned int /*channel*/, ::MidiProgramCategory & ) const { return  0; }
    bool         getMidiKeyName        ( unsigned int /*channel*/, ::MidiKeyName         & ) const { return  0; }
    bool         hasMidiProgramsChanged( unsigned int /*channel*/                          ) const { return false; }

    void processMIDIEvent( ::VstMidiEvent const & ) {}

    ::VstInt32 pluginSpecific( ::VstInt32 /*index*/, ::VstIntPtr /*integerParam*/, void * /*pData*/, float /*floatParam*/ ) { return 0; }

    bool offlineNotify ( ::VstAudioFile   const &, unsigned int /*numAudioFiles*/, bool /*start*/ ) { return false; }
    bool offlinePrepare( ::VstOfflineTask const &, unsigned int /*count*/                         ) { return false; }
    bool offlineRun    ( ::VstOfflineTask const &, unsigned int /*count*/                         ) { return false; }

    bool preStreamingInitialization() { return false; }
    bool postStreamingCleanup      () { return false; }

#if 0 //...mrmlj...compilation errors workaround...
    template <PluginCapability pluginCapability>
    bool queryDynamicCapability() const { return false; }
#endif


private: // Private metafunctions.
    // http://lists.boost.org/Archives/boost/2004/11/75661.php
    // http://www.nabble.com/Using-SFINAE-to-detect-presence-of-member-function---Unreliable-td21746545.html
    // http://www.cpptalk.net/detecting-existence-of-a-member-function-sfinae--vt12565.html
    // http://lists.boost.org/Archives/boost/2004/11/75621.php
    // http://lists.boost.org/Archives/boost/2004/11/75661.php

    template <class Implementation>
    class HasProcessDoubleReplacing
    {
    private:
        typedef char      no ;
        typedef long long yes;
        static_assert( sizeof( no ) != sizeof( yes ), "" );

        template <typename ProcessDoubleReplacingPointer, ProcessDoubleReplacingPointer>
        struct SFINAEHelper {};

        template <class T>
        static yes sfinaeTester( SFINAEHelper<void (T::*)( double const * const *, double * *, size_t ), &T::process> * );
        template <class T>
        static no  sfinaeTester( ... );

    public:
        static bool const value = ( sizeof( sfinaeTester<Implementation>( 0 ) ) == sizeof( yes ) );
        typedef boost::mpl::bool_<value> type;
    }; // class HasProcessDoubleReplacing

protected:
           Impl       & impl(             );
           Impl const & impl(             ) const { return const_cast<Plugin &>( *this ).impl(); }
    static Impl       & impl( ::AEffect * );

private:

#ifdef __GNUC__
    //...mrmlj...clang bogus (as we use -fno-strict-aliasing) undefined-behaviour-checker-induced-crashes workarounds...
    // http://blog.qt.digia.com/blog/2011/06/10/type-punning-and-strict-aliasing
    #define LE_MA __attribute__(( __may_alias__ ))

    template <typename T>
    union LE_MA AliasedType
    {
        T    LE_MA value;
        char LE_MA aliasRuleRelaxer0                   ;
        char LE_MA aliasRuleRelaxer1[ sizeof( value ) ];

        constexpr AliasedType LE_MA & operator = ( T const other ) { value = other; return *this; }

        operator T       & ()       { return value; }
        operator T const & () const { return value; }
    } LE_MA;

    template <typename Target> struct AliasRuleBender                 { typedef AliasedType<Target> LE_MA type   ; };
    template <typename Target> struct AliasRuleBender<Target const  > { typedef AliasedType<Target> LE_MA type   ; };
    template <unsigned int N > struct AliasRuleBender<char       [N]> { typedef char                      type[N]; };
    template <unsigned int N > struct AliasRuleBender<char const [N]> { typedef char const                type[N]; };

    template <typename Target>
    static typename AliasRuleBender<Target>::type *
    castTo( void * const source ) { return static_cast<typename AliasRuleBender<Target>::type *>( source ); }

    template <typename Target>
    static typename AliasRuleBender<Target>::type &
    castToReference( void * const source ) { return *castTo<Target>( source ); }

    template <typename Target>
    static typename AliasRuleBender<Target>::type &
    castToReference( std::intptr_t const source ) { return castToReference<Target>( reinterpret_cast<void *>( source ) ); }

    template <typename Target>
    static Target const & castToReference( void const * const source ) { return *static_cast<Target const *>( source ); }

#else
    template <typename Target, typename Pointer>
    #define LE_MA
    static Target & castToReference( Pointer const pointer )
    {
        Target * LE_RESTRICT const pT( reinterpret_cast<Target *>( pointer ) );
        LE_ASSUME( pT );
        return *pT;
    }
#endif // __GNUC__

    static AEffectProcessDoubleProc getDoubleReplacingCallback( boost::mpl::true_  /* has double replacing */           ) { return static_cast<AEffectProcessDoubleProc>( &Plugin::processReplacing ); }
    static AEffectProcessDoubleProc getDoubleReplacingCallback( boost::mpl::false_ /* does not have double replacing */ ) { return 0; }

    template <PluginCapability pluginCapability>
    CanDoAnswer queryImplementationCapability() const;

    LE_COLD LE_NOTHROWNOALIAS
    bool LE_FASTCALL useDynamicParameterLists() const
    {
        return host().knownToSupportDynamicParameterLists() || VSTPluginBase::allParametersInspected_;
    }

    LE_COLD LE_NOTHROW
    void LE_FASTCALL inspectedParameter( ParameterIndex const index ) const
    {
        auto & inspectedParameters( this->inspectedParameters() );
        BOOST_ASSERT( index < Impl::maxNumberOfParameters );
        BOOST_ASSERT( index < inspectedParameters.size()  );
        inspectedParameters.set( index );
        VSTPluginBase::allParametersInspected_ |= inspectedParameters.all();
    }

    //...mrmlj...cannot create normal a member because we cannot look into the
    //...mrmlj...Impl class until this class is fully instantiated (standard CRTP problem)...
    class InspectedParameters : public std::bitset<Impl::maxNumberOfParameters>
    {
    #if defined( __APPLE__ ) && ( !defined( _LIBCPP_VERSION ) || ( __GLIBCXX__ < 20110325 ) )
        public: bool LE_NOTHROW all() const { return this->count() == this->size(); }
    #endif
    }; // class InspectedParameters
    LE_COLD LE_NOTHROWNOALIAS
    static InspectedParameters & inspectedParameters() { static InspectedParameters inspectedParameters_; return inspectedParameters_; }

private: // Callbacks.
    LE_NOTHROW        static VstIntPtr LE_CDECL dispatcher         ( ::AEffect *, ::VstInt32 opCode, ::VstInt32 index, ::VstIntPtr integerParam, void * pData, float floatParam );

    LE_NOTHROWNOALIAS static float     LE_CDECL getParameter       ( ::AEffect *, ::VstInt32 index              );
    LE_NOTHROW        static void      LE_CDECL setParameter       ( ::AEffect *, ::VstInt32 index, float value );

    LE_NOTHROW        static void      LE_CDECL processAccumulating( ::AEffect * const /*pEffect*/, float  * * const /*inputs*/, float  * * const /*outputs*/, ::VstInt32 const /*sampleFrames */) { BOOST_ASSERT( !"Unsupported/Should not get called for VST 2.4 plugins." ); }
    LE_NOTHROW        static void      LE_CDECL processReplacing   ( ::AEffect * const   pEffect  , float  * * const   inputs  , float  * * const   outputs  , ::VstInt32 const   sampleFrames   ) { impl( pEffect ).process( inputs, outputs, sampleFrames ); }
    LE_NOTHROW        static void      LE_CDECL processReplacing   ( ::AEffect * const   pEffect  , double * * const   inputs  , double * * const   outputs  , ::VstInt32 const   sampleFrames   ) { impl( pEffect ).process( inputs, outputs, sampleFrames ); }

public:
    // Implementation note:
    //   The VSTMainInjector helper class needs to access the aEffect() member
    // function to properly implement the VST main function (to convert a
    // VST24Plugin pointer to an aEffect pointer). Trying to predeclare it in
    // the global namespace and give it friend access like this:
    // template <class Plugin> friend class ::VSTMainInjector;
    // did not work (actually it did seem to work at the beginning an then it
    // 'somehow' stopped working: to be further investigated) so as a workaround
    // we publicly export the inherited aEffect() member function.
    //                                        (20.08.2009.) (Domagoj Saric)
    /// \todo Try to find a better solution to the above described encapsulation
    /// problem.
    ///                                       (20.08.2009.) (Domagoj Saric)
    using VSTPluginBase::aEffect;

#ifndef NDEBUG
private:
    static unsigned int pluginInstanceCount_;
    unsigned int const pluginInstanceID_;
#endif
}; // class Plugin<ImplParam, Protocol::VST24>

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "plugin.inl"

#endif // plugin_hpp
