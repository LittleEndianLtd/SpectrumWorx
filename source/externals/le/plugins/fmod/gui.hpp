////////////////////////////////////////////////////////////////////////////////
///
/// \file gui.hpp
/// -------------
///
/// Copyright (c) 2014. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef gui_hpp__F1C46CB8_A6C0_48D8_B620_30EF0B7EE8B8
#define gui_hpp__F1C46CB8_A6C0_48D8_B620_30EF0B7EE8B8
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/plugins/entryPoint.hpp"
#include "le/plugins/plugin.hpp"
#include "le/utility/objcfwdhelpers.hpp"
#include "le/utility/parentFromMember.hpp"

#include "fmod.h"
#include "fmod_studio_plugin_sdk.hpp"

#include "boost/optional/optional.hpp" // Boost sandbox

#include "boost/utility/in_place_factory.hpp"
//------------------------------------------------------------------------------
#if defined( _WIN32 )
    struct HWND__;
    typedef struct HWND__ * HWND;
#endif // _WIN32
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

extern FMOD_DSP_DESCRIPTION const * LE_RESTRICT pUgh; //...mrmlj...

////////////////////////////////////////////////////////////////////////////////
///
/// \class FMODEditorBase
///
/// \brief Provides the bare minimum for an FMOD Designer/Studio editor.
///
////////////////////////////////////////////////////////////////////////////////

class FMODEditorBase
{
public:
    typedef
        #if   defined( _WIN32    )
            HWND
        #elif defined( __APPLE__ )
            ObjC::NSView *
        #endif
    WindowHandle;

    typedef ParameterIndex ParameterSelector;

    // dummy implementations of unsupported operations...
    static bool parameterListChanged() { return false; }
    static bool updateDisplay       () { return false; }
  //static bool reportNewNumberOfIOChannels( unsigned int /*inputs*/, unsigned int /*sideInputs*/, unsigned int /*outputs*/ ) { return false; }
    static bool reportNewLatencyInSamples( unsigned int /*samples*/ ) { return false; }

public: // Parameter automation.
           void automatedParameterChanged  ( ParameterIndex, float ) const;
    static void automatedParameterBeginEdit( ParameterIndex        ) {}
    static void automatedParameterEndEdit  ( ParameterIndex        ) {}

    void gestureBegin( char const * description ) const;
    void gestureEnd  (                          ) const;

    void presetChangeBegin() const { gestureBegin( "Preset change" ); }
    void presetChangeEnd  () const { gestureEnd  (                 ); }

    float getParameter( ParameterSelector ) const;

    static bool wantsManualDependentParameterNotifications() { return false; }

protected:
    static void syncStudioWindowSize( WindowHandle, unsigned int width, unsigned int height );

protected:
    typedef FMOD::                   PLUGIN_PARAMETER_VALUE   AutomatedParameterValue;

    typedef FMOD::StudioAudioPlugin::EDITOR_TO_HOST_CALLBACKS Editor2HostCallbacks;
    typedef FMOD::StudioAudioPlugin::HOST_TO_EDITOR_CALLBACKS Host2EditorCallbacks;

    static FMOD_RESULT F_CALLBACK setHostCallbacks( Editor2HostCallbacks const * const pCallbacks ) { plugin2HostCallbacks_ = *pCallbacks; return FMOD_OK; }

protected:
    static Editor2HostCallbacks plugin2HostCallbacks_;
}; // class FMODEditorBase


template <class Impl>
class FMODEditor : public FMODEditorBase
{
public:
    static Host2EditorCallbacks const & callbacks() { return host2PluginCallbacks_; }

protected:
    FMOD_RESULT getLocalState( void * * const ppState, unsigned int * const pStateSize ) const
    {
        typedef decltype( impl().getPosition() ) position_t;
        static position_t position;
        position = impl().getPosition();
        *ppState    = &position;
        *pStateSize = sizeof( position );
        return FMOD_OK;
    }

    FMOD_RESULT setState( void const * const pState, unsigned int const stateSize )
    {
        if ( !stateSize )
            return FMOD_OK;
        typedef decltype( impl().getPosition() ) position_t;
        BOOST_ASSERT_MSG( pState && ( stateSize == sizeof( position_t ) ), "Invalid editor state object" );
        impl().setTopLeftPosition( *static_cast<position_t const *>( pState ) );
        return FMOD_OK;
    }

private:
    typedef boost::optional2<Impl> OptionalEditor;

    Impl       & impl()       { return **optionalEditorFromBase( this, true ); }
    Impl const & impl() const { return const_cast<FMODEditor &>( *this ).impl(); }

