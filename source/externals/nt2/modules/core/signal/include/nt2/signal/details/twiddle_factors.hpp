//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2015   Domagoj Saric, Little Endian Ltd.
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#ifndef NT2_SIGNAL_DETAILS_TWIDDLE_FACTORS_HPP_INCLUDED
#define NT2_SIGNAL_DETAILS_TWIDDLE_FACTORS_HPP_INCLUDED
#ifdef _MSC_VER
    #pragma once
    #define _USE_MATH_DEFINES
#endif

#include <nt2/signal/details/missing_functionality.hpp> //...mrmlj...to be moved elsewhere...
#include <nt2/signal/details/operators_lite.hpp>
//#include <nt2/signal/details/static_sincos.hpp>

#include <nt2/include/functions/scalar/sincospi.hpp>
#include <nt2/include/functions/simd/sinecosine.hpp>
#include <nt2/include/functions/simd/sincosd.hpp>
#include <nt2/include/functions/simd/sincospi.hpp>
#include <nt2/include/constants/mzero.hpp>

#include <boost/simd/sdk/simd/extensions.hpp>
#include <boost/simd/include/functions/simd/enumerate.hpp>
#include <boost/simd/preprocessor/aligned_type.hpp>

#include <boost/concept_check.hpp>
#include <boost/preprocessor/arithmetic/add.hpp>
#include <boost/preprocessor/arithmetic/mul.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

#include <math.h>
//------------------------------------------------------------------------------
namespace nt2
{
//------------------------------------------------------------------------------

template <typename T>
struct twiddle_pair
{
    T /*const*/ wr;
    T /*const*/ wi;
}; // twiddle_pair

template <typename T>
struct split_radix_twiddles
{
    twiddle_pair<T> /*const*/ w0;
    twiddle_pair<T> /*const*/ w3;
}; // split_radix_twiddles


/*
template
<
    unsigned FFTSize,
    typename T
>
struct twiddles_interleaved;
{
    typedef
        BOOST_SIMD_ALIGN_ON( BOOST_SIMD_CONFIG_ALIGNMENT )
        boost::array<twiddle_pair<T> const, fft_size / data_vector_size>
    factors_t;
    static factors_t const factors;
};
*/


////////////////////////////////////////////////////////////////////////////////
// Runtime-static initialisation
//  + simple
//  + compile-time lightweight
//  - does not get placed in the read-only text section
//  - creates dynamic initialisers for each N
//  - forces one of the sincos functions (depending on the used twiddle
//    calculator) to be present in the binary
//  - requires effort to achieve maximum possible precision
////////////////////////////////////////////////////////////////////////////////

namespace detail
{
    typedef long double long_double_t;

    ////////////////////////////////////////////////////////////////////////////
    // Twiddle calculator implementations.
    ////////////////////////////////////////////////////////////////////////////
    /// \note Simple radians based sinecosine calculation of twiddles does not
    /// give maximally accurate values because rational values of Pi (a
    /// transcendental number) cannot be represented accurately enough [this
    /// becomes most adverse for inputs which should produce an exact zero, like
    /// cos( Pi/2 )].
    ///                                       (17.07.2012.) (Domagoj Saric)
    ////////////////////////////////////////////////////////////////////////////

    struct twiddle_calculator_scalar
    {
    #ifdef _MSC_VER
        #pragma warning( push )
        #pragma warning( disable : 4510 ) // Default constructor could not be generated.
        #pragma warning( disable : 4512 ) // Assignment operator could not be generated.
        #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
    #endif
        struct input_t
        {
            long_double_t const omega_scale;
            long_double_t const N          ;
            int           const index      ;
        };
    #ifdef _MSC_VER
        #pragma warning( pop )
    #endif

        template <typename Vector>
        static BOOST_FORCEINLINE input_t generate_input( int const index, long_double_t const omega_scale, long_double_t const N )
        {
            input_t const input = { omega_scale, N, index };
            return input;
        }
    }; // struct twiddle_calculator_scalar

