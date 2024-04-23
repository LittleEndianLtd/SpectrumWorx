////////////////////////////////////////////////////////////////////////////////
///
/// plugin.inl (VST 2.4)
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "le/plugins/entryPoint.hpp"

#ifndef NDEBUG
#include "le/math/math.hpp"
#endif // NDEBUG
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tchar.hpp"
#include "le/utility/trace.hpp"

#include "boost/assert.hpp"
#include "boost/concept_check.hpp"
#include "boost/mpl/integral_c.hpp"
#include "boost/preprocessor/cat.hpp"
#ifdef _DEBUG
    #include "boost/range/algorithm/find.hpp"
#endif // _DEBUG

#include <climits>
//------------------------------------------------------------------------------

//...mrmlj...debugging section...clean this up after beta testing...
namespace LE
{
    namespace SW { namespace GUI
    {
        void  LE_NOTHROW        warningMessageBox ( boost::string_ref title, boost::string_ref message, bool canBlock );
        bool  LE_NOTHROW        warningOkCancelBox( TCHAR const * title, TCHAR const * question );
        float LE_NOTHROWNOALIAS displayScale();
        bool  LE_NOTHROWNOALIAS isGUIInitialised();
    } } // namespace GUI

    inline void customTerminationHandler()
    {
        SW::GUI::warningMessageBox( "SpectrumWorx unrecoverable failure...", "Your host is about to crash :(", true );
        std::abort();
    }
} // namespace LE


#define LE_PLUGIN_VST24_ENTRY_POINT_NAME VSTPluginMain
#define LE_PLUGIN_VST24_INJECTOR_NAME LEPlugin_VSTPluginMain_EntryPointFriendInjector
LE_ENTRY_POINT_BEGIN( AEffect *, VSTPluginMain, audioMasterCallback const audioMaster )
{
    typedef Impl Plugin;

    /// \todo Cleanup and improve the "improper-version" handling code.
    ///                                   (20.08.2009.) (Domagoj Saric)

    unsigned int const hostSupportedVSTVersion( static_cast<unsigned int>( audioMaster( 0, audioMasterVersion, 0, 0, 0, 0 ) ) );
    // Implementation note:
    //   Because of some broken hosts that report their supported VST
    // version with as some bogus value greater than zero but smaller than
    // 2000 (e.g. Chainer 1.03 - '2', Audition 1.5 build 4124.1 - '3') we
    // cannot check for pre VST 2.0 hosts with a ' < 2000' check but must
    // use the simple ' == 0' check (as is used in VST SDK examples).
    //                                    (27.05.2010.) (Domagoj Saric)
    if ( hostSupportedVSTVersion == 0 )
    {
        LE_TRACE( "\tSW VST2: host application does not support the VST 2.0 protocol. %s cannot work with it.", Plugin::name );
        BOOST_ASSERT( !"Ancient host." );
        return 0;
    }
    #if defined( _DEBUG )
    else
    if ( hostSupportedVSTVersion < 2000 )
    {
        LE_TRACE( "\tSW VST2.4: host application reported a bogus supported VST protocol version: %u.", hostSupportedVSTVersion );
    }
    else
    if ( hostSupportedVSTVersion < 2400 )
    {
        LE_TRACE( "\tSW VST2.4: host application may not fully support the VST %.1f protocol, continue at your own risk.", 2400 / 1000.0 );
    }
    #endif

    try
    {
        std::set_terminate( &LE::customTerminationHandler );
        Plugin * const pEffect( new Plugin( audioMaster ) );
        return &pEffect->aEffect();
    }
    catch ( std::exception const & e )
    {
        LE::SW::GUI::warningMessageBox( "Failed to instantiate SpectrumWorx.", e.what(), true );
    }
    catch ( ... )
    {
        LE::SW::GUI::warningMessageBox( "Failed to instantiate SpectrumWorx.", "Unknown unhandled error occurred!", true );
    }
#if !defined( NDEBUG ) && defined( _MSC_VER )
    _CrtDbgBreak();
#endif // _MSC_VER
    return nullptr;
}
LE_ENTRY_POINT_END()

#ifdef LE_HAS_FRIEND_INJECTION
    #define LE_PLUGIN_VST24_ENTRY_POINT( implClass )
#else
    #define LE_PLUGIN_VST24_ENTRY_POINT( implClass ) \
        extern "C" LE_ENTRY_POINT_ATTRIBUTES AEffect * __cdecl LE_PLUGIN_VST24_ENTRY_POINT_NAME( audioMasterCallback const audioMaster ) \
{ return ::BOOST_PP_CAT( LE_PLUGIN_VST24_ENTRY_POINT_NAME, Impl )<implClass>( audioMaster ); }
#endif // LE_HAS_FRIEND_INJECTION

namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

#ifndef NDEBUG
    template <class Impl>
    unsigned int Plugin<Impl, Protocol::VST24>::pluginInstanceCount_( 0 );
#endif

////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin<>::VST24Plugin()
// ----------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
LE_NOTHROW
Plugin<Impl, Protocol::VST24>::Plugin( ConstructionParameter const constructionParameter ) /// \throws nothing
    :
    VSTPluginBase( constructionParameter )
#ifndef NDEBUG
    ,pluginInstanceID_( pluginInstanceCount_++ )
