////////////////////////////////////////////////////////////////////////////////
///
/// spectrumWorxCore.cpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "spectrumWorxCore.hpp"

#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
    #include "modules/moduleDSPAndGUI.hpp"
#else
    #include "modules/moduleDSP.hpp"
#endif // LE_SW_SEPARATED_DSP_GUI

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/lfo.hpp"
#include "le/utility/clear.hpp"
#include "le/utility/parentFromMember.hpp"
#include "le/utility/tchar.hpp"
#include "le/utility/trace.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp"

#include <boost/assert.hpp>
#include <boost/core/ignore_unused.hpp>

#if defined( _WIN32 ) && !defined( _XBOX )
    #include <windows.h> // CRITICAL_SECTION
#endif

#include <cstdlib>
#include <ctime>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

#if LE_SW_GUI
namespace GUI
{
    void LE_NOTHROW warningMessageBox( boost::string_ref title, boost::string_ref message, bool canBlock );
} // namespace GUI
#endif // LE_SW_GUI

namespace Engine
{
    ModuleChainImpl & Processor::modules() { return SpectrumWorxCore::modules( *this ); }
} // namespace Engine

////////////////////////////////////////////////////////////////////////////////
//
// SpectrumWorxCore UI elements definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const SpectrumWorxCore::name         [] = "SpectrumWorx" SW_EDITION_STRING;
char const SpectrumWorxCore::productString[] = "SpectrumWorx (Little Endian Ltd. (c) (tm))";


////////////////////////////////////////////////////////////////////////////////
//
// SpectrumWorxCore::SpectrumWorxCore()
// ------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW
SpectrumWorxCore::SpectrumWorxCore()
{
#ifndef NDEBUG
    Utility::Tracer::pTagString = "SW";
#endif // NDEBUG

    Utility::clear( currentStorageFactors_ );

    setReportedNumberOfChannels( maxNumberOfOutputs, maxNumberOfInputs - maxNumberOfOutputs );

#ifndef NDEBUG
    suspended_ = true;
#endif // NDEBUG
}


////////////////////////////////////////////////////////////////////////////////
//
// SpectrumWorxCore::process()
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROWNOALIAS
void SpectrumWorxCore::process /// \throws nothing
(
    float const * const *       pMainChannels,
    float const * const * const pSideChannels,
    float       * const * const outputs,
    float const &               outputGainScale,
    unsigned int          const samples
)
{
    if ( !processCriticalSection_.try_lock() )
        return;
    ProcessLockUnlocker const processingLockUnlocker( *this );

#ifdef _DEBUG
    // Implementation note:
    //   To aid in algorithm debugging we locally enable FPU exceptions.
    //                                        (01.07.2010.) (Domagoj Saric)
    Math::FPUExceptionsEnabler const fpuDebuggerGuard;
    try
    {
#endif // _DEBUG

    BOOST_ASSERT_MSG( samples <= buffers_.blockSize(), "Process called with a too large block size." );
    BOOST_ASSERT_MSG( !!buffers()                    , "Input buffers not initialised."              );

    // If gain needs to be applied to input data we first need to copy it to
    // internal buffers:
    using namespace GlobalParameters;
    float const & inputGain( parameters().get<InputGain>() );
    if ( !Math::is<1>( inputGain ) )
    {
        unsigned int const numberOfChannels( engineSetup().numberOfChannels() );
        BOOST_SIMD_STACK_BUFFER( mainChannels, float const *, numberOfChannels );
        for ( unsigned int channel( 0 ); channel < numberOfChannels; ++channel )
        {
            float * LE_RESTRICT const pChannel( buffers().mainChannel( channel ).begin() );
            Math::multiply( pMainChannels[ channel ], inputGain, pChannel, samples );
            mainChannels[ channel ] = pChannel;
        }
        pMainChannels = &mainChannels[ 0 ];
    }

    Engine::Processor::process
    (
        pMainChannels,
        pSideChannels,
        outputs,
        samples,
        parameters().get<OutputGain   >() * outputGainScale,
        parameters().get<MixPercentage>()
    );

#ifdef _DEBUG
    }
    catch ( ... )
    {
        static char const message[] = "A serious error has occurred. It is recommended that you save all your work and restart the host. Please report this inconvenience to Little Endian Ltd.";
        #if LE_SW_GUI
            GUI::warningMessageBox( MB_ERROR, message, false );
        #else
            std::puts( message );
        #endif // LE_SW_GUI
    }
#endif // DEBUG
}