    template <typename Impl>
    struct twiddle_calculator_same_type
    {
        template <typename Vector>
        static BOOST_FORCEINLINE BOOST_COLD Vector generate_input( int const index, long_double_t const omega_scale, long_double_t const N )
        {
            long_double_t const omega( omega_scale * Impl::full_circle() / N );

            long_double_t const start_value( index * omega );
            long_double_t const increment  (         omega );

            typedef typename Vector::value_type scalar_t;
            Vector const result ( boost::simd::enumerate<Vector>( static_cast<scalar_t>( start_value ), static_cast<scalar_t>( increment ) ) );
            BOOST_ASSERT( result[ Vector::static_size - 1 ] <= omega_scale * Impl::full_circle() / 4 );
            return result;
        }
    }; // struct twiddle_calculator_same_type

    struct radians : twiddle_calculator_same_type<radians>
    {
        static long_double_t full_circle() { return 2 * 3.1415926535897932384626433832795028841971693993751058209749445923078164062L; }
        template <typename Vector>
        static BOOST_FORCEINLINE BOOST_COLD Vector sincos( Vector const & input, Vector & cosine ) { return sinecosine<small_>( input, cosine ); }
    }; // struct radians

    struct degrees : twiddle_calculator_same_type<degrees>
    {
        static long_double_t full_circle() { return 2 * 180; }
        template <typename Vector>
        static BOOST_FORCEINLINE BOOST_COLD Vector sincos( Vector const & input, Vector & cosine ) { return sincosd( input, cosine ); }
    }; // struct degrees

    struct pies : twiddle_calculator_same_type<pies>
    {
        static long_double_t full_circle() { return 2 * 1; }
        template <typename Vector>
        static BOOST_FORCEINLINE BOOST_COLD Vector sincos( Vector const & input, Vector & cosine ) { return sincospi( input, cosine ); }
    }; // struct pies

    struct pies_scalar_upgraded_type : twiddle_calculator_scalar
    {
        static long_double_t full_circle() { return 2 * 1; }

        template <typename Vector>
        static BOOST_FORCEINLINE BOOST_COLD Vector sincos( input_t const & input, Vector & cosine )
        {
            typedef typename Vector::value_type scalar_t;

            long_double_t const omega_scale( input.omega_scale );
            long_double_t const N          ( input.N           );
            int           const index      ( input.index       );

            Vector sine;

            for ( int i( 0 ); i < Vector::static_size; ++i )
            {
                //...zzz...should only 'upgrade' float to double and double to long double...
                //...zzz...nt2 doesn't seem to support the long double data type...
                /*long*/ double const omega( ( index + i ) * omega_scale * full_circle() / N );
                /*long*/ double precise_sin;
                /*long*/ double precise_cos;
                nt2::sincospi( omega, precise_sin, precise_cos );

                sine  [ i ] = static_cast<scalar_t>( precise_sin );
                cosine[ i ] = static_cast<scalar_t>( precise_cos );
            }

            return sine;
        }
    }; // struct pies_scalar_upgraded_type

    struct hardware_or_crt : twiddle_calculator_scalar
    {
        static long_double_t full_circle() { return 2 * 3.1415926535897932384626433832795028841971693993751058209749445923078164062L; }

