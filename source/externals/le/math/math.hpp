////////////////////////////////////////////////////////////////////////////////
///
/// \file math.hpp
/// --------------
///
/// Generic math routine collection.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef math_hpp__C20A5FD2_AC91_41D7_8F61_B10C67B19A6D
#define math_hpp__C20A5FD2_AC91_41D7_8F61_B10C67B19A6D
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#ifdef LE_HAS_NT2
    #include "boost/simd/sdk/config/arch.hpp"
    #include "boost/simd/sdk/simd/extensions.hpp"
#else
    #include "le/utility/intrinsics.hpp"
#endif // LE_HAS_NT2

#ifndef NDEBUG
    #include "boost/current_function.hpp"
#endif // NDEBUG
#include "boost/integer/static_log2.hpp"
#include "boost/noncopyable.hpp"
#include "boost/range/iterator_range_core.hpp"

#if defined( __ARM_NEON__ ) || defined( __aarch64__ )
    #include "arm_neon.h"
#endif // __ARM_NEON__

#ifndef BOOST_SIMD_HAS_SSE_SUPPORT
#include <algorithm>
#endif // BOOST_SIMD_HAS_SSE_SUPPORT
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <type_traits>
#ifndef NDEBUG
#include <string>
#endif // NDEBUG
//------------------------------------------------------------------------------
#if defined( _MSC_VER )
namespace std
{
    inline int isfinite( float  const value ) { return _finite( value ); }
    inline int isfinite( double const value ) { return _finite( value ); }
} // namespace std
#elif defined( __ANDROID__ ) && defined( _STLPORT_VERSION )
    #undef fpclassify
    #undef isfinite
    #undef signbit
namespace std
{
    inline int isfinite( float  const value ) { return __isfinitef( value ); }
    inline int isfinite( double const value ) { return __isfinite ( value ); }

    inline int fpclassify( float  const value ) { return __fpclassifyf( value ); }
    inline int fpclassify( double const value ) { return __fpclassifyd( value ); }

    inline int signbit( float  const value ) { return __signbitf( value ); }
    inline int signbit( double const value ) { return __signbit ( value ); }
} // namespace std
#endif // compiler/CRT
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

// http://www.strchr.com/optimized_abs_function
using std::abs;
inline std::uint32_t abs( unsigned int   const value ) { return value; }
inline std::uint16_t abs(          short const value ) { return static_cast<std::uint16_t>( std::abs( static_cast<int>( value ) ) ); }
inline std::uint16_t abs( unsigned short const value ) { return value; }
inline std::uint8_t  abs( unsigned char  const value ) { return value; }
inline std::uint8_t  abs(          char  const value ) { return static_cast<std::uint8_t >( std::abs( static_cast<int>( value ) ) ); }
       std::uint8_t  abs(          bool        value );

float copySign( float targetNumber, float signSource );

int floor   ( float  value );
int ceil    ( float  value );
int round   ( float  value );
int round   ( double value );
int truncate( float  value );

float        modulo( float        dividend, float        divisor );
         int modulo( int          dividend, int          divisor );
unsigned int modulo( unsigned int dividend, unsigned int divisor );

struct SplitFloat
{
    int   integer;
    float fractional;
};

SplitFloat splitFloat( float value );


bool equal( float const & left, float const & right );
bool equal( float const & left, unsigned int  right );
template <typename T>
bool equal( T const left, T const right ) { static_assert( !std::is_floating_point<T>::value, "Internal inconsistency" ); return left == right; }

bool LE_FASTCALL nearEqual( float left, float        right );
bool LE_FASTCALL nearEqual( float left, unsigned int right );

template <typename T>
bool isZero( T     const   value ) { return equal( value, T( 0 ) ); }
bool isZero( float const & value );

template <int ComparisonValue>
bool is( float const & value )
{
    static float const comparisonValue( static_cast<float>( ComparisonValue ) );
    return equal( value, comparisonValue );
}

bool isNegative( float        value );
bool isNegative(          int value );
bool isNegative( unsigned int value );

float valueIfNot( float const & value, bool condition );

namespace PositiveFloats
{
    bool isGreater( float  left, float  right );
    bool isGreater( double left, double right );

    float valueIfGreater( float testValue, float lowerBound, float value );

