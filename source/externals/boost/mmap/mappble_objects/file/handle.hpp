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
#ifndef handle_hpp__56DDDE10_05C3_4B18_8DC5_89317D689F99
#define handle_hpp__56DDDE10_05C3_4B18_8DC5_89317D689F99
#pragma once
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace boost
{
//------------------------------------------------------------------------------
namespace mmap
{
//------------------------------------------------------------------------------

template <typename Impl> class handle;

template <typename Impl>
struct file_handle : handle<Impl>
{
    file_handle( typename handle<Impl>::native_handle_t const native_handle )
        : handle<Impl>( native_handle ) {}

    typedef handle_ref< file_handle<Impl> > reference;

    operator reference () const { return reference( this->get() ); }
}; // struct file_handle

//------------------------------------------------------------------------------
} // namespace mmap
//------------------------------------------------------------------------------
} // namespace boost
//------------------------------------------------------------------------------
#endif // handle_hpp
