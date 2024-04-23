////////////////////////////////////////////////////////////////////////////////
///
/// \file spectrumWorx.hpp
/// ----------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef spectrumWorx_hpp__84A3FD1B_235D_4AA2_9279_55E5386C0870
#define spectrumWorx_hpp__84A3FD1B_235D_4AA2_9279_55E5386C0870
#pragma once
//------------------------------------------------------------------------------
#ifndef LE_SW_DISABLE_SIDE_CHANNEL
#include "external_audio/sample.hpp"
#endif // LE_SW_DISABLE_SIDE_CHANNEL
#include "core/host_interop/plugin2Host.hpp"
#include "core/spectrumWorxCore.hpp"
#include "gui/editor/spectrumWorxEditor.hpp"

#include "le/plugins/vst/2.4/plugin.hpp"
#include "le/utility/cstdint.hpp"

#include "juce/beginIncludes.hpp"
    #include "juce/juce_core/text/juce_String.h"
    #include "juce/juce_core/threads/juce_Thread.h"
#include "juce/endIncludes.hpp"

#include "boost/intrusive_ptr.hpp"

#ifdef _MSC_VER
    #include "process.h"
    #ifdef _DEBUG
        #include "windows.h"
    #endif // _DEBUG
#else
    #include "pthread.h"
#endif // _MSC_VER

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
    template
    <
        typename Target,
                 int targetRangeOffset,
        unsigned int targetRangeSize,
        unsigned int targetRangeScaleFactor,
        class Parameter
    >
    Target
    convertParameterValueToLinearValue( Parameter const & source );
} // namespace Parameters
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class  Preset;
class  PresetWithPreallocatedFixedNodes;
class  ParametersLoader;
struct PresetHeader;
namespace GUI { class SpectrumWorxEditor; }

#if LE_SW_AUTHORISATION_REQUIRED
    #define LE_LICENCE_SPECIFIC( ... ) __VA_ARGS__
#else
    #define LE_LICENCE_SPECIFIC( ... )
#endif

typedef juce::String::CharPointerType::CharType char_t;


////////////////////////////////////////////////////////////////////////////////
///
/// \class class BackgroundThread
///
////////////////////////////////////////////////////////////////////////////////

class BackgroundThread
{
public:
     BackgroundThread() : thread_( invalidHandle ) {}
    ~BackgroundThread() { stop(); }

    template <class Runner, void (Runner::*callback)()>
    bool start( Runner & runner )
    {
        BOOST_ASSERT_MSG( !isRunning(), "Background thread already running" );
    #ifdef _WIN32
        thread_ = _beginthread( &callbackWrapper<Runner, callback>, 0, &runner );
        if ( thread_ == invalidHandle )
            return false;
        BOOST_VERIFY( ::SetThreadPriority( reinterpret_cast<HANDLE>( thread_ ), THREAD_PRIORITY_IDLE ) );
        return true;
    #else
        if ( ::pthread_create( &thread_, nullptr, &callbackWrapper<Runner, callback>, &runner ) != 0 )
        {
            BOOST_ASSERT( thread_ == invalidHandle );
            return false;
        }
        sched_param schedulingParameters;
        int         schedulingPolicy    ;
        BOOST_VERIFY( ::pthread_getschedparam( thread_, &schedulingPolicy, &schedulingParameters ) == 0 );
    #ifdef SCHED_IDLE
        schedulingPolicy = SCHED_IDLE;
    #else
        schedulingPolicy = SCHED_OTHER;
    #endif // SCHED_OTHER
        schedulingParameters.sched_priority = ::sched_get_priority_min( schedulingPolicy );
        BOOST_VERIFY( ::pthread_setschedparam( thread_, schedulingPolicy, &schedulingParameters ) == 0 );
        BOOST_VERIFY( ::pthread_detach       ( thread_                                          ) == 0 );
        return true;
    #endif // _WIN32
    }

