////////////////////////////////////////////////////////////////////////////////
///
/// \file countof.hpp
/// -----------------
///
///   A _countof macro implementation (currently just a copy-pasted Microsoft
/// implementation) for compilers that do not provide it.
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef countof_hpp__2D37690D_068C_46FE_945B_13FB196E41E0
#define countof_hpp__2D37690D_068C_46FE_945B_13FB196E41E0
#pragma once
//------------------------------------------------------------------------------
#ifdef _MSC_VER
    #include <cstdlib>
#endif // _MSC_VER
//------------------------------------------------------------------------------

#ifndef _MSC_VER
    //template <typename _CountofType, size_t _SizeOfArray>
    //char (*__countof_helper(__attribute__((__packed__)) _CountofType (&_Array)[_SizeOfArray]))[_SizeOfArray];
    //#define _countof(_Array) (sizeof(*__countof_helper(_Array)) + 0)
    template <typename T, int N> char (&__countof_helper(T(&)[N]))[N];
    #define _countof( _Array ) ( sizeof __countof_helper( _Array ) )
#endif // _MSC_VER

//------------------------------------------------------------------------------
#endif // countof_hpp