    unsigned int ceil ( float value );
    unsigned int floor( float value );
#if ( defined( __clang__ ) && defined( BOOST_SIMD_ARCH_ARM ) )
    // http://en.wikipedia.org/wiki/Rounding#Rounding_to_integer // round to nearest up, incorrect for negative values...
    inline unsigned int round( float const floatingPointValue ) { return static_cast<unsigned int>( floatingPointValue + 0.5f ); }
#else
    //using Math::round; //...mrmlj...compilation errors with 'anonymous impl namespaces'...
#endif // __clang__

    float modulo( float dividend, float divisor );

    bool isZero( float const & value );
} // namespace PositiveFloats

namespace PowerOfTwo
{
    unsigned int ceil ( float        );
    unsigned int floor( unsigned int );
    unsigned int round( unsigned int );
    std::uint8_t log2 ( unsigned int );
}

LE_CONST_FUNCTION float        LE_FASTCALL_ABI log2( float         value );
LE_CONST_FUNCTION std::uint8_t LE_FASTCALL_ABI log2( int           value );
LE_CONST_FUNCTION std::uint8_t LE_FASTCALL_ABI log2( unsigned int  value );
LE_CONST_FUNCTION std::uint8_t LE_FASTCALL_ABI log2( unsigned long value );

LE_CONST_FUNCTION float LE_FASTCALL_ABI ln   ( float );
LE_CONST_FUNCTION float LE_FASTCALL_ABI log10( float );
LE_CONST_FUNCTION float LE_FASTCALL_ABI exp  ( float );
LE_CONST_FUNCTION float LE_FASTCALL_ABI exp2 ( float );

LE_NOTHROWNOALIAS
void LE_FASTCALL_ABI addPolar( float amp1, float phase1, float & amp2, float & phase2 );

float         clamp( float        value, float         lowerBound, float         upperBound );
std::uint64_t clamp( std::int64_t value, std::uint64_t lowerBound, std::uint64_t upperBound );
std::uint32_t clamp( std::int32_t value, std::uint32_t lowerBound, std::uint32_t upperBound );
std::uint16_t clamp( std::int16_t value, std::uint16_t lowerBound, std::uint16_t upperBound );
std::uint8_t  clamp( std::int8_t  value, std::uint8_t  lowerBound, std::uint8_t  upperBound );

std::uint16_t clamp( std::uint16_t value, std::uint16_t lowerBound, std::uint16_t upperBound );

std::uint8_t firstSetBit    ( int );
std::uint8_t lastSetBit     ( int );
std::uint8_t numberOfSetBits( int );

bool isPowerOfTwo(          int );
bool isPowerOfTwo( unsigned int );
bool isPowerOfTwo( float        );

template <unsigned int value>
struct IsPowerOfTwo : std::integral_constant<bool, ( 1 << boost::static_log2<value>::value ) == value> {};

inline bool isNormalisedValue( float const value ) { return ( value >= 0 ) && ( value <= 1 ); }

void rngSeed();

float LE_FASTCALL normalisedRand();

float         LE_FASTCALL rangedRand(                       float         maximum );
std::uint32_t LE_FASTCALL rangedRand(                       std::uint32_t maximum );
std::uint16_t LE_FASTCALL rangedRand(                       std::uint16_t maximum );
float         LE_FASTCALL rangedRand( float        minimum, float         maximum );
std:: int32_t LE_FASTCALL rangedRand( std::int32_t minimum, std::uint32_t maximum );


template <class UnsignedInteger>
UnsignedInteger roundUpUnsignedIntegerDivision( UnsignedInteger const dividend, UnsignedInteger const divisor )
{
    static_assert( std::is_unsigned<UnsignedInteger>::value, "" );
    return ( dividend + ( divisor - 1 ) ) / divisor;
}


////////////////////////////////////////////////////////////////////////////////
///
/// \class FPUDisableDenormalsGuard
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class FPUDisableDenormalsGuard : boost::noncopyable
{
public:
     FPUDisableDenormalsGuard();
    ~FPUDisableDenormalsGuard();

protected:
    unsigned int const originalFloatingPointControlWord_;
}; // class FPUDisableDenormalsGuard


////////////////////////////////////////////////////////////////////////////////
///
/// \class FPUExceptionsGuard
///
/// \brief Saves and restores the FPU control word exception mask.
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class FPUExceptionsGuard : boost::noncopyable
{
public:
     FPUExceptionsGuard();
    ~FPUExceptionsGuard();

protected:
#ifdef _MSC_VER
    unsigned int const originalFloatingPointControlWord_;

    static unsigned int const exceptionsMask = EM_OVERFLOW /*| EM_UNDERFLOW*/ /*| EM_INEXACT*/ | EM_ZERODIVIDE | EM_INVALID | EM_DENORMAL;
#endif // _MSC_VER
}; // class FPUExceptionsGuard