    static FMODEditorBase * baseFromOptionalEditor( OptionalEditor * const pOptionalEditor )
    {
        LE_ASSUME( pOptionalEditor );
        Impl * const pImpl( reinterpret_cast<Impl *>( pOptionalEditor ) );
        BOOST_ASSERT( pOptionalEditor->get_ptr() == pImpl || pOptionalEditor->get_ptr() == nullptr );
        return pImpl;
    }

    static FMODEditorBase * baseFromConstructedEditor( OptionalEditor * const pOptionalEditor )
    {
        LE_ASSUME( pOptionalEditor );
        Impl * const pImpl( pOptionalEditor->get_ptr() );
        LE_ASSUME( pImpl );
        return pImpl;
    }

    static OptionalEditor * optionalEditorFromBase( FMODEditorBase * const pEditorBase, bool const assumeConstructed = false )
    {
        LE_ASSUME( pEditorBase );
        Impl           * const pImpl          ( static_cast     <Impl           *>( pEditorBase ) );
        OptionalEditor * const pOptionalEditor( reinterpret_cast<OptionalEditor *>( pImpl       ) );
        LE_ASSUME( ( pOptionalEditor->get_ptr() == pImpl ) || ( !assumeConstructed && pOptionalEditor->get_ptr() == nullptr ) );
        return pOptionalEditor;
    }

private:
    static FMOD_RESULT F_CALLBACK create              ( void * * ppEditor                                                                     );
    static FMOD_RESULT F_CALLBACK destroy             ( void *   pEditor                                                                      );
    static FMOD_RESULT F_CALLBACK show                ( void *   pEditor, int windowHandle, void const *    pState, unsigned int    stateSize );
    static FMOD_RESULT F_CALLBACK getLocalState       ( void *   pEditor,                   void       * * ppState, unsigned int * pStateSize );
    static FMOD_RESULT F_CALLBACK updateParameterValue( void *   pEditor, int index, AutomatedParameterValue const *                          );
    static FMOD_RESULT F_CALLBACK updateSharedState   ( void *   pEditor, void const * pSharedState, unsigned int sharedStateSize             );
    static FMOD_RESULT F_CALLBACK printParameterValue ( void *   pEditor, unsigned int parameterIndex, FMOD::PLUGIN_PARAMETER_VALUE const &, FMOD::StudioAudioPlugin::ParameterValueStringBuffer & );

private:
    static Host2EditorCallbacks const host2PluginCallbacks_;
}; // class FMODEditor


#ifdef LE_SW_FMOD_SHARED_BUILD
#define LE_PLUGIN_FMOD_GUI_ENTRY_POINT( implClass )                                                                                 \
    extern "C" F_DECLSPEC F_DLLEXPORT                                                                                               \
    FMOD::StudioAudioPlugin::HOST_TO_EDITOR_CALLBACKS * F_STDCALL FMODStudioAudioPluginEditorCallbacks()                            \
    {                                                                                                                               \
        return const_cast<FMOD::StudioAudioPlugin::HOST_TO_EDITOR_CALLBACKS *>( &LE::Plugins::FMODEditor<implClass>::callbacks() ); \
    }
#else
#define LE_PLUGIN_FMOD_GUI_ENTRY_POINT( implClass )                                                                                 \
    extern "C"                                                                                                                      \
    FMOD::StudioAudioPlugin::HOST_TO_EDITOR_CALLBACKS * F_STDCALL FMOD_LittleEndian_SpectrumWorx_StudioAudioPluginEditorCallbacks() \
    {                                                                                                                               \
        return const_cast<FMOD::StudioAudioPlugin::HOST_TO_EDITOR_CALLBACKS *>( &LE::Plugins::FMODEditor<implClass>::callbacks() ); \
    }
#endif // LE_SW_FMOD_SHARED_BUILD


template <class Impl>
FMODEditorBase::Host2EditorCallbacks const FMODEditor<Impl>::host2PluginCallbacks_ =
{
    /*sdkversion           =*/ FMOD_STUDIO_SDK_VERSION,
    /*setHostCallbacks     =*/ &FMODEditorBase::setHostCallbacks    ,
    /*create               =*/ &FMODEditor    ::create              ,
    /*destroy              =*/ &FMODEditor    ::destroy             ,
    /*show                 =*/ &FMODEditor    ::show                ,
    /*getLocalState        =*/ &FMODEditor    ::getLocalState       ,
    /*updateParameterValue =*/ &FMODEditor    ::updateParameterValue,
    /*updateSharedState    =*/ &FMODEditor    ::updateSharedState   ,
    /*printParameterValue  =*/ &FMODEditor    ::printParameterValue ,
};


