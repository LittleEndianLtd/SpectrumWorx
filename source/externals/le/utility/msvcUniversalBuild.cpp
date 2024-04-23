////////////////////////////////////////////////////////////////////////////////
///
/// msvcUniversalBuild.cpp
/// ----------------------
///
/// Fallback implementations for functions not available in older MSVC CRTs
/// (allow client side usage/linking with older Visual Studio versions).
///
/// Copyright (c) 2015 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
#if defined( LE_SDK_BUILD ) || defined( LE_SW_FMOD )
//------------------------------------------------------------------------------
#include "platformSpecifics.hpp"
#include "windowsLite.hpp"

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>

#include <crtdbg.h>
#include <xmmintrin.h>

#include <cstdint>
#include <cstdio>
//------------------------------------------------------------------------------

// http://blogs.msdn.com/b/freik/archive/2005/10/26/485276.aspx
// http://en.wikipedia.org/wiki/Visual_C%2B%2B_name_mangling
// http://demangler.com
// http://stackoverflow.com/questions/2290587/gcc-style-weak-linking-in-visual-studio/2290843#2290843
// http://en.wikipedia.org/wiki/Name_mangling#C_name_decoration_in_Microsoft_Windows
// http://stackoverflow.com/questions/13385935/64bit-name-mangling-for-c

#ifdef _M_X64
    #define LE_ALTERNATE_SYMBOL_ARCH_SELECT( x86symbol, x64symbol ) x64symbol
    #define LE_ALTERNATE_CDECL_SYMBOL_PREFIX( symbol ) symbol
#else
    #define LE_ALTERNATE_SYMBOL_ARCH_SELECT( x86symbol, x64symbol ) x86symbol
    #define LE_ALTERNATE_CDECL_SYMBOL_PREFIX( symbol ) BOOST_PP_CAT( _, symbol )
#endif

#define LE_ALTERNATE_SYMBOL_IMPL( originalSymbol, alternateSymbol )   \
    __pragma( comment( linker, BOOST_PP_STRINGIZE( BOOST_PP_CAT( BOOST_PP_CAT( /alternatename:, originalSymbol ), BOOST_PP_CAT( =, alternateSymbol ) ) ) ) ) \
    __pragma( comment( linker, BOOST_PP_STRINGIZE( BOOST_PP_CAT( BOOST_PP_CAT( /alternatename:, BOOST_PP_CAT( __imp_, originalSymbol ) ), BOOST_PP_CAT( =, alternateSymbol ) ) ) ) )

#define LE_ALTERNATE_SYMBOL( originalx86Symbol, originalx64Symbol, alternateSymbol )   \
    LE_ALTERNATE_SYMBOL_IMPL( LE_ALTERNATE_SYMBOL_ARCH_SELECT( originalx86Symbol, originalx64Symbol ), alternateSymbol )


#define LE_ALTERNATE_CDECL_SYMBOL( originalx86Symbol, originalx64Symbol, alternateSymbol )   \
    LE_ALTERNATE_SYMBOL( originalx86Symbol, originalx64Symbol, LE_ALTERNATE_CDECL_SYMBOL_PREFIX( alternateSymbol ) )


extern "C" void         __cdecl _leimpl_Xbad_alloc  (     ) { LE_UNREACHABLE_CODE();                 }
extern "C" char const * __cdecl _leimpl_Syserror_map( int ) { LE_UNREACHABLE_CODE(); return nullptr; }
extern "C" char const * __cdecl _leimpl_Winerror_map( int ) { LE_UNREACHABLE_CODE(); return nullptr; }


LE_ALTERNATE_CDECL_SYMBOL( ?_Xbad_alloc@std@@YAXXZ     , ?_Xbad_alloc@std@@YAXXZ      , _leimpl_Xbad_alloc   )
LE_ALTERNATE_CDECL_SYMBOL( ?_Syserror_map@std@@YAPBDH@Z, ?_Syserror_map@std@@YAPEBDH@Z, _leimpl_Syserror_map )
LE_ALTERNATE_CDECL_SYMBOL( ?_Winerror_map@std@@YAPBDH@Z, ?_Winerror_map@std@@YAPEBDH@Z, _leimpl_Winerror_map )


extern "C" __m128 __vectorcall _leimpl___vdecl_log10f4(      __m128 );
extern "C" float  __cdecl      _leimpl_exp2f          ( _In_ float  );
extern "C" float  __cdecl      _leimpl_log2f          ( _In_ float  );

extern "C" float  __cdecl      _leimpl_roundf         ( _In_ float _X ) { return static_cast<float>( _mm_cvtss_si32( _mm_set_ss( _X ) ) ); }

LE_ALTERNATE_SYMBOL(       ___vdecl_log10f4, __vdecl_log10f4, _leimpl___vdecl_log10f4@@16 )
LE_ALTERNATE_CDECL_SYMBOL( _exp2f          , exp2f          , _leimpl_exp2f               )
LE_ALTERNATE_CDECL_SYMBOL( _log2f          , log2f          , _leimpl_log2f               )
LE_ALTERNATE_CDECL_SYMBOL( _roundf         , roundf         , _leimpl_roundf              )


