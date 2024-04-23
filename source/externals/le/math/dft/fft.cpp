////////////////////////////////////////////////////////////////////////////////
///
/// fft.cpp
/// -------
///
/// Copyright (r) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------

#ifdef _MSC_VER
    #pragma runtime_checks( "", off )
    #pragma check_stack   (     off )
#endif // MSVC

#include "le/utility/platformSpecifics.hpp"

LE_OPTIMIZE_FOR_SPEED_BEGIN()
LE_FAST_MATH_ON()

#include "fft.hpp"

#ifdef LE_ACC_FFT
    #include <cmath>
#endif // LE_ACC_FFT
#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/utility/buffers.hpp"

#include <boost/assert.hpp>

// Implementation specific includes.
#ifdef LE_ACC_FFT
    #define Point CarbonDummyPointName // (workaround to avoid definition of "Point" by old Carbon headers)
    #include <Accelerate/Accelerate.h> //vDSP.h
    #undef Point
#else
    //#define LE_SORENSEN_PURE_REAL_FFT_TEST
    #if defined( LE_SORENSEN_PURE_REAL_FFT_TEST ) && defined( LE_PURE_REAL_FFT_TEST )
        #error Cannot test Sorensen and our own internal pure real fft at the same time...
    #endif
#if defined(  __APPLE__ )
    #undef nil
#endif // __APPLE__
    #include "nt2/signal/static_fft.hpp"
#endif // LE_ACC_FFT
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// FFT_float_real_1D::FFT_float_real_1D()
// --------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

FFT_float_real_1D::FFT_float_real_1D() /// \throws nothing
    :
    size_( 0 )
#ifdef LE_ACC_FFT
    ,fftSetup_( 0 )
#endif // LE_ACC_FFT
{
}


////////////////////////////////////////////////////////////////////////////////
//
// FFT_float_real_1D::~FFT_float_real_1D()
// ---------------------------------------
//
////////////////////////////////////////////////////////////////////////////////

#ifdef LE_ACC_FFT
FFT_float_real_1D::~FFT_float_real_1D() /// \throws nothing
{
    if ( fftSetup_ )
        vDSP_destroy_fftsetup( fftSetup_ );
}
#endif // LE_ACC_FFT

#ifdef LE_ACC_FFT
::DSPSplitComplex & FFT_float_real_1D::workBufferSplit() const
{
    static_assert( sizeof  ( ::DSPSplitComplex        ) == sizeof  ( FFT_float_real_1D::DSPSplitComplex        ), "Internal inconsistency" );
    static_assert( offsetof( ::DSPSplitComplex, realp ) == offsetof( FFT_float_real_1D::DSPSplitComplex, realp ), "Internal inconsistency" );
    static_assert( offsetof( ::DSPSplitComplex, imagp ) == offsetof( FFT_float_real_1D::DSPSplitComplex, imagp ), "Internal inconsistency" );
    return const_cast< ::DSPSplitComplex &>( reinterpret_cast< ::DSPSplitComplex const &>( workBufferSplit_ ) );
}
#endif // LE_ACC_FFT


////////////////////////////////////////////////////////////////////////////////
//
// FFT_float_real_1D::resize()
// ---------------------------
//
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW
void FFT_float_real_1D::resize( SW::Engine::StorageFactors const & factors, SW::Engine::Storage & storage ) /// \throws nothing
{
    auto const size( factors.fftSize );

    BOOST_ASSERT_MSG( size <= LE::SW::Engine::Constants::maximumFFTSize, "FFT size too large." );

    /// \note The work buffer has to be resized (relocated) even if the FFT size
    /// hasn't changed because shared storage might have been reallocated.
    ///                                       (22.02.2013.) (Domagoj Saric)
    workBuffer_.resize( factors, storage );

#if defined( LE_ACC_FFT )
    workBufferSplit_.realp = workBuffer_.begin()                             ;
    workBufferSplit_.imagp = workBuffer_.begin() + ( workBuffer_.size() / 2 );
    BOOST_ASSERT_MSG( ( reinterpret_cast<std::size_t>( workBufferSplit_.realp ) % Utility::Constants::vectorAlignment ) == 0, "Buffer misaligned." );
    BOOST_ASSERT_MSG( ( reinterpret_cast<std::size_t>( workBufferSplit_.imagp ) % Utility::Constants::vectorAlignment ) == 0, "Buffer misaligned." );

    if ( size != size_ )
    {
        unsigned int const newLog2Size( log2( size )                                      );
        FFTSetup     const newFFTSetup( ::vDSP_create_fftsetup( newLog2Size, kFFTRadix2 ) );
        if ( newFFTSetup )
        {
            if ( fftSetup_ )
                ::vDSP_destroy_fftsetup( fftSetup_ );
            fftSetup_ = newFFTSetup;
        }
        else { BOOST_ASSERT( !"FFT failure" ); /*...mrmlj...proper error handling...*/ }
    }
#endif // LE_ACC_FFT

    size_ = size;
}


