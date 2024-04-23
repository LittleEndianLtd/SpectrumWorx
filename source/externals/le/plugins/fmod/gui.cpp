////////////////////////////////////////////////////////////////////////////////
///
/// gui.cpp
/// -------
///
/// Copyright (c) 2014. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "gui.hpp"

#if defined( __APPLE__ )
    #include <Cocoa/Cocoa.h>
#elif defined( _WIN32 )
    #include "windows.h"
#endif // OS
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

FMODEditorBase::Editor2HostCallbacks FMODEditorBase::plugin2HostCallbacks_;

FMOD_DSP_DESCRIPTION const * LE_RESTRICT pUgh; //...mrmlj...get/setParameter quick-fixes...


void FMODEditorBase::gestureBegin( char const * const description ) const
{
    BOOST_VERIFY( plugin2HostCallbacks_.beginEdit( const_cast<FMODEditorBase *>( this ), description ) == FMOD_OK );
}


void FMODEditorBase::gestureEnd() const
{
    BOOST_VERIFY( plugin2HostCallbacks_.endEdit( const_cast<FMODEditorBase *>( this ) ) == FMOD_OK );
}


void FMODEditorBase::automatedParameterChanged( ParameterIndex const parameterIndex, float const parameterValue ) const
{
    //...mrmlj...properly fix/solve this (use a discriminated union for passing
    //...mrmlj...around automated parameter values and let individual protocols
    //...mrmlj...do whatever they require)...
    FMOD_DSP_PARAMETER_DESC const & desc( *pUgh->paramdesc[ parameterIndex.value ] );
    AutomatedParameterValue value;
    value.type = desc.type;
    switch ( value.type )
    {
        case FMOD_DSP_PARAMETER_TYPE_FLOAT: value.floatvalue = static_cast<float>( parameterValue )   ; LE_ASSUME( parameterValue >= desc.floatdesc.min && parameterValue <= desc.floatdesc.max ); break;
        case FMOD_DSP_PARAMETER_TYPE_INT  : value.intvalue   = static_cast<int  >( parameterValue )   ; LE_ASSUME( parameterValue >= desc.intdesc  .min && parameterValue <= desc.intdesc  .max ); break;
        case FMOD_DSP_PARAMETER_TYPE_BOOL : value.boolvalue  =                     parameterValue != 0; LE_ASSUME( parameterValue == 0                  || parameterValue == 1                  ); break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }

    BOOST_VERIFY( plugin2HostCallbacks_.setParameter( const_cast<FMODEditorBase *>( this ), parameterIndex.value, &value ) == FMOD_OK );
}


float FMODEditorBase::getParameter( ParameterSelector const index ) const
{
    //...mrmlj...properly fix/solve this (use a discriminated union for passing
    //...mrmlj...around automated parameter values and let individual protocols
    //...mrmlj...do whatever they require)...
    FMOD_DSP_PARAMETER_DESC const & desc( *pUgh->paramdesc[ index.value ] );
    AutomatedParameterValue value;
    value.type = desc.type;
    BOOST_VERIFY( plugin2HostCallbacks_.getParameter( const_cast<FMODEditorBase *>( this ), index.value, &value ) == FMOD_OK );
    switch ( desc.type )
    {
        case FMOD_DSP_PARAMETER_TYPE_FLOAT: return value.floatvalue; break;
        case FMOD_DSP_PARAMETER_TYPE_INT  : return value.intvalue  ; break;
        case FMOD_DSP_PARAMETER_TYPE_BOOL : return value.boolvalue ; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


void FMODEditorBase::syncStudioWindowSize( WindowHandle const studioWindowHandle, unsigned int const width, unsigned int const height )
{
#if defined( __APPLE__ )

    auto const studioView    ( reinterpret_cast<NSView *>( studioWindowHandle ) );
    auto const studioViewSize( studioView.frame.size );
    auto const newStudioViewSize
    (
        NSMakeSize
        (
            studioViewSize.width  + width,
            studioViewSize.height + height
        )
    );
    studioView.frame.size = newStudioViewSize;

#elif defined( _WIN32 )

    // Turn off the 'resizeable' and 'always on top' window styles
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms633591(v=vs.85).aspx
    LONG_PTR const studioWindowStyles        ( ::GetWindowLongPtr( studioWindowHandle, GWL_STYLE   ) );
    LONG_PTR const studioWindowExtendedStyles( ::GetWindowLongPtr( studioWindowHandle, GWL_EXSTYLE ) );
    BOOST_VERIFY( ::SetWindowLongPtr( studioWindowHandle, GWL_STYLE  , studioWindowStyles         & ~WS_SIZEBOX    ) == studioWindowStyles         );
    BOOST_VERIFY( ::SetWindowLongPtr( studioWindowHandle, GWL_EXSTYLE, studioWindowExtendedStyles & ~WS_EX_TOPMOST ) == studioWindowExtendedStyles );
    BOOST_VERIFY
    (
        ::SetWindowPos
        (
            studioWindowHandle, nullptr,
            0, 0, 0, 0,
            SWP_FRAMECHANGED | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOREDRAW | SWP_NOSENDCHANGING | SWP_NOSIZE
        )
    );

    // Sync the parent window size
    // http://msdn.microsoft.com/en-us/library/windows/desktop/ms632599(v=vs.85).aspx
    // http://stackoverflow.com/questions/431470/window-border-width-and-height-in-win32-how-do-i-get-it
    ::RECT parentFullRectangle  ; BOOST_VERIFY( ::GetWindowRect( studioWindowHandle, &parentFullRectangle   ) );
    ::RECT parentClientRectangle; BOOST_VERIFY( ::GetClientRect( studioWindowHandle, &parentClientRectangle ) );

    unsigned int const fullWidth   ( parentFullRectangle  .right  - parentFullRectangle  .left );
    unsigned int const fullHeight  ( parentFullRectangle  .bottom - parentFullRectangle  .top  );

    unsigned int const clientWidth ( parentClientRectangle.right  - parentClientRectangle.left );
    unsigned int const clientHeight( parentClientRectangle.bottom - parentClientRectangle.top  );

    unsigned int const studioWindowExtraWidth ( fullWidth  - clientWidth  );
    unsigned int const studioWindowExtraHeight( fullHeight - clientHeight );

    BOOST_VERIFY
    (
        ::SetWindowPos
        (
            studioWindowHandle, nullptr,
            0, 0, width + studioWindowExtraWidth, height + studioWindowExtraHeight,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_NOREDRAW
        )
    );

#endif // OS
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
