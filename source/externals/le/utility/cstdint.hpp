////////////////////////////////////////////////////////////////////////////////
///
/// \file cstdint.hpp
/// -----------------
///
/// Copyright (c) 2014 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef cstdint_hpp__096C6E8A_EEA5_4488_BFC8_2DF4FCEA8F4E
#define cstdint_hpp__096C6E8A_EEA5_4488_BFC8_2DF4FCEA8F4E
#pragma once
//------------------------------------------------------------------------------
#if defined( __APPLE__ )
#if !__has_include( <cstdint> )
    #include <tr1/cstdint>
    namespace std
    {
        using tr1::uint8_t ;
        using tr1::uint16_t;
        using tr1::uint32_t;

        using tr1::uint_fast8_t ;
        using tr1::uint_fast16_t;
        using tr1::uint_fast32_t;
    } // namespace std
#else // OSX/iOS
#include <cstdint>
#endif // OSX/iOS
#else
#include <cstdint>
#endif
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
#endif // cstdint_hpp