////////////////////////////////////////////////////////////////////////////////
/// Real DFT
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW
void FFT_float_real_1D::transform( float * LE_RESTRICT const data /*in time, out DFT reals*/, float * LE_RESTRICT const imaginaryTargetSubRange, std::uint16_t const size ) const
{
    BOOST_ASSERT( size <= this->size() );
#if defined( LE_ACC_FFT ) || defined( LE_SORENSEN_PURE_REAL_FFT_TEST )
    std::uint16_t const halfSize( size / 2 );
    vDSP_ctoz    ( reinterpret_cast<DSPComplex const *>( data ), 2, &workBufferSplit(), 1, halfSize );
    vDSP_fft_zrip( fftSetup_, &workBufferSplit(), 1, log2( size ), FFT_FORWARD ); // https://developer.apple.com/library/mac/documentation/Accelerate/Reference/vDSPRef/Reference/reference.html#//apple_ref/c/func/vDSP_fft_zript
    // http://developer.apple.com/library/ios/documentation/Performance/Conceptual/vDSP_Programming_Guide/UsingFourierTransforms/UsingFourierTransforms.html#//apple_ref/doc/uid/TP40005147-CH202-15411
    float const scale( 1 / ( 2 * std::sqrt( convert<float>( size ) ) ) );
    // http://developer.apple.com/library/ios/documentation/Performance/Conceptual/vDSP_Programming_Guide/UsingFourierTransforms/UsingFourierTransforms.html#//apple_ref/doc/uid/TP40005147-CH202-15398
    multiply( workBufferSplit().realp    , scale, data                       , halfSize     );
    multiply( workBufferSplit().imagp + 1, scale, imaginaryTargetSubRange + 1, halfSize - 1 );
    data                   [ halfSize ] = workBufferSplit().imagp[ 0 ] * scale;
    imaginaryTargetSubRange[ 0        ] = 0;
    imaginaryTargetSubRange[ halfSize ] = 0;
#else
    float const scale( std::sqrt( nt2::real_fft_normalization_factor<float>( size ) ) );
    multiply( data, scale, workBuffer_.begin(), size );
    nt2::static_fft<128, 8192, float>::real_forward_transform( workBuffer_.begin(), data, imaginaryTargetSubRange, size );
#endif // LE_ACC_FFT
}


LE_NOTHROW
void FFT_float_real_1D::inverseTransform( float * LE_RESTRICT const data /*in DFT reals, out time*/, float const * LE_RESTRICT const imaginarySourceSubRange, std::uint16_t const size ) const
{
    BOOST_ASSERT( size <= this->size() );
#if defined( LE_ACC_FFT ) || defined( LE_SORENSEN_PURE_REAL_FFT_TEST )
    std::uint16_t const halfSize( size / 2 );
    // http://developer.apple.com/library/ios/documentation/Performance/Conceptual/vDSP_Programming_Guide/UsingFourierTransforms/UsingFourierTransforms.html#//apple_ref/doc/uid/TP40005147-CH202-15411
    float const scale( 1 / std::sqrt( convert<float>( size ) ) );
    // http://developer.apple.com/library/ios/documentation/Performance/Conceptual/vDSP_Programming_Guide/UsingFourierTransforms/UsingFourierTransforms.html#//apple_ref/doc/uid/TP40005147-CH202-15398
    multiply( data                       , scale, workBufferSplit().realp    , halfSize     );
    multiply( imaginarySourceSubRange + 1, scale, workBufferSplit().imagp + 1, halfSize - 1 );
    workBufferSplit().imagp[ 0 ] = data[ halfSize ] * scale;
    vDSP_fft_zrip( fftSetup_, &workBufferSplit(), 1, log2( size ), FFT_INVERSE );
    vDSP_ztoc    ( &workBufferSplit(), 1, reinterpret_cast<DSPComplex *>( data ), 2, halfSize );
#else
    nt2::static_fft<128, 8192, float>::real_inverse_transform( data, const_cast<float *>( imaginarySourceSubRange ), workBuffer_.begin(), size );
    float const scale( std::sqrt( nt2::real_fft_normalization_factor<float>( size ) ) );
    multiply( workBuffer_.begin(), scale, data, size );
#endif // LE_ACC_FFT
}

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4389 ) // Signed/unsigned mismatch
#endif // _MSC_VER
LE_NOTHROW
void FFT_float_real_1D::transform( float * const timeDomainData, DataRange const & imaginarySubRange, bool const doFFTShift ) const
{
    if ( doFFTShift )
        fftshift( timeDomainData );
    BOOST_ASSERT( imaginarySubRange.size() == ( size() / 2 ) + 1 );
    transform( timeDomainData, imaginarySubRange.begin(), size() );
}