        template <typename Vector>
        static BOOST_FORCEINLINE BOOST_COLD Vector sincos( input_t const & input, Vector & cosine )
        {
            Vector sine;

        #if defined( _MSC_VER ) && defined( _M_IX86 )
            Vector::value_type * const p_sine  ( sine  .data() );
            Vector::value_type * const p_cosine( cosine.data() );

            // http://software.intel.com/en-us/forums/showthread.php?t=74354
            // http://www.devmaster.net/forums/showthread.php?t=5784

            unsigned int  const vector_size( Vector::static_size );
            long_double_t const omega_scale( input.omega_scale   );
            long_double_t const N          ( input.N             );
            unsigned int        local_index( input.index         );
            __asm
            {
                mov ecx, vector_size
                mov edx, p_sine
                mov edi, p_cosine
            sincos_loop:
                fldpi
                fldpi
                fadd
                //...zzz...direct version seems to work the same?
                //fld [omega_scale]
                //fmul
                //fild [local_index]
                //fmul
                //fld [N]
                //fdiv
                fmul  [omega_scale]
                fimul [local_index]
                fdiv  [N]
                fsincos
                fstp [edi]
                add edi, 4
                fstp [edx]
                add edx, 4

                inc [local_index]
                dec ecx
                jnz sincos_loop
            }

        #else // 32 bit x86 MSVC

            long_double_t const omega_scale( input.omega_scale );
            long_double_t const N          ( input.N           );
            int           const index      ( input.index       );

            for ( unsigned i( 0 ); i < Vector::static_size; ++i )
            {
                long_double_t const omega( ( index + i ) * omega_scale * full_circle() / N );

                sine  [ i ] = std::sin( omega );
                cosine[ i ] = std::cos( omega );
            }

        #endif // 32 bit x86 MSVC

            return sine;
        }
    }; // struct hardware_or_crt


#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4996 ) // '_controlfp': This function or variable may be unsafe.
#endif // _MSC_VER

    /// \note Regardless of the input data layout we always use twiddles
    /// interleaved on vector boundaries in order to improve memory locality and
    /// reduce the number of pointers that must be tracked to help architectures
    /// with a small general purpose register file (32 bit x86 is already
    /// maximally stretched out).
    ///                                       (05.06.2012.) (Domagoj Saric)
    template <typename Vector>
    BOOST_NOINLINE
    void BOOST_FASTCALL BOOST_COLD calculate_twiddles
    (
        /// \note Packing these parameters into static const structs (as they
        /// are all known at compile time) proved fruitless because MSVC10 would
        /// either create dynamic initialisers or initialise the objects in
        /// wrong order (since, per the standard, static initialisation order is
        /// completely undefined for templates).
        ///                                   (18.07.2012.) (Domagoj Saric)
        twiddle_pair<Vector> * BOOST_DISPATCH_RESTRICT const p_twiddles,
        unsigned int                                   const N_int,
        unsigned int                                   const stride,
        unsigned int                                   const omega_scale_int,
        unsigned int                                   const start_index
    )
    {
    #if defined( _MSC_VER ) && ( defined( _M_IX86 ) /*...mrmlj...assertion failure in the CRT?...|| defined( _M_AMD64 )*/ || defined( _M_IA64 ) )
        unsigned int const current_precision( ::_controlfp( _PC_64, _MCW_PC ) );
    #endif // _MSC_VER

        twiddle_pair<Vector> * BOOST_DISPATCH_RESTRICT p_w( p_twiddles );

        /// \todo Since cos( a ) = sin( a + Pi/2 ) = -sin( a + 3*Pi/2 ) separate
        /// cos/wr and sin/wi twiddle values could be avoided. The elements of
        /// the array would be half the size (no longer pairs) but there would
        /// have to be extra elements for the cosine offset: N/4 in the first
        /// case (if we store positive sines) or 3N/4 in the second case (if we
        /// store negative sines).
        ///                                   (02.03.2012.) (Domagoj Saric)

        /// \note N/4 values are required for split-radix.
        ///                                   (21.05.2012.) (Domagoj Saric)

        long_double_t const N          ( static_cast<int>( N_int           ) );
        long_double_t const omega_scale( static_cast<int>( omega_scale_int ) );

        unsigned       i        ( start_index             );
        unsigned const end_index( N_int / 4 + start_index );
        while ( i < end_index )
        {
            /// \note The various calculator implementations approximately rank
            /// in this order (from least to most precise):
            /// radians
            /// degrees
            /// hardware_or_crt
            /// pies
            /// pies_scalar_upgraded_type (this requires further investigation).
            ///                               (20.07.2012.) (Domagoj Saric)
            /// \note Clang 3.3 generates wrong code/results/factors with
            /// -ffast-math and emulation (tested with x86 and ARM CPUs).
            ///                               (06.11.2013.) (Domagoj Saric)
        #if defined( __FAST_MATH__ ) && !defined( BOOST_SIMD_DETECTED )
            typedef hardware_or_crt impl;
        #else
            typedef pies            impl;
        #endif // __FAST_MATH__

            p_w->wi = impl::sincos( impl::generate_input<Vector>( i, omega_scale, N ), p_w->wr ) ^ Mzero<Vector>();

            i   += Vector::static_size;
            p_w += stride;
        }

        BOOST_ASSERT( p_w == &p_twiddles[ N_int / 4 / Vector::static_size * stride ] );

    #if defined( _MSC_VER ) && ( defined( _M_IX86 ) /*...mrmlj...assertion failure in the CRT?...|| defined( _M_AMD64 )*/ || defined( _M_IA64 ) )
        ::_controlfp( current_precision, _MCW_PC );
    #endif // _MSC_VER
    }

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4503 ) // Mangled name truncated.
#endif // _MSC_VER

