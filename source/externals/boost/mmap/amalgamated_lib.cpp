////////////////////////////////////////////////////////////////////////////////
///
/// \file amalgamated_lib.cpp
/// -------------------------
///
/// Copyright (c) Domagoj Saric 2010.-2013.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
/// (obsolete version, latest @ https://github.com/psiha/mmap
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #error this file is meant for non header only builds
#endif // BOOST_MMAP_HEADER_ONLY

#include "handles/handle.inl"
//..zzz...by utility.inl...#include "mappble_objects/file/file.inl"
#include "mappble_objects/file/flags.inl"
#include "mappble_objects/file/utility.inl"
#ifndef _WIN32
#include "mappble_objects/shared_memory/posix/flags.inl"
#else
#include "mappble_objects/shared_memory/win32/flags.inl"
#endif // _WIN32
#include "mapped_view/mapped_view.inl"
#include "mapping/mapping.inl"
//------------------------------------------------------------------------------