////////////////////////////////////////////////////////////////////////////////
///
/// \class FPUExceptionsEnabler
///
/// \brief Locally enables FPU exceptions.
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class FPUExceptionsEnabler : public FPUExceptionsGuard
{
public:
    FPUExceptionsEnabler();
}; // class FPUExceptionsEnabler


////////////////////////////////////////////////////////////////////////////////
///
/// \class FPUExceptionsDisabler
///
/// \brief Locally disables FPU exceptions.
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class FPUExceptionsDisabler : public FPUExceptionsGuard
{
public:
    FPUExceptionsDisabler();
}; // class FPUExceptionsDisabler

#if defined( _DEBUG ) && !( defined( LE_SW_SDK_BUILD ) && !defined( __MSVC_RUNTIME_CHECKS ) )
    #define LE_LOCALLY_DISABLE_FPU_EXCEPTIONS() ::LE::Math::FPUExceptionsDisabler const fpuDebuggerGuard
#else
    #define LE_LOCALLY_DISABLE_FPU_EXCEPTIONS()
#endif


enum FPClass
{
    SignalingNaN = 1 << 0,
    QuietNaN     = 1 << 1,
    Infinity     = 1 << 3,
    Positive     = 1 << 4,
    Negative     = 1 << 5,
    Normalised   = 1 << 6,
    Denormalised = 1 << 7,
    Zero         = 1 << 8,

    NaN           = SignalingNaN | QuietNaN,
    Invalid       = NaN | Infinity,
    InvalidOrSlow = Invalid | Denormalised
};

template <unsigned FPClasses>
unsigned int LE_NOINLINE has( float const * LE_RESTRICT pRange, std::size_t rangeSize )
{
    LE_LOCALLY_DISABLE_FPU_EXCEPTIONS();

    unsigned int result( 0 );

    while ( rangeSize-- && ( result != FPClasses ) )
    {
        /// \note To handle the case where user input data contains denormals
        /// and the fpclassify function handles denormals while we have them
        /// disabled we also check how the value compares to zero if the
        /// fpclassify function detects a denormal value.
        ///                                   (24.05.2016.) (Domagoj Saric)
        auto const value       ( *pRange++  );
        auto const valueNonZero( value != 0 );
    #ifdef _MSC_VER
        switch( /*std*/::_fpclass( value ) )
        {
            case _FPCLASS_SNAN: result |= ( FPClasses & SignalingNaN                                   ); break;
            case _FPCLASS_QNAN: result |= ( FPClasses & QuietNaN                                       ); break;
            case _FPCLASS_NINF: result |= ( FPClasses & Infinity                                       ); break;
            case _FPCLASS_NN  : result |= ( FPClasses & ( Negative | Normalised                      ) ); break;
            case _FPCLASS_ND  : result |= ( FPClasses & ( Negative | ( Denormalised * valueNonZero ) ) ); break;
            case _FPCLASS_NZ  : result |= ( FPClasses & ( /*Negative |*/ Zero                        ) ); break; //...mrmlj...Talking Wind...
            case _FPCLASS_PZ  : result |= ( FPClasses & ( Positive | Zero                            ) ); break;
            case _FPCLASS_PD  : result |= ( FPClasses & ( Positive | ( Denormalised * valueNonZero ) ) ); break;
            case _FPCLASS_PN  : result |= ( FPClasses & ( Positive | Normalised                      ) ); break;
            case _FPCLASS_PINF: result |= ( FPClasses & Infinity                                       ); break;
        }
    #else
        switch ( std::fpclassify( value ) )
        {
        #ifdef FP_NANS
            case FP_NANS     :                     result |= ( FPClasses & SignalingNaN ); break;
        #endif // FP_NANS
            case FP_NAN      :                     result |= ( FPClasses & QuietNaN     ); break;
            case FP_INFINITE :                     result |= ( FPClasses & Infinity     ); break;
            case FP_ZERO     :                     result |= ( FPClasses & Zero         ); break;
            case FP_SUBNORMAL: if ( valueNonZero ) result |= ( FPClasses & Denormalised ); break;
            case FP_NORMAL   :                     result |= ( FPClasses & Normalised   ); break;
        }
        if ( std::signbit( value ) && /*...mrmlj...Talking Wind...*/ value < -std::numeric_limits<float>::epsilon() )
            result |= ( FPClasses & Negative );
        else
            result |= ( FPClasses & Positive );
    #endif // _MSC_VER
    }

    return result;
}

