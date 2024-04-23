////////////////////////////////////////////////////////////////////////////////
///
/// spectrumWorx.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "spectrumWorx.hpp"

#include "core/modules/moduleDSPAndGUI.hpp"
#include "gui/gui.hpp"

#if LE_SW_AUTHORISATION_REQUIRED
    #include "le/license_key/le_key_01_modulus.hpp"
    #include "le/licenser/cryptography/base64.hpp"
    #include "le/licenser/cryptography/signing.hpp"
    #include "le/licenser/license/predefinedItems.hpp"
#endif // LE_SW_AUTHORISATION_REQUIRED
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/fusionAdaptors.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/spectrumworx/presets.hpp"
#include "le/utility/parentFromMember.hpp"

// Boost sandbox
#include "boost/filesystem/directory_iterator.hpp"
#include "boost/mmap/mappble_objects/file/utility.hpp"

#include "boost/simd/preprocessor/stack_buffer.hpp" // NT2

#include <boost/assert.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/fusion/algorithm/iteration/for_each.hpp>
#include <boost/utility/in_place_factory.hpp>

#ifdef __GNUC__
    #include <cstdlib>
    #include <iconv.h>
#endif

#include <algorithm>
#if SW_ENABLE_UPGRADE
#include <set>
#endif // SW_ENABLE_UPGRADE
//------------------------------------------------------------------------------
#ifdef __APPLE__
    extern void const * swDLLAddress;
#endif // __APPLE__
#ifdef _WIN32
    extern "C" IMAGE_DOS_HEADER __ImageBase;
    static void const * const swDLLAddress( &__ImageBase );
#endif // _WIN32
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

LE_NOTHROW
SpectrumWorx::SpectrumWorx( bool const runningAsAU )
    :
#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    pListenerToNotifyWhenSampleLoaded_(                                  nullptr ),
#endif // LE_SW_DISABLE_SIDE_CHANNEL
    currentProgram_                   (                                        0 ),
#if LE_SW_ENGINE_INPUT_MODE >= 2
    inputModeToSetOnRestart_          ( static_cast<InputMode::value_type>( -1 ) ),
#endif // LE_SW_ENGINE_INPUT_MODE >= 2
    loadLastSessionOnStartup_         (                                    false )
#ifdef __APPLE__
   ,runningAsAU_                      ( runningAsAU                              )
#endif // __APPLE__
{
#ifndef __APPLE__
    LE_ASSUME( runningAsAU == false );
#endif // __APPLE__

    for ( auto & program : programs() )
        std::strcpy( &program.name()[ 0 ],"Empty" );

    SpectrumWorxCore::setProgram( programs()[ getProgram() ] );

    LE_LICENCE_SPECIFIC( setDemoGain(); )
}


LE_NOTHROW
SpectrumWorx::~SpectrumWorx()
{
    //...mrmlj...rethink this...
    if ( GUI::havePathsBeenInitialised() )
    {
    #if LE_SW_ENGINE_INPUT_MODE >= 2
        if ( inputModeToSetOnRestart_ != static_cast<InputMode::value_type>( -1 ) )
            parameters().set<InputMode>( inputModeToSetOnRestart_ );
    #endif // LE_SW_ENGINE_INPUT_MODE >= 2

        try { savePreset( lastSessionPresetFile().getFullPathName(), currentSampleFile(), juce::String::empty, program() ); } catch ( ... ) {}

        saveSettings();
    }

    LE_TRACE_IF( gui(), "\tSW: host destroyed the plugin w/o closing the GUI." );

#if LE_SW_AUTHORISATION_REQUIRED
    //stopLicenceVerificationTimer();
    authorizationThread_.kill();
#endif // LE_SW_AUTHORISATION_REQUIRED

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    BOOST_ASSERT( !pListenerToNotifyWhenSampleLoaded_ );
#endif // LE_SW_DISABLE_SIDE_CHANNEL
}


////////////////////////////////////////////////////////////////////////////////
//
// SpectrumWorx::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

namespace
{
    LE_NOTHROWNOALIAS
    float const * LE_RESTRICT getChannelDataChunk
    (
        Sample::ChannelData const &                   channelData,
        std::uint32_t             &                   startingPosition,
        std::uint16_t                                 chunkSize,
        float                     * LE_RESTRICT const workBuffer
    )
    {
        auto const dataSize( static_cast<std::uint32_t>( channelData.size() ) );
        BOOST_ASSERT( startingPosition <= dataSize );
        if ( dataSize > ( startingPosition + chunkSize ) )
        {
            float const * const pChunk( &channelData[ startingPosition ] );
            startingPosition += chunkSize;
            return pChunk;
        }
        else
        {
            float * workBufferPosition( workBuffer );
            while ( chunkSize )
            {
                if ( startingPosition == dataSize )
                    startingPosition = 0;
                auto const channelDataPosition( &channelData[ startingPosition ] );
                auto const amountToCopy       ( static_cast<std::uint16_t>( std::min<std::uint32_t>( dataSize - startingPosition, chunkSize ) ) );
                Math::copy( channelDataPosition, workBufferPosition, amountToCopy );
                workBufferPosition += amountToCopy;
                startingPosition   += amountToCopy;
                chunkSize          -= amountToCopy;
            }
            return workBuffer;
        }
    }
} // anonymous namespace

#pragma warning( push )
#pragma warning( disable : 4701 ) // Potentially uninitialized local variable 'samplePosition' used.

LE_NOTHROWNOALIAS
void SpectrumWorx::process /// \throws nothing
(
    float const * const * const inputs ,
    float       *       * const outputs,
    std::uint32_t         const samples
)
{
    // Implementation note:
    //   A normal lock cannot be used here because that could cause the GUI
    // thread to block the processing thread (e.g. while processing is active a
    // user changes the preset, which requires locking the process critical
    // section, that in turn, for whatever reason, causes a message box to pop
    // up while the lock is still held) and this seems to freeze certain hosts
    // (e.g. Reaper 3.22). Wavelab 5, VST Scanner and SoundForge 9.0 were found
    // not to suffer from this problem.
    //                                        (05.02.2010.) (Domagoj Saric)
    if ( !processCriticalSection_.try_lock() )
        return;
    ProcessLockUnlocker const processingLockUnlocker( *this );

    Math::FPUDisableDenormalsGuard const disableDenormals;

    // We give higher priority to external samples loaded through SW rather than
    // side channel data provided by the host:
    float const * const * pSideChannels;
    if ( hasExternalSample() )
    {
        auto const numberOfExternalAudioChannels( std::min<uint8_t>( engineSetup().numberOfChannels(), 2U ) );
        /// \note External samples currently force-load as stereo always so we
        /// must allow buffers().numberOfSideChannels() to be larger than
        /// numberOfExternalAudioChannels (stereo > mono).
        ///                                   (20.03.2013.) (Domagoj Saric)
        BOOST_ASSERT( numberOfExternalAudioChannels <= buffers().numberOfSideChannels() );
        BOOST_SIMD_STACK_BUFFER( sideChannels, float const *, numberOfExternalAudioChannels );
        std::uint32_t samplePosition;
        for ( std::uint8_t channel( 0 ); channel < numberOfExternalAudioChannels; ++channel )
        {
            samplePosition = sample_.samplePosition();
            sideChannels[ channel ] =
                getChannelDataChunk
                (
                    sample_.channel( channel ),
                    samplePosition,
                    samples,
                    buffers().sideChannel( channel ).begin()
                );
        }
        pSideChannels = &sideChannels[ 0 ];
        /// \todo Think of a smarter solution.
        ///                                   (08.02.2010.) (Domagoj Saric)
        BOOST_ASSERT( samplePosition != sample_.samplePosition() );
        sample_.samplePosition() = samplePosition;
    }
    else
    if ( engineSetup().hasSideChannel() )
    {
        pSideChannels = &inputs[ engineSetup().numberOfChannels() ];
        BOOST_ASSERT( *pSideChannels );
    }
    else
    {
        pSideChannels = nullptr;
    }

#if !LE_SW_AUTHORISATION_REQUIRED
    static float const outputGainScale_( 1 );
#endif // LE_SW_AUTHORISATION_REQUIRED
    SpectrumWorxCore::process( inputs, pSideChannels, outputs, outputGainScale_, samples );
}

#pragma warning( pop )

#if LE_SW_AUTHORISATION_REQUIRED
////////////////////////////////////////////////////////////////////////////////
// Authorization
////////////////////////////////////////////////////////////////////////////////

bool SpectrumWorx::initialisePathsAndVerifyLicence()
{
    BOOST_ASSERT( isCurrentlyCrippled() );
    startLicenceVerificationTimer();
    if ( GUI::initializePaths() )
    {
        /// \note The demoCripplingThread() function is responsible for removing
        /// the demo gain/attenuation when/if verifyLicence() loads a valid
        /// licence.
        ///                                   (28.08.2014.) (Domagoj Saric)
        verifyLicence();
        return true;
    }
    return false;
}


void SpectrumWorx::startLicenceVerificationTimer()
{
    Math::rngSeed();
    BOOST_VERIFY(( authorizationThread_.start<SpectrumWorx, &SpectrumWorx::demoCripplingThread>( *this ) ));
                   authorizationThread_.setDebugName( "Demo thread" );
}


void SpectrumWorx::stopLicenceVerificationTimer()
{
    if ( !authorised() )
    {
        BOOST_ASSERT( !"Not implemented" );//stopDemoTimer();
        setDemoGain  ();
    }
#ifndef NDEBUG
    else
    {
        // Implementation note:
        //   Verification that the demo limitations have been properly removed.
        // If the editor is closed quickly after authorizing it might happen
        // that the timer still hasn't cycled (and thus hasn't turned itself
        // off) in which case we must skip the check. We cannot simply wait for
        // the timer to turn itself off because we would block the GUI thread
        // effectively blocking the timer callback with it.
        //                                    (29.07.2010.) (Domagoj Saric)
        BOOST_ASSERT( authorizationThread_.isRunning() || !isCurrentlyCrippled() );
    }
#endif // NDEBUG
}


float SpectrumWorx::demoGain()
{
    // -25 dB -> pow( 10.0, -25.0/20.0 ) = 0.05623
    // -30 dB -> pow( 10.0, -30.0/20.0 ) = 0.03162
    // -35 dB -> pow( 10.0, -35.0/20.0 ) = 0.01778
    // -40 dB -> pow( 10.0, -40.0/20.0 ) = 0.01000
    // -45 dB -> pow( 10.0, -45.0/20.0 ) = 0.00562
    // -50 dB -> pow( 10.0, -50.0/20.0 ) = 0.00316

    return 0.01000f; // Using 40 dB.
}


void SpectrumWorx::   setDemoGain() { outputGainScale_ = demoGain(); }
void SpectrumWorx::removeDemoGain() { outputGainScale_ = 1.0f      ; }


bool SpectrumWorx::isCurrentlyCrippled() const
{
    BOOST_ASSERT( ( outputGainScale_ == 1.0f ) || ( outputGainScale_ == demoGain() ) );
    return !Math::is<1>( outputGainScale_ );
}


