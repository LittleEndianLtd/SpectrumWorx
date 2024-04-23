////////////////////////////////////////////////////////////////////////////////
///
/// \file handle.hpp
/// ----------------
///
/// Copyright (c) Domagoj Saric 2010 - 2015.
///
///  Use, modification and distribution is subject to the
///  Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef handle_hpp__63113526_C3F1_46DC_850E_D8D8C62031DB
#define handle_hpp__63113526_C3F1_46DC_850E_D8D8C62031DB
#pragma once
//------------------------------------------------------------------------------
#include "../handle_ref.hpp"
#include "../../implementations.hpp"

#include "boost/config.hpp"
#include "boost/noncopyable.hpp"
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> class handle;

template <>
class handle<posix> : noncopyable
{
public:
    typedef int                         native_handle_t;
    typedef handle_ref< handle<posix> > reference;

    explicit handle<posix>( native_handle_t );
    handle<posix>( handle<posix> && source ) : handle_( source.handle_ ) { const_cast<native_handle_t &>( source.handle_ ) = -1; }
    ~handle<posix>();

    native_handle_t const & get() const { return handle_; }

    bool operator! () const { return !handle_; }
    operator reference () const { return reference( handle_ ); }

private:
    native_handle_t const handle_;
}; // class handle<posix>

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #include "handle.inl"
#endif

#endif // handle_hpp