template <unsigned FPClasses>
unsigned int has( boost::iterator_range<float const *> const & range ) { return has<FPClasses>( range.begin(), range.size() ); }


template <unsigned FPClasses>
void LE_FASTCALL verifyFPValues
(
    float const * const pRange, std::size_t const rangeSize,
    char const * const valueName,
    char const * const function ,
    char const * const file     ,
    long         const line
)
{
#ifdef NDEBUG
    boost::ignore_unused_variable_warning( pRange && rangeSize && valueName );
#else
    #ifdef BOOST_ENABLE_ASSERT_HANDLER
        #define LE_AUX_VERIFY_FP_VALUES_FAILURE( ... ) boost::assertion_failed_msg( __VA_ARGS__ )
    #else
        #define LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, errorString, function, file, line ) BOOST_ASSERT_MSG( false, ( std::string( errorString " @ " ) + valueName ).c_str() )
    #endif // BOOST_ENABLE_ASSERT_HANDLER
    unsigned int const fpClasses( has<FPClasses>( pRange, rangeSize ) );
    if ( fpClasses & FPClasses & NaN          ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "NaN value found"         , function, file, line );
    if ( fpClasses & FPClasses & Infinity     ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "Infinite value found"    , function, file, line );
    if ( fpClasses & FPClasses & Positive     ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "Positive value found"    , function, file, line );
    if ( fpClasses & FPClasses & Negative     ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "Negative value found"    , function, file, line );
    if ( fpClasses & FPClasses & Normalised   ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "Normalised value found"  , function, file, line );
    if ( fpClasses & FPClasses & Denormalised ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "Denormalised value found", function, file, line );
    if ( fpClasses & FPClasses & Zero         ) LE_AUX_VERIFY_FP_VALUES_FAILURE( valueName, "Zero value found"        , function, file, line );
    #undef LE_AUX_VERIFY_FP_VALUES_FAILURE
#endif // NDEBUG
}

template <unsigned FPClasses>
void verifyFPValues
(
    boost::iterator_range<float const *> const & range,
    char const * const valueName,
    char const * const function ,
    char const * const file     ,
    long         const line
)
{
    return verifyFPValues<FPClasses>( range.begin(), range.size(), valueName, function, file, line );
}

#ifdef NDEBUG
    #ifndef LE_MATH_VERIFY_VALUES // required for unity builds
        #define LE_MATH_VERIFY_VALUES( fpClasses, range, valueName ) (void(0))
    #endif // LE_MATH_VERIFY_VALUES
#else
    #define LE_MATH_VERIFY_VALUES( fpClasses, range, valueName )   \
        /*::LE::*/Math::verifyFPValues<fpClasses>( range, valueName, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__ )
#endif // NDEBUG


////////////////////////////////////////////////////////////////////////////////
/// Force-inlined implementations for functions that MSVC10 does not inline when
/// optimizing for size.
////////////////////////////////////////////////////////////////////////////////

LE_FORCEINLINE
float clamp( float const value, float const lowerBound, float const upperBound )
{
#if defined( BOOST_SIMD_HAS_SSE_SUPPORT )
    __m128 vectorResult( _mm_set_ss( value ) );
    vectorResult = _mm_max_ss( vectorResult, _mm_set_ss( lowerBound ) );
    vectorResult = _mm_min_ss( vectorResult, _mm_set_ss( upperBound ) );
    return _mm_cvtss_f32( vectorResult );
#elif defined( BOOST_SIMD_ARCH_ARM )
    // NEON http://stackoverflow.com/questions/11516935/efficient-neon-implementation-of-clipping?rq=1
         if ( value < lowerBound ) return lowerBound;
    else if ( value > upperBound ) return upperBound;
                                   return value     ;
#else
    float result( value );
    result = max( result, lowerBound );
    result = min( result, upperBound );
    return result;
#endif // BOOST_SIMD_HAS_SSE_SUPPORT
}


