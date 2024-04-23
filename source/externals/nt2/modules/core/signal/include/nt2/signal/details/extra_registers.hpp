//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2013   Domagoj Saric, Little Endian Ltd.
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef EXTRA_REGISTERS_HPP_INCLUDED
#define EXTRA_REGISTERS_HPP_INCLUDED
//------------------------------------------------------------------------------
#include <boost/simd/sdk/config/arch.hpp>
#include <boost/simd/sdk/simd/extensions.hpp>

#include <boost/cstdint.hpp>

#ifdef BOOST_SIMD_HAS_MMX_SUPPORT
#include "mmintrin.h"
#endif // BOOST_SIMD_HAS_MMX_SUPPORT
//------------------------------------------------------------------------------
namespace boost
{
namespace simd
{

#ifndef BOOST_SIMD_HAS_MMX_SUPPORT
    /// \note
    /// - MSVC does not support __m64 operations in x64 bit mode.
    /// - GCC and Clang (seem to) require MMX to be explicitly enabled
    ///   regardless of SSE.
    ///                                       (09.11.2012.) (Domagoj Saric)
    #if defined( BOOST_SIMD_ARCH_X86 ) && !( defined( BOOST_SIMD_ARCH_X86_64 ) && defined( _MSC_VER ) ) && ( !defined( __GNUC__ ) || defined( __MMX__ ) )
        #define BOOST_SIMD_HAS_MMX_SUPPORT
    #endif
#endif // BOOST_SIMD_HAS_MMX_SUPPORT

#if defined( _MSC_VER ) && defined( BOOST_SIMD_ARCH_X86_64 ) && defined( BOOST_SIMD_HAS_MMX_SUPPORT )
    #undef BOOST_SIMD_HAS_MMX_SUPPORT
#endif // MSVC & x64

/// \note Clang (Xcode 4.5.1 till 5.0) completely brainfarts when MMX is used
/// (constantly converts between MMX and GP registers and/or keeps copies of
/// pointers in both registers and all of that through the stack of course).
///                                       (31.10.2012.) (Domagoj Saric)
#if defined( BOOST_SIMD_HAS_MMX_SUPPORT ) && !defined( __clang__ )

    #define BOOST_SIMD_HAS_EXTRA_GP_REGISTERS

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4799 ) // Function has no EMMS instruction.
#endif // _MSC_VER

    struct extra_integer_register
    {
        extra_integer_register() {}
        extra_integer_register( unsigned int const value ) : register_( _mm_cvtsi32_si64( value ) ) {}

        extra_integer_register & operator+=( unsigned int const value ) { register_ = _mm_add_pi32( register_, _mm_cvtsi32_si64( value ) ); return *this; }
        extra_integer_register & operator-=( unsigned int const value ) { register_ = _mm_sub_pi32( register_, _mm_cvtsi32_si64( value ) ); return *this; }

        extra_integer_register & operator++() { return this->operator += ( 1 ); }
        extra_integer_register & operator--() { return this->operator -= ( 1 ); }

        unsigned int operator++( int )
        {
            unsigned int const result( this->operator unsigned int() );
            this->operator++();
            return result;
        }

        unsigned int operator--( int )
        {
            unsigned int const result( this->operator unsigned int() );
            this->operator--();
            return result;
        }

        extra_integer_register & operator=( unsigned int const value ) { register_ = _mm_cvtsi32_si64( value ); return *this; }
        BOOST_FORCEINLINE
        operator unsigned int () const { return _mm_cvtsi64_si32( register_ ); }

        operator __m64 const & () const { return register_; }
        __m64 register_;

    #ifdef BOOST_MSVC
        /// \note See the note for the native<> copy constructor.
        ///                                   (10.10.2013.) (Domagoj Saric)
        extra_integer_register( __m64                  const & builtin ) : register_( builtin         ) {}
        extra_integer_register( extra_integer_register const & other   ) : register_( other.register_ ) {}
    #endif // _MSC_VER

        extra_integer_register & operator=( __m64                  const & builtin ) { register_ = builtin        ; return *this; }
        extra_integer_register & operator=( extra_integer_register const & other   ) { register_ = other.register_; return *this; }
    }; // struct extra_integer_register

#ifdef BOOST_SIMD_ARCH_X86_64
    /// \note MMX cannot store and operate on 64 bit values (64 bit pointers).
    ///                                       (09.11.2012.) (Domagoj Saric)
    template <typename T> struct make_extra_pointer_register { typedef T * BOOST_DISPATCH_RESTRICT type; };
#else
    #define BOOST_SIMD_HAS_EXTRA_GP_POINTER_REGISTERS

    template <typename T>
    struct extra_pointer_register : extra_integer_register
    {
        extra_pointer_register() {}
        extra_pointer_register( T * BOOST_DISPATCH_RESTRICT const pointer ) : extra_integer_register( reinterpret_cast<unsigned int>( pointer ) ) {}

        extra_pointer_register & operator+=( unsigned int const value ) { return static_cast<extra_pointer_register &>( extra_integer_register::operator+=( value * sizeof( T ) ) ); }
        extra_pointer_register & operator-=( unsigned int const value ) { return static_cast<extra_pointer_register &>( extra_integer_register::operator-=( value * sizeof( T ) ) ); }

        extra_pointer_register & operator++() { return this->operator += ( 1 ); }
        extra_pointer_register & operator--() { return this->operator -= ( 1 ); }

        extra_pointer_register & operator=( T * const pointer ) { return static_cast<extra_pointer_register &>( extra_integer_register::operator=( reinterpret_cast<unsigned int>( pointer ) ) ); }

        T * BOOST_DISPATCH_RESTRICT const operator++( int )
        {
            T * BOOST_DISPATCH_RESTRICT const result( this->operator->() );
            this->operator++();
            return result;
        }

        T &                         operator *  () const { return *static_cast<T * BOOST_DISPATCH_RESTRICT>( *this ); }
        T * BOOST_DISPATCH_RESTRICT operator -> () const { return  static_cast<T * BOOST_DISPATCH_RESTRICT>( *this ); }

        operator T * BOOST_DISPATCH_RESTRICT () const { return reinterpret_cast<T * BOOST_DISPATCH_RESTRICT>( extra_integer_register::operator unsigned int() ); }
    };

    template <typename T> struct make_extra_pointer_register { typedef extra_pointer_register<T> type; };
#endif // BOOST_SIMD_ARCH_X86_64


    struct extra_registers_cleanup
    {
    #if defined( _MSC_VER ) && ( ( _MSC_VER == 1700 ) || ( _MSC_VER == 1800 ) )
        /// \note Workaround for a MSVC11/12 codegen regression.
        /// https://connect.microsoft.com/VisualStudio/feedback/details/804579/msvc-serious-sse-codegen-regression
        ///                                   (10.10.2013.) (Domagoj Saric)
        __declspec( noinline nothrow noalias )
    #endif // MSVC12
        ~extra_registers_cleanup() { _mm_empty(); }
    };

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

#else // BOOST_SIMD_HAS_MMX_SUPPORT

    typedef unsigned int extra_integer_register;
    template <typename T> struct make_extra_pointer_register { typedef T * BOOST_DISPATCH_RESTRICT type; };
    struct extra_registers_cleanup {};

#endif // BOOST_SIMD_HAS_MMX_SUPPORT

} // namespace simd
} // namespace boost
//------------------------------------------------------------------------------
#endif // EXTRA_REGISTERS_HPP_INCLUDED