void SpectrumWorxCore::suspend() // pause
{
    //...mrmlj...could be called on startup when the flag is already set by the
    //constructor...
    //BOOST_ASSERT( !suspended_ );
#ifndef NDEBUG
    suspended_ = true;
#endif // NDEBUG
}


void SpectrumWorxCore::resume()
{
#ifndef __APPLE__ //...mrmlj...isHost( "Vst2Au2" )
    BOOST_ASSERT( suspended_ );
#endif // __APPLE__
    reset();
#ifndef NDEBUG
    suspended_ = false;
#endif // NDEBUG
}


void SpectrumWorxCore::reset()
{
    Math::rngSeed();
    moduleChain().resetAll();
    resetChannelBuffers();
    Engine::Processor::reset();
}


bool SpectrumWorxCore::InputBuffers::resize
(
    std::uint16_t const blockSize,
    std::uint8_t  const numberOfMainChannels,
    std::uint8_t        numberOfSideChannels
)
{
    //...mrmlj...ugh...external audio problems ugly quick-fix...
    numberOfSideChannels = std::max<std::uint8_t>( numberOfSideChannels, forceSideChannel_ * numberOfMainChannels );

    if
    (
        ( blockSize            == this->blockSize           () ) &&
        ( numberOfMainChannels == this->numberOfMainChannels() ) &&
        ( numberOfSideChannels == this->numberOfSideChannels() )
    )
        return true;

    static_assert( std::is_pointer<float * LE_RESTRICT>::value, "" ); //...mrmlj...typeTraits.hpp debugging...
    using Utility::align;
    std::uint8_t  const numberOfChannels  ( numberOfMainChannels + numberOfSideChannels );
    std::uint8_t  const baseChannelStorage( sizeof( Channels::iterator )                );
    std::uint16_t const blockBytes        ( blockSize * sizeof( Engine::real_t )        );
    auto          const channelDataStorage( align( blockBytes )                         );
    auto          const requiredStorage
    (
        align( numberOfChannels * baseChannelStorage ) +
               numberOfChannels * channelDataStorage
    );

    if ( !storage_.resize( requiredStorage ) )
        return false;

    blockSize_ = blockSize;

    Engine::Storage storage( storage_ );
    mainChannels_.resize( numberOfMainChannels * baseChannelStorage, storage );
    sideChannels_.resize( numberOfSideChannels * baseChannelStorage, storage );
    initializeChannelPointers( blockBytes, mainChannels_, storage );
    initializeChannelPointers( blockBytes, sideChannels_, storage );
    BOOST_ASSERT_MSG( storage.size() < 16, "Requested storage not consumed." );

    return true;
}


void SpectrumWorxCore::InputBuffers::initializeChannelPointers( unsigned int const blockBytes, Channels & channels, Engine::Storage & storage )
{
    char * LE_RESTRICT pStorage( storage.begin() );
    for ( auto & pointer : channels )
    {
        typedef Channels::value_type pointer_t;
        pointer_t    const newBeginning  ( static_cast<pointer_t>( Math::align( pStorage ) ) );
        unsigned int const alignmentFixup( static_cast<unsigned int>( reinterpret_cast<char const *>( newBeginning ) - pStorage ) );
        pointer = newBeginning;
        pStorage += alignmentFixup + blockBytes;
        BOOST_ASSERT( pStorage <= storage.end() );
    }
    storage = Engine::Storage( pStorage, storage.end() );
}


unsigned int SpectrumWorxCore::InputBuffers::blockSize() const
{
    return blockSize_;
}


bool SpectrumWorxCore::ModuleInitialiser::operator()( Module & module, std::uint8_t /*const moduleIndex*/ ) const
{
    /// \note AUs can have their parameters changed while uninitialised.
    ///                                       (19.04.2013.) (Domagoj Saric)
    bool const initialised( storageFactors.complete() );
#if defined( _WIN32 ) || defined( LE_SW_FMOD )
    LE_ASSUME( initialised );
#endif // _WIN32
    if ( !initialised || module.resize( storageFactors ) )
    {
        /// \note resize() must also call reset() so we don't have to.
        ///                                   (05.04.2012.) (Domagoj Saric)
        //...mrmlj...preProcess() should not require initialisation/resizing (ChannelState allocation)...
        module.initialise( effect.engineSetup() );
        return true;
    }
    return false;
}