LE_NOTHROW
void FFT_float_real_1D::inverseTransform( float * const dftData, ReadOnlyDataRange const & imaginarySubRange, bool const doFFTShift ) const
{
    BOOST_ASSERT( imaginarySubRange.size() == ( size() / 2 ) + 1 );
    inverseTransform( dftData, imaginarySubRange.begin(), size() );
    if ( doFFTShift )
        fftshift( dftData );
}
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER


////////////////////////////////////////////////////////////////////////////////
/// Complex DFT
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW
void FFT_float_real_1D::transform( float * LE_RESTRICT const pReals, float * LE_RESTRICT const pImags ) const
{
#if defined( LE_ACC_FFT )
    BOOST_ASSERT( !"Not implemented!" );
#else // NT2
    std::uint16_t const halfSize( size() / 2 );
    nt2::static_fft<128 / 2, 8192 / 2, float>::forward_transform( pReals, pImags, halfSize );
    float const scale( std::sqrt( 2 / convert<float>( halfSize ) / 4 /*mrmlj?*/ ) );
    multiply( pReals, scale, halfSize );
    multiply( pImags, scale, halfSize );
#endif // LE_ACC_FFT
}

LE_NOTHROW
void FFT_float_real_1D::inverseTransform( float * LE_RESTRICT const pReals, float * LE_RESTRICT const pImags ) const
{
#if defined( LE_ACC_FFT )
    BOOST_ASSERT( !"Not implemented!" );
#else
    std::uint16_t const halfSize( size() / 2 );
    nt2::static_fft<128 / 2, 8192 / 2, float>::inverse_transform( pReals, pImags, halfSize );
    float const scale( std::sqrt( 2 / convert<float>( halfSize ) / 4 /*mrmlj?*/ ) );
    multiply( pReals, scale, halfSize );
    multiply( pImags, scale, halfSize );
#endif // LE_ACC_FFT
}


#ifdef LE_PURE_REAL_FFT_TEST
namespace
{
    void lock( void const * const address, std::size_t const /*size*/ )
    {
    #ifdef _WIN32
        DWORD old_protection;
        BOOST_VERIFY( ::VirtualProtect( const_cast<void *>( address ), 4/*size * sizeof( float )*/, PAGE_READONLY, &old_protection ) );
        BOOST_ASSERT( old_protection == PAGE_READWRITE );
    #endif // _WIN32
    }

    void unlock( void const * const address, std::size_t const /*size*/ )
    {
    #ifdef _WIN32
        DWORD old_protection;
        BOOST_VERIFY( ::VirtualProtect( const_cast<void *>( address ), 4/*size * sizeof( float )*/, PAGE_READWRITE, &old_protection ) );
        BOOST_ASSERT( old_protection == PAGE_READONLY );
    #endif // _WIN32
    }
}

LE_NOTHROW
void FFT_float_real_1D::transform
(
    float const * const pTimeDomainData,
    float const * const pWindow,
    float       * const pReals,
    DataRange const & imags
) const
{
    workBuffer_.clear();
    clear( imags );
    clear( pReals, imags.size() );

    lock( pTimeDomainData, size() );
    lock( pWindow        , size() );
    lock( &workBuffer_[ size() + 1024 ], 256 );

    multiply( pTimeDomainData, pWindow, &workBuffer_[ 0 ], size() );

    nt2::static_fft<128, 8192, float>::real_forward_transform
    (
        &workBuffer_[ 0 ],
        pReals,
        imags.begin(),
        size()
    );

    //move( &imags[ 0 ], &imags[ 1 ], imags.size() - 1 );
    //imags.front() = 0;
    //imags.back () = 0;

    //pTimeDomainData; pReals; imags; pWindow;

    unlock( pTimeDomainData, size() );
    unlock( pWindow        , size() );
    unlock( &workBuffer_[ size() + 1024 ], 256 );
}


LE_NOTHROW
void FFT_float_real_1D::inverseTransform
(
    float       * const pTimeDomainData,
    float const * pReals,
    ReadOnlyDataRange const & imags
) const
{
    nt2::static_fft<128, 8192, float>::real_inverse_transform
    (
        pReals,
        imags.begin(),
        pTimeDomainData,
        size()
    );
}