#ifdef _DEBUG
    LE_ALTERNATE_SYMBOL( ___libm_sse2_sqrt_precise, __libm_sse2_sqrt_precise, _leimpl___libm_sse2_sqrt_precise@@8 )
    LE_ALTERNATE_SYMBOL( ___libm_sse2_exp_precise , __libm_sse2_exp_precise , _leimpl___libm_sse2_exp_precise@@8  )
    LE_ALTERNATE_SYMBOL( ___libm_sse2_log_precise , __libm_sse2_log_precise , _leimpl___libm_sse2_log_precise@@8  )
    LE_ALTERNATE_SYMBOL( ___libm_sse2_pow_precise , __libm_sse2_pow_precise , _leimpl___libm_sse2_pow_precise@@16 )
    LE_ALTERNATE_SYMBOL( ___libm_sse2_sin_precise , __libm_sse2_sin_precise , _leimpl___libm_sse2_sin_precise@@8  )
    LE_ALTERNATE_SYMBOL( ___libm_sse2_cos_precise , __libm_sse2_cos_precise , _leimpl___libm_sse2_cos_precise@@8  )

    LE_ALTERNATE_SYMBOL( __libm_sse2_sqrt_precise  , _libm_sse2_sqrt_precise , _leimpl___libm_sse2_sqrt_precise@@8  )
    LE_ALTERNATE_SYMBOL( __libm_sse2_exp_precise   , _libm_sse2_exp_precise  , _leimpl___libm_sse2_exp_precise@@8   )
    LE_ALTERNATE_SYMBOL( __libm_sse2_log_precise   , _libm_sse2_log_precise  , _leimpl___libm_sse2_log_precise@@8   )
    LE_ALTERNATE_SYMBOL( __libm_sse2_log10_precise , _libm_sse2_log10_precise, _leimpl___libm_sse2_log10_precise@@8 )
    LE_ALTERNATE_SYMBOL( __libm_sse2_pow_precise   , _libm_sse2_pow_precise  , _leimpl___libm_sse2_pow_precise@@16  )
    LE_ALTERNATE_SYMBOL( __libm_sse2_sin_precise   , _libm_sse2_sin_precise  , _leimpl___libm_sse2_sin_precise@@8   )
    LE_ALTERNATE_SYMBOL( __libm_sse2_cos_precise   , _libm_sse2_cos_precise  , _leimpl___libm_sse2_cos_precise@@8   )
#endif // _DEBUG


#ifndef _M_X64
    // https://connect.microsoft.com/VisualStudio/feedback/details/1175765/performance-regression-compared-to-earlier-visual-studio-due-to-dtoui3-casting
    // https://connect.microsoft.com/VisualStudio/feedback/details/808199
    extern "C" std::uint32_t __vectorcall _leimpl__ftoui3( float  const _X ) { return _mm_cvttss_si32( _mm_set_ss( _X ) ); }
    extern "C" std::uint32_t __vectorcall _leimpl__dtoui3( double const _X ) { std::uint32_t integer; { __asm fld _X __asm fistp integer } return integer; }
    extern "C" std:: int64_t __vectorcall _leimpl__ftol3 ( float  const _X ) { std:: int64_t integer; { __asm fld _X __asm fistp integer } return integer; }
    extern "C" std:: int64_t __vectorcall _leimpl__dtol3 ( double const _X ) { std:: int64_t integer; { __asm fld _X __asm fistp integer } return integer; }
    extern "C" double        __vectorcall _leimpl__ltod3 ( std::uint32_t const _Xlower, std::int32_t const _Xupper )
    {
        // https://software.intel.com/en-us/forums/topic/301988
        std::int64_t const integer( ( int64_t( _Xupper ) << 32 ) | _Xlower );
        double floater;
        __asm
        {
            fild integer
            fstp floater
        }
        return floater;
    }

    LE_ALTERNATE_SYMBOL( __ftoui3, _ftoui3, _leimpl__ftoui3@@4 )
    LE_ALTERNATE_SYMBOL( __dtoui3, _dtoui3, _leimpl__dtoui3@@8 )
    LE_ALTERNATE_SYMBOL( __ftol3 , _ftol3 , _leimpl__ftol3@@4  )
    LE_ALTERNATE_SYMBOL( __dtol3 , _dtol3 , _leimpl__dtol3@@8  )
    LE_ALTERNATE_SYMBOL( __ltod3 , _ltod3 , _leimpl__ltod3@@8  )
#endif // x86

extern "C" std::int64_t __cdecl _leimpl__Xtime_get_ticks()
{
    auto const EPOCH( 0x19DB1DED53E8000i64 );
    FILETIME ft; ::GetSystemTimeAsFileTime( &ft );
    return ( ( static_cast<std::int64_t>( ft.dwHighDateTime ) << 32 ) | ft.dwLowDateTime ) - EPOCH;
}

