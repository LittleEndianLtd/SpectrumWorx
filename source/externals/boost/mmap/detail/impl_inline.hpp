////////////////////////////////////////////////////////////////////////////////
///
/// \file impl_inline.hpp
/// ---------------------
///
/// Copyright (c) Domagoj Saric 2011.-2013.
///
///  Use, modification and distribution is subject to the Boost Software License, Version 1.0.
///  (See accompanying file LICENSE_1_0.txt or copy at
///  http://www.boost.org/LICENSE_1_0.txt)
///
/// For more information, see http://www.boost.org
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef impl_inline_hpp__EFACE887_4390_4C0D_898B_1E32DDFD719C
#define impl_inline_hpp__EFACE887_4390_4C0D_898B_1E32DDFD719C
#pragma once
//------------------------------------------------------------------------------

#ifdef BOOST_MMAP_HEADER_ONLY
    #define BOOST_IMPL_INLINE inline
#else
    #define BOOST_IMPL_INLINE
#endif // BOOST_MMAP_HEADER_ONLY

//------------------------------------------------------------------------------
#endif // impl_inline_hpp