    /// \note A C++11/14 experiment on using variadic templates, index lists and
    /// constexpressions to statically generate the twiddle factor arrays.
    ///
    /// http://stackoverflow.com/questions/17424477/implementation-c14-make-integer-sequence
    /// http://stackoverflow.com/questions/25372805/how-exactly-is-stdmake-integer-sequence-implemented
    /// http://stackoverflow.com/questions/16387354/template-tuple-calling-a-function-on-each-element
    /// http://stackoverflow.com/questions/24110398/insert-a-transformed-integer-sequence-into-a-variadic-template-argument
    /// http://stackoverflow.com/questions/19016099/lookup-table-with-constexpr/19016627
    /// http://stackoverflow.com/questions/2978259/programmatically-create-static-arrays-at-compile-time-in-c/2981617#2981617
    /// http://stackoverflow.com/questions/6060103/c11-create-static-array-with-variadic-templates
    ///
    /// https://connect.microsoft.com/VisualStudio/feedback/details/813466/an-implementation-of-make-integer-sequence-results-to-an-internal-compiler-error-code-fine-on-clang
    /// https://connect.microsoft.com/VisualStudio/feedback/details/814000/ice-compiling-recursive-template-c-17-integer-sequence-implementation
    ///                                       (28.01.2015.) (Domagoj Saric)
    template <unsigned...> struct seq { using type = seq; };

    template <class S1, class S2> struct concat_impl;
    template <unsigned... I1, unsigned... I2>
    struct concat_impl<seq<I1...>, seq<I2...>> : seq<I1..., (sizeof...(I1)+I2)...> {};

    template <class S1, class S2>
    using concatenate = typename concat_impl<S1, S2>::type;

    template <unsigned N> struct gen_seq;
    template <unsigned N>
    struct gen_seq : concatenate<typename gen_seq<N/2>::type, typename gen_seq<N - N/2>::type> {};

    template <> struct gen_seq<0> : seq< > {};
    template <> struct gen_seq<1> : seq<0> {};

    template <typename Init, typename Vector, typename> struct array_aux;
    template <typename Init, typename Vector, boost::uint16_t... Indices>
    struct array_aux<Init, Vector, seq<Indices...>>
    {
        static boost::uint16_t const N = sizeof...( Indices );
        typedef
            boost::array
            <
                split_radix_twiddles<typename boost::simd::meta::compiler_vector<Vector>::type> const,
                N
            > factors_t;

        typedef BOOST_SIMD_ALIGNED_TYPE_ON( factors_t, 64 ) cache_aligned_factors_t;

        static cache_aligned_factors_t const factors;
    }; // struct array_aux

    template <typename Init, typename Vector, boost::uint16_t... Indices>
    typename array_aux<Init, Vector, seq<Indices...>>::cache_aligned_factors_t const
        array_aux<Init, Vector, seq<Indices...>>::factors = { { Init:: template value<Indices>()... } };

