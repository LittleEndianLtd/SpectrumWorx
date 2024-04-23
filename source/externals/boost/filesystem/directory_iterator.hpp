////////////////////////////////////////////////////////////////////////////////
///
/// \file directory_iterator.hpp
/// ----------------------------
///
/// Copyright (c) 2011 - 2016. Domagoj Saric (Little Endian Ltd.)
///
/// Use, modification and distribution is subject to
/// the Boost Software License, Version 1.0.
/// (See accompanying file LICENSE_1_0.txt or copy at
/// http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef directory_iterator_hpp__26605FB7_400C_46FE_A981_65D402D7D9FF
#define directory_iterator_hpp__26605FB7_400C_46FE_A981_65D402D7D9FF
#pragma once
//------------------------------------------------------------------------------
#include <boost/assert.hpp>
#include <boost/noncopyable.hpp>
#include <boost/system/api_config.hpp>

#include <cstring>

#ifdef BOOST_WINDOWS_API
    #define NOMINMAX
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#else // POSIX
    #include <dirent.h>
    #include <errno.h>
    #include <fnmatch.h>
#endif
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace filesystem
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class directory_iterator
///
////////////////////////////////////////////////////////////////////////////////

class directory_iterator : noncopyable
{
public:
#ifdef BOOST_WINDOWS_API
    //...mrmlj...http://blogs.msdn.com/b/bclteam/archive/2007/02/13/long-paths-in-net-part-1-of-3-kim-hamilton.aspx
    //...mrmlj...http://trac.heidi.ie/ticket/41
    //...mrmlj...http://www.cocoadev.com/index.pl?UnicodeFilenames
    #define BOOST_FS_DIRECTORY_ITERATOR_SUFFIX() L"\\*"

    using char_type = wchar_t;

    class entry
    {
    public:
        char_type const * name        () const { return data_.cFileName; }
        bool              is_directory() const { return ( data_.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0; }

    private:
        entry() {}

        entry          ( entry const & );
        void operator =( entry const & );

    private: friend class directory_iterator;
        WIN32_FIND_DATAW data_;
    }; // class entry

    directory_iterator( char_type const * const path )
        :
        handle_( ::FindFirstFileW( path, &entry_.data_ ) ),
        failed_( handle_ == INVALID_HANDLE_VALUE         )
    {}
#else // POSIX
    #define BOOST_FS_DIRECTORY_ITERATOR_SUFFIX() ""

    using char_type = char;

    class entry
    {
    // http://www.gnu.org/s/hello/manual/libc/Directory-Entries.html
    // http://www.cocoadev.com/index.pl?NSDirectoryEnumerator
    #if !defined( _DIRENT_HAVE_D_TYPE ) && !defined( __APPLE__ )
        #error ...zzz...
    #endif
    public:
        char_type const * name        () const { return p_entry_->d_name; }
        bool              is_directory() const { return p_entry_->d_type == DT_DIR; }

    private:
        entry() : p_entry_( nullptr ) {}

        entry          ( entry const & );
        void operator =( entry const & );

    private: friend class directory_iterator;
        dirent * p_entry_;
    }; // class entry

    directory_iterator( char_type const * const path )
        :
        p_dir_ ( ::opendir( path ) ),
        failed_( p_dir_ == NULL    )
    {
        get_next_entry();
    }
#endif // OS API

    ~directory_iterator()
    {
        #ifdef BOOST_WINDOWS_API
            BOOST_VERIFY( ::FindClose( handle_ ) || handle_ == INVALID_HANDLE_VALUE );
        #else // POSIX
            ::closedir( p_dir_ );
        #endif // OS API
    }
    
    entry const * operator->() const { BOOST_ASSERT( !failed_ ); return &entry_; }
    entry const & operator* () const { BOOST_ASSERT( !failed_ ); return  entry_; }

    directory_iterator & operator++()
    {
        get_next_entry();
        return *this;
    }

    bool operator! () const { return failed_; }

    static char_type const * path_suffix() { return BOOST_FS_DIRECTORY_ITERATOR_SUFFIX(); }

protected:
    bool get_next_entry()
    {
        BOOST_ASSERT( !failed_ );
        #ifdef BOOST_WINDOWS_API
            bool failed( false );
            if ( !::FindNextFileW( handle_, &entry_.data_ ) )
            {
                //...zzz...
                failed = ( ::GetLastError() != ERROR_NO_MORE_FILES );
                BOOST_VERIFY( ::FindClose( handle_ ) );
                handle_ = INVALID_HANDLE_VALUE;
                failed_ = true;
            }
        #else // POSIX
            BOOST_ASSERT_MSG( !!*this, "Already reached the end of the directory." );
            errno = 0;
            entry_.p_entry_ = ::readdir( p_dir_ );
            bool const failed( !entry_.p_entry_ && errno );
            failed_ = !entry_.p_entry_;
        #endif // OS API

        return !failed;
    }

private:
    entry entry_;
    #if defined( __APPLE__z ) //...mrmlj...
        // http://forums.macrumors.com/archive/index.php//t-1045842.html
        // http://developer.apple.com/library/mac/#documentation/Carbon/Reference/File_Manager/Reference/reference.html#//apple_ref/c/func/PBCatalogSearchSync
    #elif defined( BOOST_WINDOWS_API )
        HANDLE /*const*/ handle_;
    #elif defined( BOOST_POSIX_API )
        DIR * const p_dir_;
    #endif // OS API

    bool failed_;
}; // class directory_iterator


////////////////////////////////////////////////////////////////////////////////
///
/// \class filtered_directory_iterator
///
////////////////////////////////////////////////////////////////////////////////

class filtered_directory_iterator : public directory_iterator
{
public:
#ifdef BOOST_WINDOWS_API
    filtered_directory_iterator( char_type const * const path_and_wild_card )
        : directory_iterator( path_and_wild_card ) {}
    #define BOOST_FS_MAKE_FILTERED_DIRECTORY_ITERATOR( path, wild_card ) filtered_directory_iterator( path L"\\" wild_card )
#else // BOOST_POSIX_API
    filtered_directory_iterator( char_type const * const path, char_type const * const wild_card )
        :
        directory_iterator( path      ),
        wild_card_        ( wild_card )
    {}
    #define BOOST_FS_MAKE_FILTERED_DIRECTORY_ITERATOR( path, wild_card ) filtered_directory_iterator( path, wild_card )

    filtered_directory_iterator & operator++()
    {
        do
        {
            get_next_entry();
        } while ( !!*this && ::fnmatch( wild_card_, (**this).name(), FNM_NOESCAPE | FNM_CASEFOLD ) != 0 );
        return *this;
    }
#endif // BOOST_x_API

    filtered_directory_iterator( filtered_directory_iterator const & );

#if defined( BOOST_POSIX_API )
private:
    char_type const * const wild_card_;
#endif // BOOST_POSIX_API
};

//------------------------------------------------------------------------------
} // namespace filesystem
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------
#endif // directory_iterator_hpp