LE_ALTERNATE_CDECL_SYMBOL( __Xtime_get_ticks, _Xtime_get_ticks, _leimpl__Xtime_get_ticks )


#if _MSC_VER >= 1900 // VS2015

extern "C" int __cdecl _leimpl___stdio_common_vsprintf
(
    _In_                                    unsigned __int64 const _Options,
    _Out_writes_z_(_BufferCount)            char*            const _Buffer,
    _In_                                    size_t           const _BufferCount,
    _In_z_ _Printf_format_string_params_(2) char const*      const _Format,
    _In_opt_                                _locale_t        const _Locale,
    va_list          _ArgList
)
{
    (void)_Options;
    (void)_BufferCount;
    (void)_Locale;
    _ASSERTE( _BufferCount < 1024 );
    _ASSERTE( !_Locale );
    auto const result( ::wvsprintfA( _Buffer, _Format, _ArgList ) );
    _ASSERTE( result < _BufferCount );
    return result;
}


extern "C" int __CRTDECL _leimpl__vsnprintf_l(
    _Out_writes_(_BufferCount)              char*       const _Buffer,
    _In_                                    size_t      const _BufferCount,
    _In_z_ _Printf_format_string_params_(2) char const* const _Format,
    _In_opt_                                _locale_t   const _Locale,
                                            va_list           _ArgList
    )
{
    int const _Result = __stdio_common_vsprintf(
        _CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
        _Buffer, _BufferCount, _Format, _Locale, _ArgList);

    return _Result < 0 ? -1 : _Result;
}


extern "C" int __CRTDECL _leimpl__vsnprintf(
    _Out_writes_(_BufferCount) _Post_maybez_ char*       const _Buffer,
    _In_                                     size_t      const _BufferCount,
    _In_z_ _Printf_format_string_            char const* const _Format,
                                             va_list           _ArgList
    )
{
    int const _Result = __stdio_common_vsprintf(
        _CRT_INTERNAL_LOCAL_PRINTF_OPTIONS | _CRT_INTERNAL_PRINTF_LEGACY_VSPRINTF_NULL_TERMINATION,
        _Buffer, _BufferCount, _Format, nullptr, _ArgList);
    return _Result < 0 ? -1 : _Result;
}

LE_ALTERNATE_CDECL_SYMBOL(  _vsnprintf  ,  vsnprintf  , _leimpl__vsnprintf   )
LE_ALTERNATE_CDECL_SYMBOL( __vsnprintf  , _vsnprintf  , _leimpl__vsnprintf   )
LE_ALTERNATE_CDECL_SYMBOL( __vsnprintf_l, _vsnprintf_l, _leimpl__vsnprintf_l )



extern "C" _CRTIMP FILE * __cdecl         __iob_func();
extern "C"         FILE * __cdecl _leimpl___iob_func() { return __acrt_iob_func( 0 ); }

LE_ALTERNATE_CDECL_SYMBOL( ___iob_func, __iob_func, _leimpl___iob_func )

extern "C" FILE * __cdecl _leimpl___acrt_iob_func( unsigned const id ) { return &__iob_func()[ id ]; }

LE_ALTERNATE_CDECL_SYMBOL( ___stdio_common_vsprintf, __stdio_common_vsprintf, _leimpl___stdio_common_vsprintf )
LE_ALTERNATE_CDECL_SYMBOL( ___acrt_iob_func        , __acrt_iob_func        , _leimpl___acrt_iob_func         )


#include "vcruntime_exception.h"

extern "C" void __cdecl _leimpl___std_exception_copy(
    __std_exception_data const* const from,
    __std_exception_data*       const to
    )
{
    _ASSERTE(to->_What == nullptr && to->_DoFree == false);

    if (!from->_DoFree || !from->_What)
    {
        to->_What   = from->_What;
        to->_DoFree = false;
        return;
    }

    size_t const buffer_count = strlen(from->_What) + 1;

    char * const buffer( new (std::nothrow) char[buffer_count] );
    if (!buffer)
    {
        return;
    }

    strcpy_s(buffer, buffer_count, from->_What);
    to->_What   = buffer;
    to->_DoFree = true;
}

extern "C" void __cdecl _leimpl___std_exception_destroy(
    __std_exception_data* const data
    )
{
    if (data->_DoFree)
    {
        delete data->_What;
    }

    data->_DoFree = false;
    data->_What   = nullptr;
}

LE_ALTERNATE_CDECL_SYMBOL( ___std_exception_copy   , __std_exception_copy   , _leimpl___std_exception_copy    )
LE_ALTERNATE_CDECL_SYMBOL( ___std_exception_destroy, __std_exception_destroy, _leimpl___std_exception_destroy )

#endif // _MSC_VER >= 1900 (VS2015)
//------------------------------------------------------------------------------
#endif // LE_SDK_BUILD || LE_SW_FMOD