    /// \note MSVC12 explodes/dies a slow death if we try to use the
    /// static_(sine/cosine)() function templates (from static_sincos.hpp) in
    /// twiddle_calculator<>::value().
    ///                                       (28.01.2015.) (Domagoj Saric)
    /// \note GCC and Clang are supposed to be capable of compile-time
    /// evaluation of <cmath> functions:
    /// https://groups.google.com/a/isocpp.org/forum/#!topic/std-discussion/a7D2SYqUX-I
    /// MSVC is as usual "won't do it" on this topic
    /// https://connect.microsoft.com/VisualStudio/Feedback/Details/807275
    ///                                       (30.01.2015.) (Domagoj Saric)
    BOOST_FORCEINLINE long double BOOST_FASTCALL BOOST_CONSTEXPR negsin_aux( long double const x, long double const xx )
    {
        return
            -
            (
                x      *(1-xx/ 2/ 3*(1-xx
                 / 4/ 5*(1-xx/ 6/ 7*(1-xx
                 / 8/ 9*(1-xx/10/11*(1-xx
                 /12/13*(1-xx/14/15*
                (1-xx/16/17*
                (1-xx/18/19*(1-xx
                /20/21))))))))))
            );
    }
    BOOST_FORCEINLINE long double BOOST_FASTCALL BOOST_CONSTEXPR negsin_aux( long double const x                              ) { return negsin_aux( x, x * x  ); }
    BOOST_FORCEINLINE long double BOOST_FASTCALL BOOST_CONSTEXPR negsin    ( boost::uint16_t const i, long double const omega ) { return negsin_aux( i * omega ); }


    BOOST_FORCEINLINE long double BOOST_FASTCALL BOOST_CONSTEXPR cos_aux( long double const xx )
    {
        return
             1-xx    /2*(1-xx/ 3/ 4*
            (1-xx/ 5/ 6*(1-xx/ 7/ 8*
            (1-xx/ 9/10*(1-xx/11/12*
            (1-xx/13/14*(1-xx/15/16*
            (1-xx/17/18*(1-xx/19/20*
            (1-xx/21/22*(1-xx/23/24
            )))))))))));
    }
    BOOST_FORCEINLINE long double BOOST_FASTCALL BOOST_CONSTEXPR cos( boost::uint16_t const i, long double const omega ) { return cos_aux( i * i * omega * omega ); }

    template <boost::uint16_t N, typename Vector>
    struct twiddle_calculator
    {
        static BOOST_FORCEINLINE long double BOOST_FASTCALL BOOST_CONSTEXPR o() { return 2 * 3.1415926535897932384626433832795028841971693993751058209749445923078164062L / N; }

        template <boost::uint16_t index>
        static BOOST_FORCEINLINE split_radix_twiddles<typename boost::simd::meta::compiler_vector<Vector>::type> BOOST_CONSTEXPR BOOST_FASTCALL value()
        {
            boost::uint16_t BOOST_CONSTEXPR_OR_CONST i    ( index * boost::simd::meta::cardinal_of<Vector>::value );
            auto            BOOST_CONSTEXPR_OR_CONST omega( o()                                                   );
            return
            {
                { // wn
                    {    cos(   i + 0      , omega ),    cos(   i + 1      , omega ),    cos(   i + 2      , omega ),    cos(   i + 3      , omega ) },
                    { negsin(   i + 0      , omega ), negsin(   i + 1      , omega ), negsin(   i + 2      , omega ), negsin(   i + 3      , omega ) },
                },
                { // w3n
                    {    cos( ( i + 0 ) * 3, omega ),    cos( ( i + 1 ) * 3, omega ),    cos( ( i + 2 ) * 3, omega ),    cos( ( i + 3 ) * 3, omega ) },
                    { negsin( ( i + 0 ) * 3, omega ), negsin( ( i + 1 ) * 3, omega ), negsin( ( i + 2 ) * 3, omega ), negsin( ( i + 3 ) * 3, omega ) },
                }
            };
        }
    }; // twiddle_calculator