    void setDebugName( char const * const threadName )
    {
    #ifdef _DEBUG
        #if defined( __OBJC__ )
            // http://stackoverflow.com/questions/2057960/how-to-set-a-threadname-in-macosx
            [[NSThread currentThread] setName: [NSString stringWithUTF8String:threadName]];
        #elif defined( _WIN32 )
            #if _WIN32_WINNT >= 0x0502
            // http://msdn.microsoft.com/en-us/library/xcb2z8hs(VS.90).aspx
            DWORD const MS_VC_EXCEPTION( 0x406D1388 );
            #pragma pack( push, 8 )
            struct THREADNAME_INFO
            {
               DWORD  dwType    ; // Must be 0x1000.
               LPCSTR szName    ; // Pointer to name (in user addr space).
               DWORD  dwThreadID; // Thread ID (-1=caller thread).
               DWORD  dwFlags   ; // Reserved for future use, must be zero.
            };
            #pragma pack( pop )
            THREADNAME_INFO info;
            info.dwType     = 0x1000;
            info.szName     = threadName;
            info.dwThreadID = ::GetThreadId( reinterpret_cast<HANDLE>( thread_ ) );
            info.dwFlags    = 0;
            __try
            {
                ::RaiseException( MS_VC_EXCEPTION, 0, sizeof( info ) / sizeof( ULONG_PTR ), reinterpret_cast<ULONG_PTR *>( &info ) );
            }
            __except( EXCEPTION_EXECUTE_HANDLER ) {}
            #else
                threadName;
            #endif
        #else
            #ifndef __APPLE__
                BOOST_VERIFY( ::pthread_setname_np( thread_, threadName ) == 0 );
            #endif // __APPLE__
        #endif // API
    #else
        (void)threadName;
    #endif // _DEBUG
    }

    void stop()
    {
        if ( !isRunning() )
            return;
    #ifdef _WIN32
        BOOST_VERIFY( ::WaitForSingleObject( reinterpret_cast<HANDLE>( thread_ ), 5000 ) == WAIT_OBJECT_0 );
    #else
        BOOST_VERIFY( ::pthread_cancel( thread_ ) );
        thread_ = invalidHandle;
    #endif // _WIN32
        kill();
    }

    void kill()
    {
    #ifdef _WIN32
        BOOST_VERIFY( ::TerminateThread( reinterpret_cast<HANDLE>( thread_ ), 0 ) || thread_ == invalidHandle );
    #else
        BOOST_VERIFY( ::pthread_cancel( thread_ ) == 0 || thread_ == invalidHandle );
    #endif // _WIN32
        thread_ = invalidHandle;
    }

    bool isRunning() const { return thread_ != invalidHandle; }

    // http://stackoverflow.com/questions/2156353/how-do-you-query-a-pthread-to-see-if-it-is-still-running
    void markAsDone() /*...mrmlj...ugh...*/ { thread_ = invalidHandle; }

private:
    template <class Runner, void (Runner::*callback)()>
    static void
    #ifndef _WIN32
        *
    #endif // POSIX
    callbackWrapper( void * const pHandler )
    {
        (static_cast<Runner *>( pHandler )->*callback)();
    #ifndef _WIN32
        return nullptr;
    #endif // _WIN32
    }

private:
#ifdef _WIN32
    typedef uintptr_t handle_t;
    static handle_t const invalidHandle = handle_t( -1 );
#else
    typedef pthread_t handle_t;
    constexpr static handle_t const invalidHandle = handle_t();
#endif // _WIN32

    handle_t thread_;
}; // BackgroundThread