template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::create( void * * const ppEditor )
{
    try
    {
        OptionalEditor * const pNewEditorHolder( new OptionalEditor( boost::in_place() ) );
        (*pNewEditorHolder)->setInvisible();
        *ppEditor = baseFromOptionalEditor( pNewEditorHolder );
        return FMOD_OK;
    }
    catch ( ... )
    {
        *ppEditor = nullptr;
        return FMOD_ERR_MEMORY;
    }
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::destroy( void * const pEditor )
{
    OptionalEditor * const pOptionalEditor( optionalEditorFromBase( static_cast<FMODEditorBase *>( pEditor ), true ) );
    delete pOptionalEditor;
    return FMOD_OK;
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::show( void * const pEditor, int const parentWindowHandle, void const * const pState, unsigned int const stateSize )
{
    OptionalEditor & editor( *optionalEditorFromBase( static_cast<FMODEditorBase *>( pEditor ), true ) );
    FMODEditorBase::WindowHandle const parentWindow( reinterpret_cast<FMODEditorBase::WindowHandle>( parentWindowHandle ) );
    editor->attachToHostWindow( parentWindow      );
    editor->setState          ( pState, stateSize );
    syncStudioWindowSize( parentWindow, editor->getWidth(), editor->getHeight() );
    /// \note FMOD Studio does not seem to set the current state of
    /// parameters when creating the GUI so we have to manually iterate,
    /// poll and set them accordingly.
    ///                                   (20.10.2014.) (Domagoj Saric)
    //dependentParameterCache_.reloadModuleChain( *this );
    std::uint_fast16_t const numberOfParameters( /*Impl::maxNumberOfParameters*/unsigned( pUgh->numparameters ) - 1 /*side chain parameter*/ );
    for ( Plugins::ParameterIndex parameterIndex( 0 ); parameterIndex.value < numberOfParameters; ++parameterIndex.value )
        editor->setParameter( parameterIndex, editor->getParameter( parameterIndex ) );
    editor->setVisible();
    return FMOD_OK;
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::getLocalState( void * const pEditor, void * * const ppState, unsigned int * const pStateSize )
{
    OptionalEditor & editor( *optionalEditorFromBase( static_cast<FMODEditorBase *>( pEditor ), true ) );
    BOOST_ASSERT_MSG( editor.is_initialized(), "Editor not yet shown" );
    editor->getLocalState( ppState, pStateSize );
    return FMOD_OK;
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::updateParameterValue( void * const pEditor, int const index, AutomatedParameterValue const * LE_RESTRICT const pValue )
{
    OptionalEditor & editor( *optionalEditorFromBase( static_cast<FMODEditorBase *>( pEditor ), true ) );
    ParameterIndex const parameterIndex( index );
    float value;
    switch ( pValue->type )
    {
        case FMOD_DSP_PARAMETER_TYPE_FLOAT: value = pValue->floatvalue; break;
        case FMOD_DSP_PARAMETER_TYPE_INT  : value = pValue->intvalue  ; break;
        case FMOD_DSP_PARAMETER_TYPE_BOOL : value = pValue->boolvalue ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    BOOST_ASSERT_MSG( editor->getParameter( parameterIndex ) == value, "Updated parameter value out of sync" );
    return static_cast<FMOD_RESULT>( editor->setParameter( parameterIndex, value ) );
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::updateSharedState( void * /*const pEditor*/, void const * /*const pSharedState*/, unsigned int /*const sharedStateSize*/ )
{
    return FMOD_ERR_UNIMPLEMENTED;
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK FMODEditor<Impl>::printParameterValue
(
    void * const pEditor,
    unsigned int const parameterIndex,
    FMOD::PLUGIN_PARAMETER_VALUE const & value,
    FMOD::StudioAudioPlugin::ParameterValueStringBuffer & buffer
)
{
    OptionalEditor & editor( *optionalEditorFromBase( static_cast<FMODEditorBase *>( pEditor ), true ) );
    float internalValue;
    switch ( value.type )
    {
        case FMOD_DSP_PARAMETER_TYPE_FLOAT: internalValue = value.floatvalue; break;
        case FMOD_DSP_PARAMETER_TYPE_INT  : internalValue = value.intvalue  ; break;
        case FMOD_DSP_PARAMETER_TYPE_BOOL : internalValue = value.boolvalue ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    editor->getParameterDisplay( ParameterIndex( parameterIndex ), buffer, internalValue );
    return FMOD_OK;
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // gui_hpp