#endif
{
    LE_TRACE( "---- %s VST 2.4 instance %d created ----\n", Impl::name, pluginInstanceID_ );

#ifdef LE_HAS_FRIEND_INJECTION
    boost::ignore_unused_variable_warning(  ::LE_PLUGIN_VST24_INJECTOR_NAME<Impl>() );
    boost::ignore_unused_variable_warning( &::LE_PLUGIN_VST24_ENTRY_POINT_NAME      );
#else
#endif // LE_HAS_FRIEND_INJECTION

    aEffect().dispatcher   = &Plugin::dispatcher  ;
    aEffect().setParameter = &Plugin::setParameter;
    aEffect().getParameter = &Plugin::getParameter;
#ifndef NDEBUG
    aEffect().DECLARE_VST_DEPRECATED( process ) = &processAccumulating;
#endif // NDEBUG
    aEffect().processReplacing       = static_cast<AEffectProcessProc>( &Plugin::processReplacing );
    aEffect().processDoubleReplacing = getDoubleReplacingCallback( typename HasProcessDoubleReplacing<Impl>::type() );

    aEffect().numPrograms = Impl::maxNumberOfPrograms  ;
    aEffect().numParams   = Impl::maxNumberOfParameters;
    aEffect().numInputs   = Impl::maxNumberOfInputs    ;
    aEffect().numOutputs  = Impl::maxNumberOfOutputs   ;
    aEffect().uniqueID    = Impl::vstUniqueID          ;
    aEffect().version     = Impl::version              ;

    BOOST_ASSERT( aEffect().object == this    );
    BOOST_ASSERT( aEffect().user   == nullptr );

#ifndef NDEBUG
    reinterpret_cast<std::pair<std::uint16_t, std::uint16_t> &>( aEffect().user ) = std::pair<std::uint16_t, std::uint16_t>( Impl::maxNumberOfInputs, Impl::maxNumberOfOutputs );
#endif // NDEBUG

    BOOST_ASSERT( aEffect().flags == 0 );
    aEffect().flags = ( effFlagsHasEditor                                                     ) |
                      ( effFlagsProgramChunks                                                 ) |
                      ( effFlagsCanReplacing                                                  ) | // mandatory in VST 2.4.
                      ( effFlagsNoSoundInStop      * ( Impl::maxTailSize == 0               ) ) |
                      ( effFlagsIsSynth            * ( Impl::category == kPlugCategSynth    ) ) |
                      ( effFlagsCanDoubleReplacing * HasProcessDoubleReplacing<Impl>::value   ) |
                      ( DECLARE_VST_DEPRECATED( effFlagsCanMono ) * ( queryImplementationCapability<Ch1in2out>() == Can ) );

    aEffect().initialDelay = Impl::maxLookAhead;

    if ( queryImplementationCapability<ReceiveMIDIEvent>() == Can )
        call( DECLARE_VST_DEPRECATED( audioMasterWantMidi ) );

    // pre VST 2.4 deprecated begin
    // effFlagsHasVu - "can supply vu meter values"
    // effFlagsHasClip - "can detect clipping / can return > 1.0 in getVu"
    // effFlagsExtIsAsync - "plugin wants asynchronous operation (process returns immediately) http://homepage.hik.se/staff/tkama/audio_procME/doc/html/plug/2.0/AudioEffectX.html#wantAsyncOperation"
    // effFlagsExtHasBuffer - "Plugin uses an external buffer (like hardware dsp)"
    // pre VST 2.4 deprecated end
}


////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin<>::~VST24Plugin()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

#ifndef NDEBUG
template <class Impl>
Plugin<Impl, Protocol::VST24>::~Plugin()
{
    LE_TRACE( "---- %s VST 2.4 instance %d destroyed ----\n", Impl::name, pluginInstanceID_ );
    --pluginInstanceCount_;
}
#endif


