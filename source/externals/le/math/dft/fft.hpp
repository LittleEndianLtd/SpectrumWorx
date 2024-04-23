////////////////////////////////////////////////////////////////////////////////
///
/// \file fft.hpp
/// -------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef fft_hpp__3EFDE32C_A81E_4A2B_9BDB_252EC2DD74ED
#define fft_hpp__3EFDE32C_A81E_4A2B_9BDB_252EC2DD74ED
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/engine/buffers.hpp"

#if defined( __APPLE__ )
    #define LE_ACC_FFT
#else
    //#define LE_PURE_REAL_FFT_TEST
    //#define LE_SORENSEN_PURE_REAL_FFT_TEST
#endif

#ifdef LE_ACC_FFT
    typedef struct OpaqueFFTSetup * FFTSetup   ;
    typedef unsigned long           vDSP_Length;
    struct  DSPSplitComplex;
#endif // LE_ACC_FFT

#include "boost/range/iterator_range_core.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

//...mrmlj...cleanup these duplicated typedefs (also in effects.hpp)...
typedef boost::iterator_range<float       * LE_RESTRICT>         DataRange;
typedef boost::iterator_range<float const * LE_RESTRICT> ReadOnlyDataRange;

////////////////////////////////////////////////////////////////////////////////
///
/// \class FFT_float_real_1D
///
/// \brief Performs single precision 1D forward and inverse FFT on real data.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   Check revision 5921 for the last version containing the ACML based
// implementation. Other alternative implementations can be found in the SVN
// revision 746 of this module.
//                                            (24.02.2012.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

class FFT_float_real_1D
{
public:
     FFT_float_real_1D();
#ifdef LE_ACC_FFT
    ~FFT_float_real_1D();
#endif // __APPLE__

    // real
    LE_NOTHROW void transform       ( float * data /*inplace: in time     , out DFT reals*/, DataRange         const & imaginaryTargetSubRange, bool doFFTShift ) const;
    LE_NOTHROW void inverseTransform( float * data /*inplace: in DFT reals, out time     */, ReadOnlyDataRange const & imaginarySourceSubRange, bool doFFTShift ) const;

    LE_NOTHROW void transform       ( float * data /*inplace: in time     , out DFT reals*/, float       * imaginaryTargetSubRange, std::uint16_t size ) const;
    LE_NOTHROW void inverseTransform( float * data /*inplace: in DFT reals, out time     */, float const * imaginarySourceSubRange, std::uint16_t size ) const;

    // complex
    LE_NOTHROW void transform       ( float * pReals, float * pImags ) const;
    LE_NOTHROW void inverseTransform( float * pReals, float * pImags ) const;

#ifdef LE_PURE_REAL_FFT_TEST
    LE_NOTHROW void transform       ( float const * pTimeDomainData, float const * pWindow, float       * pReals,         DataRange const & imags ) const;
    LE_NOTHROW void inverseTransform( float       * pTimeDomainData,                        float const * pReals, ReadOnlyDataRange const & imags ) const;
    LE_NOTHROW void inverseTransform( DataRange const & reals, DataRange const & imags ) const;
#endif // LE_PURE_REAL_FFT_TEST

    LE_NOTHROW void resize( SW::Engine::StorageFactors const & factors, SW::Engine::Storage & );

    std::uint16_t size() const { return size_; } //...mrmlj...actually "maximum allowed size"...

    static float maximumAmplitude( float size );

private:
    #if defined( LE_ACC_FFT )
        ::DSPSplitComplex & workBufferSplit() const;
    #endif // LE_ACC_FFT

    void fftshift( float * pTimeDomainData ) const;

private:
    std::uint16_t size_;

#if defined( LE_ACC_FFT )
    FFTSetup fftSetup_;

    struct DSPSplitComplex
    {
        float * realp;
        float * imagp;
    };
    DSPSplitComplex workBufferSplit_;
    typedef SW::Engine::DoubleFFTBuffer<> WorkBuffer;
#else // NT2
    typedef
        SW::Engine::SharedStorageFFTBasedBuffer
        <
            SW::Engine::real_t, 1, 1, Utility::Constants::vectorAlignment / sizeof( SW::Engine::real_t )
        > WorkBuffer;
#endif // FFT implementation

    mutable WorkBuffer workBuffer_;

public:
    static LE_CONST_FUNCTION std::uint32_t requiredStorage( SW::Engine::StorageFactors const & );
}; // class FFT_float_real_1D

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // fft_hpp