////////////////////////////////////////////////////////////////////////////////
///
/// \class SpectrumWorx
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE SpectrumWorx
    :
    public SpectrumWorxCore,
    public Plugin2HostInteropControler
{
public:typedef Plugin2HostInteropControler::ParameterValueForAutomation ParameterValueForAutomation;
    void LE_NOTHROWNOALIAS process( float const * const * inputs, float * * outputs, std::uint32_t samples );

    void resume();

public:
    static SpectrumWorx const & fromEngineSetup( Engine::Setup const & );

    //...mrmlj...for ParametersLoader
    using Programs = std::array<Program, Constants::numberOfPrograms>;
    Programs       & programs()       { return programs_; }
    Programs const & programs() const { return programs_; }

    using Module    = SW::Module;
    using ModulePtr = boost::intrusive_ptr<Module>;
    struct ModuleInitialiser;
    ModuleInitialiser moduleInitialiser();

protected:
    LE_NOTHROW  SpectrumWorx( bool runningAsAU );
    LE_NOTHROW ~SpectrumWorx();

    bool initialise();

    bool updateEngineSetup();

    void updatePosition( std::uint32_t deltaSamples );

    void handleTimingInformationChange( LFO::Timer::TimingInformationChange );

    bool blockAutomation() const;


    ////////////////////////////////////////////////////////////////////////////
    // Authorization
    ////////////////////////////////////////////////////////////////////////////
#if LE_SW_AUTHORISATION_REQUIRED
    public:
        char_t const & authorised() const { return authorizationData().authorised(); }

    protected:
        AuthorisationData const & authorizationData() const { return authorizationData_; }

        bool initialisePathsAndVerifyLicence();

    private:
        LE_NOTHROW char const * verifyLicence();

        void startLicenceVerificationTimer();
        void  stopLicenceVerificationTimer();

        static float demoGain      ();
               void  setDemoGain   ();
               void  removeDemoGain();

        bool isCurrentlyCrippled() const;

        void demoCripplingThread();

    private:
        BackgroundThread  authorizationThread_;
        AuthorisationData authorizationData_  ;
        float             outputGainScale_    ;

        /// \todo Find a proper spot for these (used to be in the license
        /// related predefinedItems.hpp header before most of the licensing
        /// functionality was extracted into the Licenser SDK).
        ///                                   (26.04.2016.) (Domagoj Saric)
        #define SW_LICENCE_FILE_EXTENSION_STANDARD        _T( ".SWLic"    )
        #define SW_LICENCE_FILE_EXTENSION_VERSION_UPGRADE _T( ".SWULic"   )
        #define SW_LICENCE_FILE_EXTENSION_OS_UPGRADE      _T( ".SWOSULic" )

    /* </Authorization> */
#else
public:
    static char_t const & authorised() { static char_t const a( true ); return a; }
#endif // LE_SW_AUTHORISATION_REQUIRED


#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    ////////////////////////////////////////////////////////////////////////////
    // External samples
    ////////////////////////////////////////////////////////////////////////////

    public:
        bool hasExternalSample() const { return static_cast<bool>( sample_ ); }
        juce::File const & currentSampleFile() const { return sample_.sampleFile(); }

        void setNewSample( juce::File const & );
        void   registerSampleLoadedListener( GUI::SpectrumWorxEditor       & listenerToRegister   );
        void deregisterSampleLoadedListener( GUI::SpectrumWorxEditor const & listenerToDeregister );
        bool isSampleLoadInProgress() const;

    private:
        bool LE_NOTHROWNOALIAS setNewSampleWorker( juce::File const & );
        void sampleLoadingLoop();

    private:
        Sample                    sample_;
        juce::File                pendingSampleToLoad_;
        GUI::SpectrumWorxEditor * pListenerToNotifyWhenSampleLoaded_;
        BackgroundThread          sampleLoadingThread_;

    /* </External samples> */
#endif // LE_SW_DISABLE_SIDE_CHANNEL


    ////////////////////////////////////////////////////////////////////////////
    // IO configuration
    ////////////////////////////////////////////////////////////////////////////

    public:
        bool LE_NOTHROW LE_FASTCALL setNumberOfChannelsFromHost( std::uint8_t numberOfInputChannels, std::uint8_t numberOfOutputChannels );
    #ifndef LE_SW_FMOD
        bool LE_NOTHROW LE_FASTCALL setNumberOfChannelsFromUser( std::uint8_t numberOfInputChannels, std::uint8_t numberOfOutputChannels );
    #endif // LE_SW_FMOD

        bool enableSideChannelInput( bool const enable )
        {
            std::uint8_t const numberOfChannels( engineSetup().numberOfChannels() );
            return setNumberOfChannelsFromHost
            (
                enable
                    ? numberOfChannels * 2
                    : numberOfChannels,
                numberOfChannels
            );
        }

        bool setNumberOfChannels( std::uint8_t const numberOfInputChannels, std::uint8_t const numberOfOutputChannels ) { return setNumberOfChannelsFromHost( numberOfInputChannels, numberOfOutputChannels ); }

    protected:
        void clearSideChannelData               ();
        void clearSideChannelDataIfNoSideChannel();

        bool haveSideChannel() const;

    /* </IO configuration> */


    ////////////////////////////////////////////////////////////////////////////
    // Programs and presets
    ////////////////////////////////////////////////////////////////////////////

    public:
                         void         setProgram( std::uint8_t program );
        LE_PURE_FUNCTION std::uint8_t getProgram(                      ) const { return currentProgram_; }

        void setProgramName(                       char const *                  name )      ;
        void setProgramName( std::uint8_t program, char const *                  name )      ;
        void getProgramName(                       boost::iterator_range<char *> name ) const;
        void getProgramName( std::uint8_t program, boost::iterator_range<char *> name ) const;

        char const * currentProgramName() const;

        LE_NOTHROW bool loadPreset( juce::File const &     , bool ignoreExternalSample, juce::String       * pComment, char_t const * presetName                       );
        LE_NOTHROW bool loadPreset( char             * data, bool ignoreExternalSample, juce::String       * pComment                           , std::uint8_t program );

        bool         loadProgramState( std::uint8_t programIndex, char const * pProgramName, void const * pData, std::uint32_t dataSize )      ;
        unsigned int saveProgramState( std::uint8_t programIndex,                            void       * pData, std::uint32_t dataSize ) const;

        static std::uint16_t maximumProgramSize() { return 4096; } //...mrmlj...

        static juce::File defaultPresetsFolder();

    protected:
        /// \note Module chain instances have to come before the OptionalEditor
        /// instance so that it gets destroyed after the OptionalEditor instance
        /// (because the Module destructor expects its UI to be predestroyed).
        ///                                   (19.03.2013.) (Domagoj Saric)
        std::uint8_t currentProgram_;
        Programs     programs_      ;

    private:
        struct PresetConsumer;
        struct PresetLoader;

    /* </Programs and presets> */

    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////
    protected:
        typedef GlobalParameters::Parameters Parameters;
        void LE_FASTCALL resetForGlobalParameters( Parameters const & );
        bool LE_FASTCALL canParameterBeAutomated( ParameterID, Program const * ) const;
protected: // Global parameter change wrapper functions
        friend class SpectrumWorxCore; //...mrmlj...
        using SpectrumWorxCore::setGlobalParameter;
        bool LE_FASTCALL setGlobalParameter( FFTSize          &, FFTSize         ::param_type );
        bool LE_FASTCALL setGlobalParameter( OverlapFactor    &, OverlapFactor   ::param_type );
    #if LE_SW_ENGINE_INPUT_MODE >= 2
        bool LE_FASTCALL setGlobalParameter( InputMode        &, InputMode       ::param_type );
    #endif // LE_SW_ENGINE_INPUT_MODE >= 2
    #if LE_SW_ENGINE_WINDOW_PRESUM
        bool LE_FASTCALL setGlobalParameter( WindowSizeFactor &, WindowSizeFactor::param_type );
    #endif // LE_SW_ENGINE_WINDOW_PRESUM
        bool LE_FASTCALL setGlobalParameter( GlobalParameters::InputGain     & parameter, float const newValue ) { parameter.setValue( newValue ); updateGUIForGlobalParameterChange(); return true; }
        bool LE_FASTCALL setGlobalParameter( GlobalParameters::OutputGain    & parameter, float const newValue ) { parameter.setValue( newValue ); updateGUIForGlobalParameterChange(); return true; }
        bool LE_FASTCALL setGlobalParameter( GlobalParameters::MixPercentage & parameter, float const newValue ) { parameter.setValue( newValue ); updateGUIForGlobalParameterChange(); return true; }
    /* </Parameters> */


#if LE_SW_GUI
    ////////////////////////////////////////////////////////////////////////////
    // GUI
    ////////////////////////////////////////////////////////////////////////////

    public:
        using Editor         = GUI::SpectrumWorxEditor;
        using OptionalEditor = boost::optional<Editor>;

        bool createGUI ();
        void destroyGUI();

        OptionalEditor       & gui()       { return editor_; }
        OptionalEditor const & gui() const { return editor_; }

        static SpectrumWorx & effect( Editor & );

    protected:
        void updateGUIForGlobalParameterChange();

    private:
        void updateGUIForEngineSetupChanges();

    private:
        OptionalEditor editor_;

        friend class GUI::SpectrumWorxEditor; // ...mrmlj... clean up this encapsulation/design error...

    /* </GUI> */
#endif // LE_SW_GUI

    ////////////////////////////////////////////////////////////////////////////
    // Settings
    ////////////////////////////////////////////////////////////////////////////

    public:
        bool shouldLoadLastSessionOnStartup(                                     ) const { return !runningAsAU() && loadLastSessionOnStartup_; }
        void shouldLoadLastSessionOnStartup( bool const loadLastSessionOnStartup )       { loadLastSessionOnStartup_ = loadLastSessionOnStartup; }

    #if LE_SW_ENGINE_INPUT_MODE >= 1
        void setInputModeToSetOnRestart( InputMode );
    #endif // LE_SW_ENGINE_INPUT_MODE >= 1

        bool completelyDisableIOChanges() const { return runningAsAU(); }

    protected:
        void LE_NOTHROW loadSettings();
        void LE_NOTHROW saveSettings();

        static juce::File lastSessionPresetFile();
        static juce::File settingsFile         ();

    private:protected: //...mrmlj...GlobalParameterUpdater
        LE_CONST_FUNCTION bool LE_FASTCALL runningAsAU() const;

    private:
    #if LE_SW_ENGINE_INPUT_MODE >= 1
        InputMode::value_type inputModeToSetOnRestart_ ;
    #endif // LE_SW_ENGINE_INPUT_MODE >= 1
        bool                  loadLastSessionOnStartup_;
    #ifdef __APPLE__
        bool const            runningAsAU_             ;
    #endif // __APPLE__

    /* </Settings> */

}; // class SpectrumWorx


#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
struct SpectrumWorx::ModuleInitialiser
{
    typedef SpectrumWorx::Module Module;

    bool operator()( Module &, std::uint8_t moduleIndex ) const;

    SpectrumWorxCore::ModuleInitialiser   const dspInitialiser;
    GUI::SpectrumWorxEditor             * const pEditor       ;
}; // struct SpectrumWorx::ModuleInitialiser
#pragma warning( pop )


inline void intrusive_ptr_add_ref( SpectrumWorx const * const pEffect ) { LE_ASSUME( pEffect ); /*...mrmlj...for generic GUI::postMessage...*/ }
inline void intrusive_ptr_release( SpectrumWorx const * const pEffect ) { LE_ASSUME( pEffect ); /*...mrmlj...for generic GUI::postMessage...*/ }

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // spectrumWorx_hpp
