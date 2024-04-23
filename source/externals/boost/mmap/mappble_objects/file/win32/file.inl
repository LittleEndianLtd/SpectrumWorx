////////////////////////////////////////////////////////////////////////////////
///
/// \file file.inl
/// --------------
///
/// Copyright (c) Domagoj Saric 2010 - 2014.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef file_inl__FB482005_18D9_4E3B_9193_A13DBFE88F45
#define file_inl__FB482005_18D9_4E3B_9193_A13DBFE88F45
#pragma once
//------------------------------------------------------------------------------
#include "file.hpp"

#include "open_flags.hpp"
#include "../../detail/impl_inline.hpp"
#include "../../detail/windows.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

namespace
{
    // http://en.wikipedia.org/wiki/File_locking#In_UNIX
    DWORD const default_unix_shared_semantics( FILE_SHARE_READ | FILE_SHARE_WRITE );
}

BOOST_IMPL_INLINE
file_handle<win32> create_file( char const * const file_name, file_open_flags<win32> const & flags )
{
    /// \note
    ///   This typedef is required by MSVC++ 10 SP1 and must be placed before
    /// the CreateFile call, otherwise it breaks at the return statement.
    ///                                       (25.08.2011.) (Domagoj Saric)
    typedef file_handle<win32> win32_file_handle;

    BOOST_ASSERT( file_name );

    HANDLE const file_handle
    (
        ::CreateFileA
        (
            file_name, flags.desired_access, default_unix_shared_semantics, 0, flags.creation_disposition, flags.flags_and_attributes, 0
        )
    );
    BOOST_ASSERT( ( file_handle == INVALID_HANDLE_VALUE ) || ( ::GetLastError() == NO_ERROR ) || ( ::GetLastError() == ERROR_ALREADY_EXISTS ) );
    
    return win32_file_handle( file_handle );
}

BOOST_IMPL_INLINE
file_handle<win32> create_file( wchar_t const * const file_name, file_open_flags<win32> const & flags )
{
    BOOST_ASSERT( file_name );

    HANDLE const handle
    (
        ::CreateFileW
        (
            file_name, flags.desired_access, default_unix_shared_semantics, 0, flags.creation_disposition, flags.flags_and_attributes, 0
        )
    );
    BOOST_ASSERT( ( handle == INVALID_HANDLE_VALUE ) || ( ::GetLastError() == NO_ERROR ) || ( ::GetLastError() == ERROR_ALREADY_EXISTS ) );
    
    return file_handle<win32>( handle );
}


BOOST_IMPL_INLINE
bool delete_file( char    const * const file_name, win32 )
{
    return ::DeleteFileA( file_name ) != false;
}

BOOST_IMPL_INLINE
bool delete_file( wchar_t const * const file_name, win32 )
{
    return ::DeleteFileW( file_name ) != false;
}


BOOST_IMPL_INLINE
bool set_size( file_handle<win32>::reference const file_handle, std::size_t const desired_size )
{
    // It is 'OK' to send null/invalid handles to Windows functions (they will
    // simply fail), this simplifies error handling (it is enough to go through
    // all the logic, inspect the final result and then throw on error).
    #ifdef _WIN64
        BOOST_VERIFY
        (
            ::SetFilePointerEx( file_handle, reinterpret_cast<LARGE_INTEGER const &>( desired_size ), NULL, FILE_BEGIN ) ||
            ( file_handle == INVALID_HANDLE_VALUE )
        );
    #else // _WIN32/64
        DWORD const new_size( ::SetFilePointer( file_handle, desired_size, NULL, FILE_BEGIN ) );
        BOOST_ASSERT( ( new_size == desired_size ) || ( file_handle == INVALID_HANDLE_VALUE ) );
        ignore_unused_variable_warning( new_size );
    #endif // _WIN32/64

    BOOL const success( ::SetEndOfFile( file_handle ) );

    #ifdef _WIN64
        LARGE_INTEGER const offset = { 0 };
        BOOST_VERIFY
        (
            ::SetFilePointerEx( file_handle, offset, NULL, FILE_BEGIN ) ||
            ( file_handle == INVALID_HANDLE_VALUE )
        );
    #else // _WIN32/64
        BOOST_VERIFY( ( ::SetFilePointer( file_handle, 0, NULL, FILE_BEGIN ) == 0 ) || ( file_handle == INVALID_HANDLE_VALUE ) );
    #endif // _WIN32/64

    return success != false;
}


BOOST_IMPL_INLINE
std::size_t get_size( file_handle<win32>::reference const file_handle )
{
    #ifdef _WIN64
        LARGE_INTEGER file_size;
        BOOST_VERIFY( ::GetFileSizeEx( file_handle, &file_size ) || ( file_handle == INVALID_HANDLE_VALUE ) );
        return file_size.QuadPart;
    #else // _WIN32/64
        DWORD const file_size( ::GetFileSize( file_handle, 0 ) );
        BOOST_ASSERT( ( file_size != INVALID_FILE_SIZE ) || ( file_handle == INVALID_HANDLE_VALUE ) || ( ::GetLastError() == NO_ERROR ) );
        return file_size;
    #endif // _WIN32/64
}


BOOST_IMPL_INLINE
mapping<win32> create_mapping( file_handle<win32>::reference const file, file_mapping_flags<win32> const & flags )
{
    HANDLE const mapping_handle
    (
        ::CreateFileMappingW( file, NULL, flags.create_mapping_flags, 0, 0, NULL )
    );
    // CreateFileMapping accepts INVALID_HANDLE_VALUE as valid input but only if
    // the size parameter is not null.
    BOOST_ASSERT
    (
        ( file != INVALID_HANDLE_VALUE ) || !mapping_handle
    );
    return mapping<win32>( mapping_handle, flags.map_view_flags );
}

//------------------------------------------------------------------------------
} // mmap
//------------------------------------------------------------------------------
} // boost
//------------------------------------------------------------------------------

#endif // file_inl