SpectrumWorxCore::ModuleInitialiser SpectrumWorxCore::moduleInitialiser() { return { currentStorageFactors(), *this }; }


////////////////////////////////////////////////////////////////////////////////
// IO setup
////////////////////////////////////////////////////////////////////////////////

std::uint8_t SpectrumWorxCore::numberOfChannels() const { return engineSetup().numberOfChannels(); }

LE_NOTHROW
SpectrumWorxCore::IOChangeResult SpectrumWorxCore::setNumberOfChannels( std::uint8_t const numberOfInputChannels, std::uint8_t const numberOfOutputChannels )
{
    if ( !checkChannelConfiguration( numberOfInputChannels, numberOfOutputChannels ) )
        return IOChangeResult::Failed;

    auto const numberOfMainChannels( numberOfOutputChannels                         );
    auto const numberOfSideChannels( numberOfInputChannels - numberOfOutputChannels );

    if
    (
        ( numberOfMainChannels == engineSetup().numberOfChannels    () ) &&
        ( numberOfSideChannels == engineSetup().numberOfSideChannels() )
    )
        return IOChangeResult::NoChangeRequired;

    return static_cast<IOChangeResult>( setNumberOfChannelsImpl( numberOfMainChannels, numberOfSideChannels ) );
}

LE_NOTHROW
bool SpectrumWorxCore::setNumberOfChannelsImpl( std::uint8_t const numberOfMainChannels, std::uint8_t const numberOfSideChannels )
{
    BOOST_ASSERT( currentThreadOwnsTheProcessLock() );

    if ( !buffers().resize( buffers().blockSize(), numberOfMainChannels, numberOfSideChannels ) )
        return false;

    Engine::StorageFactors newStorageFactors( currentStorageFactors() );
    newStorageFactors.numberOfChannels = numberOfMainChannels;
    if ( !resize( newStorageFactors ) )
    {
        BOOST_VERIFY( buffers().resize( buffers().blockSize(), engineSetup().numberOfChannels(), engineSetup().numberOfSideChannels() ) );
        return false;
    }

    updateInputModeForIOConfig ( numberOfMainChannels, numberOfSideChannels );
    setReportedNumberOfChannels( numberOfMainChannels, numberOfSideChannels ); //...mrmlj...spaghetti...
    return true;
}

LE_NOTHROW
bool SpectrumWorxCore::checkChannelConfiguration( std::uint8_t const numberOfInputChannels, std::uint8_t const numberOfOutputChannels )
{
#ifndef __APPLE__
    /// \note AU hosts can try any IO configuration (the auval tool specifically
    /// will). See the related note in
    /// PropertyHandler<kAudioUnitProperty_StreamFormat>::set() in the
    /// au/plugin.inl file.
    ///                                       (27.08.2014.) (Domagoj Saric)
    BOOST_ASSERT_MSG( !( numberOfInputChannels <     numberOfOutputChannels ), "SW cannot/does not downmix side channels." );
    BOOST_ASSERT_MSG( !( numberOfInputChannels > 2 * numberOfOutputChannels ), "SW cannot 'produce' channels."             );
#endif // __APPLE__

    bool canDoIt( ( numberOfInputChannels == numberOfOutputChannels ) || ( numberOfInputChannels == 2 * numberOfOutputChannels ) );
    LE_TRACE_IF( !canDoIt, "\tSW: requested an unsupported IO configuration (%u : %u).", numberOfInputChannels, numberOfOutputChannels );
    return canDoIt;
}


void SpectrumWorxCore::updateInputModeForIOConfig( std::uint8_t const numberOfMainChannels, std::uint8_t const numberOfSideChannels )
{
#if LE_SW_ENGINE_INPUT_MODE >= 1
    InputMode & inputMode( parameters().get<InputMode>() );
    switch ( numberOfMainChannels | numberOfSideChannels << 16 )
    {
        case 1 | 0 << 16: inputMode = InputMode::Mono           ; break;
        case 1 | 1 << 16: inputMode = InputMode::MonoSideChain  ; break;
        case 2 | 0 << 16: inputMode = InputMode::Stereo         ; break;
        case 2 | 2 << 16: inputMode = InputMode::StereoSideChain; break;
        default: /*custom*/ break;
    }
#else
    boost::ignore_unused( numberOfMainChannels, numberOfSideChannels );
#endif
}