namespace
{
    void sleepMilliseconds( unsigned int const milliseconds )
    {
    #ifdef _WIN32
        ::Sleep ( milliseconds        );
    #else
        ::usleep( milliseconds * 1000 );
    #endif // _WIN32
    }

/*
    // http://stackoverflow.com/questions/11763786/timercallback-function-based-on-standard-template-library-without-boost
#ifdef _WIN32
    // http://blogs.msdn.com/b/oldnewthing/archive/2004/12/23/331246.aspx
    // http://stackoverflow.com/questions/772343/how-to-use-timers-in-windows
    HANDLE timer;
    ::CreateTimerQueueTimer( &timer, nullptr, &callback<EventHandler>, &handler, 20000, 0, WT_EXECUTEONLYONCE | WT_EXECUTEINTIMERTHREAD )
#else
    // http://stackoverflow.com/questions/16654544/background-thread-with-pthreads
    // https://developer.apple.com/library/mac/documentation/General/Conceptual/ConcurrencyProgrammingGuide/GCDWorkQueues/GCDWorkQueues.html
    // https://developer.apple.com/library/mac/documentation/performance/reference/gcd_libdispatch_ref/Reference/reference.html
    // http://en.wikipedia.org/wiki/Kqueue
    ::dispatch_source_t const pTimer( ::dispatch_source_create( DISPATCH_SOURCE_TYPE_TIMER, nullptr, 0, dispatch_get_main_queue() ) );
    if ( pTimer == nullptr )
        return false;
    ::dispatch_source_set_timer          ( pTimer, ::dispatch_walltime( nullptr, 0 ), interval, NSEC_PER_SEC / 1000 / 1000 * 10 );
    ::dispatch_source_set_event_handler_f( pTimer, * );
    ::dispatch_set_context               ( pTimer, &handler );
    ::dispatch_resume(timer);

    double delayInMilliseconds = 100.0;
    ::dispatch_time_t const popTime( ::dispatch_time( DISPATCH_TIME_NOW, (int64_t)(delayInMilliseconds * NSEC_PER_MSEC) );
    ::dispatch_after(popTime, dispatch_get_main_queue(), ^(void){});
#endif // _WIN32
*/
} // anonymous namespace

LE_NOTHROW LE_COLD
void SpectrumWorx::demoCripplingThread()
{
    BOOST_ASSERT_MSG( isCurrentlyCrippled(), "Licencing logic broken" );
    removeDemoGain();

    unsigned int const initialDelayBeforeDemoLimitting( 34567 );
    sleepMilliseconds( initialDelayBeforeDemoLimitting );

    unsigned int const crippledDuration( 2012 );
    unsigned int const minimumNonCrippledDuration
    (
    #ifdef _DEBUG
        45000
    #else
        25000
    #endif
    );
    unsigned int const maximumNonCrippledDuration
    (
    #ifdef _DEBUG
        85000
    #else
        45000
    #endif
    );

    while ( !authorised() )
    {
        unsigned int newDuration;
        if ( isCurrentlyCrippled() )
        {
            removeDemoGain();
            newDuration = Math::rangedRand( minimumNonCrippledDuration, maximumNonCrippledDuration );
        }
        else // not currently crippled
        {
            setDemoGain();
            newDuration = crippledDuration;
        }
        sleepMilliseconds( newDuration );
    }

    removeDemoGain();

    unsigned int const finalCrackObscuringWait( initialDelayBeforeDemoLimitting * 2 / 3 );
    sleepMilliseconds( finalCrackObscuringWait );

    BOOST_ASSERT_MSG( isCurrentlyCrippled() != ( authorised() != false ), "Licencing logic broken" );

    authorizationThread_.markAsDone(); //...mrmlj...ugh...
}


////////////////////////////////////////////////////////////////////////////////
//
// SpectrumWorx::verifyLicence()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \return An error string or nullptr if a valid licence is found
/// \throws none
///
/// http://www.seoxys.com/3-easy-tips-to-prevent-a-binary-crack/#checksum
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Move to a separate module.
///                                           (21.04.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

namespace // Private implementation details.
{
    //...mrmlj...anti-piracy hackery...to be documented if it proves useful...

    static std::size_t            volatile       globalHacked ( false               );
    static std::size_t volatile * volatile const pGlobalHacked( &globalHacked + 101 );

    namespace VerificationResults
    {
        static char const * volatile       noError( nullptr );
        static char const * volatile const previousVersion
        (
            "A valid licence found but for a previous version and/or for a different OS.\n"
            "None or insufficient valid upgrade licences found.\n"
            "Please provide a newer full licence or additional upgrade licence file(s)."
        );
        static char const * volatile const wrongOS
        (
            "A valid licence found but for a different OS.\n"
            "Please provide an additional upgrade licence file."
        );
        static char const * volatile const noFullLicence
        (
            "No valid full licences found."
        );
        static char const * volatile const noValidLicence
        (
            "Valid licence not found."
        );
        static char const * volatile const allocationFailure
        (
            "Resource allocation failure."
        );
    } // namespace VerificationResults

    namespace Constants
    {
        unsigned int const myVersion
        (
            ( SW_VERSION_MAJOR + '0' ) << 16 |
            ( SW_VERSION_MINOR + '0' ) <<  8 |
            ( SW_VERSION_PATCH + '0' ) <<  0
        );
    } // namespace Constants


    struct VersionInfo
    {
        std::uint32_t minimumTargetVersion ;
        std::uint32_t maximumTargetVersion ;
        std::uint32_t minimumUpgradeVersion;
        std::uint32_t maximumUpgradeVersion;

        void clear()
        {
            minimumTargetVersion = maximumTargetVersion = minimumUpgradeVersion = maximumUpgradeVersion = 0;
        }

        bool operator< ( VersionInfo const & other ) const
        {
            BOOST_ASSERT
            (
                ( minimumTargetVersion < other.minimumTargetVersion ) ==
                ( maximumTargetVersion < other.maximumTargetVersion )
            );
            return minimumTargetVersion < other.minimumTargetVersion;
        }
    }; // struct VersionInfo

    using Version = std::pair<std::uint32_t, boost::string_ref>;

    Version LE_FASTCALL extractVersion( Utility::XML::Object const & xmlElement )
    {
        auto const versionString( Utility::XML::value( xmlElement ) );
        if ( versionString.size() < 5 )
            return Version();

        if
        (
            ( versionString[ 1 ] != '.' ) ||
            ( versionString[ 3 ] != '.' )
        )
        {
            BOOST_ASSERT( !"Invalid version string in licence!" );
            return Version();
        }

        return Version
               (
                   ( versionString[ 0 ] << 16 ) |
                   ( versionString[ 2 ] <<  8 ) |
                   ( versionString[ 4 ] <<  0 ),
                   boost::string_ref( versionString.begin() + 5, versionString.end() - ( versionString.begin() + 5 ) )
               );
    }


    void LE_FASTCALL appendOptionalLicenceItemToJuceString
    (
        Utility::XML::Object const * const pItem,
        juce::String               &       targetString,
        char_t               const * const separatorString = 0
    )
    {
        if ( !pItem )
            return;
        auto const shareItUTF8StringRef( Utility::XML::value( *pItem ) );
        BOOST_ASSERT( !shareItUTF8StringRef.empty() );
        if ( targetString.isNotEmpty() )
        {
            BOOST_ASSERT( separatorString );
            targetString += separatorString;
        }
        targetString += juce::String::fromUTF8( shareItUTF8StringRef.begin(), static_cast<unsigned int>( shareItUTF8StringRef.size() ) );
    }

    class Verifyer
    {
    private:
        enum LicenceAuthorizationScope
        {
            Full,
            VersionUpgrade,
            OSUpgrade
        };

    public:
        Verifyer() : osUpgradeLicenceRequired_( false ) { maxFoundVersion_.clear(); }

        LE_NOTHROWNOALIAS ~Verifyer() = default;

        void findFullLicence()
        {
            auto directoryIterator
            (
                createDirectoryIterator( LE_MSVC_SPECIFIC( _T( "\\" ) ) _T( "*" ) SW_LICENCE_FILE_EXTENSION_STANDARD )
            );

            using Entry = boost::filesystem::directory_iterator::entry;

            while ( !!directoryIterator )
            {
                if ( checkFile( *this, directoryIterator->name(), Full ) )
                {
                    BOOST_ASSERT( authorizationData().authorised() );
                    if ( maxFoundVersion().maximumTargetVersion >= Constants::myVersion )
                        break;
                }
                ++directoryIterator;
            }
        }

        void findVersionUpgradeLicence()
        {
        #if SW_ENABLE_UPGRADE
            using UpgradeLicenceSet = std::set<VersionInfo>;

            BOOST_ASSERT( ( maxFoundVersion().minimumTargetVersion > 0 ) && "No full licence found yet." );

            VersionInfo const maxFoundFullVersion( maxFoundVersion() );
            UpgradeLicenceSet upgradeLicenceSet;

            auto directoryIterator
            (
                createDirectoryIterator( LE_MSVC_SPECIFIC( _T( "\\" ) ) _T( "*" ) SW_LICENCE_FILE_EXTENSION_VERSION_UPGRADE )
            );

            while ( !!directoryIterator )
            {
                if ( checkFile( *this, directoryIterator->name(), VersionUpgrade ) )
                {
                    BOOST_ASSERT( authorizationData().authorised() );
                    upgradeLicenceSet.insert( maxFoundVersion() );
                }
                ++directoryIterator;
            }

            VersionInfo const * pCurrentVersionInfo( &maxFoundFullVersion );
            for ( auto const & nextVersionInfo : upgradeLicenceSet )
            {
                // Implementation note:
                //   The next version in the chain has to link to the current
                // one so that the current maximum target version fits within
                // the upgrade range of the next licence. This means that the
                // current maximum/target version has to be exactly greater
                // than the minimum upgrade version of the next licence because
                // of the semi-open range rule (the current licence authorizes
                // up to but not including the maximum target version). It also
                // means that the current maximum target version has to be
                // less-than-or-equal to the maximum upgrade version of the next
                // licence, the 'or-equal' part is allowed because of the
                // semi-open range rule (because the upper limit, the maximum
                // target version, is not included in the range,
                // less-than-or-equal still excludes the version specified by
                // the maximumTargetVersion from the upgrade range).
                //                            (23.04.2010.) (Domagoj Saric)
                if
                (
                    pCurrentVersionInfo->maximumTargetVersion >  nextVersionInfo.minimumUpgradeVersion &&
                    pCurrentVersionInfo->maximumTargetVersion <= nextVersionInfo.maximumUpgradeVersion
                )
                {
                    maxFoundVersion_ = nextVersionInfo;
                    if ( currentVersionEnough() )
                    {
                        return;
                    }
                    else
                    {
                        // Chain still valid/unbroken but we have not yet
                        // reached the required version...
                        pCurrentVersionInfo = &nextVersionInfo;
                    }
                }
                else
                {
                    // Broken chain...
                    break;
                }
            }
        #endif // SW_ENABLE_UPGRADE
        }


        char_t const & saveAuthorizationDataIfVersionOK( AuthorisationData & targetData ) const
        {
            LE_ASSUME( !targetData.authorised() );

            unsigned const versionOK( currentVersionEnough() );
            //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
            AuthorisationData * LE_RESTRICT const targetAuthorizationDataAntiHack[] =
            {
                &targetData,
                &const_cast<AuthorisationData &>( authorizationData_ )
            };
            targetData.swap( *targetAuthorizationDataAntiHack[ versionOK ] );
            if ( versionOK )
            {
                LE_ASSUME(  maxFoundVersion_.minimumTargetVersion <= Constants::myVersion );
                LE_ASSUME( !authorizationData_.authorised() );
                LE_ASSUME(  targetData        .authorised() );
            }
            else
            {
                LE_ASSUME( !authorizationData_.authorised() );
                LE_ASSUME( !targetData        .authorised() );
            }

            return targetData.authorised();
        }