LE_NOTHROW void FFT_float_real_1D::inverseTransform
(
    DataRange const & reals,
    DataRange const & imags
) const
{
    workBuffer_.clear();
    float * pReals( &workBuffer_[ 0 ] );
    float * pImags( &workBuffer_[ size() ] );
    copy( reals.begin(), pReals, reals.size() );
    copy( imags.begin(), pImags, imags.size() );
    clear( reals );
    clear( imags );
    nt2::static_fft<128, 8192, float>::real_inverse_transform
    (
        pReals,
        pImags,
        reals.begin(),
        size()
    );
    float const scale( 2.0f / size() );
    multiply( reals.begin(), scale, size() );
}
#endif // LE_PURE_REAL_FFT_TEST


void FFT_float_real_1D::fftshift( float * const pTimeDomainData ) const
{
    /// \note
    ///   Emulate the situation where the zero-th sample is in the middle of the
    /// window. In current SW framework, windows are of the length N and causal,
    /// centered around the bin N/2. If the input sample contains only one
    /// sinusoid, its DFT will be equal to the window DFT shifted to the
    /// sinusoid frequency. If the window is symmetric around the zero-th bin,
    /// then it has zero phase and all the bins affected by this sinusoid will
    /// have the same phase. But if (as in SW) the window is symmetric around
    /// the bin N/2 then the phase of the k-th bin of window DFT is actually
    /// 2*Pi/N * (N/2) * k = k*Pi. This effect is also called "residual phase
    /// rotation" (Handbook of Digital Signal Processing, Elliot, 1987, page
    /// 658).
    ///   For effects sensitive to phase relations between neighbouring bins
    /// (e.g. the phase vocoder and the issue of vertical coherence) it is
    /// advisable that the phase behaves more smoothly, so we compensate this
    /// phase shift with the equivalent of Matlab's fftshift function.
    ///   Note that the window actually has two axes of symmetry, so this may
    /// be less important in the non-zero-padded-case, but it is advised in the
    /// cited paper (Bernardini.pdf).
    ///                                        (21.05.2010.) (Ivan Dokmanic)

    /// \note
    /// It is not clear what should be the center of rotation, the N/2 or N/2+1
    /// bin. According to most, but not all, posts/documents it should be N/2
    /// (i.e. a simple swap of the first and second halves of the signal),
    /// contrary to the centre point for "DFT-even" windows (see the related
    /// note in the calculateWindow() function). Matlab's one-based indexing
    /// further obscures the solution. The aubio and Rubber Band libraries
    /// simply swap the two halves of the signal and this is the approach that
    /// we use.
    /// Additional info on the issue:
    /// http://www.ece.uvic.ca/~peterd/48409/Bernardini.pdf (3.3 FFT centering)
    /// http://www.mathworks.com/matlabcentral/fileexchange/25473-why-use-fftshiftfftfftshiftx-in-matlab-instead-of-fftx
    /// (click Download All in the above page)
    /// http://www.dsprelated.com/dspbooks/sasp/Zero_Phase_Zero_Padding.html
    /// http://www.dsprelated.com/showmessage/41851/1.php
    /// http://www.dsprelated.com/showmessage/122711/1.php
    /// http://www.katjaas.nl/FFToutput/centeredFFT.html
    /// http://www.groupsrv.com/computers/about656453.html
    /// http://groups.google.com/group/comp.dsp/browse_thread/thread/5a2f53f28d12496a
    ///                                       (17.04.2012.) (Domagoj Saric)

    /// \todo This (fftshift) pass does not seem to be necessary when using the
    /// window presum/time aliasing technique (with sinc-ed windows).
    /// Research this more thoroughly...
    ///                                       (24.04.2012.) (Domagoj Saric)

    BOOST_ASSERT_MSG( size() % 2 == 0, "Even data size expected." );
    float * const pHalfPoint( pTimeDomainData + size() / 2 );
    swap( pTimeDomainData, pHalfPoint, pHalfPoint );
}


////////////////////////////////////////////////////////////////////////////////
//
// FFT_float_real_1D::updateMaximumAmplitude()
// -------------------------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// Calculates the maximum amplitude value (absolute amplitude at 0db)
/// corresponding to the current FFT size.
///
////////////////////////////////////////////////////////////////////////////////

float FFT_float_real_1D::maximumAmplitude( float const size )
{
    return std::sqrt( size ) / 2;
}

LE_COLD LE_CONST_FUNCTION
std::uint32_t FFT_float_real_1D::requiredStorage( SW::Engine::StorageFactors const & factors )
{
    return WorkBuffer::requiredStorage( factors );
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SPEED_END()