void SpectrumWorxCore::setReportedNumberOfChannels( std::uint8_t const numberOfMainChannels, std::uint8_t const numberOfSideChannels )
{
    Engine::Processor::setNumberOfChannels( numberOfMainChannels, numberOfSideChannels );
}


bool SpectrumWorxCore::currentThreadOwnsTheProcessLock() const
{
#if defined( _WIN32 ) && !defined( _XBOX )
    CRITICAL_SECTION const & guard( reinterpret_cast<CRITICAL_SECTION const &>( processCriticalSection_ ) );
    //...mrmlj...FMOD...BOOST_ASSERT( guard.OwningThread || suspended_ );
    return
        !guard.OwningThread ||
        ( guard.OwningThread == reinterpret_cast<HANDLE>( ::GetCurrentThreadId() ) );
#else
    //...mrmlj...BOOST_ASSERT( !"Implement!" );
    return true;
#endif // _WIN32
}


bool SpectrumWorxCore::setBlockSize( unsigned int const newBlockSize )
{
    if ( newBlockSize == buffers().blockSize() )
        return true;
    BOOST_ASSERT( currentThreadOwnsTheProcessLock() );
    return buffers().resize
    (
        newBlockSize,
        engineSetup().numberOfChannels    (),
        engineSetup().numberOfSideChannels()
    );
}


bool SpectrumWorxCore::isEngineSetupUpToDate() const
{
    using namespace Engine;
    Parameters const & parameters( this->parameters() );
    return
    (
        ( uncheckedEngineSetup().fftSize                <unsigned int>() == parameters.get<FFTSize         >() ) &&
        ( uncheckedEngineSetup().windowOverlappingFactor<unsigned int>() == parameters.get<OverlapFactor   >() )
    #if LE_SW_ENGINE_WINDOW_PRESUM
     && ( uncheckedEngineSetup().windowSizeFactor                     () == parameters.get<WindowSizeFactor>() )
    #endif // LE_SW_ENGINE_WINDOW_PRESUM
    );
}


Engine::Setup const & SpectrumWorxCore::engineSetup() const
{
    BOOST_ASSERT
    (
        isEngineSetupUpToDate() ||
        !currentStorageFactors().complete()
    );
    return uncheckedEngineSetup();
}


Engine::Setup const & SpectrumWorxCore::uncheckedEngineSetup() const
{
    return Engine::Processor::engineSetup();
}


bool LE_NOTHROW SpectrumWorxCore::resize( Engine::StorageFactors const & newfactors )
{
    BOOST_ASSERT( currentThreadOwnsTheProcessLock() );
    return Engine::Processor::resize
    (
        currentStorageFactors_,
        newfactors,
        static_cast<Engine::Setup::Window>( parameters().get<WindowFunction>().getValue() ),
        sharedStorage_
    );
}


LE_NOTHROW
bool SpectrumWorxCore::updateEngineSetup()
{
    using namespace Engine;

    //...mrmlj...rethink this...BOOST_ASSERT( !isEngineSetupUpToDate() );
    Setup const & setup( uncheckedEngineSetup() );

    BOOST_ASSERT( currentThreadOwnsTheProcessLock() );

    Parameters & parameters( this->parameters() );

    BOOST_ASSERT( unsigned( setup.windowFunction() ) == parameters.get<WindowFunction>() );

#if defined( _DEBUG ) && LE_SW_ENGINE_INPUT_MODE >= 1
    {
        // Verify that the Engine::Setup and InputMode are synchronized (if a
        // non-custom InputMode is set).
        if ( ( setup.numberOfChannels() <= 2 ) && ( setup.numberOfSideChannels() <= 2 ) )
        {
            std::uint8_t numberOfMainChannels;
            std::uint8_t numberOfSideChannels;
            switch ( parameters.get<InputMode>().getValue() )
            {
                case InputMode::Mono           : numberOfMainChannels = 1; numberOfSideChannels = 0; break;
                case InputMode::MonoSideChain  : numberOfMainChannels = 1; numberOfSideChannels = 1; break;
                case InputMode::Stereo         : numberOfMainChannels = 2; numberOfSideChannels = 0; break;
                case InputMode::StereoSideChain: numberOfMainChannels = 2; numberOfSideChannels = 2; break;
                LE_DEFAULT_CASE_UNREACHABLE();
            }
            BOOST_ASSERT_MSG( setup.numberOfChannels    () == numberOfMainChannels, "Engine::Setup and InputMode out of sync" );
            BOOST_ASSERT_MSG( setup.numberOfSideChannels() == numberOfSideChannels, "Engine::Setup and InputMode out of sync" );
        }
    }
#endif // _DEBUG

    StorageFactors storageFactors
    (
        Processor::makeFactors
        (
            parameters.get<FFTSize         >(),
        #if LE_SW_ENGINE_WINDOW_PRESUM
            parameters.get<WindowSizeFactor>(),
        #endif // LE_SW_ENGINE_WINDOW_PRESUM
            parameters.get<OverlapFactor   >(),
            setup.numberOfChannels          (),
            setup.sampleRate<std::uint32_t> ()
        )
    );

    if ( resize( storageFactors ) )
        return true;

    // Restore previous settings on failure:
    parameters.set<FFTSize         >( setup.fftSize                <FFTSize      ::value_type>() );
    parameters.set<OverlapFactor   >( setup.windowOverlappingFactor<OverlapFactor::value_type>() );
#if LE_SW_ENGINE_WINDOW_PRESUM
    parameters.set<WindowSizeFactor>( setup.windowSizeFactor                                  () );
#endif // LE_SW_ENGINE_WINDOW_PRESUM
    updateInputModeForIOConfig( setup.numberOfChannels(), setup.numberOfSideChannels() );

    return false;
}