    template <unsigned N, typename Vector>
    struct static_twiddle_holder
        : array_aux<twiddle_calculator<N, Vector>, Vector, typename gen_seq<N / boost::simd::meta::cardinal_of<Vector>::value>::type>
    {
        static BOOST_SIMD_ALIGNED_TYPE_ON( split_radix_twiddles<Vector>, 64 ) const * factors()
        {
            return reinterpret_cast<split_radix_twiddles<Vector> const *>( static_twiddle_holder::array_aux::factors.begin() );
        }
    }; // static_twiddle_holder


    template <unsigned N, typename Vector>
    struct runtime_twiddle_holder
    {
        typedef typename boost::simd::meta::vector_of
        <
            typename boost::simd::meta::scalar_of  <Vector>::type,
                     boost::simd::meta::cardinal_of<Vector>::value
        >::type full_vector_t;

        struct BOOST_SIMD_ALIGN_ON( 64 ) cache_aligned_factors_t
        {
            BOOST_COLD cache_aligned_factors_t()
            {
                calculate_twiddles<full_vector_t>( reinterpret_cast<twiddle_pair<full_vector_t> *>( &factors.front().w0 ), N, 2, 1, 0 );
                calculate_twiddles<full_vector_t>( reinterpret_cast<twiddle_pair<full_vector_t> *>( &factors.front().w3 ), N, 2, 3, 0 );
            }

            typedef boost::array
                <
                    split_radix_twiddles<Vector> /*const*/,
                    N / 4 / full_vector_t::static_size
                >
                factors_t;

            factors_t factors;
        }; // cache_aligned_factors_t

        static BOOST_SIMD_ALIGNED_TYPE_ON( split_radix_twiddles<Vector>, 64 ) const * factors()
        {
            return storage.factors.begin();
        }

        static cache_aligned_factors_t const storage;
    }; // struct twiddle_holder

    template <unsigned N, typename Vector>
    typename runtime_twiddle_holder<N, Vector>::cache_aligned_factors_t const runtime_twiddle_holder<N, Vector>::storage;

    template <typename Vector, unsigned N>
    struct real_separation_twiddles_holder
    {
        BOOST_COLD real_separation_twiddles_holder()
        {
            calculate_twiddles<Vector>( &factors.front(), N, 1, 1, 1 );
        }

        typedef
            boost::array
            <
                twiddle_pair<Vector> /*const*/,
                N / 4 / Vector::static_size
            >
            factors_t;

        typedef BOOST_SIMD_ALIGNED_TYPE_ON( factors_t, 64 ) cache_aligned_factors_t;

        cache_aligned_factors_t /*const*/ factors;
    }; // struct real_separation_twiddles_holder
} // namespace detail

template <unsigned N, typename Vector>
using twiddles_interleaved = typename boost::mpl::if_c
      <
        ( N <= 64 ), // a heuristic measure against bloat, compile time and memory exhaustion...
        detail::static_twiddle_holder <N, Vector>,
        detail::runtime_twiddle_holder<N, Vector>
      >::type;


template <unsigned N, typename Vector>
struct real_separation_twiddles
{
public:
    static BOOST_SIMD_ALIGNED_TYPE_ON( twiddle_pair<Vector>, 64 ) const * factors() { return twiddles_.factors.begin(); }

private:
    typedef detail::real_separation_twiddles_holder<Vector, N> twiddle_holder;
    static twiddle_holder const twiddles_;
};

template <unsigned N, typename Vector>
BOOST_SIMD_ALIGN_ON( 64 )
typename real_separation_twiddles<N, Vector>::twiddle_holder const real_separation_twiddles<N, Vector>::twiddles_;

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

#if 0
//...zzz...the below approaches are still radix-2 specific and use old/ancient
//...zzz...approaches with strides and separate arrays for real and imaginary
//...zzz...parts (so that they don't need to be parameterized with the SIMD data
//...zzz...vector size)...

