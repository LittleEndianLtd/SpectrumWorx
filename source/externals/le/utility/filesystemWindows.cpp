////////////////////////////////////////////////////////////////////////////////
///
/// filesystemWindows.cpp
/// ---------------------
///
/// Target platform specific boilerplate code.
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "filesystemImpl.hpp"

#include "platformSpecifics.hpp"
#include "trace.hpp"

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/smart_ptr/scoped_array.hpp>

#include "windowsLite.hpp"
#include <knownfolders.h>
#include <Shlobj.h>

#include <array>
#include <cerrno>
#include <cstring>
#include <new>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

using CString = boost::scoped_array<char>;

namespace
{
    bool LE_FASTCALL domainDirectory( ::KNOWNFOLDERID const & knownFolder, Path & path )
    {
        wchar_t * pPath;
        HRESULT const result( ::SHGetKnownFolderPath( knownFolder, KF_FLAG_DONT_VERIFY | KF_FLAG_DONT_UNEXPAND | KF_FLAG_NO_ALIAS, nullptr, &pPath ) );
        if ( FAILED( result ) )
        {
            BOOST_ASSERT( !pPath );
            return false;
        }
        BOOST_VERIFY( ::WideCharToMultiByte( CP_ACP, 0, pPath, -1, &path[ 0 ], static_cast<unsigned int>( path.size() ), nullptr, nullptr ) > 0 );
        ::CoTaskMemFree( pPath );
        return true;
    }

    template <SpecialLocations> bool pathFor( Path & buffer );

  //template <> bool pathFor<AppData        >( Path & buffer ) { return BOOST_VERIFY( ::getcwd( &buffer[ 0 ], buffer.size() ) == &buffer[ 0 ] ); }
    template <> bool pathFor<AppData        >( Path & buffer ) { return domainDirectory( FOLDERID_ProgramData    , buffer ); }
    template <> bool pathFor<Documents      >( Path & buffer ) { return domainDirectory( FOLDERID_Documents      , buffer ); }
    template <> bool pathFor<SharedDocuments>( Path & buffer ) { return domainDirectory( FOLDERID_PublicDocuments, buffer ); }
    template <> bool pathFor<Library        >( Path & buffer ) { return domainDirectory( FOLDERID_Libraries      , buffer ); }
    template <> bool pathFor<Resources      >( Path & buffer ) { return pathFor<AppData>( buffer ); }
  //template <> bool pathFor<SDCard         >( Path & buffer ) { return nullptr; }
    template <> bool pathFor<ExternalStorage>( Path & /*buffer*/ ) { return false; }
    template <> bool pathFor<Temporaries    >( Path & buffer ) { return ::GetTempPathA( static_cast<unsigned int>( buffer.size() ), &buffer[ 0 ] ) != FALSE; }
  //template <> bool pathFor<Temporaries    >( Path & absoluteFilePath ) { return BOOST_VERIFY( ::GetEnvironmentVariable( "TEMP", &absoluteFilePath[ 0 ], absoluteFilePath.size() ) ); }
} // anonymous namespace

template <SpecialLocations rootDirectory>
template <class Result, class Functor>
Result PathResolver<rootDirectory>::apply( char const * const relativePath, Functor const f )
{
    Path absoluteFilePath;
    pathFor<rootDirectory>( absoluteFilePath );
    std::strcat( &absoluteFilePath[ 0 ], "/"          );
    std::strcat( &absoluteFilePath[ 0 ], relativePath );
    return f( &absoluteFilePath[ 0 ] );
}

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "filesystemImpl.inl"