void SpectrumWorxCore::updateForWindowChange( unsigned int const window )
{
    BOOST_ASSERT( window == parameters().get<WindowFunction>() );
    /// \note AU can set a preset before initialise() (and thus
    /// updateEngineSetup()) is called.
    ///                                       (05.03.2013.) (Domagoj Saric)
    bool const storageFactorsComplete( currentStorageFactors().complete() );
#ifdef _WIN32
    LE_ASSUME( storageFactorsComplete );
#endif // _WIN32
    if ( storageFactorsComplete )
        Engine::Processor::changeWindowFunction( static_cast<Engine::Constants::Window>( window ) );
}


bool SpectrumWorxCore::setSampleRate( float const & sampleRate )
{
    return Engine::Processor::setSampleRate( sampleRate, currentStorageFactors_ );
}


bool SpectrumWorxCore::initialise()
{
    /// \note We do not abort early on error here to 'handle' bad protocols like
    /// VST2.4 (that do not allow a return code/error for the initialise() call)
    /// by performing as much initialisation as possible to minimize the chance
    /// of crashing the whole host if something fails here.
    ///                                       (05.03.2013.) (Domagoj Saric)
    bool success( true );
#if LE_SW_ENGINE_INPUT_MODE >= 1
    // Update/create the initial Engine::Setup and shared storage with the
    // default and/or so far partially set parameters.
	//...mrmlj...check if channel configuration has already been set up and skip
	//...mrmlj...the parameters-engine setup synchronization in that case
	//...mrmlj...a custom io mode might have been set and this would override it
	//...mrmlj...clean this up...
    if ( !currentStorageFactors().numberOfChannels )
    {
        auto const ioChannelsConfig( ioChannels( parameters().get<InputMode>() ) );
        success = ( setNumberOfChannels( ioChannelsConfig.first, ioChannelsConfig.second ) != IOChangeResult::Failed );
    }
    else
    {
        BOOST_ASSERT( currentStorageFactors().numberOfChannels == engineSetup().numberOfChannels() );
        success = true;
    }
    //...mrmlj...AU...BOOST_ASSERT_MSG( !!buffers(), "Input buffers not initialised." );
#endif // LE_SW_ENGINE_INPUT_MODE >= 1
    success &= updateEngineSetup();

    Math::rngSeed();

    return success;
}


void SpectrumWorxCore::uninitialise()
{
    BOOST_ASSERT_MSG( sharedStorage_, "Not initialised." );
    //Utility::clear( currentStorageFactors_ );
    setNumberOfChannelsImpl( 0, 0 );
    BOOST_ASSERT( currentStorageFactors_.numberOfChannels == 0 );
    BOOST_ASSERT( buffers().mainChannels().size() == 0 );
    BOOST_ASSERT( buffers().sideChannels().size() == 0 );
    BOOST_VERIFY( moduleChain().resizeAll( currentStorageFactors(), currentStorageFactors() ) );
    sharedStorage_.resize( 0 );
}


