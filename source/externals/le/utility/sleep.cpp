////////////////////////////////////////////////////////////////////////////////
///
/// sleep.cpp
/// ---------
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "sleep.hpp"
#ifdef _WIN32
extern "C" __declspec( dllimport ) void __stdcall Sleep( unsigned long dwMilliseconds );
#else
#include <unistd.h>
#endif // _WIN32
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_NOTHROWNOALIAS void LE_FASTCALL_ABI sleep( unsigned int const seconds )
{
#ifdef _WIN32
    ::Sleep( seconds * 1000 );
#else // POSIX
    auto unslept( seconds );
    while ( ( unslept = ::sleep( unslept ) ) ) {}
#endif // OS
}

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