LE_FORCEINLINE
float copySign( float const targetNumber, float const signSource )
{
    // http://stackoverflow.com/questions/2922619/how-to-efficiently-compare-the-sign-of-two-floating-point-values-while-handling-n

#if defined( __GNUC__ )
    float const result( __builtin_copysignf( targetNumber, signSource ) );
#elif defined( BOOST_SIMD_HAS_SSE_SUPPORT )
    __m128 const signBitMaskVector       ( _mm_set_ps1( -0.0f ) );
    __m128 const targetNumberAbsoluteBits( _mm_andnot_ps( signBitMaskVector       , _mm_set_ss( targetNumber ) ) );
    __m128 const signSourceSignBit       ( _mm_and_ps   ( signBitMaskVector       , _mm_set_ss( signSource   ) ) );
    __m128 const resultVector            ( _mm_or_ps    ( targetNumberAbsoluteBits, signSourceSignBit          ) );
    float const result( _mm_cvtss_f32( resultVector ) );
#else
    int const targetNumberAbsoluteBits( reinterpret_cast<int const &>( targetNumber ) & 0x7FFFFFFF );
    int const signSourceSignBit       ( reinterpret_cast<int const &>( signSource   ) & 0x80000000 );
    int const resultBits              ( targetNumberAbsoluteBits | signSourceSignBit               );

    float const result( reinterpret_cast<float const &>( resultBits ) );
#endif // BOOST_SIMD_HAS_SSE_SUPPORT

#ifdef _MSC_VER
    BOOST_ASSERT( result == /*std*/::_copysign( targetNumber, signSource ) );
#else
    BOOST_ASSERT( result == /*std*/:: copysign( targetNumber, signSource ) );
#endif // _MSC_VER
    return result;
}


LE_FORCEINLINE
float valueIfNot( float const & value, bool const condition )
{
    auto const mask( static_cast<std::uint32_t>( static_cast<std::uint8_t>( condition ) - 1 ) );
#if defined( BOOST_SIMD_HAS_SSE2_SUPPORT )
    __m128 const maskVector  ( _mm_castsi128_ps( _mm_cvtsi32_si128( mask ) ) );
    __m128 const resultVector( _mm_and_ps( maskVector, _mm_set_ss( value ) ) );
    float const result( _mm_cvtss_f32( resultVector ) );
#else
    union { std::uint32_t bits; float value; } resultBits;
    resultBits.value  = value;
    resultBits.bits  &= mask ;
    float const result( resultBits.value );
#endif // BOOST_SIMD_HAS_SSE_SUPPORT
    BOOST_ASSERT( result == ( condition ? 0 : value ) );
    return result;
}


////////////////////////////////////////////////////////////////////////////////
//
// round()
// -------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \ingroup TypeConversion Type conversion
/// \brief Rounds a floating point value to the nearest integer.
///   Faster than a simple static_cast as it bypasses hidden ftol() calls/FPU
/// setup code inserted by the (MSVC) compiler.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Investigate:
///    - boost::numeric_cast<>
///    - http://ldesoras.free.fr/doc/articles/rounding_en.pdf
///    - http://www.mega-nerd.com/FPcast
///    - http://www.codeproject.com/KB/cpp/floatutils.aspx
///    - http://www.stereopsis.com/FPU.html
///    - http://stackoverflow.com/questions/2550281/floating-point-vs-integer-calculations-on-modern-hardware
///    - http://chrishecker.com/Miscellaneous_Technical_Articles#Floating_Point
///    - http://stackoverflow.com/questions/78619/what-is-the-fastest-way-to-convert-float-to-int-on-x86
///    - http://www.devmaster.net/forums/showthread.php?t=10153
///    - http://software.intel.com/en-us/articles/fast-floating-point-to-integer-conversions
///    - http://stereopsis.com/sree/fpu2006.html
///    - http://chrishecker.com/images/f/fb/Gdmfp.pdf
///    - http://www.cs.uaf.edu/2009/fall/cs301/lecture/12_09_float_to_int.html
///                                           (14.12.2009.) (Domagoj Saric)
/// \todo Implement float->float rounding (it could speed up the phase vocoder/
/// mapTo2Pi function).
///                                           (19.02.2016.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