void SpectrumWorxCore::clearSideChannelData()
{
    BOOST_ASSERT( currentThreadOwnsTheProcessLock() );
    Engine::Processor::clearSideChannelData();
}


void SpectrumWorxCore::clearSideChannelDataIfNoSideChannel()
{
    if ( !haveSideChannel() )
        clearSideChannelData();
}


bool SpectrumWorxCore::haveSideChannel() const
{
    return engineSetup().hasSideChannel();
}


void SpectrumWorxCore::resetChannelBuffers()
{
#ifdef LE_SW_FMOD
    auto const lock( getProcessingLock() );
#endif // LE_SW_FMOD
    BOOST_ASSERT( currentThreadOwnsTheProcessLock() );
    Engine::Processor::resetChannelBuffers();
}


LE_NOTHROW
void SpectrumWorxCore::handleTimingInformationChange( LFO::Timer::TimingInformationChange const timingInformationChange )
{
    /// \note We assume that SpectrumWorxCore is used by protocols that do not
    /// provide tempo information.
    ///                                       (02.02.2012.) (Domagoj Saric)
    LE_ASSUME( !timingInformationChange.timingInfoChanged() );
}

//namespace GUI { bool isThisTheGUIThread(); bool isGUIInitialised(); }
SpectrumWorxCore const & SpectrumWorxCore::fromEngineSetup( Engine::Setup const & engineSetup )
{
    //BOOST_ASSERT_MSG( !GUI::isGUIInitialised() || !GUI::isThisTheGUIThread(), "Ambiguous!" ); //...mrmlj...
    return static_cast<SpectrumWorxCore const &>( Engine::Processor::fromEngineSetup( engineSetup ) );
}


void SpectrumWorxCore::moveModule( std::uint8_t const sourceIndex, std::uint8_t const targetIndex )
{
    moduleChain().moveModule( sourceIndex, targetIndex );
}


Utility::CriticalSectionLock LE_NOTHROW SpectrumWorxCore::getProcessingLock() const { return Utility::CriticalSectionLock( processCriticalSection_ ); }


#if LE_SW_ENGINE_INPUT_MODE >= 2
std::pair<std::uint8_t, std::uint8_t> SpectrumWorxCore::ioChannels( InputMode::value_type const ioMode )
{
    std::uint8_t numberOfInputChannels ;
    std::uint8_t numberOfOutputChannels;
    switch ( ioMode )
    {
        case InputMode::Mono           : numberOfInputChannels = 1; numberOfOutputChannels = 1; break;
        case InputMode::MonoSideChain  : numberOfInputChannels = 2; numberOfOutputChannels = 1; break;
        case InputMode::Stereo         : numberOfInputChannels = 2; numberOfOutputChannels = 2; break;
        case InputMode::StereoSideChain: numberOfInputChannels = 4; numberOfOutputChannels = 2; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    return std::make_pair( numberOfInputChannels, numberOfOutputChannels );
}
#endif // LE_SW_ENGINE_INPUT_MODE >= 2


bool SpectrumWorxCore::setGlobalParameter( FFTSize & parameter, FFTSize::param_type const newValue )
{
    Utility::CriticalSectionLock const processLock( this->getProcessingLock() );
    parameter.setValue( newValue );
    return updateEngineSetup();
}

bool SpectrumWorxCore::setGlobalParameter( OverlapFactor & parameter, OverlapFactor::param_type const newValue )
{
    Utility::CriticalSectionLock const processLock( this->getProcessingLock() );
    parameter.setValue( newValue );
    return updateEngineSetup();
}

bool SpectrumWorxCore::setGlobalParameter( WindowFunction & parameter, WindowFunction::param_type const newValue )
{
    parameter.setValue( newValue );
    updateForWindowChange( newValue );
    return true;
}

#if LE_SW_ENGINE_INPUT_MODE >= 2
#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
bool SpectrumWorxCore::setGlobalParameter( InputMode const &, InputMode::param_type )
{
    LE_UNREACHABLE_CODE(); //...mrmlj...
    return false;
}
#pragma warning( pop )
#endif // LE_SW_ENGINE_INPUT_MODE >= 2

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
#ifndef NDEBUG
LE_WEAK_SYMBOL extern char const assertionFailureMessageTitle[] = "SpectrumWorx internal error. Press Ok to ignore or Cancel to abort...";
#endif // NDEBUG
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
