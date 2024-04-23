////////////////////////////////////////////////////////////////////////////////
///
/// debugConsole.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
// http://cmake.3232098.n2.nabble.com/VS2010-subsystem-console-only-in-debug-build-td7348127.html
#if defined( _WIN32 ) && !defined( _DEBUG )
    #pragma comment( linker, "/SUBSYSTEM:WINDOWS" )
#endif // _WIN32

#if defined( _DEBUG ) && defined( _WIN32 )
#include "boost/optional/optional.hpp" // Boost sandbox

#include "boost/assert.hpp"
#include "boost/noncopyable.hpp"
#include "boost/utility/in_place_factory.hpp"

#include "windows.h"

#include <cstdio>
#include "tchar.h"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

class Console : boost::noncopyable
{
public:
    Console()
    {
        BOOST_VERIFY( ::AllocConsole() );
        BOOST_VERIFY( std::freopen( "conin$" , "r", stdin  ) );
        BOOST_VERIFY( std::freopen( "conout$", "w", stdout ) );
        BOOST_VERIFY( std::freopen( "conout$", "w", stderr ) );
    }
    ~Console()
    {
        BOOST_VERIFY( ::FreeConsole() );
    }
};

boost::optional<Console> debugConsole;

//------------------------------------------------------------------------------
} // namespace LE

static struct Ha
{
    Ha()
    {
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF );
        //_CrtSetBreakAlloc(1);
        //_CrtSetBreakAlloc(2);
        //_CrtSetBreakAlloc(4);
        //_CrtSetBreakAlloc(149);
        //_CrtSetBreakAlloc(159);
        //_CrtSetBreakAlloc(238);
        //_CrtSetBreakAlloc(241);
    }
} const ha;

BOOL WINAPI DllMain( HINSTANCE /*hInst*/, DWORD const dwReason, LPVOID /*lpvReserved*/ )
{
	switch ( dwReason )
	{
	    case DLL_PROCESS_ATTACH:
            // Implementation note:
            //   On Win XP the debug console window sometimes remains hanging
            // even after the parent process is terminated and there is no way
            // to close it, even the OS refuses to shutdown or restart. For this
            // reason we do not turn on the debugging console by default on
            // pre-Vista Windows.
            //                                (07.04.2011.) (Domagoj Saric)
            OSVERSIONINFO versionInfo; versionInfo.dwOSVersionInfoSize = sizeof( versionInfo );
            BOOST_VERIFY( ::GetVersionEx( &versionInfo ) );
            if ( versionInfo.dwMajorVersion > 5 )
            {
                /// \note Handle multiple instances: do not try to open more
                /// than one console window.
                ///                           (03.04.2012.) (Domagoj Saric)
                if ( !LE::debugConsole )
                    LE::debugConsole = boost::in_place();
            }
            TCHAR fileName[ MAX_PATH ];
            ::GetModuleFileName( nullptr, fileName, _countof( fileName ) );
            /*std*/::_tprintf( _T( "Plugin DLL loaded by %s.\n" ), fileName );
		    break;

    	case DLL_PROCESS_DETACH:
            ::puts( "Plugin unloaded." /*" Press enter to continue..."*/ );
            //getchar();
            LE::debugConsole = boost::none;
		    break;
	};

    return 1;
}

//------------------------------------------------------------------------------

#endif // _DEBUG
