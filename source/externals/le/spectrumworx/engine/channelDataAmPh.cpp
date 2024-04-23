////////////////////////////////////////////////////////////////////////////////
///
/// channelDataAmPh.cpp
/// -------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "channelDataAmPh.hpp"

#include "le/spectrumworx/effects/indexRange.hpp" //...mrmlj...

#include "le/math/vector.hpp"

#include <cstdint>
#ifndef NDEBUG
#include <limits>
#endif // NDEBUG
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{
////////////////////////////////////////////////////////////////////////////////
//...mrmlj...these should go into engine/buffers.cpp...
namespace Detail
{
    LE_NOTHROWNOALIAS
    DataRange LE_FASTCALL resize( DataRange const & range, IndexRange const & workingRange )
    {
    #ifndef NDEBUG
        if ( !workingRange )
            return DataRange();
    #endif // NDEBUG
        return DataRange( &range[ workingRange.begin() ], &range[ workingRange.end() - 1 ] + 1 );
    }

    LE_WEAK_FUNCTION LE_NOTHROW LE_CONST_FUNCTION LE_COLD
    std::uint16_t LE_FASTCALL_ABI fftBufferSize
    (
        std::uint8_t  const a,
        std::uint8_t  const b,
        std::uint8_t  const c,
        std::uint8_t  const sizeOfT,
        std::uint16_t const fftSize
    )
    {
        using Utility::Constants::vectorAlignment;
        BOOST_ASSERT_MSG( fftSize * a / b < std::numeric_limits<std::uint16_t>::max(), "Short integer overflow" );
        auto const storageBytes( std::uint16_t( std::uint16_t( std::uint32_t( fftSize * a ) / b ) + c ) * sizeOfT );
        BOOST_ASSERT_MSG( storageBytes    < std::numeric_limits<std::uint16_t>::max(), "Short integer overflow" );
        return storageBytes;
    }
} // namespace Detail
////////////////////////////////////////////////////////////////////////////////
} // namespace Engine
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

ChannelData_AmPh::ChannelData_AmPh( FullChannelData_AmPh & data, IndexRange const & workingRange )
    :
    SubRange<FullChannelData_AmPh, DataRange>( data, workingRange )
#ifdef LE_PV_USE_TSS
    ,pAnalysisState ( nullptr ),
     pSynthesisState( nullptr )
#endif // LE_PV_USE_TSS
{
}

namespace
{ //...mrmlj...using internal knowledge of ChannelData_AmPh storage requirements
  //...mrmlj...(that it depends only on the FFT size) only to avoid including
  //...mrmlj...engine/setup.hpp...
    Engine::StorageFactors storageFactors( std::uint16_t const fftSize ) { return { fftSize, 0, 0, 0 }; }
} // anonymous namespace

ChannelData_AmPhStorage::ChannelData_AmPhStorage
(
    std::uint16_t const fftSize,
    std::uint16_t const beginBin,
    std::uint16_t const endBin,
    Storage             storage
)
    :
    ChannelData_AmPh
    (
        constructFull( storageFactors( fftSize ), storage ),
        IndexRange   ( beginBin, endBin                   )
    )
{
    //...mrmlj...(failures in pitch shifter if data outside user range is not zeroed)...
    Math::clear( full().amps  ().begin(), this-> amps  ().begin() );
    Math::clear( this-> amps  ().end  (), full().amps  ().end  () );
    Math::clear( full().phases().begin(), this-> phases().begin() );
    Math::clear( this-> phases().end  (), full().phases().end  () );
}


std::uint32_t ChannelData_AmPhStorage::requiredStorage( std::uint16_t const fftSize )
{
    return FullChannelData_AmPh::requiredStorage( storageFactors( fftSize ) );
}


DataRange subRange( DataRange const & range, std::uint16_t const beginIndex, std::uint16_t const endIndex )
{
    BOOST_ASSERT_MSG( beginIndex <= endIndex                , "Backward range."     );
    BOOST_ASSERT_MSG( endIndex   <= unsigned( range.size() ), "Index out of range." );
    return DataRange( &range.begin()[ beginIndex ], &range.begin()[ endIndex ] );
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
