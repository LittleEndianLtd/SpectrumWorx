////////////////////////////////////////////////////////////////////////////////
///
/// \file filesystemImpl.hpp
/// ------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef filesystemImpl_hpp__33637349_32C0_4924_9A7A_6DF8307C41EF
#define filesystemImpl_hpp__33637349_32C0_4924_9A7A_6DF8307C41EF
#pragma once
//------------------------------------------------------------------------------
#include "filesystem.hpp"
#include "platformSpecifics.hpp"

#include <boost/smart_ptr/scoped_array.hpp>

#include <fcntl.h>
#include <sys/stat.h>
#ifdef _MSC_VER
    #pragma warning( disable : 4996 ) // '<...>': The POSIX name for this item is deprecated. Instead, use the ISO C++ conformant name: _<...>
    #include <direct.h>
    #include <io.h>
    int const binaryFlag ( _O_BINARY | _O_SEQUENTIAL | _O_NOINHERIT );
    int const accessFlags( _S_IREAD | _S_IWRITE );
#else // POSIX implementation
    #include <unistd.h>
    #include <sys/resource.h>
    #include <sys/time.h>
    int const binaryFlag ( 0 );
    int const accessFlags( S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP );
#endif // POSIX implementation


#include <array>
#include <cerrno>
#include <cstdint>
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

using CString = boost::scoped_array<char>;

#ifdef _WIN32
    using Path = std::array<char, 260 /*MAX_PATH*/>;
#else
    using Path = std::array<char, 2048/*PATH_MAX*/>;
#endif


template <SpecialLocations rootLocation>
struct PathResolver
{
    template <class Result, class Functor>
    static Result apply( char const * relativePath, Functor );
};

template <>
template <class Result, class Functor>
LE_FORCEINLINE
Result PathResolver<AbsolutePath>::apply( char const * const absolutePath, Functor const f )
{
    return f( absolutePath );
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // filesystemImpl_hpp
