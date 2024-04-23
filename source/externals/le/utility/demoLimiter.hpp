////////////////////////////////////////////////////////////////////////////////
///
/// \file demoLimiter.hpp
/// ---------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef demoLimiter_hpp__83A7ED06_BA2D_470E_AB8F_0B88C6AAA69F
#define demoLimiter_hpp__83A7ED06_BA2D_470E_AB8F_0B88C6AAA69F
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include "boost/assert.hpp"

#include <cstddef>
#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

/// \note Cryptic symbol names to hinder hacking of static lib builds.
///                                           (22.01.2014.) (Domagoj Saric)
#define LE_DEMO_LIMITER AMjernik
#define LE_DEMO_CRIPPLE( ... ) ripple( __VA_ARGS__ )

template
<
    std::uint32_t crippledDuration_ms,
    std::uint64_t uncrippledDuration_ms,
    std::uint64_t initialUncrippledDuration_ms = uncrippledDuration_ms
>
class LE_DEMO_LIMITER
{
public:
    LE_DEMO_LIMITER()
    {
        reset();
        setup( 44100, 1 );
    }

    void setup( std::uint32_t const sampleRate, std::uint8_t const numberOfChannels )
    {
        numberOfInitialUncrippledSamples_ = initialUncrippledDuration_ms * sampleRate / 1000;
        numberOfUncrippledSamples_        = uncrippledDuration_ms        * sampleRate / 1000;
        numberOfCrippledSamples_          = crippledDuration_ms          * sampleRate / 1000;

        numberOfChannels_ = numberOfChannels;
    }

    void reset()
    {
        nonCrippledSamples_ = numberOfUncrippledSamples_ - numberOfInitialUncrippledSamples_;
        crippledSamples_    = 0;
    }

    bool inCrippledPhase() const { return nonCrippledSamples_ >= numberOfUncrippledSamples_; }

    typedef float * LE_RESTRICT const * LE_RESTRICT SeparatedData  ;
    typedef float * LE_RESTRICT                     InterleavedData;

    void LE_FASTCALL LE_DEMO_CRIPPLE( SeparatedData   const pSeparatedOutputs  , unsigned int const samples ) const { LE_DEMO_CRIPPLE( pSeparatedOutputs, nullptr            , samples ); }
    void LE_FASTCALL LE_DEMO_CRIPPLE( InterleavedData const pInterleavedOutputs, unsigned int const samples ) const { LE_DEMO_CRIPPLE( nullptr          , pInterleavedOutputs, samples ); }

private:
    static bool const periodic = ( crippledDuration_ms != 0 );

    void LE_FASTCALL LE_DEMO_CRIPPLE
    (
        SeparatedData   pSeparatedOutputs,
        InterleavedData pInterleavedOutputs,
        std::uint32_t   samples
    ) const;

private:
    mutable std::uint32_t nonCrippledSamples_;
    mutable std::uint32_t crippledSamples_   ;

    std::uint32_t numberOfCrippledSamples_         ;
    std::uint32_t numberOfUncrippledSamples_       ;
    std::uint32_t numberOfInitialUncrippledSamples_;

    std::uint8_t numberOfChannels_;
}; // class LE_DEMO_LIMITER


#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.

template
<
    std::uint32_t crippledDuration_ms,
    std::uint64_t uncrippledDuration_ms,
    std::uint64_t initialUncrippledDuration_ms
>
void LE_HOT LE_FASTCALL LE_DEMO_LIMITER<crippledDuration_ms, uncrippledDuration_ms, initialUncrippledDuration_ms>::LE_DEMO_CRIPPLE
(
    SeparatedData   const pSeparatedOutputs,
    InterleavedData const pInterleavedOutputs,
    std::uint32_t   const samples
) const
{
    BOOST_ASSERT_MSG( ( !pSeparatedOutputs != !pInterleavedOutputs ) || ( !pSeparatedOutputs && !pInterleavedOutputs ), "Either interleaved or separated or none" );

    auto const nonCrippledDurationInSamples( numberOfUncrippledSamples_ );
    auto const crippledDurationInSamples   ( numberOfCrippledSamples_   );
    auto       nonCrippledSamples          ( nonCrippledSamples_        );
    auto       crippledSamples             ( crippledSamples_           );

    auto const numberOfChannels( numberOfChannels_ );
    LE_ASSUME( numberOfChannels <= 8 );
    LE_ASSUME( numberOfChannels >  0 );

    std::uint32_t sample( 0 );
    while ( sample < samples )
    {
        // skip non crippled samples (if any):
        while
        (
            ( nonCrippledSamples < nonCrippledDurationInSamples ) &&
            ( sample             < samples                      )
        )
        {
            ++nonCrippledSamples;
            ++sample;
        }
        // zero crippled samples (if any):
    #if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
        auto const lastNonZeroedSample( sample );
    #endif // !NDEBUG && !LE_PUBLIC_BUILD
        while
        (
            ( crippledSamples < crippledDurationInSamples || !periodic ) &&
            ( sample          < samples                                )
        )
        {
            if ( pSeparatedOutputs )
            {
                for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
                {
                    pSeparatedOutputs[ channel ][ sample ] = 0;
                }
            }
            else
            if ( pInterleavedOutputs )
            {
                for ( std::uint8_t channel( 0 ); channel < numberOfChannels; ++channel )
                {
                    pInterleavedOutputs[ numberOfChannels * sample + channel ] = 0;
                }
            }
            ++crippledSamples;
            ++sample;
        }
    #if !defined( NDEBUG ) && !defined( LE_PUBLIC_BUILD )
        auto const zeroedSamples( sample - lastNonZeroedSample );
        if ( zeroedSamples ) /*std*/::printf( "zeroed samples %u (%u, %X)\n", zeroedSamples, pSeparatedOutputs || pInterleavedOutputs, this );
    #endif // !NDEBUG && !LE_PUBLIC_BUILD
        // reset (non)crippled positions if we have gone through one
        // noncrippled->crippled cycle:
        if ( crippledSamples == crippledDurationInSamples )
        {
            if ( periodic ) nonCrippledSamples = 0;
                            crippledSamples    = 0;
        }
    }
    nonCrippledSamples_ = nonCrippledSamples;
    crippledSamples_    = crippledSamples   ;
}

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // demoLimiter_hpp
