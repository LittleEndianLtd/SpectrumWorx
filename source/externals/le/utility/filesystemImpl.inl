////////////////////////////////////////////////////////////////////////////////
///
/// \file filesystemImpl.inl
/// ------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef filesystemImpl_inl__C580EBB7_A92A_4E8C_BB8E_E660933A9583
#define filesystemImpl_inl__C580EBB7_A92A_4E8C_BB8E_E660933A9583
#pragma once
//------------------------------------------------------------------------------
#include "filesystemImpl.hpp"

#include "platformSpecifics.hpp"

#include "boost/mmap/mappble_objects/file/utility.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

template <SpecialLocations rootDirectory> LE_NOTHROWNOALIAS
typename File::Impl<rootDirectory>::type::MemoryMapping
File::map( char const * const relativeFilePath )
{
    return PathResolver<rootDirectory>:: template apply<File::MemoryMapping>
    (
        relativeFilePath,
        [=]( char const * const fullPath )
        {
            auto const mapping_reference( boost::mmap::map_read_only_file( fullPath ) );
            LE_TRACE_IF( !mapping_reference, "Failed to mmap file %s (errno: %d)", relativeFilePath, errno );
            return File::MemoryMapping( File::MemoryMapping::Range( mapping_reference.begin(), mapping_reference.end() ) );
        }
    );
}


template <SpecialLocations rootDirectory> LE_NOTHROWNOALIAS
typename File::Impl<rootDirectory>::type::Stream
File::open( char const * const relativeFilePath, unsigned int const flags )
{
    return PathResolver<rootDirectory>:: template apply<File::Stream>
    (
        relativeFilePath,
        [=]( char const * const fullPath ){ return File::Stream( ::open( fullPath, flags | binaryFlag, accessFlags ) ); }
    );
}

template LE_NOTHROWNOALIAS File::Impl<AbsolutePath   >::type::MemoryMapping LE_FASTCALL_ABI File::map<AbsolutePath   >( char const * );
template LE_NOTHROWNOALIAS File::Impl<AppData        >::type::MemoryMapping LE_FASTCALL_ABI File::map<AppData        >( char const * );
template LE_NOTHROWNOALIAS File::Impl<Documents      >::type::MemoryMapping LE_FASTCALL_ABI File::map<Documents      >( char const * );
template LE_NOTHROWNOALIAS File::Impl<Library        >::type::MemoryMapping LE_FASTCALL_ABI File::map<Library        >( char const * );
template LE_NOTHROWNOALIAS File::Impl<Resources      >::type::MemoryMapping LE_FASTCALL_ABI File::map<Resources      >( char const * );
template LE_NOTHROWNOALIAS File::Impl<ExternalStorage>::type::MemoryMapping LE_FASTCALL_ABI File::map<ExternalStorage>( char const * );
template LE_NOTHROWNOALIAS File::Impl<Temporaries    >::type::MemoryMapping LE_FASTCALL_ABI File::map<Temporaries    >( char const * );
#ifdef __ANDROID__
template LE_NOTHROWNOALIAS File::Impl<ToolOutput     >::type::MemoryMapping LE_FASTCALL_ABI File::map<ToolOutput     >( char const * );
#endif // __ANDROID__

template LE_NOTHROWNOALIAS File::Impl<AbsolutePath   >::type::Stream LE_FASTCALL_ABI File::open<AbsolutePath   >( char const *, unsigned int );
template LE_NOTHROWNOALIAS File::Impl<AppData        >::type::Stream LE_FASTCALL_ABI File::open<AppData        >( char const *, unsigned int );
template LE_NOTHROWNOALIAS File::Impl<Documents      >::type::Stream LE_FASTCALL_ABI File::open<Documents      >( char const *, unsigned int );
template LE_NOTHROWNOALIAS File::Impl<Library        >::type::Stream LE_FASTCALL_ABI File::open<Library        >( char const *, unsigned int );
template LE_NOTHROWNOALIAS File::Impl<Resources      >::type::Stream LE_FASTCALL_ABI File::open<Resources      >( char const *, unsigned int );
template LE_NOTHROWNOALIAS File::Impl<ExternalStorage>::type::Stream LE_FASTCALL_ABI File::open<ExternalStorage>( char const *, unsigned int );
template LE_NOTHROWNOALIAS File::Impl<Temporaries    >::type::Stream LE_FASTCALL_ABI File::open<Temporaries    >( char const *, unsigned int );
#ifdef __ANDROID__
template LE_NOTHROWNOALIAS File::Impl<ToolOutput     >::type::Stream LE_FASTCALL_ABI File::open<ToolOutput     >( char const *, unsigned int );
#endif // __ANDROID__

template <SpecialLocations location>
LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath( char const * const relativePath )
{
    static Path path;
    PathResolver<location>:: template apply<void>
    (
        relativePath,
        []( char const * const fullPath ){ std::strcpy( &path[ 0 ], fullPath ); }
    );
    return &path[ 0 ];
}

template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<AbsolutePath   >( char const * );
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<AppData        >( char const * );
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<Documents      >( char const * );
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<Library        >( char const * );
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<Resources      >( char const * );
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<ExternalStorage>( char const * );
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<Temporaries    >( char const * );
#ifdef __ANDROID__
template LE_NOTHROWNOALIAS char const * LE_FASTCALL_ABI fullPath<ToolOutput     >( char const * );
#endif // __ANDROID__


template <SpecialLocations location, bool writeAccess>
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible()
{
#ifdef _MSC_VER
    static unsigned int const W_OK = 02;
    static unsigned int const R_OK = 04;
#endif // _MSC_VER
    return PathResolver<location>:: template apply<bool>
    (
        "",
        []( char const * const fullPath ){ return ::access( fullPath, writeAccess ? R_OK | W_OK : R_OK ) != -1; }
    );
}


template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<AbsolutePath   , false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<AbsolutePath   , true>();
template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<AppData        , false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<AppData        , true>();
template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Documents      , false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Documents      , true>();
template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Library        , false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Library        , true>();
template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Resources      , false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Resources      , true>();
template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<ExternalStorage, false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<ExternalStorage, true>();
template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Temporaries    , false>(); template LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI accessible<Temporaries    , true>();

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // filesystemImpl_inl