////////////////////////////////////////////////////////////////////////////////
// Recursive compile-time initialisation
//  + no dynamic initialisers ((hopefully, depending on the compiler)
//  - very compile-time heavyweight (MSVC10 seems to have a template recursion
//    level limit of ~512 so it cannot compile this for FFT sizes >= 2048 even
//    with the recursion "unrolled" into packs of four values)
//  - questionable whether the resulting "array" would still get placed in the
//    text section considering that individual twiddles are not PODs but have
//    default constructors
////////////////////////////////////////////////////////////////////////////////

namespace detail
{
    template
    <
        template <unsigned N, unsigned I> class SinCos,
        unsigned I,
        unsigned N,
        unsigned stride,
        typename T
    >
    struct twiddle
        :
        twiddle<SinCos, I - ( stride * 4 ), N, stride, T>
    {
        twiddle()
            :
            value0( SinCos<N, I - ( stride * 3 )>::value ),
            value1( SinCos<N, I - ( stride * 2 )>::value ),
            value2( SinCos<N, I - ( stride * 1 )>::value ),
            value3( SinCos<N, I - ( stride * 0 )>::value )
        {}

        T const value0;
        T const value1;
        T const value2;
        T const value3;
    };

    template <template <unsigned N, unsigned I> class SinCos, unsigned N, unsigned stride, typename T>
    struct twiddle<SinCos, 0, N, stride, T> {};
} // namespace detail

template <unsigned N, unsigned Stride, typename T>
struct real_twiddles
{
    static float const * factors() { return factors_.begin(); }

    typedef BOOST_SIMD_ALIGN_ON( BOOST_SIMD_CONFIG_ALIGNMENT ) detail::twiddle<static_cosine, N, N, Stride, T> factors_t;
    static factors_t const factors_;
};

template <unsigned N, unsigned Stride, typename T>
typename real_twiddles<N, Stride, T>::factors_t const real_twiddles<N, Stride, T>::factors_;

template <unsigned N, unsigned Stride, typename T>
struct imag_twiddles
{
    static float const * factors() { return factors_.begin(); }

    typedef BOOST_SIMD_ALIGN_ON( BOOST_SIMD_CONFIG_ALIGNMENT ) detail::twiddle<static_sine, N, N, Stride, T> factors_t;
    static factors_t const factors_;
};

template <unsigned N, unsigned Stride, typename T>
typename imag_twiddles<N, Stride, T>::factors_t const imag_twiddles<N, Stride, T>::factors_;


////////////////////////////////////////////////////////////////////////////////
// Preprocessor + compile-time initialisation
//  + fully static
//  + bearable compile-time hit
//  - too heavy on the preprocessor (requires a smarter rewrite to workaround
//    Boost.Preprocessor and compiler limits)
//  - requires in advance definition of explicit specializations for all sizes
//    that will be used in a particular application
////////////////////////////////////////////////////////////////////////////////


#define NT2_AUX_REAL_TWIDDLE( z, i, context )   static_cosine<BOOST_PP_TUPLE_ELEM( 3, 0, context ), i * BOOST_PP_TUPLE_ELEM( 3, 2, context )>::value,
#define NT2_AUX_IMAG_TWIDDLE( z, i, context ) - static_sine  <BOOST_PP_TUPLE_ELEM( 3, 0, context ), i * BOOST_PP_TUPLE_ELEM( 3, 2, context )>::value,

#define NT2_AUX_TWIDDLE( z, i, context )                                                                                                                                                             \
{                                                                                                                                                                                                    \
    { BOOST_PP_REPEAT_FROM_TO( BOOST_PP_MUL( i, BOOST_PP_TUPLE_ELEM( 3, 1, context ) ), BOOST_PP_MUL( BOOST_PP_INC( i ), BOOST_PP_TUPLE_ELEM( 3, 1, context ) ), NT2_AUX_REAL_TWIDDLE, context ) },  \
    { BOOST_PP_REPEAT_FROM_TO( BOOST_PP_MUL( i, BOOST_PP_TUPLE_ELEM( 3, 1, context ) ), BOOST_PP_MUL( BOOST_PP_INC( i ), BOOST_PP_TUPLE_ELEM( 3, 1, context ) ), NT2_AUX_IMAG_TWIDDLE, context ) },  \
},

