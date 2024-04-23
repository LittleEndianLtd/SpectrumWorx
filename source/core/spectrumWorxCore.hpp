////////////////////////////////////////////////////////////////////////////////
///
/// \file spectrumWorxCore.hpp
/// --------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef spectrumWorxCore_hpp__8FE46440_DFB7_4B10_AFC4_2536AE1BFE2B
#define spectrumWorxCore_hpp__8FE46440_DFB7_4B10_AFC4_2536AE1BFE2B
#pragma once
//------------------------------------------------------------------------------
#include "automatedModuleChain.hpp"
#include "configuration/versionConfiguration.hpp"
#include "host_interop/host2Plugin.hpp"
#include "host_interop/parameters.hpp"
#include "host_interop/plugin2Host.hpp"

#include "le/plugins/plugin.hpp"
#include "le/spectrumworx/engine/configuration.hpp"
#include "le/spectrumworx/engine/processor.hpp"
#include "le/utility/buffers.hpp"
#include "le/utility/criticalSection.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/intrinsics.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

class Module   ;
class ModuleDSP;

////////////////////////////////////////////////////////////////////////////////
///
/// \class SpectrumWorxCore
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE SpectrumWorxCore
    :
    public    Host2PluginInteropControler,
    public    Plugin2HostPassiveInteropController,
    protected Engine::Processor
{
protected:
    class InputBuffers;

public:
    // Plugin required traits.
    /// \note Currently changing the order of reported parameters (or major
    /// groups of parameters) requires the manual updating of the following
    /// SpectrumWorxCore member functions:
    ///  - invokeFunctorOnIdentifiedParameter()
    ///  - parameterIDFromIndex
    ///  - parameterIndexFromBinaryID
    ///                                       (07.03.2013.) (Domagoj Saric)
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST maxNumberOfParameters    = ParameterCounts::maxNumberOfParameters;
    enum /*...mrmlj...Xcode7 linker errors*/    { maxNumberOfOutputs       = 13 + 1                 }; // Auro 13.1 http://www.auro-technologies.com/system/engine
    enum                                        { maxNumberOfInputs        = maxNumberOfOutputs * 2 };
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST maxLatency               = Engine::Constants::maximumFFTSize;
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST maxLookAhead             = Engine::Constants::maximumFFTSize;
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST maxTailSize              = 0;
    static std::uint16_t BOOST_CONSTEXPR_OR_CONST minimumProcessBufferSize = 0;
    static std::uint8_t  BOOST_CONSTEXPR_OR_CONST requiredBufferAlignement = Utility::Constants::vectorAlignment;
    static std::uint8_t  BOOST_CONSTEXPR_OR_CONST versionMajor             = SW_VERSION_MAJOR;
    static std::uint8_t  BOOST_CONSTEXPR_OR_CONST versionMinor             = SW_VERSION_MINOR;
    static std::uint8_t  BOOST_CONSTEXPR_OR_CONST versionPatch             = SW_VERSION_PATCH;
    static std::uint32_t BOOST_CONSTEXPR_OR_CONST version                  = ( versionMajor * 1000 ) + ( versionMinor * 100 ) + ( versionPatch * 10 );
    static char          const                    name         [];
    static char          const                    productString[];

public:
    /// \todo Further research and test the meaning and application of "inserts"
    /// and "sends".
    /// http://en.wikipedia.org/wiki/Insert_(effects_processing)
    /// http://en.wikipedia.org/wiki/Aux-send
    /// http://www.chellman.org/audio/sends_versus_inserts.html
    /// http://www.sweetwater.com/expert-center/techtips/d--02/12/2001
    /// http://www.ccisolutions.com/StoreFront/category/channel-inserts?Origin=ArticlesBox
    /// http://www.kvraudio.com/forum/viewtopic.php?t=97120
    /// http://www.kvraudio.com/forum/viewtopic.php?t=97194
    ///                                       (08.07.2010.) (Domagoj Saric)
    DECLARE_PLUGIN_CAPABILITIES
    (
        Plugins::AsInsert          ,
        Plugins::AsSend            ,
        Plugins::Ch1in1out         ,
        Plugins::Ch2in1out         ,
        Plugins::Ch2in2out         ,
        Plugins::Ch4in2out         ,
        Plugins::Ch4in4out         ,
        Plugins::Ch8in4out         ,
        Plugins::Ch8in8out         ,
        Plugins::ReceiveVSTTimeInfo,
        Plugins::MixDryWet         ,
        Plugins::UsesAFixedSizeGUI
    );

public: // AU effect interface:
    std::uint8_t  numberOfInputChannels () const { return engineSetup().numberOfChannels();     }
    std::uint8_t  numberOfOutputChannels() const { return numberOfInputChannels();              }
    std::uint8_t  numberOfSideChannels  () const { return engineSetup().numberOfSideChannels(); }
    std::uint32_t processBlockSize      () const { return buffers().blockSize();                }

    float getSampleRate() const { return engineSetup().sampleRate<float>(); }

    void reset       ();
    void uninitialise();

public: // VST2.4 effect interface:
    void resume ();
    void suspend();

    bool setBlockSize ( unsigned int         blockSize  );
    bool setSampleRate( float        const & sampleRate );

public: // Shared plugin interface:
    bool initialise();

public:
    Engine::Setup const & engineSetup         () const;
    Engine::Setup const & uncheckedEngineSetup() const;

    Processor::LFO::Timer const & lfoTimer() const { return Engine::Processor::lfoTimer(); }

    Utility::CriticalSectionLock getProcessingLock() const;

    Engine::StorageFactors const & currentStorageFactors() const { return currentStorageFactors_; }

    static SpectrumWorxCore const & fromEngineSetup( Engine::Setup const & );

protected:
    LE_NOTHROW  SpectrumWorxCore();
#ifndef NDEBUG
    LE_NOTHROW ~SpectrumWorxCore() {}
#endif // NDEBUG

    void LE_NOTHROWNOALIAS process( float const * const * inputs, float const * const * pSideChannels, float * const * outputs, float const & outputGainScale, unsigned int samples );

    bool LE_NOTHROW updateEngineSetup();

public:
    using LFO        = LE::Parameters::LFOImpl;
    using Parameters = GlobalParameters::Parameters;

    Program       & program()       { LE_ASSUME( pProgram_ ); return *pProgram_; }
    Program const & program() const { LE_ASSUME( pProgram_ ); return *pProgram_; }

    Parameters       & parameters()       { return program().parameters(); }
    Parameters const & parameters() const { return program().parameters(); }

    ModuleChain       & moduleChain()       { return program().moduleChain(); }
    ModuleChain const & moduleChain() const { return program().moduleChain(); }

    void setProgram( Program & program ) { pProgram_ = &program; }

    Program const & dynamicParameterAccessContext() const { return program(); } //...mrmlj...for lack of implicit conversion to Program...

#if !LE_SW_GUI || LE_SW_SEPARATED_DSP_GUI
    typedef SW::ModuleDSP Module;
#else
    typedef SW::Module    Module;
#endif // LE_SW_SEPARATED_DSP_GUI

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
#endif // _MSC_VER
    struct ModuleInitialiser
    {
        typedef SpectrumWorxCore::Module Module;

        bool operator()( Module &, std::uint8_t moduleIndex ) const;

        Engine::StorageFactors const & storageFactors;
        SpectrumWorxCore       const & effect        ;
    }; // struct ModuleInitialiser
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

    ModuleInitialiser moduleInitialiser();


    ////////////////////////////////////////////////////////////////////////////
    // IO setup
    ////////////////////////////////////////////////////////////////////////////

    public:
        std::uint8_t numberOfChannels() const;

        enum struct IOChangeResult : std::uint8_t { Failed = false, Succeeded = true, NoChangeRequired };
        IOChangeResult setNumberOfChannels( std::uint8_t numberOfInputChannels, std::uint8_t numberOfOutputChannels );

    protected:
        void updateInputModeForIOConfig( std::uint8_t numberOfMainChannels, std::uint8_t numberOfSideChannels );

        void setReportedNumberOfChannels( std::uint8_t numberOfMainChannels, std::uint8_t numberOfSideChannels );

        static bool LE_NOTHROW checkChannelConfiguration( std::uint8_t numberOfInputChannels, std::uint8_t numberOfOutputChannels );
               bool LE_NOTHROW setNumberOfChannelsImpl  ( std::uint8_t numberOfMainChannels , std::uint8_t numberOfSideChannels   );

        bool haveSideChannel() const;

        void clearSideChannelData               ();
        void clearSideChannelDataIfNoSideChannel();

    #if LE_SW_ENGINE_INPUT_MODE >= 2
        static std::pair<std::uint8_t, std::uint8_t> ioChannels( GlobalParameters::InputMode::value_type );
    #endif // LE_SW_ENGINE_INPUT_MODE >= 2

    /* </IO configuration> */

    protected:
    public:
        template <class Parameter>
        static bool setGlobalParameter( Parameter & parameter, typename Parameter::param_type const newValue )
        {
            parameter.setValue( newValue );
            return true;
        }

        #pragma warning( push )
        #pragma warning( disable : 4389 ) // Signed/unsigned mismatch.
        template <class Parameter, class SWImpl> //...mrmlj...ugly duplication workaround...
        static bool LE_NOTHROW LE_FASTCALL setGlobalParameter( SWImpl & swImpl, typename Parameter::param_type const newValue )
        {
            Parameter & parameter( swImpl.parameters(). template get<Parameter>() );
            typename Parameter::value_type const oldValue( parameter );
        #if LE_SW_ENGINE_INPUT_MODE >= 2
            if ( std::is_same<Parameter, InputMode>::value ) { /*...IO mode can be "custom" which cannot be represented with the InputMode enum...*/ }
            else
        #endif // LE_SW_ENGINE_INPUT_MODE >= 2
            if ( newValue == oldValue )
                return true;
            bool const result( swImpl.setGlobalParameter( parameter, newValue ) );
            BOOST_ASSERT_MSG
            (
                (  result && ( parameter.getValue() == newValue ) ) ||
                ( !result && ( parameter.getValue() == oldValue ) ),
                "setGlobalParameter left the engine parameters and/or setup in an inconsistent state."
            );
            return result;
        }
        #pragma warning( pop )

        bool LE_FASTCALL setGlobalParameter( FFTSize        &, FFTSize       ::param_type newValue );
        bool LE_FASTCALL setGlobalParameter( OverlapFactor  &, OverlapFactor ::param_type newValue );
        bool LE_FASTCALL setGlobalParameter( WindowFunction &, WindowFunction::param_type newValue );
    #if LE_SW_ENGINE_INPUT_MODE >= 2
        static bool LE_FASTCALL setGlobalParameter( InputMode const &, InputMode::param_type );
    #endif // LE_SW_ENGINE_INPUT_MODE >= 2

protected:
    void updateForWindowChange( unsigned int window );

    bool currentThreadOwnsTheProcessLock() const;

    void moveModule( std::uint8_t sourceIndex, std::uint8_t targetIndex );

    InputBuffers       & buffers()       { return buffers_; }
    InputBuffers const & buffers() const { return buffers_; }

protected:
    bool blockAutomation() const
    {
        //...mrmlj...
        LE_ASSUME( Host2PluginInteropControler::blockAutomation() == false );
        return false;
    }

private:
    bool isEngineSetupUpToDate() const;

    void resetChannelBuffers();

    void LE_NOTHROW handleTimingInformationChange( LFO::Timer::TimingInformationChange );

    bool LE_NOTHROW resize( Engine::StorageFactors const & newfactors );

private: friend class Engine::Processor;
    static Engine::ModuleChainImpl & modules( Engine::Processor & processor ) { return static_cast<SpectrumWorxCore &>( processor ).moduleChain(); }

protected:
    class InputBuffers
    {
    public:
        using Channels = Utility::SharedStorageBuffer<Engine::real_t * LE_RESTRICT>;

        InputBuffers() : blockSize_( 0 ), forceSideChannel_( false ) {}
        bool resize( std::uint16_t blockSize, std::uint8_t numberOfMainChannels, std::uint8_t numberOfSideChannels );

        Engine::DataRange mainChannel( unsigned int const channel ) const { return Engine::DataRange( mainChannels_[ channel ], mainChannels_[ channel ] + blockSize() ); }
        Engine::DataRange sideChannel( unsigned int const channel ) const { return Engine::DataRange( sideChannels_[ channel ], sideChannels_[ channel ] + blockSize() ); }

        Channels const & mainChannels() const { return mainChannels_; }
        Channels const & sideChannels() const { return sideChannels_; }

        std::uint32_t blockSize() const;

        std::uint8_t numberOfMainChannels() const { return static_cast<std::uint8_t>( mainChannels_.size() ); }
        std::uint8_t numberOfSideChannels() const { return static_cast<std::uint8_t>( sideChannels_.size() ); }

        void forceSideChannel( bool const value ) { forceSideChannel_ = value; }

        bool operator!() const { return !storage_; }

    private:
        static void initializeChannelPointers( unsigned int blockBytes, Channels &, Engine::Storage & );

    private:
        Channels mainChannels_;
        Channels sideChannels_;
        std::uint16_t blockSize_;
        Engine::HeapSharedStorage storage_;

        //...mrmlj...
        bool forceSideChannel_;
    }; // class InputBuffers

    class ProcessLockUnlocker;

private:
    InputBuffers buffers_;

    Program * LE_RESTRICT pProgram_;

protected: //...mrmlj...
    mutable Utility::CriticalSection processCriticalSection_;
private:
    #ifndef NDEBUG
        bool suspended_;
    #endif // NDEBUG

    Engine::StorageFactors    currentStorageFactors_;
    Engine::HeapSharedStorage sharedStorage_        ;
}; // class SpectrumWorxCore


////////////////////////////////////////////////////////////////////////////////
///
/// \class SpectrumWorxCore::ProcessLockUnlocker
///
////////////////////////////////////////////////////////////////////////////////

class SpectrumWorxCore::ProcessLockUnlocker : boost::noncopyable
{
public:
    ProcessLockUnlocker( SpectrumWorxCore & effect ) : processCriticalSectionGuard_( effect.processCriticalSection_ ) {}
    LE_NOTHROWNOALIAS ~ProcessLockUnlocker() { processCriticalSectionGuard_.unlock(); }
private:
    Utility::CriticalSection & processCriticalSectionGuard_;
};

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // spectrumWorxCore_hpp