        bool appropriateLicenceFoundButForAnotherOS() const { return osUpgradeLicenceRequired_; }

        VersionInfo       const & maxFoundVersion  () const { return maxFoundVersion_  ; }
        AuthorisationData       & authorizationData()       { return authorizationData_; }

    private:
        bool findOSUpgradeLicence()
        {
            auto directoryIterator
            (
                createDirectoryIterator( LE_MSVC_SPECIFIC( _T( "\\" ) ) _T( "*" ) SW_LICENCE_FILE_EXTENSION_OS_UPGRADE )
            );

            while ( !!directoryIterator )
            {
                if ( checkFile( *this, directoryIterator->name(), OSUpgrade ) )
                {
                    BOOST_ASSERT( authorizationData().authorised() );
                    return true;
                }
                ++directoryIterator;
            }

            osUpgradeLicenceRequired_ = true;
            return false;
        }

        unsigned currentVersionEnough() const
        {
            // Development version licences target only a specific version while
            // retail version licences target a semi-open range of versions.
            //unsigned const versionOK
            //(
            //    ( SW_IS_RETAIL && SW_ENABLE_UPGRADE )
            //        ? ( maxFoundVersion_.maximumTargetVersion >  Constants::myVersion )
            //        : ( maxFoundVersion_.maximumTargetVersion >= Constants::myVersion )
            //);
              signed const versionDiff    ( maxFoundVersion_.maximumTargetVersion - Constants::myVersion );
              signed const comparisonValue( ( SW_IS_RETAIL && SW_ENABLE_UPGRADE ) ? 0 : -1               );
            unsigned const versionOK      ( versionDiff > comparisonValue                                );
            return versionOK - ( *( pGlobalHacked - 101 ) != 0 );
        }

        static boost::filesystem::filtered_directory_iterator createDirectoryIterator( char_t const * const licenceExtension )
        {
            return
            {
            //...mrmlj...BOOST_FS_MAKE_FILTERED_DIRECTORY_ITERATOR( GUI::licencesPath().getFullPathName().getCharPointer(), licenceExtension )
            #ifdef BOOST_WINDOWS_API
                ( GUI::licencesPath().getFullPathName() + licenceExtension ).getCharPointer().getAddress()
            #else
                GUI::licencesPath().getFullPathName().getCharPointer(), licenceExtension
            #endif // BOOST_WINDOWS_API
            };
        };

    private: friend bool checkFile( Verifyer &, char_t const * file, LicenceAuthorizationScope ); //...mrmlj...
        VersionInfo       maxFoundVersion_         ;
        AuthorisationData authorizationData_       ;
        bool              osUpgradeLicenceRequired_;
    }; // class Verifyer

    #pragma warning( push )
    #pragma warning( disable : 4127 ) // Conditional expression is constant.
    #pragma warning( disable : 4512 ) // LicenceElements : assignment operator could not be generated

    bool LE_NOTHROW checkFile
    (
        Verifyer                                  &       verifyer,
        char_t                              const * const fileName,
        Verifyer::LicenceAuthorizationScope const         licenceAuthorizationType
    )
    {
        bool const upgradeLicence( licenceAuthorizationType == Verifyer::VersionUpgrade );
    #if !SW_ENABLE_UPGRADE
        LE_ASSUME( !upgradeLicence );
    #endif

        auto & maxFoundVersion  ( verifyer.maxFoundVersion_    );
        auto & authorizationData( verifyer.authorizationData() );

        ////////////////////////////////////////////////////////////////////////
        // Parsing phase...
        ////////////////////////////////////////////////////////////////////////

        try
        {
            /// \note 'Old'/original licence verification code: cannot yet fully
            /// utilize Licenser SDK functionality because its licence file
            /// format diverged:
            /// - new licences have a root element ("LE.License")
            /// - old licences separate the XML and the signature with a null
            /// (which is included in the signature calculation) while new ones
            /// include the signature as an XML comment (which is created and
            /// appended to the XML _after_ the signature generation).
            ///                               (26.04.2016.) (Domagoj Saric)
            boost::mmap::mapped_view<char const> const licenceFile
            (
                boost::mmap::map_read_only_file( GUI::licencesPath().getChildFile( fileName ).getFullPathName().getCharPointer() )
            );
            if ( !licenceFile )
                return false;

            auto const xmlSize( static_cast<std::uint16_t>( std::strlen( licenceFile.begin() ) ) );
            BOOST_ASSERT_MSG( xmlSize < unsigned( licenceFile.size() ), "Invalid licence file - no signature" );

            namespace Cryptography = Lic::Cryptography;
            Cryptography::Signature signature;
            {
                auto const pEncodedSignature   ( licenceFile.begin() + xmlSize + 1 );
                auto       encodedSignatureSize( static_cast<std::uint16_t>( licenceFile.end() - pEncodedSignature ) );
                BOOST_VERIFY_MSG
                (
                    Cryptography::Base64::decode
                    (
                        pEncodedSignature, encodedSignatureSize,
                        &signature[ 0 ]  , static_cast<std::uint16_t>( signature.size() )
                    )
                        ==
                    signature.size(),
                    "Invalid license - signature decoding failed"
                );
            }

            auto const verificationSuccess
            (
                Cryptography::verifySignature
                (
                    boost::make_iterator_range( &licenceFile[ Lic::utf8BOM.size() ], &licenceFile[ xmlSize + 1 ] ),
                    signature,
                    Lic::PregeneratedKeys::le_key_01_modulus
                )
            );

            /// \note We have to copy the licence XML data because of RapidXML's
            /// destructive parsing.
            ///                               (21.10.2013.) (Domagoj Saric)
            BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( xmlLicence, char, xmlSize + 1 );
            std::memcpy( xmlLicence.begin(), licenceFile.begin(), xmlLicence.size() );
            //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
            char * const xmlBeginnings[] = { xmlLicence.begin(), /*bogus 'crash on me' pointer*/const_cast<char *>( &licenceFile[ xmlSize + 1 ] ) };

            Utility::XML::Document licence;
            licence.parse( xmlBeginnings[ !verificationSuccess ] );

            auto const & productDataNode ( *licence.element( "ProductData"  ) );
            auto const & licenseeDataNode( *licence.element( "LicenseeData" ) );

            using namespace Lic::PredefinedItems;
            using namespace Utility;
            #pragma warning( push )
            #pragma warning( disable : 4510 ) // Default constructor could not be generated.
            #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
            struct LicenceElements
            {
                XML::Attribute const * const pMinUpgradeVersionNode;
                XML::Attribute const * const pMaxUpgradeVersionNode;

                XML::Attribute const * const pNameNode       ;
                XML::Attribute const * const pTargetOSNode   ;
                XML::Attribute const *       pLicenceTypeNode;
                XML::Attribute const * const pMinVersionNode ;
                XML::Attribute const * const pMaxVersionNode ;

                XML::Attribute const * const pRegName  ;
                XML::Attribute const * const pFirstName;
                XML::Attribute const * const pLastName ;
                XML::Attribute const * const pCompany  ;
                XML::Attribute const * const pEmail    ;
                XML::Attribute const * const pStreet   ;
                XML::Attribute const * const pCity     ;
                XML::Attribute const * const pState    ;
                XML::Attribute const * const pCountry  ;
            } /*const*/ licenceElements =
            {
                productDataNode .attribute( ProductData::minimumUpgradeVersion ),
                productDataNode .attribute( ProductData::maximumUpgradeVersion ),

                productDataNode .attribute( "ProductName"                     ),
                productDataNode .attribute( ProductData::targetOS             ),
                productDataNode .attribute( ProductData::licenseType          ),
                productDataNode .attribute( ProductData::minimumTargetVersion ),
                productDataNode .attribute( ProductData::maximumTargetVersion ),

                licenseeDataNode.attribute( LicenseeData::registeredName ),
                licenseeDataNode.attribute( LicenseeData::firstName      ),
                licenseeDataNode.attribute( LicenseeData::lastName       ),
                licenseeDataNode.attribute( LicenseeData::company        ),
                licenseeDataNode.attribute( LicenseeData::email          ),
                licenseeDataNode.attribute( LicenseeData::street         ),
                licenseeDataNode.attribute( LicenseeData::city           ),
                licenseeDataNode.attribute( LicenseeData::state          ),
                licenseeDataNode.attribute( LicenseeData::country        )
            };
            #pragma warning( pop )

            ////////////////////////////////////////////////////////////////////
            // Verification phase...
            ////////////////////////////////////////////////////////////////////

            if ( XML::value( *licenceElements.pNameNode ) != "SpectrumWorx" )
                return false;

            {
                if ( !upgradeLicence && ( licenceElements.pMinUpgradeVersionNode || licenceElements.pMaxUpgradeVersionNode ) )
                    return false;

                // Implementation note:
                //   Verify that all required items are present. For non upgrade
                // licences we allow upgrade nodes to be empty. The licence type
                // node is also allowed to be empty (which is interpreted as a
                // full commercial licence) in order to accept the first public
                // licences that did not have that information.
                //                            (12.11.2010.) (Domagoj Saric)

                if ( !licenceElements.pLicenceTypeNode )
                {
                    struct DefaultLicenceType : XML::Attribute
                    {
                        DefaultLicenceType() { XML::Attribute::value( LicenseTypes::fullCommercial.begin(), LicenseTypes::fullCommercial.length() ); }
                    };
                    static DefaultLicenceType const defaultLicenceType;
                    licenceElements.pLicenceTypeNode = &defaultLicenceType;
                }

                auto const licenceElementsBegin( upgradeLicence ? &licenceElements.pMinUpgradeVersionNode : &licenceElements.pNameNode );
                auto const licenceElementsEnd  ( &licenceElements.pCompany                                                             );
                if ( std::find( licenceElementsBegin, licenceElementsEnd, nullptr ) != licenceElementsEnd )
                    return false;
            }

            Version const minimumVersion( extractVersion( *licenceElements.pMinVersionNode ) );
            Version const maximumVersion( extractVersion( *licenceElements.pMaxVersionNode ) );
            switch ( licenceAuthorizationType )
            {
                case Verifyer::Full:
                case Verifyer::VersionUpgrade:
                {
                    if ( Constants::myVersion < minimumVersion.first )
                        return false; // Meant for a later version...
                    if ( maxFoundVersion.maximumTargetVersion > maximumVersion.first )
                        return false; // A later licence already found...
                    break;
                }

                case Verifyer::OSUpgrade:
                {
                    // OS upgrade licences must match the target version exactly...
                    BOOST_ASSERT( !maxFoundVersion.minimumUpgradeVersion );
                    BOOST_ASSERT( !maxFoundVersion.maximumUpgradeVersion );
                    if
                    (
                        ( maxFoundVersion.minimumTargetVersion != minimumVersion.first ) ||
                        ( maxFoundVersion.maximumTargetVersion != maximumVersion.first )
                    )
                        return false;
                    break;
                }

                LE_DEFAULT_CASE_UNREACHABLE();
            }

            bool const retail( SW_IS_RETAIL );
            BOOST_ASSERT( !( !retail && upgradeLicence ) );
            if ( retail )
            {
                if ( !minimumVersion.second.empty() || !maximumVersion.second.empty() )
                    return false;

                if
                (
                    ( !licenceElements.pMinUpgradeVersionNode != !licenceElements.pMaxUpgradeVersionNode ) ||
                    ( !licenceElements.pMinUpgradeVersionNode != !upgradeLicence                         )
                )
                    return false;

                if ( upgradeLicence )
                {
                    BOOST_ASSERT( licenceElements.pMinUpgradeVersionNode );
                    BOOST_ASSERT( licenceElements.pMaxUpgradeVersionNode );
                    Version const minimumUpgradeVersion( extractVersion( *licenceElements.pMinUpgradeVersionNode ) );
                    Version const maximumUpgradeVersion( extractVersion( *licenceElements.pMaxUpgradeVersionNode ) );
                    BOOST_ASSERT( minimumUpgradeVersion.second.empty() );
                    BOOST_ASSERT( maximumUpgradeVersion.second.empty() );
                    maxFoundVersion.minimumUpgradeVersion = minimumUpgradeVersion.first;
                    maxFoundVersion.maximumUpgradeVersion = maximumUpgradeVersion.first;
                }
            }
            else
            if
            (
                ( licenceElements.pMinUpgradeVersionNode != 0         ) ||
                ( licenceElements.pMaxUpgradeVersionNode != 0         ) ||
                ( minimumVersion        != maximumVersion             ) ||
                ( minimumVersion.first  != Constants::myVersion       ) ||
                ( minimumVersion.second != " " SW_VERSION_DESCRIPTION )
            )
                return false;

            {
                auto const licenceOSList( XML::value( *licenceElements.pTargetOSNode ) );
                switch ( licenceAuthorizationType )
                {
                    case Verifyer::Full:
                    {
                        if ( licenceOSList.find( OSTypes::currentOS ) == licenceOSList.npos )
                        {
                            // Licence for a different OS, search for an appropriate
                            // upgrade licence for owners of a full commercial
                            // licence...

                            bool const fullCommercialLicence
                            (
                                XML::value( *licenceElements.pLicenceTypeNode )
                                    ==
                                LicenseTypes::fullCommercial
                            );
                            if ( !fullCommercialLicence )
                                return false;

                            //...mrmlj...clean this up...
                            VersionInfo maxFoundVersionBackup( maxFoundVersion );
                            maxFoundVersion.minimumTargetVersion = minimumVersion.first;
                            maxFoundVersion.maximumTargetVersion = maximumVersion.first;

                            bool const foundOSUpgradeLicence( verifyer.findOSUpgradeLicence() );
                            maxFoundVersion = maxFoundVersionBackup;
                            if ( !foundOSUpgradeLicence )
                                return false;
                        }
                        break;
                    }

                    case Verifyer::VersionUpgrade:
                    {
                        if ( licenceOSList.find( OSTypes::currentOS ) == licenceOSList.npos )
                        {
                            // Licence for a different OS, OS upgrade licences
                            // are not provided/allowed for version upgrade
                            // licences...
                            return false;
                        }
                        break;
                    }

                    case Verifyer::OSUpgrade:
                        if ( licenceOSList.find( OSTypes::upgrade ) == licenceOSList.npos )
                            return false;
                        break;

                    LE_DEFAULT_CASE_UNREACHABLE();
                }
            }

            if ( !upgradeLicence || ( maximumVersion.first > maxFoundVersion.maximumTargetVersion ) )
            {
                authorizationData.clear();

                appendOptionalLicenceItemToJuceString( licenceElements.pCompany, authorizationData.line( 2 ) );
                if ( authorizationData.line( 2 ).isEmpty() )
                    appendOptionalLicenceItemToJuceString( licenceElements.pStreet, authorizationData.line( 2 ) );

                appendOptionalLicenceItemToJuceString( licenceElements.pCity   , authorizationData.line( 3 )             );
                appendOptionalLicenceItemToJuceString( licenceElements.pState  , authorizationData.line( 3 ), _T( ", " ) );
                appendOptionalLicenceItemToJuceString( licenceElements.pCountry, authorizationData.line( 3 ), _T( ", " ) );

                appendOptionalLicenceItemToJuceString( licenceElements.pEmail  , authorizationData.line( 4 ) );

                appendOptionalLicenceItemToJuceString( licenceElements.pFirstName, authorizationData.line( 1 )            );
                appendOptionalLicenceItemToJuceString( licenceElements.pLastName , authorizationData.line( 1 ), _T( " " ) );

                appendOptionalLicenceItemToJuceString( licenceElements.pLicenceTypeNode, authorizationData.licenceType() );
            }

            maxFoundVersion.minimumTargetVersion = minimumVersion.first;
            maxFoundVersion.maximumTargetVersion = maximumVersion.first;

            BOOST_ASSERT( authorizationData.authorised() );
            BOOST_ASSERT( Constants::myVersion >= minimumVersion.first );
            return true;
        }
        catch ( ... )
        {
            return false;
        }
    } // checkFile()

