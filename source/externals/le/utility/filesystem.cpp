////////////////////////////////////////////////////////////////////////////////
///
/// filesystem.cpp
/// --------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "filesystem.hpp"
#include "filesystemImpl.hpp"

#include "platformSpecifics.hpp"
#include "trace.hpp"

// Boost sandbox
#include "boost/mmap/mapped_view/mapped_view.hpp"
#include "boost/mmap/mappble_objects/file/utility.hpp"
#ifndef BOOST_MMAP_HEADER_ONLY
    #include "boost/mmap/amalgamated_lib.cpp"
#endif // BOOST_MMAP_HEADER_ONLY

#include <boost/assert.hpp>
#include <boost/config.hpp>

#include <algorithm>
#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

namespace
{
    void unmap( File::MemoryMapping::Range const & rangePair )
    {
        using mapped_view_reference_t = boost::mmap::mapped_view_reference<char const>;
        auto         const range                ( mapped_view_reference_t::memory_range_t( rangePair.first, rangePair.second ) );
        auto const &       mapped_view_reference( static_cast<mapped_view_reference_t const &>( range ) );
        mapped_view_reference_t::unmap( mapped_view_reference );
    }
}

LE_NOTHROWNOALIAS File::MemoryMapping::MemoryMapping() {}
LE_NOTHROWNOALIAS File::MemoryMapping::MemoryMapping( Range const & range ) : Range( range ) {}
LE_NOTHROW        File::MemoryMapping::MemoryMapping( MemoryMapping && other ) : Range( other ) { static_cast<Range &>( other ) = Range(); }
LE_NOTHROW        File::MemoryMapping::~MemoryMapping() { unmap( *this ); }

LE_NOTHROW File::MemoryMapping & File::MemoryMapping::operator=( File::MemoryMapping && other )
{
    unmap( *this );
    static_cast<Range &>( *this ) = static_cast<Range const &>( other );
    static_cast<Range &>( other ) = Range();
    return *this;
}

namespace
{
    int const invalidHandle( -1 );
} // anonymous namespace

LE_NOTHROWNOALIAS File::Stream::Stream(                          ) : handle_( invalidHandle  ) {}
LE_NOTHROWNOALIAS File::Stream::Stream( int const fileDescriptor ) : handle_( fileDescriptor ) {}
LE_NOTHROW        File::Stream::Stream( Stream && other          ) : handle_( other.handle_ ) { other.handle_ = -1; }
LE_NOTHROW        File::Stream::~Stream() { close(); }

LE_NOTHROW LE_COLD
void File::Stream::close()
{
    if ( handle_ != invalidHandle )
    {
        BOOST_VERIFY( ::close( handle_ ) == 0 );
        handle_ = invalidHandle;
    }
}

LE_NOTHROWNOALIAS bool File::Stream::operator! () const { return handle_ == invalidHandle; }

LE_NOTHROW File::Stream & File::Stream::operator=( File::Stream && other )
{
    close();
    this->handle_ = other.handle_;
    other.handle_ = invalidHandle;
    return *this;
}

LE_NOTHROWNOALIAS std::uint32_t File::Stream::read ( void       * pBuffer, std::uint32_t numberOfBytesToRead  ) { BOOST_ASSERT_MSG( handle_ != invalidHandle, "No file open" ); auto const result( ::read ( handle_, pBuffer, numberOfBytesToRead  ) ); LE_TRACE_IF( result < 0, "File read error (%d)." , errno ); return static_cast<std::uint32_t>( std::max( 0, static_cast<std::int32_t>( result ) ) ); }
LE_NOTHROWNOALIAS std::uint32_t File::Stream::write( void const * pBuffer, std::uint32_t numberOfBytesToWrite ) { BOOST_ASSERT_MSG( handle_ != invalidHandle, "No file open" ); auto const result( ::write( handle_, pBuffer, numberOfBytesToWrite ) ); LE_TRACE_IF( result < 0, "File write error (%d).", errno ); return static_cast<std::uint32_t>( std::max( 0, static_cast<std::int32_t>( result ) ) ); }

#ifdef _MSC_VER
LE_NOTHROWNOALIAS std::uint32_t File::Stream::position(                                                      ) const { return static_cast<std::uint32_t>( ::tell ( handle_                 ) );     }
#else //...mrmlj...no tell on android or ios...
LE_NOTHROWNOALIAS std::uint32_t File::Stream::position(                                                      ) const { return static_cast<std::uint32_t>( ::lseek( handle_, 0, SEEK_CUR    ) );     }
#endif // _MSC_VER
LE_NOTHROWNOALIAS bool          File::Stream::seek    ( std::int32_t const offset, std::uint8_t const whence )       { return                             ::lseek( handle_, offset, whence ) != -1; }

LE_NOTHROWNOALIAS std::uint32_t File::Stream::size() const
{
#ifdef BOOST_MSVC
    return static_cast<std::uint32_t>( /*std*/::_filelength( handle_ ) );
#else
    struct stat file_status;
    BOOST_VERIFY( ::fstat( handle_, &file_status ) == 0 );
    return static_cast<std::uint32_t>( file_status.st_size );
#endif // BOOST_MSVC
}

LE_NOTHROWNOALIAS int File::Stream::asPOSIXFile( ::off_t & startOffset, std::size_t & size ) const
{
    int const newDescriptor( ::dup( handle_ ) );
    if ( newDescriptor == -1 )
        return -1;
    startOffset = 0;
    size        = this->size();
    return newDescriptor;
}

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