LE_FORCEINLINE
std::int32_t round( float const floatingPointValue )
{
    // Implementation note:
    // MSVC, GCC and Clang do not provide in-memory operand versions of SSE
    // intrinsics however we do not use inline assembler instead as this does
    // more harm than good because it hinders the optimizer.
    //                                        (06.12.2010.) (Domagoj Saric)
#ifdef BOOST_SIMD_HAS_SSE_SUPPORT
    return _mm_cvtss_si32( _mm_set_ss( floatingPointValue ) );
#elif defined( _MSC_VER )
    #ifdef _XBOX
        return __frnd( floatingPointValue );
    #else
        std::int32_t integerValue;
        __asm
        {
            fld   floatingPointValue
            fistp integerValue
        }
        return integerValue;
    #endif
#elif defined( BOOST_SIMD_ARCH_ARM ) && defined( __GNUC__ )
    #if defined( __SOFTFP__ ) || ( defined( __thumb__ ) && ( __ARM_ARCH < 7 ) )
        // https://connect.microsoft.com/VisualStudio/feedback/details/1179496/cq-intrinsics-vc-lrintf-is-376-times-longer-than-gcc-clang-version
        // https://gcc.gnu.org/bugzilla/show_bug.cgi?id=48139
        // https://llvm.org/bugs/show_bug.cgi?id=11544 Trivial math builtins not inlined
        // http://lists.apple.com/archives/perfoptimization-dev/2007/Jan/msg00002.html
        return __builtin_lrintf( floatingPointValue );
    #elif ( __ARM_ARCH == 7 )
        // https://www.crest.iu.edu/projects/conceptcpp/docs/html-ext/CGBuiltin_8cpp_source.html
        //...mrmlj...Clang (3.6 and Xcode 7.2.1) crashes here with "fatal error: error in backend: Cannot select: intrinsic %llvm.arm.vcvtr"...
        // return __builtin_arm_vcvtr_f( floatingPointValue, 0 );

        // http://www.ethernut.de/en/documents/arm-inline-asm.html
        // http://hardwarebug.org/2010/07/06/arm-inline-asm-secrets
        // This uses the 'current' rounding mode, which defaults to what we want
        // (see FPDSCR, Floating-point Default Status Control Register), i.e.
        // round to nearest.
        float mutableInput( floatingPointValue );
        std::int32_t integerValue;
        __asm__
	    (
		    "vcvtr.s32.f32 %1, %1\n"
            "vmov.s32      %0, %1\n"
            : "=r"( integerValue ), "+&w"( mutableInput )
	    );
        BOOST_ASSERT( integerValue == __builtin_lrintf( floatingPointValue ) );
        return integerValue;
    #elif ( __ARM_ARCH >= 8 )
        return vcvtns_s32_f32( floatingPointValue );
    #else
        // vcvt_* "rounds towards zero" which actually seems to be plain truncation...
        // http://stackoverflow.com/questions/10762620/vectorized-floating-point-rounding-using-neon
        #if 1
            std::int32_t const integerValue( floatingPointValue + __builtin_copysignf( 0.5f, floatingPointValue ) );
        #elif 0 && defined( BOOST_LITTLE_ENDIAN )
            double constexpr magic( ( 1ULL << 52 ) * 1.5 ); //...mrmlj...float( 1<<23 ) should work/be enough for positive numbers...
            union { double asDouble; std::int32_t asInteger; } bits = { floatingPointValue + magic };
            auto const integerValue( bits.asInteger );
        #elif 0 // Clang 3.2 issues two FPU control register fetch instructions so slower than the last one
            // http://stackoverflow.com/questions/485525/round-for-float-in-c
            auto  const truncatedValue( static_cast<std::int32_t>( floatingPointValue )  );
            float const difference    ( floatingPointValue - truncatedValue              );
            auto  const adjustment    ( ( difference >= 0.5f ) - ( difference <= -0.5f ) );
            auto  const integerValue  ( truncatedValue + adjustment                      );
        #else
            int const integerValue( floatingPointValue + ( ( floatingPointValue > 0 ) ? +0.5f : -0.5f ) );
        #endif
        //...mrmlj...BOOST_ASSERT( integerValue == ::__builtin_rintf( floatingPointValue ) ) ); // rounds to nearest even
        return integerValue;
    #endif
#elif defined( __GNUC__ )
    /// \note Neither Clang nor GCC inline a call to __builtin_lrintf (at
    /// least when targeting the ARM).
    ///                                   (02.11.2012.) (Domagoj Saric)
    return static_cast<std::int32_t>( ::__builtin_lrintf( floatingPointValue ) );
#endif
}