    using hack_float =
        boost::mpl::if_c
        <
            (sizeof( std::ptrdiff_t ) > sizeof( float )),
            double,
            float
        >::type volatile;

    //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
    static unsigned char const * const pCheckFile   ( static_cast<unsigned char const *>( reinterpret_cast<void const *>( &checkFile ) ) + 1   );
    //...mrmlj...if written as ( pCheckFile + 960 ) MSVC 10 generates a dynamic initializer...
    static unsigned char const * const pCheckFileEnd( static_cast<unsigned char const *>( reinterpret_cast<void const *>( &checkFile ) ) + 960 );

    static auto const verifyAddress( &Lic::Cryptography::verifySignature );

    template <typename SearchValue>
#ifndef _DEBUG
    LE_FORCEINLINE
#endif // _DEBUG
    void antiHackSearch
    (
        unsigned char  const * const pBegin,
        unsigned char  const * const pEnd,
        SearchValue            const searchValue,
        std::ptrdiff_t       &       statusValue
    )
    {
        BOOST_ASSERT( statusValue == 0 );

        unsigned char const * pByte( pBegin );
        while ( pByte != pEnd )
        {
            unsigned char const * const pCurrentPosition( pByte++ );
          //SearchValue const currentValue( *reinterpret_cast<SearchValue const *>( pCurrentPosition ) );...mrmlj...strict aliasing/-fcatch-undefined-behaviour
            SearchValue currentValue;
            std::memcpy( &currentValue, pCurrentPosition, sizeof( currentValue ) );
            int        const found     ( currentValue == searchValue );
            hack_float const found_hack( reinterpret_cast<hack_float const &>( found ) );
            reinterpret_cast<hack_float &>( statusValue ) += found_hack;
            BOOST_ASSERT( statusValue == 0 );
        }

        BOOST_ASSERT( statusValue == 0 );
    }
} // anonymous namespace

LE_NOTHROW
char const * SpectrumWorx::verifyLicence()
{
    static_assert( !( !SW_IS_RETAIL && SW_ENABLE_UPGRADE ), "You should disable upgrades for non-retail builds..." );

    BOOST_ASSERT( !authorised() );

    try
    {
        Verifyer verifyer;

        //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
        //...mrmlj...in debug (and sometimes in Mac release) builds we might run into two consecutive 0x90 bytes...
        hack_float const nop      ( 0x90   );
        hack_float const doubleNop( 0x9090 );
        boost::ignore_unused( nop, doubleNop );
    #if ( defined( _DEBUG ) || ( defined( __APPLE__ ) && __LP64__ ) || defined( _WIN64 ) ) //...mrmlj...
        std::ptrdiff_t nopAddress( reinterpret_cast<std::ptrdiff_t>( std::wmemchr( static_cast<wchar_t const *>( *static_cast<void const * const *>( static_cast<void const *>( &verifyAddress ) ) ) + 1, Math::convert<wchar_t >( doubleNop ),  50 ) ) );
    #else
        std::ptrdiff_t nopAddress( reinterpret_cast<std::ptrdiff_t>( std::memchr ( static_cast<char    const *>( *static_cast<void const * const *>( static_cast<void const *>( &verifyAddress ) ) ) + 1, Math::convert<unsigned>( nop       ), 100 ) ) );
    #endif // _DEBUG
        BOOST_ASSERT( nopAddress == 0 );

    #if defined( _DEBUG )
    #elif defined( _WIN64 )
        reinterpret_cast<hack_float &>( nopAddress ) += ( std::count( pCheckFile, pCheckFileEnd, nop ) - 4 );
    #elif defined( __APPLE__ ) && __LP64__
        reinterpret_cast<hack_float &>( nopAddress ) += ( std::count( pCheckFile, pCheckFileEnd, nop ) - 0 );
    #else
        antiHackSearch( pCheckFile, pCheckFileEnd, Math::convert<std::uint16_t>( doubleNop ), nopAddress );
    #endif // _DEBUG

        verifyer.findFullLicence();

        using VerificationResults::noError;
        static std::ptrdiff_t volatile hacked( false );
        {
            //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
            namespace Cryptography = Lic::Cryptography;
            auto const   dummyLicence  ( boost::make_iterator_range_n( reinterpret_cast<char const *>( this ) + 543, 1999 ) );
            auto const & dummySignature( static_cast<Cryptography::Signature const *>( ::swDLLAddress )[ 77 ] );
            auto const verificationResult
            (
                Cryptography::verifySignature
                (
                    dummyLicence,
                    dummySignature,
                    Lic::PregeneratedKeys::le_key_01_modulus
                )
            );
            BOOST_ASSERT_MSG( !verificationResult, "Must-fail anti-hack verifySignature() test failed!" );

            char const * volatile & pNoErr = const_cast<char const * volatile &>( noError );
                                            hacked   -= Math::convert<std::ptrdiff_t>( verificationResult * /*reinterpret_cast<hack_float const &>( dummyDataSizeAndLessObviousResultReturn.outputResult ) **/ reinterpret_cast<hack_float const &>( nopAddress ) );
            static std::ptrdiff_t decoy;    decoy    += hacked - ( /*dummyDataSizeAndLessObviousResultReturn.outputResult +*/ nopAddress );
            reinterpret_cast<hack_float &>( pNoErr ) += hacked;
            reinterpret_cast<hack_float &>( pNoErr ) += nopAddress;
        }

        AuthorisationData * const targetAuthorizationDataAntiHack[] =
        {
            &this->authorizationData_,
            &verifyer.authorizationData()
        };
        BOOST_ASSERT( !targetAuthorizationDataAntiHack[ hacked ]->authorised() );

        //...mrmlj...anti-piracy hackery...to be documented if it proves useful...
        {
        /* //...mrmlj...libtomcrypt leftovers...reimplement this check for our new code...
            unsigned char         const retInstruction( 0xC3 );
            unsigned char const * const pRet          ( std::find( ppkcs_1_pss_decode, ppkcs_1_pss_decode_end, retInstruction ) );
            std::ptrdiff_t              retAddress    ( reinterpret_cast<std::ptrdiff_t>( ( pRet != ppkcs_1_pss_decode_end ) ? pRet : 0 ) );
            #ifndef NDEBUG //...mrmlj...false assertion failures in debug builds
                retAddress = 0;
            #endif
            BOOST_ASSERT( retAddress == 0 );
            hack_float const orInstruction ( 0x830801 );
            hack_float const jmpInstruction( 0xE976   );
            antiHackSearch( ppkcs_1_pss_decode, ppkcs_1_pss_decode_end, Math::convert<unsigned int  >( orInstruction  ), retAddress );
            antiHackSearch( ppkcs_1_pss_decode, ppkcs_1_pss_decode_end, Math::convert<unsigned short>( jmpInstruction ), retAddress );
        */
            reinterpret_cast<hack_float &>( globalHacked ) =
                ( reinterpret_cast<hack_float const &>( hacked     ) * 1 ) +
                ( reinterpret_cast<hack_float const &>( noError    ) * 2 ) /*+
                ( reinterpret_cast<hack_float const &>( retAddress ) * 3 ) +
                    +
                ( 2 * reinterpret_cast<hack_float const &>( retAddress ) )*/;
        }

        BOOST_ASSERT( globalHacked == false   );
        BOOST_ASSERT( hacked       == false   );
        BOOST_ASSERT( noError      == nullptr );

        AuthorisationData & targetAuthorizationData( *targetAuthorizationDataAntiHack[ hacked ] );
        targetAuthorizationData.clear();

        if ( verifyer.saveAuthorizationDataIfVersionOK( targetAuthorizationData ) )
            return VerificationResults::noError;

        if ( SW_ENABLE_UPGRADE && ( verifyer.maxFoundVersion().maximumTargetVersion > 0 ) )
        {
            verifyer.findVersionUpgradeLicence();

            if ( verifyer.saveAuthorizationDataIfVersionOK( *targetAuthorizationDataAntiHack[ hacked ] ) )
                return VerificationResults::noError;
            else
                return VerificationResults::previousVersion;
        }
        else
        {
            BOOST_ASSERT( !authorised() );
            return
                verifyer.appropriateLicenceFoundButForAnotherOS()
                    ? VerificationResults::wrongOS
                    : ( SW_ENABLE_UPGRADE
                         ? VerificationResults::noFullLicence
                         : VerificationResults::noValidLicence );
        }
    }
    catch ( ... )
    {
        return VerificationResults::allocationFailure;
    }
}
#endif // LE_SW_AUTHORISATION_REQUIRED