////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin::dispatcher()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Implements the VST 2.x dispatch callback.
///
/// http://ygrabit.steinberg.de/~ygrabit/public_html/vstgui/vstsupport.html
/// http://ygrabit.steinberg.de/~ygrabit/public_html/vstgui/vstsupport5.html
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
::VstIntPtr LE_NOTHROW Plugin<Impl, Protocol::VST24>::dispatcher
(
    ::AEffect   * LE_RESTRICT const pEffect     ,
    ::VstInt32                const opCode      ,
    ::VstInt32                const index       ,
    ::VstIntPtr               const integerParam,
      void      *             const pData       ,
      float                   const floatParam
)
{
    BOOST_ASSERT_MSG( _MM_GET_ROUNDING_MODE() == _MM_ROUND_NEAREST, "Unexpected rounding mode." );

#ifndef NDEBUG
    // Properly typed opcode for better visualization to aid in debugging.
    ::AEffectOpcodes  const vstOpCode ( static_cast<::AEffectOpcodes >( opCode ) ); boost::ignore_unused_variable_warning( vstOpCode  );
    ::AEffectXOpcodes const vstOpCodeX( static_cast<::AEffectXOpcodes>( opCode ) ); boost::ignore_unused_variable_warning( vstOpCodeX );
#endif

    LE_ASSUME( opCode >= 0                                              );
    LE_ASSUME( opCode <= UCHAR_MAX  || /*FL11*/ opCode == USHRT_MAX + 1 );

  //LE_ASSUME( index  >= 0         ); /*Audition CS6*/
  //LE_ASSUME( index  <= USHRT_MAX ); /*Cubase 5.5  */

    Impl & impl( Plugin::impl( pEffect ) );

    auto const * LE_RESTRICT const pContextForDynamicParameterAccess( impl.useDynamicParameterLists() ? &impl.dynamicParameterAccessContext() : nullptr );

#ifdef _DEBUG
    /// \note For easier debugging - prehandle/skip repetitive opcodes.
    ///                                       (04.09.2014.) (Domagoj Saric)
    if ( opCode == effEditIdle ) { if ( impl.gui() ) impl.gui()->idle(); return 0; }
#endif // _DEBUG

    auto const parameterIndex( ParameterIndex{ static_cast<ParameterIndex::value_type>( index ) } );

    // http://www.asseca.com/vst-24-specs/efDispatches.html
    switch ( opCode )
    {
        case effOpen :           BOOST_VERIFY( impl.initialise() ); break;
        case effClose:           impl.finalise(); return true;

        case effSetProgram:      BOOST_ASSERT( integerParam < pEffect->numPrograms ); impl.setProgram( static_cast<unsigned int>( integerParam ) ); break;
        case effGetProgram:      return impl.getProgram();

        case effSetProgramName:  BOOST_ASSERT( std::strlen( static_cast<char const *>( pData ) ) <= kVstMaxProgNameLen );
                                 impl.setProgramName( static_cast<char const *>( pData ) );
                                 break;
        case effGetProgramName:  impl.getProgramName( boost::make_iterator_range( castToReference<ProgramNameBuf>( pData ) ) );
                                 break;
        case effGetParamLabel  : Impl::getParameterLabel  ( parameterIndex, boost::make_iterator_range( castToReference<ParameterStringBuf>( pData ) ), pContextForDynamicParameterAccess ); break;
        case effGetParamDisplay: impl. getParameterDisplay( parameterIndex, boost::make_iterator_range( castToReference<ParameterStringBuf>( pData ) ), nullptr                           ); break;
        case effGetParamName   : Impl::getParameterName   ( parameterIndex, boost::make_iterator_range( castToReference<ParameterStringBuf>( pData ) ), pContextForDynamicParameterAccess );
                                 impl.inspectedParameter( parameterIndex );
                                 break;

        case effSetSampleRate:   impl.setSampleRate(                            floatParam     ); break;
        case effSetBlockSize :   impl.setBlockSize ( static_cast<unsigned int>( integerParam ) ); break;

        case effStartProcess:    return impl.preStreamingInitialization();
        case effStopProcess :    return impl.postStreamingCleanup      ();
        // http://www.kvraudio.com/forum/viewtopic.php?t=148304
        case effMainsChanged:    std::free( impl.pChunkData_ ); impl.pChunkData_ = nullptr;
                                 if ( integerParam ) impl.resume ();
                                 else                impl.suspend();
                                 break;

        //---Persistence-------
        case effGetChunk:
        {
            void const * & result = *static_cast<void const * *>( pData );

            bool         const currentProgramOnly( index != 0                                         );
            std::uint8_t const programsToGet     ( currentProgramOnly ? 1 : Impl::maxNumberOfPrograms );
            std::uint8_t const startProgramIndex ( currentProgramOnly ? impl.getProgram() : 0         );
            std::uint8_t const endProgramIndex   ( startProgramIndex + programsToGet                  );
            auto         const maximumProgramSize( impl.maximumProgramSize()                          );

            std::uint32_t const totalAllocationSize( Impl::maxNumberOfPrograms * ( maximumProgramSize + kVstMaxProgNameLen ) );
            void * const pChunkData( std::realloc( impl.pChunkData_, totalAllocationSize ) );
            if ( !pChunkData )
            {
                result = nullptr;
                return 0;
            }
            LE_TRACE_IF( impl.pChunkData_ && impl.pChunkData_ != pChunkData, "\tSW VST2.4: chunk storage relocated." );
            impl.pChunkData_ = pChunkData;

            char * pCurrentPosition( static_cast<char *>( pChunkData ) );

            castToReference<std::uint32_t>( pCurrentPosition ) = Impl::vstUniqueID; pCurrentPosition += sizeof( std::uint32_t );
            castToReference<std::uint32_t>( pCurrentPosition ) = Impl::version    ; pCurrentPosition += sizeof( std::uint32_t );
            castToReference<std::uint32_t>( pCurrentPosition ) = programsToGet    ; pCurrentPosition += sizeof( std::uint32_t );

            for ( auto programIndex( startProgramIndex ); programIndex < endProgramIndex; ++programIndex )
            {
                auto LE_MA & programNameSize( castToReference<std::uint32_t >( pCurrentPosition ) ); pCurrentPosition += sizeof( programNameSize );
                auto LE_MA & programName    ( castToReference<ProgramNameBuf>( pCurrentPosition ) );
                impl.getProgramName( programIndex, boost::make_iterator_range( programName ) );
                programNameSize = static_cast<std::uint32_t>( std::strlen( programName ) ) + 1;
                BOOST_ASSERT( programNameSize <= sizeof( programName ) );
                pCurrentPosition += programNameSize;
                auto LE_MA & programSize( castToReference<std::uint32_t>( pCurrentPosition ) )                                      ; pCurrentPosition += sizeof( programSize );
                std::uint32_t const actualProgramSize( impl.saveProgramState( programIndex, pCurrentPosition, maximumProgramSize ) ); pCurrentPosition += actualProgramSize;
                programSize = actualProgramSize;
            }
            auto const actualStorageUsed( static_cast<std::uint32_t>( pCurrentPosition - reinterpret_cast<char const *>( pChunkData ) ) );
            BOOST_ASSERT( actualStorageUsed <= totalAllocationSize );
            void * LE_RESTRICT const pShrunkChunkData( std::realloc( pChunkData, actualStorageUsed ) );
            BOOST_ASSERT( pShrunkChunkData );
        #if !( defined( __APPLE__ ) && __LP64__ ) //...mrmlj...investigate...
            BOOST_ASSERT( pShrunkChunkData == impl.pChunkData_ );
        #else
            LE_TRACE_IF( pShrunkChunkData == impl.pChunkData_, "\tSW VST2.4: chunk storage relocated when shrinking (%u -> %u).", totalAllocationSize, actualStorageUsed );
        #endif // __APPLE__ && __LP64__
            impl.pChunkData_ = pShrunkChunkData;
            result           = pShrunkChunkData;
            return actualStorageUsed;
        }

        case effSetChunk:
        {
            // http://www.kvraudio.com/forum/viewtopic.php?p=1096418

            char const * pCurrentPosition( static_cast<char *>( pData ) );

            auto const id( castToReference<std::uint32_t const>( pCurrentPosition ) ); pCurrentPosition += sizeof( id );
            if ( id != Impl::vstUniqueID )
            {
                LE_TRACE( "\tSW VST2.4: unrecognized chunk data." );
                return false;
            }

            bool         const currentProgramOnly( index != 0 );
            std::uint8_t const programsToGet     ( currentProgramOnly ? 1 : Impl::maxNumberOfPrograms );
            std::uint8_t const startProgramIndex ( currentProgramOnly ? impl.getProgram() : 0         );
            std::uint8_t const endProgramIndex   ( startProgramIndex + programsToGet                  );

            bool success( true );

            std::uint32_t const version( castToReference<std::uint32_t const>( pCurrentPosition ) ); pCurrentPosition += sizeof( version );
            if ( version < 2800 ) //...mrmlj...pre 2.8 SW hardcoded handling...
            {
                struct SWPre28Data
                {
                    typedef char ProgramData      [ 4096                   ];
                    typedef char ProgramNameBuffer[ kVstMaxProgNameLen + 1 ];
                    ProgramData       data [ Impl::maxNumberOfPrograms ];
                    ProgramNameBuffer names[ Impl::maxNumberOfPrograms ];
                };

                auto         const activeProgram( castToReference<std::uint32_t const>( pCurrentPosition ) ); pCurrentPosition += sizeof( activeProgram );
                auto const &       state        ( castToReference<SWPre28Data   const>( pCurrentPosition ) ); pCurrentPosition += sizeof( state         );
                BOOST_ASSERT( activeProgram == 0 || !currentProgramOnly );
                boost::ignore_unused_variable_warning( activeProgram );
                typename SWPre28Data::ProgramData       const * LE_RESTRICT pData( &state.data [ 0 ] );
                typename SWPre28Data::ProgramNameBuffer const * LE_RESTRICT pName( &state.names[ 0 ] );

                for ( std::uint8_t programIndex( startProgramIndex ); programIndex < endProgramIndex; ++programIndex )
                {
                    success &= impl.loadProgramState( programIndex, *pName++, *pData++, sizeof( *pData ) );
                }
            }
            else
            {
                std::uint32_t const numberOfPrograms( castToReference<std::uint32_t const>( pCurrentPosition ) ); pCurrentPosition += sizeof( numberOfPrograms );
                BOOST_ASSERT( numberOfPrograms == programsToGet );
                boost::ignore_unused_variable_warning( numberOfPrograms );
                for ( std::uint8_t programIndex( startProgramIndex ); programIndex < endProgramIndex; ++programIndex )
                {
                    auto         const programNameSize( castToReference<std::uint32_t const>( pCurrentPosition ) ); pCurrentPosition += sizeof( programNameSize );
                    char const * const pProgramName   ( pCurrentPosition                                         ); pCurrentPosition +=         programNameSize  ;
                    auto         const programSize    ( castToReference<std::uint32_t const>( pCurrentPosition ) ); pCurrentPosition += sizeof( programSize     );
                    BOOST_ASSERT( pProgramName[ programNameSize - 1 ] == 0 );
                    success &= impl.loadProgramState( programIndex, pProgramName, pCurrentPosition, programSize );
                    pCurrentPosition += programSize;
                }
            }

            return success;
        }

        //---VstEvents----------------------
        case effProcessEvents:
        {
            // SysEx
            // http://www.2writers.com/eddie/TutSysEx.htm
            // http://search.cpan.org/~hayashi/Win32API-MIDI-0.05/MIDI/SysEX/Roland.pm
            // http://www.keyboardmag.com/article/midi-system-exclusive/dec-06/24654
            // http://www.geocities.com/CapeCanaveral/Hangar/6226/sysex.html
            // http://en.wikipedia.org/wiki/General_MIDI
            // http://www.harmony-central.com/MIDI/Doc/table1.html
            ::VstEvents const & vstEvents( castToReference<::VstEvents const>( pData ) );
            // Implementation note:
            //   EnergyXT 2.5.2 sends zero-sized VST events so we must allow for
            // this value also.
            //                                (24.05.2010.) (Domagoj Saric)
            BOOST_ASSERT( vstEvents.numEvents >= 0 );
            ::VstEvent const * LE_RESTRICT const * LE_RESTRICT       ppEvent     = &vstEvents.events[ 0                   ];
            ::VstEvent const * LE_RESTRICT const * LE_RESTRICT const ppEventsEnd = &vstEvents.events[ vstEvents.numEvents ];
            while ( ppEvent != ppEventsEnd )
            {
                ::VstEvent const & event( *(*ppEvent++) );
                BOOST_ASSERT( event.type == kVstMidiType );
                impl.processMIDIEvent( reinterpret_cast<::VstMidiEvent const &>( event ) );
            }
            // VST SDK 2.4 documentation states that the return value is ignored.
            break;
        }

	    //---Parameters and Programs----------------------
	    case effCanBeAutomated:
            // http://www.asseca.org/vst-24-specs/efCanBeAutomated.html
            BOOST_ASSERT_MSG( index < pEffect->numParams, "VST2.4 host specified parameter index out of range." );
            return impl.canParameterBeAutomated( parameterIndex, pContextForDynamicParameterAccess );

        case effGetParameterProperties:
            return Impl::getParameterProperties
            (
                parameterIndex,
                static_cast<ParameterInformation &>( static_cast<::VstParameterProperties &>( castToReference<::VstParameterProperties>( pData ) ) ),
                pContextForDynamicParameterAccess
            );

	    case effGetProgramNameIndexed:
            BOOST_ASSERT( index < pEffect->numPrograms );
            //...mrmlj... Reaper still uses this...
            //BOOST_ASSERT( !integerParam ); // category deprecated in VST 2.4
            LE_TRACE_IF( integerParam && ( integerParam != -1 ), "\tSW VST2.4: a program category (deprecated in VST2.4) was specified (%d).", integerParam );
		    impl.getProgramName( index, boost::make_iterator_range( castToReference<ProgramNameBuf>( pData ) ) );
            return true;

        case effBeginSetProgram: return impl.beginSetProgram();
        case effEndSetProgram  : return impl.endSetProgram  ();

        case effBeginLoadBank   : return impl.beginLoadBank   ( castToReference<::VstPatchChunkInfo const>( pData ) );
        case effBeginLoadProgram: return impl.beginLoadProgram( castToReference<::VstPatchChunkInfo const>( pData ) );

        case effString2Parameter:
            // implement return string2parameter (index, (char*)ptr) ? 1 : 0;
            BOOST_ASSERT( pData == nullptr ); // A null pointer is used to check the capability (return true).
            return false;

        //---Connections, Configuration----------------------
	    case effGetInputProperties : if ( unsigned( index ) < impl.host().getNumInputs () ) { impl.getInputProperties ( index, castToReference<::VstPinProperties>( pData ) ); return true; }
                                     else return false;
        case effGetOutputProperties: if ( unsigned( index ) < impl.host().getNumOutputs() ) { impl.getOutputProperties( index, castToReference<::VstPinProperties>( pData ) ); return true; }
                                     else return false;
        case effSetSpeakerArrangement:
        {
            ::VstSpeakerArrangement const & inputArrangement ( castToReference<::VstSpeakerArrangement const>( integerParam ) );
            ::VstSpeakerArrangement const & outputArrangement( castToReference<::VstSpeakerArrangement const>( pData        ) );
            bool const success( impl.setSpeakerArrangement( inputArrangement, outputArrangement ) );
            if ( success )
            {
                //...mrmlj...
                pEffect->numInputs  = inputArrangement .numChannels;
                pEffect->numOutputs = outputArrangement.numChannels;
                bool const callbackNotificationSucceeded( impl.host().ioChanged() );
                BOOST_ASSERT( callbackNotificationSucceeded || !impl.host(). template canDo<AcceptIOChanges>() );
                boost::ignore_unused_variable_warning( callbackNotificationSucceeded );
            }
            return success;
        }
        case effGetSpeakerArrangement:
        {
            ::VstSpeakerArrangement * LE_RESTRICT & pInputArrangement  = castToReference<::VstSpeakerArrangement *>( integerParam );
            ::VstSpeakerArrangement * LE_RESTRICT & pOutputArrangement = castToReference<::VstSpeakerArrangement *>( pData        );

            unsigned int const preallocatedSpeakers    ( _countof( pInputArrangement->speakers )                                                    );
            unsigned int const speakerStorage          (  sizeof( *pInputArrangement->speakers )                                                    );
            unsigned int const inputChannels           ( impl.host().getNumInputs ()                                                                );
            unsigned int const outputChannels          ( impl.host().getNumOutputs()                                                                );
            unsigned int const inputArrangementStorage ( sizeof( *pInputArrangement  ) + ( inputChannels  - preallocatedSpeakers ) * speakerStorage );
            unsigned int const outputArrangementStorage( sizeof( *pOutputArrangement ) + ( outputChannels - preallocatedSpeakers ) * speakerStorage );
            unsigned int const totalArrangementStorage ( inputArrangementStorage + outputArrangementStorage                                         );
            unsigned char * LE_RESTRICT const pArrangementStorage( static_cast<unsigned char *>( std::realloc( impl.pSpeakerArrangementStorage_, totalArrangementStorage ) ) );
            if ( !pArrangementStorage )
            {
                BOOST_ASSERT( pInputArrangement  == nullptr );
                BOOST_ASSERT( pOutputArrangement == nullptr );
                return false;
            }
            LE_TRACE_IF( impl.pSpeakerArrangementStorage_ && impl.pSpeakerArrangementStorage_ != pArrangementStorage, "\tSW VST2.4: speaker arrangement storage relocated." );
            impl.pSpeakerArrangementStorage_ = pArrangementStorage;
            std::memset( pArrangementStorage, 0, totalArrangementStorage );
            pInputArrangement  = &reinterpret_cast<::VstSpeakerArrangement &>( pArrangementStorage[ 0                       ] );
            pOutputArrangement = &reinterpret_cast<::VstSpeakerArrangement &>( pArrangementStorage[ inputArrangementStorage ] );

            bool const success( impl.getSpeakerArrangement( *pInputArrangement, *pOutputArrangement ) );
            BOOST_ASSERT( success );
            LE_ASSUME   ( unsigned( pInputArrangement ->numChannels ) == inputChannels  );
            LE_ASSUME   ( unsigned( pOutputArrangement->numChannels ) == outputChannels );
            BOOST_ASSERT( impl.host().getNumInputs ()                 == inputChannels  );
            BOOST_ASSERT( impl.host().getNumOutputs()                 == outputChannels );
            return success;
        }

        case effSetPanLaw: return impl.setPanLaw( static_cast<::VstPanLawType>( integerParam ), floatParam );

        case effSetBypass: return impl.setBypass( integerParam != 0 );

        case effSetProcessPrecision: return impl.setProcessPrecision( static_cast<::VstProcessPrecision>( integerParam ) );

        case effGetTailSize:
        {
            unsigned int const tailSize( impl.getTailSize() );
            BOOST_ASSERT_MSG
            (
                tailSize <= Impl::maxTailSize,
                "You must not override getTailSize() and return more than you specified with the maxTailSize static constant."
            );
            return ( Impl::maxTailSize == 0 ) ? 1 : tailSize;
        }

        case effGetPlugCategory: return Impl::category;

        case effGetVstVersion: return 2400;

	    //---GUI/Editor----------------------
        case effEditOpen   :
        {
            bool const success( impl.createGUI( reinterpret_cast<Editor::WindowHandle>( pData ) ) );
            if ( success )
            {
                auto & gui ( *impl.gui() );
                auto & rect( impl.rect_  );
                float const scale       ( SW::GUI::displayScale() );
                auto  const width       ( gui.getWidth () );
                auto  const height      ( gui.getHeight() );
                float const scaledWidth ( Math::convert<float>( width  ) * scale );
                float const scaledHeight( Math::convert<float>( height ) * scale );
                rect.bottom = Math::convert<VstInt16>( scaledHeight );
                rect.right  = Math::convert<VstInt16>( scaledWidth  );
            }
            return success;
        }
        case effEditClose  : impl.destroyGUI(); break;
        case effEditIdle   : if ( impl.gui() ) impl.gui()->idle(); break;

	    case effEditKeyDown:
                             #if defined( __APPLE__ ) && !__LP64__
                                 if ( !impl.gui() ) return false; // Check if this is part of the cleanup message pump loop (see the note in ~ReferenceCountedGUIInitializationGuard())...
                             #endif // OSX x86
                             return impl.gui()->onKeyDown( static_cast<char>( index ), static_cast<VstVirtualKey>( integerParam ), static_cast<VstModifierKey>( static_cast<unsigned int>( floatParam ) ) );
	    case effEditKeyUp  :
                             #if defined( __APPLE__ ) && !__LP64__
                                 if ( !impl.gui() ) return false;
                             #endif // OSX x86
                             return impl.gui()->onKeyUp  ( static_cast<char>( index ), static_cast<VstVirtualKey>( integerParam ), static_cast<VstModifierKey>( static_cast<unsigned int>( floatParam ) ) );

        case effEditGetRect: BOOST_ASSERT( pData );
                             {
                                 /// \note Some hosts (like VSTScanner and
                                 /// Reaper) call GetRect before EditOpen so we
                                 /// must explicitly check for this...
                                 ///          (25.09.2014.) (Domagoj Saric)
                                 auto & rect( impl.rect_ );
                                 BOOST_ASSERT( !rect.left && !rect.top );
                                 // Implementation note:
                                 //   Wavelab does not check whether the editor
                                 // creation succeeded and seems to
                                 // access-violate if we "return" a null pointer
                                 // here so we create a dummy ERect.
                                 //           (23.11.2009.) (Domagoj Saric)
                                 // Implementation note:
                                 //   jBridge, Cubase 6 x64 bridge and possibly
                                 // the VST-AU bridges ask for the editor rect
                                 // before the editor is created and display a
                                 // small empty window if a valid size is not
                                 // returned (the Cubase bridge even does this
                                 // concurrently: calls effEditOpen and
                                 // effEditGetRect simultaneously from different
                                 // threads). Because of this, we create a
                                 // "best guess" ERect. We also have to return
                                 // true here, otherwise the Symbiosis VST-AU
                                 // wrapper fails.
                                 //           (05.05.2011.) (Domagoj Saric)
                                 if ( BOOST_UNLIKELY( !impl.gui() ) )
                                 {
                                     LE_TRACE( "\tSW VST2.4: host called EditGetRect before creating the editor." );
                                     namespace GUI = SW::GUI;
                                     auto const scale( GUI::isGUIInitialised() ? GUI::displayScale() : 1.0f );
                                     rect.bottom = Math::convert<VstInt16>( Impl::Editor::estimatedHeight * scale );
                                     rect.right  = Math::convert<VstInt16>( Impl::Editor::estimatedWidth  * scale );
                                 }
                                 BOOST_ASSERT( rect.bottom && rect.right );
                                 *reinterpret_cast<ERect const * *>( pData ) = &rect;
                                 return true;
                             }

        case effSetEditKnobMode: return impl.gui() ? impl.gui()->setKnobMode( static_cast<Editor::KnobMode>( integerParam ) ) : false;

        //---MIDI----------------------
        case effGetMidiProgramName      : return impl.getMidiProgramName    ( index, castToReference<::MidiProgramName    >( pData ) );
	    case effGetCurrentMidiProgram   : return impl.getCurrentMidiProgram ( index, castToReference<::MidiProgramName    >( pData ) );
        case effGetMidiProgramCategory  : return impl.getMidiProgramCategory( index, castToReference<::MidiProgramCategory>( pData ) );
	    case effHasMidiProgramsChanged  : return impl.hasMidiProgramsChanged( index                                                  );
        case effGetMidiKeyName          : return impl.getMidiKeyName        ( index, castToReference<::MidiKeyName        >( pData ) );

        case effGetNumMidiInputChannels : return impl.getNumMidiInputChannels ();
        case effGetNumMidiOutputChannels: return impl.getNumMidiOutputChannels();

	    //---OfflineProcessing----------------------
	    case effOfflineNotify : return impl.offlineNotify ( castToReference<VstAudioFile   const>( pData ), static_cast<unsigned int>( integerParam ), index != 0 );
	    case effOfflinePrepare: return impl.offlinePrepare( castToReference<VstOfflineTask const>( pData ), static_cast<unsigned int>( integerParam )             );
	    case effOfflineRun    : return impl.offlineRun    ( castToReference<VstOfflineTask const>( pData ), static_cast<unsigned int>( integerParam )             );

        case effProcessVarIo  : return impl.processVariableIo( static_cast<VstVariableIo const *>( pData ) );

        case effSetTotalSampleToProcess:
            impl.setTotalSamplesToProcess( static_cast<unsigned int>( integerParam ) );
            return true; // ??

	    //---Others----------------------
	    case effGetEffectName   : std::strcpy( castToReference<EffectNameBuf   >( pData ), Impl::name          ); return true;
	    case effGetVendorString : std::strcpy( castToReference<VendorStringBuf >( pData ), "Little Endian"     ); return true;
	    case effGetProductString: std::strcpy( castToReference<ProductStringBuf>( pData ), Impl::productString ); return true;
        case effGetVendorVersion: return Impl::version;

        case effVendorSpecific: return impl.pluginSpecific( index, integerParam, pData, floatParam );

        case effCanDo:
        {
            BOOST_ASSERT( pData );
            boost::string_ref const capability( static_cast<char const *>( pData ) );

            LE_TRACE( "\tSW VST2.4: capability query: %s. Answer:\n\t", capability.begin() );

                 if ( capability == "sendVstEvents"         ) return impl. template queryImplementationCapability<SendVSTEvents        >();
            else if ( capability == "sendVstMidiEvent"      ) return impl. template queryImplementationCapability<SendMIDIEvent        >();
            else if ( capability == "receiveVstEvents"      ) return impl. template queryImplementationCapability<ReceiveVSTEvents     >();
            else if ( capability == "receiveVstMidiEvent"   ) return impl. template queryImplementationCapability<ReceiveMIDIEvent     >();
            else if ( capability == "receiveVstTimeInfo"    ) return impl. template queryImplementationCapability<ReceiveVSTTimeInfo   >();
            else if ( capability == "offline"               ) return impl. template queryImplementationCapability<OfflineProcessing    >();
            else if ( capability == "midiProgramNames"      ) return impl. template queryImplementationCapability<MidiProgramNames     >();
            else if ( capability == "bypass"                ) return impl. template queryImplementationCapability<Bypass               >();
            else if ( capability == "mixDryWet"             ) return impl. template queryImplementationCapability<MixDryWet            >();
            else if ( capability == "noRealTime"            ) return impl. template queryImplementationCapability<NonRealTimeProcessing>();
            else if ( capability == "plugAsChannelInsert"   ) return impl. template queryImplementationCapability<AsInsert             >();
            else if ( capability == "plugAsSend"            ) return impl. template queryImplementationCapability<AsSend               >();
            else if ( capability == "1in1out"               ) return impl. template queryImplementationCapability<Ch1in1out            >();
            else if ( capability == "1in2out"               ) return impl. template queryImplementationCapability<Ch1in2out            >();
            else if ( capability == "2in1out"               ) return impl. template queryImplementationCapability<Ch2in1out            >();
            else if ( capability == "2in2out"               ) return impl. template queryImplementationCapability<Ch2in2out            >();
            else if ( capability == "2in4out"               ) return impl. template queryImplementationCapability<Ch2in4out            >();
            else if ( capability == "4in2out"               ) return impl. template queryImplementationCapability<Ch4in2out            >();
            else if ( capability == "4in4out"               ) return impl. template queryImplementationCapability<Ch4in4out            >();
            else if ( capability == "4in8out"               ) return impl. template queryImplementationCapability<Ch4in8out            >();
            else if ( capability == "8in4out"               ) return impl. template queryImplementationCapability<Ch8in4out            >();
            else if ( capability == "8in8out"               ) return impl. template queryImplementationCapability<Ch8in8out            >();
            else if ( capability == "conformsToWindowRules" ) return impl. template queryImplementationCapability<UsesAFixedSizeGUI    >();
        #if defined( __APPLE__ ) && !defined( __x86_64__ )
            //...mrmlj...for testing...
            //else if ( capability == "hasCockosViewAsConfig" ) return 0xBEEF0000;
        #endif
            else
            {
            #ifdef _DEBUG
                if
                (
                    capability == "multipass"          ||
                    capability == "metapass"           ||
                    // Host specific can-do strings:
                    capability == "LiveWithoutToolbar" ||   // Cubase SX
                    capability == "supportsARA"             // Studio One 2
                )
                {
                    LE_TRACE( "\tSW VST2.4: a 'known but not understood' canDo query was received (%s).\n", capability.begin() );
                }
                else
                {
                    LE_TRACE( "\tSW VST2.4: an unknown canDo query was received (%s).\n"                  , capability.begin() );
                }
            #endif // _DEBUG
                return DontKnow;
            }
        }

	    case effShellGetNextPlugin:
            BOOST_ASSERT( !"Currently we do not support nor plan 'shell plugins' so this should not get called;" );
            LE_UNREACHABLE_CODE();
            break;

    #if !VST_FORCE_DEPRECATED

        //---Parameters and Programs----------------------
        case DECLARE_VST_DEPRECATED( effGetNumProgramCategories   ): // http://homepage.hik.se/staff/tkama/audio_procME/doc/html/plug/2.0/AudioEffectX.html#getNumCategories
        case DECLARE_VST_DEPRECATED( effCopyProgram               ): // http://homepage.hik.se/staff/tkama/audio_procME/doc/html/plug/2.0/AudioEffectX.html#copyProgram
        //---Connections, Configuration----------------------
        case DECLARE_VST_DEPRECATED( effConnectInput              ):
        case DECLARE_VST_DEPRECATED( effConnectOutput             ):
        //---Realtime----------------------
        case DECLARE_VST_DEPRECATED( effGetCurrentPosition        ):
        case DECLARE_VST_DEPRECATED( effGetDestinationBuffer      ):
        //---GUI/Editor----------------------
    #ifdef __APPLE__
        case DECLARE_VST_DEPRECATED( effEditMouse                 ):
        case DECLARE_VST_DEPRECATED( effEditKey                   ):
    #endif // __APPLE__
        /// \note Samplitude Pro X sends these even on Windows.
        ///                                   (04.01.2012.) (Domagoj Saric)
        case DECLARE_VST_DEPRECATED( effEditDraw                  ):
        case DECLARE_VST_DEPRECATED( effEditTop                   ):
        /// \note Cantabile 2 sends this one even on Windows.
        ///                                   (27.09.2013.) (Domagoj Saric)
        case DECLARE_VST_DEPRECATED( effEditSleep                 ):

        //---Others----------------------
        case DECLARE_VST_DEPRECATED( effSetBlockSizeAndSampleRate ):
        case DECLARE_VST_DEPRECATED( effGetErrorText              ):
        case DECLARE_VST_DEPRECATED( effGetIcon                   ):
        case DECLARE_VST_DEPRECATED( effSetViewPosition           ):
        case DECLARE_VST_DEPRECATED( effIdle                      ):
        case DECLARE_VST_DEPRECATED( effKeysRequired              ):
        case DECLARE_VST_DEPRECATED( effGetVu                     ):
            // Implementation note:
            //   We do not warn about "effIdle" to avoid spamming the output
            //   console/log (Reaper is known to use it).
            //                                (13.10.2009.) (Domagoj Saric)
            LE_TRACE_IF
            (
                opCode != DECLARE_VST_DEPRECATED( effIdle ),
                "\tSW VST2.4: deprecated VST opcode invoked (opcode: %d, integer: %d, intptr: %d, pointer: %p, float: %f).\n",
                opCode, index, integerParam, pData, floatParam
            );
            return false;

    #endif // !VST_FORCE_DEPRECATED

        case DECLARE_VST_DEPRECATED( effIdentify ): return CCONST( 'N', 'v', 'E', 'f' );

        #ifdef _DEBUG
        default:
            LE_TRACE( "\tSW VST2.4: unhandled VST opcode invoked (opcode: %d, integer: %d, intptr: %d, pointer: %p, float: %f).\n", opCode, index, integerParam, pData, floatParam );
            static const int knownUknownOpcodes[] =
            {
                   -1, // VST scanner
                65536  // FL Studio 9
            };
            if ( boost::find( knownUknownOpcodes, opCode ) == boost::end( knownUknownOpcodes ) )
              BOOST_ASSERT( !"Unhandled VST opcode." );
        #endif
    }

    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin::getParameter()
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
float LE_NOTHROWNOALIAS Plugin<Impl, Protocol::VST24>::getParameter( ::AEffect * LE_RESTRICT const pEffect, ::VstInt32 const index ) /// \throws nothing
{
    BOOST_ASSERT_MSG( _MM_GET_ROUNDING_MODE() == _MM_ROUND_NEAREST, "Unexpected rounding mode." );
    auto const & impl( Plugin::impl( pEffect ) );
    auto const parameterIndex( ParameterIndex{ static_cast<ParameterIndex::value_type>( index ) } );
    impl.inspectedParameter( parameterIndex );
    AutomatedParameterValue const result( impl.getParameter( parameterIndex ) );
    BOOST_ASSERT( std ::isfinite         ( result ) );
    BOOST_ASSERT( Math::isNormalisedValue( result ) );
    return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin::setParameter()
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
void LE_NOTHROW Plugin<Impl, Protocol::VST24>::setParameter( ::AEffect * LE_RESTRICT const pEffect, ::VstInt32 const index, float const value ) /// \throws nothing
{
    BOOST_ASSERT_MSG( _MM_GET_ROUNDING_MODE() == _MM_ROUND_NEAREST, "Unexpected rounding mode." );
    LE_ASSUME( std::isfinite( value ) );
    BOOST_ASSERT( Math::isNormalisedValue( value ) );
    auto const parameterIndex( ParameterIndex{ static_cast<ParameterIndex::value_type>( index ) } );
    if ( impl( pEffect ).setParameter( parameterIndex, value ) == ErrorCode::Success )
    {
    #ifndef NDEBUG
        /// \note A simple/direct comparison only works for float parameters.
        ///                                   (11.09.2014.) (Domagoj Saric)
        ::VstParameterProperties info;
        if
        (
            Impl::getParameterProperties
            (
                parameterIndex,
                static_cast<ParameterInformation &>( info ),
                &impl( pEffect ).dynamicParameterAccessContext()
            )
            &&
            ( info.flags & ( kVstParameterUsesIntegerMinMax | kVstParameterUsesFloatStep | kVstParameterUsesIntStep ) ) == 0
        )
        {
            BOOST_ASSERT( getParameter( pEffect, index ) == value );
        }
    #endif // NDEBUG
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin<>::queryImplementationCapability<>()
// ------------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \brief Returns whether the plugin supports a given capability (either
/// always/from the static Capabilities list, or currently/dynamically).
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
template <PluginCapability pluginCapability>
VSTPluginBase::CanDoAnswer Plugin<Impl, Protocol::VST24>::queryImplementationCapability() const
{
#ifndef NDEBUG
    Plugins::PluginCapability const capability( pluginCapability );
    boost::ignore_unused_variable_warning( capability );
#endif

    bool const isImplementationCapable
    (
        boost::mpl::has_key
        <
            typename Impl::Capabilities,
            boost::mpl::integral_c<typename Impl::Capabilities::value_type, pluginCapability>
        >::value
    );

    VSTPluginBase::CanDoAnswer const result( ( isImplementationCapable || impl(). template queryDynamicCapability<pluginCapability>() ) ? Can : Cannot );
    LE_TRACE( "\t\tanswer: %d\n", result );
    return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// VST24Plugin<Impl>::impl()
// -------------------------
//
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   MSVC++ generates redundant code to check whether this == 0.
//                                            (21.09.2010.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
Impl & Plugin<Impl, Protocol::VST24>::impl() /// \throws nothing
{
    LE_ASSUME( this != 0 );
    return *static_cast<Impl * LE_RESTRICT>( this );
}

template <class Impl>
Impl & Plugin<Impl, Protocol::VST24>::impl( ::AEffect * LE_RESTRICT const pEffect ) /// \throws nothing
{
    LE_ASSUME( pEffect != 0 );
    return static_cast<Plugin * LE_RESTRICT>( fromAEffect( pEffect ) )->impl();
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