LE_FORCEINLINE
int round( double const floatingPointValue )
{
#ifdef BOOST_SIMD_HAS_SSE2_SUPPORT
    return _mm_cvtsd_si32( _mm_load_sd( &floatingPointValue ) );
#elif defined( __GNUC__ )
    return static_cast<int>( ::__builtin_lrint( floatingPointValue ) );
#elif defined( _XBOX )
    return __frnd( floatingPointValue );
#elif defined( BOOST_LITTLE_ENDIAN )
    double const magic( ( 1ULL << 52 ) * 1.5 );
    union { double asDouble; int asInteger; } bits = { floatingPointValue + magic };
    int const result( bits.asInteger );
    #if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD ) && defined( _MSC_VER ) && !defined( _XBOX )
        int integerValue;
        __asm
        {
            fld   floatingPointValue
            fistp integerValue
        }
        BOOST_ASSERT( result == integerValue );
    #endif // internal DEBUG build
    return result;
#endif
}


////////////////////////////////////////////////////////////////////////////////
//
// truncate()
// ----------
//
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   See the notes for round().
//                                            (06.12.2010.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

LE_FORCEINLINE
int truncate( float const floatingPointValue )
{
    BOOST_ASSERT_MSG( floatingPointValue < std::numeric_limits<int>::max(), "Float out of int range." );
    BOOST_ASSERT_MSG( floatingPointValue > std::numeric_limits<int>::min(), "Float out of int range." );
#ifdef BOOST_SIMD_HAS_SSE_SUPPORT
    return _mm_cvttss_si32( _mm_set_ss( floatingPointValue ) );
#else
    return static_cast<int>( floatingPointValue );
#endif
}


namespace PositiveFloats
{
    LE_FORCEINLINE
    bool isGreater( float const left, float const right )
    {
    #if defined( _MSC_VER ) && defined( BOOST_SIMD_HAS_SSE_SUPPORT )
        unsigned int const boolean( _mm_ucomigt_ss( _mm_set_ss( left ), _mm_set_ss( right ) ) );
        //LE_ASSUME( boolean == 0 || boolean == 1 ); ...mrmlj...seems to cause larger code...
        //LE_ASSUME( boolean <= 1 );
        return reinterpret_cast<bool const &>( boolean );
    #elif defined( _MSC_VER ) && defined( BOOST_SIMD_HAS_SSE2_SUPPORT )
        __m128   const maskVector( _mm_cmpge_ss( _mm_set_ss( left ), _mm_set_ss( right ) ) );
        unsigned const mask      ( _mm_cvtsi128_si32( _mm_castps_si128( maskVector ) ) );
        return reinterpret_cast<bool const &>( mask );
    #else
        return left > right;
    #endif // BOOST_SIMD_HAS_SSE_SUPPORT
    }

    LE_FORCEINLINE
    bool isGreater( double const left, double const right )
    {
    #if defined( _MSC_VER ) && defined( BOOST_SIMD_HAS_SSE2_SUPPORT )
        unsigned int const boolean( _mm_ucomigt_sd( _mm_set_sd( left ), _mm_set_sd( right ) ) );
        LE_ASSUME( boolean == 0 || boolean == 1 );
        LE_ASSUME( boolean <= 1 );
        return reinterpret_cast<bool const &>( boolean );
    #else
        return left > right;
    #endif // BOOST_SIMD_HAS_SSE_SUPPORT
    }

    // Implementation note:
    //   This function is required because MSVC10 is unable to generate good
    // code if the calling code simply uses the isGreater() and
    // fast_bool_t::mask() functions directly.
    //                                        (01.12.2011.) (Domagoj Saric)
    LE_FORCEINLINE
    float valueIfGreater( float const testValue, float const lowerBound, float const value )
    {
    #ifdef BOOST_SIMD_HAS_SSE_SUPPORT
        __m128 const mask
        (
            LE_MSVC_SPECIFIC( _mm_cmpgt_ps ) //...mrmlj...
            LE_GNU_SPECIFIC(  _mm_cmpgt_ss )
            (
                _mm_set_ss( testValue  ),
                _mm_set_ss( lowerBound )
            )
        );
        return _mm_cvtss_f32( _mm_and_ps( mask, _mm_set_ss( value ) ) );
    #else
        auto const isGreater_( isGreater( testValue, lowerBound ) );
        return isGreater_ ? value : 0;
    #endif
    }
} // namespace PositiveFloats
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // math_hpp