void SpectrumWorx::resume()
{
    // Implementation note:
    //   Tracktion seems to call this on the GUI thread (for example when
    // switching/loading presets) while the processing (thread) still has not
    // stopped which breaks our code (because the channel buffers get reset
    // while processing is still active). Because of this we need to hold a
    // process lock here.
    //                                        (24.06.2010.) (Domagoj Saric)
    Utility::CriticalSectionLock const processLock( getProcessingLock() );
    sample_.restart();
    SpectrumWorxCore::resume();
}


bool LE_NOTHROW SpectrumWorx::setNumberOfChannelsFromHost( std::uint8_t const numberOfInputChannels, std::uint8_t const numberOfOutputChannels )
{
    auto const changeSuccess( SpectrumWorxCore::setNumberOfChannels( numberOfInputChannels, numberOfOutputChannels ) );
    if ( changeSuccess == IOChangeResult::Succeeded )
    {
        // Implementation note:
        // Changing the input mode from a side-channel mode to a
        // non-side-channel mode has the same effect as unloading the sample so
        // side channel data must also be cleared (the side channel data must
        // actually be cleared only if we are changing from a side-channel to a
        // non-side-channel mode and a sample is not loaded, as samples override
        // external side-chains).
        //                                    (14.01.2010.) (Domagoj Saric)
        clearSideChannelDataIfNoSideChannel();
    #if LE_SW_ENGINE_INPUT_MODE >= 1
        updateGUIForGlobalParameterChange();
    #endif // LE_SW_ENGINE_INPUT_MODE >= 2
    }
    return changeSuccess != IOChangeResult::Failed;
}


#if LE_SW_ENGINE_INPUT_MODE >= 2
bool LE_NOTHROW SpectrumWorx::setNumberOfChannelsFromUser( std::uint8_t const numberOfInputChannels, std::uint8_t const numberOfOutputChannels )
{
    BOOST_ASSERT( checkChannelConfiguration( numberOfInputChannels, numberOfOutputChannels ) );

    std::uint8_t const currentNumberOfMainChannels( uncheckedEngineSetup().numberOfChannels    () );
    std::uint8_t const currentNumberOfSideChannels( uncheckedEngineSetup().numberOfSideChannels() );

    std::uint8_t const numberOfMainChannels( numberOfOutputChannels                         );
    std::uint8_t const numberOfSideChannels( numberOfInputChannels - numberOfOutputChannels );

    if
    (
        ( numberOfMainChannels == currentNumberOfMainChannels ) &&
        ( numberOfSideChannels == currentNumberOfSideChannels )
    )
        return true;

    /// \note The process lock must be held during both the
    /// setNumberOfChannelsImpl() and hostTryIOConfigurationChange() calls,
    /// otherwise a process() call might be issued in between the
    /// setNumberOfChannelsImpl() and hostTryIOConfigurationChange() calls
    /// (which could lead to a crash because the host would send data for the
    /// previous IO setup).
    ///                                       (04.03.2013.) (Domagoj Saric)
    Utility::CriticalSectionLock const processLock( this->getProcessingLock() );

                           setReportedNumberOfChannels (        numberOfMainChannels,        numberOfSideChannels )  ;
    bool const hostAllows( hostTryIOConfigurationChange(        numberOfMainChannels,        numberOfSideChannels ) );
                           setReportedNumberOfChannels ( currentNumberOfMainChannels, currentNumberOfSideChannels )  ;
    if ( !hostAllows )
    {
        bool const hostSupportsIOChanges( hostSupportsIOConfigurationChanges() );
        if ( gui() )
        {
            boost::string_ref errorMessage;
            if ( hostSupportsIOChanges )
            {
                errorMessage = "The host rejected the requested IO mode change.";
            }
            else
            {
                if ( runningAsAU() )
                {
                    errorMessage =
                    "A preset tried to change the IO mode but this is something "
                    "not generally supported by the AU protocol. Please adjust "
                    "the bus configuration manually through the host as "
                    "required (or use the VST version).";
                }
                else
                {
                    errorMessage =
                    "An attempt was made to change the current input-output mode "
                    "but this host does not report that it supports on-the-fly "
                    "channel configuration changes. To avoid crashing it, the "
                    "change will be made the next time the plugin is started.";
                    //...mrmlj...
                    InputMode const currentIOMode( parameters().get<InputMode>() );
                    updateInputModeForIOConfig( numberOfMainChannels, numberOfSideChannels );
                    setInputModeToSetOnRestart( parameters().get<InputMode>() );
                    parameters().set<InputMode>( currentIOMode );
                }
                BOOST_ASSERT( engineSetup().numberOfChannels    () == currentNumberOfMainChannels );
                BOOST_ASSERT( engineSetup().numberOfSideChannels() == currentNumberOfSideChannels );
            }
            GUI::warningMessageBox( MB_ERROR, errorMessage, false );
            return false;
        }
        else
        {
            /// \note If the host is "ioChanged() agnostic" and we have no GUI
            /// to inform the user of an unsuccessful change, allow the change
            /// if it is decreasing the number of channels (this should be safe
            /// crash-wise).
            ///                               (19.03.2013.) (Domagoj Saric)
            if ( hostSupportsIOChanges || /*...mrmlj...*/ runningAsAU() )
                return false;
            else
            if
            (
                ( numberOfMainChannels > currentNumberOfMainChannels ) ||
                ( numberOfSideChannels > currentNumberOfSideChannels )
            )
                return false;

            LE_TRACE( "\tSW: blindly accepting a 'downsized' IO mode change." );
        }
    }

    bool const changeSuccessful( SpectrumWorxCore::setNumberOfChannelsImpl( numberOfMainChannels, numberOfSideChannels ) );
    if ( !changeSuccessful )
    {
        BOOST_VERIFY( hostTryIOConfigurationChange( currentNumberOfMainChannels, currentNumberOfSideChannels ) || !hostSupportsIOConfigurationChanges() );
        return false;
    }

    // Implementation note:
    //   Changing the input mode from a side-channel mode to a non-side-channel
    // mode has the same effect as unloading the sample so side channel data
    // must also be cleared (the side channel data must actually be cleared only
    // if we are changing from a side-channel to a non-side-channel mode and a
    // sample is not loaded, as samples override external side-chains).
    //                                        (14.01.2010.) (Domagoj Saric)
    clearSideChannelDataIfNoSideChannel();
    return true;
}
#endif // LE_SW_ENGINE_INPUT_MODE >= 2


////////////////////////////////////////////////////////////////////////////////
///
/// Programs and presets
///
////////////////////////////////////////////////////////////////////////////////

// VST Preset Program Change Suggestions
// http://forum.cockos.com/showthread.php?p=384102
bool SpectrumWorx::loadProgramState
(
    std::uint8_t          const programIndex,
    char          const * const pProgramName,
    void          const * const pData,
    std::uint32_t         const dataSize
)
{
    /// \note We have to copy the state data because of RapidXML's destructive
    /// parsing.
    ///                                       (18.03.2013.) (Domagoj Saric)
    BOOST_SIMD_ALIGNED_SCOPED_STACK_BUFFER( preset, char, dataSize );
    std::memcpy( preset.begin(), pData, dataSize );
    if ( !loadPreset( preset.begin(), false, nullptr, programIndex ) )
        return false;
    setProgramName( programIndex, pProgramName );
    return true;
}


unsigned int SpectrumWorx::saveProgramState( std::uint8_t const programIndex, void * const pStorage, std::uint32_t const storageSize ) const
{
    unsigned int const bytesWritten( savePreset( static_cast<char *>( pStorage ), currentSampleFile(), juce::String::empty, programs()[ programIndex ] ) );
    BOOST_ASSERT( bytesWritten < storageSize );
    boost::ignore_unused_variable_warning( storageSize );
    return bytesWritten;
}


void SpectrumWorx::getProgramName( std::uint8_t const program, boost::iterator_range<char *> const name ) const
{
    copyToBuffer( &programs()[ program ].name()[ 0 ], name );
}
void SpectrumWorx::getProgramName( boost::iterator_range<char *> const name ) const { getProgramName( getProgram(), name ); }


void SpectrumWorx::setProgramName( std::uint8_t const program, char const * const programName )
{
    copyToBuffer( programName, programs()[ program ].name() );
}
void SpectrumWorx::setProgramName( char const * const programName ) { setProgramName( getProgram(), programName ); }


namespace
{
class GlobalParameterUpdater : public SpectrumWorx
{
public:
    using result_type = void;

    template <class Parameter>
    result_type operator()( Parameter const & parameter ) const
    {
        BOOST_VERIFY(( setGlobalParameter<Parameter, SpectrumWorx>( const_cast<GlobalParameterUpdater &>( *this ), parameter.getValue() ) ));
    }

#if LE_SW_ENGINE_INPUT_MODE >= 2
    using InputMode = GlobalParameters::InputMode;
    result_type operator()( InputMode const & inputMode ) const
    {
        /// \note Changing the actual number of channels based on the input
        /// mode saved in the preset/program makes no sense (we can't/don't want
        /// to actually modify the format of the track into which SW is
        /// inserted). Instead we only update the side channel mode.
        ///                                   (18.03.2013.) (Domagoj Saric)
        bool enableSideChannel;
        switch ( inputMode.getValue() )
        {
            case InputMode::MonoSideChain  :
            case InputMode::StereoSideChain: enableSideChannel = true ; break;
            default                        : enableSideChannel = false; break;
        }

        SpectrumWorx & effect( const_cast<GlobalParameterUpdater &>( *this ) );
        InputMode::value_type const currentInputMode( effect.parameters().get<InputMode>() );
        InputMode::value_type       newInputMode;

        if ( enableSideChannel )
        {
        #ifdef LE_SW_DISABLE_SIDE_CHANNEL
            GUI::warningMessageBox
            (
                MB_WARNING,
                "Loaded preset uses side channel audio which is not "
                "supported by this edition of SpectrumWorx.",
                false
            );
            return /*false*/;
        #else
            switch ( currentInputMode )
            {
                case InputMode::Mono  : newInputMode = InputMode::MonoSideChain  ; break;
                case InputMode::Stereo: newInputMode = InputMode::StereoSideChain; break;
                default               : newInputMode = inputMode                 ; break;
            }
        #endif // LE_SW_DISABLE_SIDE_CHANNEL
        }
        else
        {
            switch ( currentInputMode )
            {
                case InputMode::MonoSideChain  : newInputMode = InputMode::Mono  ; break;
                case InputMode::StereoSideChain: newInputMode = InputMode::Stereo; break;
                default                        : newInputMode = inputMode        ; break;
            }
        }
        LE_TRACE_IF( newInputMode != inputMode, "Rejecting exact InputMode from preset" );
        BOOST_VERIFY( setGlobalParameter<InputMode>( effect, newInputMode ) || runningAsAU() );
    }
#endif // LE_SW_ENGINE_INPUT_MODE >= 2

private:
    GlobalParameterUpdater();
    void operator=( GlobalParameterUpdater const & );
}; // class GlobalParameterUpdater
} // anonymous namespace