#define NT2_AUX_TWIDDLES( fft_size, stride )                                               \
template <typename T>                                                                      \
struct twiddles<fft_size, stride, T>                                                       \
{                                                                                          \
    static std::size_t const data_vector_size = twiddle_pair<T>::vector_t::static_size;    \
    typedef BOOST_SIMD_ALIGN_ON( BOOST_SIMD_CONFIG_ALIGNMENT )                             \
    boost::array                                                                           \
    <                                                                                      \
        twiddle_pair<T> const,                                                             \
        fft_size / stride / data_vector_size                                               \
    >                                                                                      \
    factors_t;                                                                             \
    static factors_t const factors;                                                        \
};                                                                                         \
                                                                                           \
template <typename T>                                                                      \
typename twiddles<fft_size, stride, T>::factors_t const                                    \
         twiddles<fft_size, stride, T>::factors =                                          \
{{                                                                                         \
    BOOST_PP_REPEAT                                                                        \
    (                                                                                      \
        BOOST_PP_DIV( BOOST_PP_DIV( fft_size, stride ), data_vector_size ),                \
        NT2_AUX_TWIDDLE,                                                                   \
        ( fft_size, data_vector_size, stride )                                             \
    )                                                                                      \
}}

/* Desired output for for fft_size = 8, data_vector_size = 4, stride = 1:
 *   template
 *   <
 *       unsigned fft_size,
 *       unsigned data_vector_size,
 *       unsigned stride,
 *       typename T
 *   >
 *   typename twiddles<fft_size, data_vector_size, stride, T>::factors_t const
 *            twiddles<fft_size, data_vector_size, stride, T>::factors =
 *   {{
 *       {
 *           {   static_cosine<fft_size, 0 * stride>::value,   static_cosine<fft_size, 1 * stride>::value,   static_cosine<fft_size, 2 * stride>::value,   static_cosine<fft_size, 3 * stride>::value, },
 *           { - static_sine  <fft_size, 0 * stride>::value, - static_sine  <fft_size, 1 * stride>::value, - static_sine  <fft_size, 2 * stride>::value, - static_sine  <fft_size, 3 * stride>::value, },
 *       },
 *       {
 *           {   static_cosine<fft_size, 4 * stride>::value,   static_cosine<fft_size, 5 * stride>::value,   static_cosine<fft_size, 6 * stride>::value,   static_cosine<fft_size, 7 * stride>::value, },
 *           { - static_sine  <fft_size, 4 * stride>::value, - static_sine  <fft_size, 5 * stride>::value, - static_sine  <fft_size, 6 * stride>::value, - static_sine  <fft_size, 7 * stride>::value, },
 *       },
 *   }};
 */

// Broken (breaches Boost.Preprocessor and compiler limits for FFT sizes 256+)...
/*
    NT2_AUX_TWIDDLES(  128, 4, 1 );
    NT2_AUX_TWIDDLES(  128, 4, 2 );
    NT2_AUX_TWIDDLES(  256, 4, 1 );
    NT2_AUX_TWIDDLES(  256, 4, 2 );
    NT2_AUX_TWIDDLES(  512, 4, 1 );
    NT2_AUX_TWIDDLES(  512, 4, 2 );
    NT2_AUX_TWIDDLES( 1024, 4, 1 );
    NT2_AUX_TWIDDLES( 1024, 4, 2 );
    NT2_AUX_TWIDDLES( 2048, 4, 1 );
    NT2_AUX_TWIDDLES( 2048, 4, 2 );
¸*/
#endif // disabled

//------------------------------------------------------------------------------
} // namespace nt2
//------------------------------------------------------------------------------
#endif // twiddle_factors_hpp