void LE_NOTHROW SpectrumWorx::resetForGlobalParameters( Parameters const & parameters )
{
    //...mrmlj...this can possibly cause multiple engine setup updates/memory reallocations...
    //...mrmlj...no error reporting...
    boost::fusion::for_each( parameters, static_cast<GlobalParameterUpdater &>( *this ) );
}


bool LE_NOTHROW SpectrumWorx::canParameterBeAutomated( ParameterID const parameter, Program const * LE_RESTRICT const pProgram ) const
{
    bool const staticParameterList( pProgram == nullptr );
    if ( staticParameterList )
        return true;

    switch ( parameter.type() )
    {
        case ParameterID::GlobalParameter     :
        case ParameterID::ModuleChainParameter:
            return true;
    }

    auto indices( parameter.value._.lfo );
    switch ( parameter.type() )
    {
        case ParameterID::LFOParameter   : ++indices.moduleParameterIndex; // Bypass
        case ParameterID::ModuleParameter:
        {
            auto const pModule( pProgram->moduleChain().module( indices.moduleIndex ) );
            if ( !pModule )
                return false;
            if ( indices.moduleParameterIndex >= pModule->numberOfParameters() )
                return false;
            return true;
        }

        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


bool SpectrumWorx::ModuleInitialiser::operator()( Module & module, std::uint8_t const moduleIndex ) const
{
    if ( dspInitialiser( module, moduleIndex ) )
    {
        if ( pEditor )
        {
            if ( module.gui() )
                module.gui()->moveToSlot( moduleIndex );
            else
                module.createGUI( *pEditor, moduleIndex );
        }
        return true;
    }
    return false;
}

LE_NOTHROW
SpectrumWorx::ModuleInitialiser SpectrumWorx::moduleInitialiser() { return { SpectrumWorxCore::moduleInitialiser(), gui().get_ptr() }; }


#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

struct SpectrumWorx::PresetLoader
{
    SpectrumWorx &       effect       ;
    std::uint8_t   const targetProgram;

    using Module = ModuleInitialiser::Module;

    Program                      & program               () { return effect.programs()[ targetProgram ]; }
    GlobalParameters::Parameters & targetGlobalParameters() { return program().parameters (); }
    AutomatedModuleChain         & targetChain           () { return program().moduleChain(); }

	AutomationBlocker            automationBlocker() const { return { effect }; }
    Utility::CriticalSectionLock processingLock   () const { return effect.getProcessingLock(); }
    ModuleInitialiser            moduleInitialiser()       { return effect.moduleInitialiser(); }

    bool onlySetParameters     (                                                    ) const { return targetProgram != effect.getProgram(); }
    bool setNewGlobalParameters( GlobalParameters::Parameters const & newParameters )       { effect.resetForGlobalParameters( newParameters ); return true; }

    void moduleChainFinished( std::uint8_t const moduleCount, bool const syncedLFOFound )
    {
        BOOST_ASSERT( !onlySetParameters() );
        if ( effect.gui() ) effect.gui()->setLastModulePosition( moduleCount );
        if
        (
            syncedLFOFound &&
            !LFO::Timer::hasTempoInformation() &&
            /// \note Some hosts (Renoise 2.8) provide tempo information
            /// lazily, after the first time they call process() so we also
            /// check if the transport has started in order to avoid false
            /// warnings.
            ///                               (03.07.2012.) (Domagoj Saric)
            effect.lfoTimer().currentTimeInBars()
        )
        {
            GUI::warningMessageBox
            (
                MB_WARNING,
                "Loaded preset uses tempo-synced LFOs but the host does not provide tempo information.",
                false
            );
        }
    }

#ifndef LE_SW_DISABLE_SIDE_CHANNEL
    bool wantsSampleFile() const { return !ignoreSampleFile && !onlySetParameters(); }
    void setSample( boost::string_ref const sampleFileName )
    {
        BOOST_ASSERT( !onlySetParameters() );
        BOOST_ASSERT( !ignoreSampleFile );
        //   The sample has to be loaded before calling
        // gui()->updateSampleNameAsync().
        //                                    (15.12.2011.) (Domagoj Saric)
        /// \todo  Clean up this spaghetti.
        ///                                   (15.12.2011.) (Domagoj Saric)
        if ( sampleFileName.empty() )
            return;
        effect.setNewSample
        (
            // Implementation note:
            //   Workaround for relative sample paths and Windows paths
            // on OS X.
            //                                (17.11.2011.) (Domagoj Saric)
            juce::File::createFileWithoutCheckingPath
            (
                juce::String::fromUTF8( sampleFileName.begin(), static_cast<unsigned int>( sampleFileName.size() ) )
            #ifdef _WIN32
                .replaceCharacter( '/', '\\' )
            #else
                .replaceCharacter( '\\', '/' )
            #endif // __APPLE__
            )
        );
        /// \todo Think of a cleaner solution.
        ///                                   (03.02.2010.) (Domagoj Saric)
        if ( effect.gui() ) effect.gui()->updateSampleNameAsync();
    }
    bool const ignoreSampleFile;
#endif // LE_SW_DISABLE_SIDE_CHANNEL
}; // struct SpectrumWorx::PresetLoader

struct SpectrumWorx::PresetConsumer
{
    using Module = PresetLoader::Module;

    PresetLoader presetLoader( bool const ignoreExternalSample ) const { return { effect, targetProgram, ignoreExternalSample }; }

    Program & program() { return effect.programs()[ targetProgram ]; }
    void notifyHostAboutPresetChangeBegin() const { BOOST_ASSERT( targetProgram == effect.getProgram() ); effect.presetChangeBegin(); }
    void notifyHostAboutPresetChangeEnd  () const { BOOST_ASSERT( targetProgram == effect.getProgram() ); effect.presetChangeEnd  (); }
    SpectrumWorx &       effect       ;
    std::uint8_t   const targetProgram;
}; // struct SpectrumWorx::PresetConsumer

#pragma warning( push )

LE_NOTHROW
bool SpectrumWorx::loadPreset
(
    char         * const inMemoryPreset,
    bool           const ignoreExternalSample,
    juce::String * const pComment,
    std::uint8_t   const program
)
{
    BOOST_ASSERT( !presetLoadingInProgress() );

    return SW::loadPreset( inMemoryPreset, ignoreExternalSample, pComment, PresetConsumer{ *this, program } );
}


bool SpectrumWorx::loadPreset
(
    juce::File   const &       file,
    bool                 const ignoreExternalSample,
    juce::String       * const pComment,
    char_t       const * const presetName
)
{
    return SW::loadPreset( file, ignoreExternalSample, pComment, presetName, PresetConsumer{ *this, getProgram() } );
}


#ifndef LE_SW_DISABLE_SIDE_CHANNEL
bool LE_NOTHROWNOALIAS SpectrumWorx::setNewSampleWorker( juce::File const & newSampleFile )
{
    bool succeeded( true );
    if ( newSampleFile.existsAsFile() )
    {
    #ifndef NDEBUG
        bool       const samplePreviouslyLoaded( sample_              );
        juce::File const previousSample        ( sample_.sampleFile() );
    #endif // NDEBUG

        //...mrmlj...
        bool bufferAllocationSucceeded;
        {
            Utility::CriticalSectionLock const processingLock( getProcessingLock() );
            buffers().forceSideChannel( true );
            bufferAllocationSucceeded = buffers().resize
            (
                buffers    ().blockSize           (),
                engineSetup().numberOfChannels    (),
                engineSetup().numberOfSideChannels()
            );
        }

        if ( bufferAllocationSucceeded )
        {
            char const * const pErrorMessage( sample_.load( newSampleFile, engineSetup().sampleRate<unsigned int>(), processCriticalSection_ ) );
            if ( pErrorMessage )
            {
                GUI::warningMessageBox( "SpectrumWorx: error loading selected sample file.", pErrorMessage, true );

                // Implementation note:
                //   Verify that the 'sample loaded' state has not changed if an
                // error occurred.
                //                                (08.07.2010.) (Domagoj Saric)
                BOOST_ASSERT( !samplePreviouslyLoaded == !sample_             );
                BOOST_ASSERT( previousSample          == sample_.sampleFile() );
                succeeded = false;
            }
        }
        buffers().forceSideChannel( hasExternalSample() );
    }
    else
    {
        Utility::CriticalSectionLock const processingLock( getProcessingLock() );
        sample_.clear();
        // Implementation note:
        //   Because of hosts like FL Studio 9 that disallow the changing of the
        // input mode which then possibly gets locked to 4in2out and which then
        // send null pointers for channels above stereo we have to clear the
        // side channel data here whenever a sample is unloaded, even if the
        // InputMode is set to a side channel mode, because we might get null
        // pointers for the 'side channels' which the rest of the code will
        // interpret as no-side-channel data and will then check that the side
        // channel data was properly cleared.
        //                                    (15.06.2010.) (Domagoj Saric)
        //...mrmlj...reinvestigate whether the assumption that the 'rest of the
        //code' will properly handle null side channel pointers...
        //clearSideChannelDataIfNoSideChannel();
        clearSideChannelData();

        //...mrmlj...
        buffers().forceSideChannel( false );

        succeeded = ( newSampleFile == juce::File::nonexistent );
    }
    return succeeded;
}


void SpectrumWorx::setNewSample( juce::File const & newSampleFile )
{
    // Implementation note:
    //   If the requested sample file does not exist we look for it in the
    // default samples folder (this is required to make our pre-installed
    // presets, that use the pre-installed samples, work even if the user
    // chooses a non-default installation folder).
    //                                        (12.07.2010.) (Domagoj Saric)
    pendingSampleToLoad_ =
        newSampleFile.exists()
            ? newSampleFile
            : GUI::rootPath().getChildFile( "Samples" ).getChildFile( newSampleFile.getFileName() );
    if ( !isSampleLoadInProgress() )
    {
        BOOST_VERIFY(( sampleLoadingThread_.start<SpectrumWorx, &SpectrumWorx::sampleLoadingLoop>( *this ) ));
                       sampleLoadingThread_.setDebugName( "Sample thread" );
    }
}


bool SpectrumWorx::isSampleLoadInProgress() const
{
    return sampleLoadingThread_.isRunning();
}


void SpectrumWorx::registerSampleLoadedListener( Editor & listenerToRegister )
{
    //...mrmlj...reconsider this...
    BOOST_ASSERT( !pListenerToNotifyWhenSampleLoaded_ || ( pListenerToNotifyWhenSampleLoaded_ == &listenerToRegister ) );
    pListenerToNotifyWhenSampleLoaded_ = &listenerToRegister;
}


void SpectrumWorx::deregisterSampleLoadedListener( Editor const & listenerToDeregister )
{
    BOOST_ASSERT( !pListenerToNotifyWhenSampleLoaded_ || ( pListenerToNotifyWhenSampleLoaded_ == &listenerToDeregister ) );
    boost::ignore_unused_variable_warning( listenerToDeregister );
    pListenerToNotifyWhenSampleLoaded_ = nullptr;
}

LE_NOTHROW
void SpectrumWorx::sampleLoadingLoop()
{
    while
    (
        ( pendingSampleToLoad_ != sample_.sampleFile() ) &&
        setNewSampleWorker( pendingSampleToLoad_ )
    ) {}

    pendingSampleToLoad_ = juce::File::nonexistent;

    if ( pListenerToNotifyWhenSampleLoaded_ )
    {
        GUI::postMessage
        (
            *this,
            []( GUI::SpectrumWorxEditor & gui )
            {
                gui.sampleArea_.setVisible();
                gui.updateSampleName();
                return true;
            }
        );
    }
    pListenerToNotifyWhenSampleLoaded_ = nullptr;

    sampleLoadingThread_.markAsDone();
}
#endif // LE_SW_DISABLE_SIDE_CHANNEL


bool LE_NOTHROW SpectrumWorx::updateEngineSetup()
{
    if ( SpectrumWorxCore::updateEngineSetup() )
    {
        updateGUIForEngineSetupChanges();
        return true;
    }
    return false;
}


void SpectrumWorx::updatePosition( std::uint32_t const deltaSamples )
{
    handleTimingInformationChange
    (
        updatePositionAndTimingInformation( deltaSamples )
    );
}


bool LE_NOTHROW SpectrumWorx::initialise()
{
    //if ( !SpectrumWorxCore::initialise() )
    //    return false;
    { //...mrmlj...copy pasted core version for different setNumberOfChannels
      //...mrmlj...and updateEngineSetup versions...clean this up...
        bool success;
        // Update/create the initial Engine::Setup and shared storage with the
        // default and/or so far partially set parameters.
        //...mrmlj...check if channel configuration has already been set up and skip
        //...mrmlj...the parameters-engine setup synchronization in that case
        //...mrmlj...a custom io mode might have been set and this would override it
        //...mrmlj...clean this up...
        if ( !currentStorageFactors().numberOfChannels )
        {
        #if LE_SW_ENGINE_INPUT_MODE >= 1
            auto const ioChannelsConfig( ioChannels( parameters().get<InputMode>() ) );
            //...mrmlj...see the note in SpectrumWorxVST24::initialise()...
            success = ( SpectrumWorxCore::setNumberOfChannels( ioChannelsConfig.first, ioChannelsConfig.second ) != IOChangeResult::Failed );
            reinterpret_cast<Plugin2HostInteropControler &>( *this ). //...ugh...mrmlj...
            hostTryIOConfigurationChange( engineSetup().numberOfChannels(), engineSetup().numberOfSideChannels() ); //...ugh...mrmlj...
        #else //...mrmlj...
            success = SpectrumWorxCore::setNumberOfChannels( 1, 1 );
        #endif // LE_SW_ENGINE_INPUT_MODE
        }
        else
        {
            BOOST_ASSERT( currentStorageFactors().numberOfChannels == engineSetup().numberOfChannels() );
            success = true;
        }
        //...mrmlj...AU...BOOST_ASSERT_MSG( !!buffers(), "Input buffers not initialised." );
        success &= updateEngineSetup();
        BOOST_ASSERT( success );
        if ( !success )
            return false;
    }

    Math::rngSeed();

    loadSettings();

    /// \note Report the actual latency after initialisation. Without this the
    /// host would be left assuming the maximum latency reported in the VST2.4
    /// constructor.
    ///                                       (24.04.2013.) (Domagoj Saric)
    latencyChanged();

    return true;
}


////////////////////////////////////////////////////////////////////////////
// Settings
////////////////////////////////////////////////////////////////////////////

namespace
{
    struct Settings : GUI::Theme::Settings
    {
        Settings() : loadLastSessionOnStartup( false ) {}
        Settings( GUI::Theme::Settings const & guiSettings, bool const loadLastSessionOnStartupParam )
            :
            GUI::Theme::Settings    ( guiSettings                   ),
            loadLastSessionOnStartup( loadLastSessionOnStartupParam )
        {}

        bool loadLastSessionOnStartup;
    }; // struct Settings
} // anoynmous namepsace

void LE_NOTHROW SpectrumWorx::loadSettings()
{
    try
    {
        using namespace boost;

        mmap::basic_read_only_mapped_view const mappedSettingsFile( mmap::map_read_only_file( settingsFile().getFullPathName().getCharPointer() ) );
        //...mrmlj...rethink this assertion...
        //BOOST_ASSERT( ( mappedSettingsFile || !this->settingsFile().existsAsFile() ) && "Unable to open existing settings file." );
        if ( mappedSettingsFile.size() != sizeof( Settings ) )
        {
            LE_TRACE( "\tSW: unrecognized settings file." );
            return;
        }

        Settings const & settings( *reinterpret_cast<Settings const *>( mappedSettingsFile.begin() ) );
        GUI::Theme::settings() = settings;
        shouldLoadLastSessionOnStartup( settings.loadLastSessionOnStartup );
        if ( shouldLoadLastSessionOnStartup() )
        {
            juce::File const lastSessionFile( lastSessionPresetFile() );
            if ( lastSessionFile.existsAsFile() )
            {
                //BOOST_VERIFY( loadPreset( lastSessionFile, false, nullptr, _T( "Last session" ) ) );
                //...mrmlj...avoid notifyHostAboutPresetChange()
                auto const pPresetData( Preset::loadIntoMemory( lastSessionFile ) );
                BOOST_VERIFY
                (
                    pPresetData.get() &&
                    loadPreset( pPresetData.get(), false, nullptr, getProgram() )
                );
                setProgramName( "Last session" );
            }
        }
    }
    catch ( ... ) {}
}


void LE_NOTHROW SpectrumWorx::saveSettings()
{
    using namespace boost;

    // Settings file

    mmap::basic_mapped_view const mappedSettingsFile( mmap::map_file( settingsFile().getFullPathName().getCharPointer(), sizeof( Settings ) ) );
    BOOST_ASSERT_MSG( mappedSettingsFile, "Unable to create settings file." );
    if ( mappedSettingsFile.empty() )
    {
        GUI::warningMessageBox( MB_ERROR, "Failed to save settings.", false );
        return;
    }

    Settings &       onDiskSettings ( *reinterpret_cast<Settings *>( mappedSettingsFile.begin() ) );
    Settings   const currentSettings( GUI::Theme::settings(), loadLastSessionOnStartup_           );
    onDiskSettings = currentSettings;

#ifndef LE_SW_FMOD
    // Paths file

    juce::String const & rootPath     ( GUI::rootPath     ().getFullPathName() );
    juce::String const & presetsFolder( GUI::presetsFolder().getFullPathName() );

    unsigned int const rootLength   ( rootPath     .length() );
    unsigned int const presetsLength( presetsFolder.length() );

    boost::mmap::basic_mapped_view const pathsFile
    (
        GUI::mapPathsFile( rootLength + sizeof( '\n' ) + presetsLength )
    );
    BOOST_ASSERT_MSG( pathsFile, "Unable to update the paths file." );
    if ( !pathsFile )
        return;

#ifdef __APPLE__
    // Implementation note:
    //   See the Mac specific note in GUI::initializePaths().
    //                                        (01.12.2010.) (Domagoj Saric)
    //...mrmlj...NEW_JUCE...UNICODE
    rootPath.copyToUTF8( &pathsFile[ 0 ], rootLength + 1 );
    pathsFile[ rootLength ] = '\n';
#else
    BOOST_ASSERT( std::memcmp( pathsFile.begin(), GUI::rootPath().getFullPathName().toUTF8(), rootLength * sizeof( char ) ) == 0 );
#endif // __APPLE__
    BOOST_ASSERT( pathsFile[ rootLength ] == '\n' );
    //...mrmlj...+1 because copyToX() wants to append the null terminator...
    presetsFolder.copyToUTF8( &pathsFile[ rootLength + sizeof( '\n' ) ], presetsLength + 1 );
#endif // LE_SW_FMOD
}

#if LE_SW_ENGINE_INPUT_MODE >= 2
void SpectrumWorx::setInputModeToSetOnRestart( InputMode const pendingInputMode )
{
    inputModeToSetOnRestart_ = pendingInputMode;
    shouldLoadLastSessionOnStartup( true );
}
#endif // LE_SW_ENGINE_INPUT_MODE >= 2

LE_CONST_FUNCTION
bool SpectrumWorx::runningAsAU() const
{
#ifdef __APPLE__
    return runningAsAU_;
#else
    return false;
#endif // __APPLE__
}


juce::File SpectrumWorx::lastSessionPresetFile()
{
    return defaultPresetsFolder().getChildFile( "__LastSession.swp" );
}


juce::File SpectrumWorx::defaultPresetsFolder()
{
    return GUI::rootPath().getChildFile( "Presets" );
}


juce::File SpectrumWorx::settingsFile()
{
    return GUI::rootPath().getChildFile( "SpectrumWorx.dat" );
}


void SpectrumWorx::clearSideChannelData()
{
    BOOST_ASSERT( !sample_ );
    SpectrumWorxCore::clearSideChannelData();
}


void SpectrumWorx::clearSideChannelDataIfNoSideChannel()
{
    if ( !haveSideChannel() )
        clearSideChannelData();
}


bool SpectrumWorx::haveSideChannel() const
{
    return sample_ || SpectrumWorxCore::haveSideChannel();
}


LE_NOTHROW
void SpectrumWorx::handleTimingInformationChange( LFO::Timer::TimingInformationChange const timingInformationChange )
{
    if ( timingInformationChange.timingInfoChanged() )
    {
        /// \todo The host should also be notified about LFO parameters changed
        /// due to tempo and/or measure changes.
        ///                                   (23.02.2011.) (Domagoj Saric)
        //...mrmlj...Processor::updateModuleLFOs() should have already been called...
        if ( gui() )
            gui()->updateForNewTimingInfo();
    }
}


SpectrumWorx const & SpectrumWorx::fromEngineSetup( Engine::Setup const & engineSetup )
{
    return static_cast<SpectrumWorx const &>( SpectrumWorxCore::fromEngineSetup( engineSetup ) );
}


bool SpectrumWorx::blockAutomation() const
{
    if ( !gui() && !authorised() )
        return true;

    if ( SpectrumWorxCore::blockAutomation() )
        return true;

    return false;
}


////////////////////////////////////////////////////////////////////////////////
///
/// Programs and presets
///
////////////////////////////////////////////////////////////////////////////////

void SpectrumWorx::setProgram( std::uint8_t const newProgramIndex )
{
    std::uint8_t const currentProgramIndex( getProgram() );
    if ( newProgramIndex != currentProgramIndex )
    {
        Program & currentProgram( programs()[ currentProgramIndex ] );
        Program & newProgram    ( programs()[ newProgramIndex     ] );
        {
            // Implementation note:
            //   Here we update only ourselves and expect the host to call
            // saveProgramState() after it calls setProgram() (Wavelab and
            // Reaper for example behave as expected while Ableton Live does not
            // call saveProgramState() but still seems to remain in a consistent
            // state, probably because of its internal caching).
            //                                (09.07.2010.) (Domagoj Saric)
            Utility::CriticalSectionLock const processLock( getProcessingLock() );
            //...mrmlj...
            Parameters &       newParametersSlot( newProgram.parameters() );
            Parameters   const newParameters    ( newParametersSlot       );
            newParametersSlot = currentProgram.parameters();
            currentProgram_ = newProgramIndex;
            SpectrumWorxCore::setProgram( newProgram );
            resetForGlobalParameters( newParameters );
        }

    #ifndef LE_SW_FMOD //...mrmlj...
        // Implementation note:
        //   Wavelab 5.0 creates a new instance when it wants to 'iterate'
        // over the presets of a plugin (when you press the "Presets"
        // button) and it does not create the GUI for that instance. Because
        // of that, here we must first check if the GUI was created.
        //                                    (22.10.2009.) (Domagoj Saric)
        // Implementation note:
        //   Cubase calls setProgram() from a non-GUI thread. A simple
        // solution of just blocking the GUI thread does not play nice in
        // Cubase.
        //                                    (17.01.2011.) (Domagoj Saric)
        GUI::postOrExecuteMessage
        (
            *this,
            [&]( GUI::SpectrumWorxEditor & gui )
            {
                auto & previousChain( currentProgram.moduleChain() );
                auto & newChain     ( newProgram    .moduleChain() );
                LE_ASSUME( &previousChain );
                gui.destroyChainGUIs( previousChain );
                gui.createChainGUIs ( newChain      );
                // Implementation note:
                //   The module arrow does not get cleared/erased (when it gets
                // moved to the left, i.e. the number of modules is smaller in
                // the set newProgram) automatically in Cubase 5 so we must
                // manually do a repaint (theoretically only the old module
                // arrow area needs to be repainted).
                //                            (19.01.2011.) (Domagoj Saric)
                /// \todo Investigate why is this required/why doesn't it happen
                /// automatically.
                ///                           (19.01.2011.) (Domagoj Saric)
                gui.repaint();
                return true;
            }
        );
    #endif // LE_SW_FMOD
    }
}


char const * SpectrumWorx::currentProgramName() const
{
    // Skip the (possible) leading asterisk...
    char const * LE_RESTRICT const pCurrentProgramName( &program().name()[ 0 ] );
    return &pCurrentProgramName[ ( pCurrentProgramName[ 0 ] == '*' ) ];
}


bool SpectrumWorx::createGUI()
{
    LE_ASSUME( !editor_ );
    try
    {
        editor_ = boost::in_place();
        LE_ASSUME( editor_.is_initialized() );
        return true;
    }
    catch ( ... )
    {
        LE_ASSUME( !editor_ );
        return false;
    }
}


void SpectrumWorx::destroyGUI()
{
    // Implementation note:
    //   Wavelab calls effEditClose even if effEditOpen failed so we
    // cannot assert( editor_.is_initialized() ) here.
    //                                        (23.11.2009.) (Domagoj Saric)
    editor_.reset();
}


void SpectrumWorx::updateGUIForGlobalParameterChange()
{
    if ( !presetLoadingInProgress() && GUI::isThisTheGUIThread() ) //...mrmlj...ughly quick-hack to detect user initiated changes and avoid calling back the GUI...
        return;
    GUI::postMessage
    (
        *this,
        []( GUI::SpectrumWorxEditor & gui ) { gui.updateForGlobalParameterChange(); return true; }
    );
}


void SpectrumWorx::updateGUIForEngineSetupChanges()
{
    if ( !presetLoadingInProgress() && GUI::isThisTheGUIThread() ) //...mrmlj...ughly quick-hack to detect user initiated changes and avoid calling back the GUI...
        return;
    GUI::postMessage
    (
        *this,
        []( GUI::SpectrumWorxEditor & gui )
        {
            gui.updateForGlobalParameterChange();
            gui.updateForEngineSetupChanges   ();
            return true;
        }
    );
}


SpectrumWorx & SpectrumWorx::effect( Editor & editor )
{
    return Utility::ParentFromOptionalMember<SpectrumWorx, Editor, &SpectrumWorx::editor_, false>()( editor );
}


////////////////////////////////////////////////////////////////////////////////
//
// Parameters
//
////////////////////////////////////////////////////////////////////////////////

bool LE_FASTCALL SpectrumWorx::setGlobalParameter( FFTSize & parameter, FFTSize::param_type const newValue )
{
    bool const result( SpectrumWorxCore::setGlobalParameter( parameter, newValue ) );
    if ( result )
    {
        /// \note Latency depends on the window size
        /// ( FFT size / zero padding * window size factor ) so notify the host
        /// when any of the relevant parameters change.
        ///                                   (23.05.2012.) (Domagoj Saric)
        /*BOOST_VERIFY*/( latencyChanged() );
        updateGUIForEngineSetupChanges();
    }
    return result;
}


bool LE_FASTCALL SpectrumWorx::setGlobalParameter( OverlapFactor & parameter, OverlapFactor::param_type const newValue )
{
    bool const result( SpectrumWorxCore::setGlobalParameter( parameter, newValue ) );
    if ( result ) updateGUIForEngineSetupChanges();
    return result;
}


#if LE_SW_ENGINE_INPUT_MODE >= 2
bool LE_FASTCALL SpectrumWorx::setGlobalParameter( InputMode & parameter, InputMode::param_type const newValue )
{
    auto const ioChannelsConfig( ioChannels( static_cast<SpectrumWorxCore::InputMode::value_type>( newValue ) ) );

    bool const success( setNumberOfChannelsFromUser( ioChannelsConfig.first, ioChannelsConfig.second ) );
    BOOST_VERIFY( ( parameter.getValue() == newValue ) || !success );
    if ( success ) updateGUIForEngineSetupChanges();
    return success;
}
#endif // LE_SW_ENGINE_INPUT_MODE >= 2


#if LE_SW_ENGINE_WINDOW_PRESUM
bool LE_FASTCALL SpectrumWorx::setGlobalParameter( WindowSizeFactor & parameter, WindowSizeFactor::param_type const newValue )
{
    bool const result( SpectrumWorxCore::setGlobalParameter( parameter, newValue ) );
    if ( result )
    {
        /// \note See the note in the FFTSize overload.
        ///                                   (23.05.2012.) (Domagoj Saric)
        /*BOOST_VERIFY*/( latencyChanged() );
        updateGUIForEngineSetupChanges();
    }
    return result;
}
#endif // LE_SW_ENGINE_WINDOW_PRESUM


#if 0 //...mrmlj...alex leftovers...

#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.

float const * SpectrumWorx::ProceedSampler( unsigned int /*const ccBuffer*/ )
{
    //...mrmlj...temporarily commented out Alex's code until it is harvested for
    //knowledge and future ideas...
    LE_UNREACHABLE_CODE();
    //const bool loaded = samplebank.get() ? samplebank->Load() : false;
	//if (loaded)
	//{
    //       switch( parameters().get<StreamMode>().getValue() )
    //       {
    //           case StreamMode::Always:
	//		    samplebank->FillBuffer( sampleChannel_.begin(), ccBuffer );
    //               break;
    //           case StreamMode::MIDITrigger: // noteon play (reset)
	//	    {
	//		    ReallocStream( blockSize );
	//		    if (retrigger) samplebank->FillBuffer( sampleChannel_.begin(), ccBuffer );
	//		    if (lastbuffer)
	//		    {
	//			    samplebank->FillBuffer( sampleChannel_.begin(), ccBuffer );
	//			    if (samplebank->getFade()==0) lastbuffer = false;
	//		    }
    //               break;
	//	    }
    //           case StreamMode::MIDIGate: // noteon play(reset)-noteoff stop
	//	    {
	//		    ReallocStream( blockSize );
	//		    if (retrigger) samplebank->FillBuffer( sampleChannel_.begin(), ccBuffer );
	//		    if (lastbuffer)
	//		    {
	//			    samplebank->FillBuffer( sampleChannel_.begin(), ccBuffer );
	//			    if (samplebank->getFade()==0) lastbuffer = false;
	//		    }
    //               break;
	//	    }
    //           default: assert( false );
    //       }
    //      return sampleChannel_.begin();
	//} // sampler
    return nullptr;
}


bool SpectrumWorx::processMIDIEvent( ::VstMidiEvent const & /*event*/ )
{
    /// \todo Properly reimplement.
    ///                                       (25.05.2010.) (Domagoj Saric)

    //...mrmlj...temporarily commented out Alex's code until it is harvested for
    //knowledge and future ideas...
    LE_UNREACHABLE_CODE();
    return false;

	//const float Norm = 1.f / 127.f;

    //assert( event.type == kVstMidiType );

	//char const * const midiData( event.midiData );

	//BYTE const status( midiData[0] & 0xF0 ); // ignoring channel

	//// TODO
	///*brothomStates: trigger / gate
	//brothomStates: trigger always just restarts the sample
	//brothomStates: gate restarts and stops
	//brothomStates: trigger = note on only
	//brothomStates: gate = note on[trig], note off[stop]*/
	//// TODO: capture special controllers  like allnotes off//all sounds off

	//if ( status == 0x90 )	// capture NoteOn
	//{
	//	//uint8  note = midiData[1] & 0x7F;
	//	//uint8  velo = midiData[2] & 0x7F;
	//	if ( parameters().get<StreamMode>() == 1 )
	//	{
	//		//if (samplebank->getFade()==0)
	//		//{
	//		//	retrigger = true;
	//		//	if (firstfade)
	//		//		samplebank->StartFade(1);
	//		//	else
	//		//	{
	//		//		samplebank->StartFade(3);
	//		//		firstfade  = true;
	//		//	}
	//		//}
	//	}
	//	if ( parameters().get<StreamMode>() == 2 )
	//	{
	//		if (retrigger) return 1;//important
	//		//if (samplebank->getFade()==0)
	//		//{
	//		//	samplebank->StartFade(3);
	//		//	retrigger = true;
	//		//}
	//	}
	//}

	//if (status == 0x90)	// capture NoteOff = NoteOn with Velocity 0
	//{
	//	//uint8  note = midiData[1] & 0x7F;
	//	//if (lastbuffer) return 1;
	//	BYTE velo = midiData[2] & 0x7F;
	//	if (velo == 0)
	//	{
	//		//Noteoff
	//		if ( parameters().get<StreamMode>() == 2 )
	//		{
	//			//if (samplebank->getFade()==0)
	//			{
	//				//samplebank->StartFade(2);
	//				retrigger = false;
	//				lastbuffer = true;
	//			}
	//		}
	//	}
	//}

	//if (status == 0x80)	// capture NoteOff
	//{
	//	//uint8  note = midiData[1] & 0x7F;
	//	//uint8  velo = midiData[2] & 0x7F;
	//	//if (lastbuffer) return 1;
	//	if ( parameters().get<StreamMode>() == 2 )
	//	{
	//		//if (samplebank->getFade()==0)
	//		{
	//			//samplebank->StartFade(2);
	//			retrigger = false;
	//			lastbuffer = true;
	//		}
	//	}
	//}

	//if (status == 0xB0)	// capture controllers
	//{
	//	currentDelta = event.deltaFrames;
	//	BYTE  controller =  midiData[1] & 0x7F;
	//	float  value      = (midiData[2] & 0x7F) * Norm;
    //       assert( !"New code does not seem to support MIDI yet." );
	//	//mainchain->processCC(controller, value);
	//}

	//return 1;	// want more
}

#pragma warning( pop )

#endif //...mrmlj...alex leftovers...

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
