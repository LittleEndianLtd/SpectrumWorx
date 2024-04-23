////////////////////////////////////////////////////////////////////////////////
///
/// \file buffers.hpp
/// -----------------
///
/// LE.SW.Engine specific buffer classes.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef buffers_hpp__1667CD4C_0651_46FC_896B_C90E2191E541
#define buffers_hpp__1667CD4C_0651_46FC_896B_C90E2191E541
#pragma once
//------------------------------------------------------------------------------
#include "configuration.hpp"

#ifdef __GNUC__
    #include "le/math/vector.hpp"
#endif // __GNUC__

#include "le/utility/buffers.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/typeTraits.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/range/iterator_range_core.hpp"

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Effects )
    class IndexRange;
LE_IMPL_NAMESPACE_END( Effects )
using Effects::IndexRange;
LE_IMPL_NAMESPACE_BEGIN( Engine )
    class ChannelData;
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
namespace Engine
{
//------------------------------------------------------------------------------

#ifdef LE_HAS_NT2
/// Helper, frequently used typedefs.
//...mrmlj...to be removed soon...
using StaticHalfFFTBuffer = Utility::AlignedBuffer<float, LE::SW::Engine::Constants::maximumFFTSize / 2 + 1, false, Utility::Constants::vectorAlignment>;
#endif // LE_HAS_NT2

using real_t = float;

#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
//...mrmlj...
struct StorageFactors
{
    std::uint16_t /*const*/ fftSize         ;
#if LE_SW_ENGINE_WINDOW_PRESUM
    std::uint8_t  /*const*/ windowSizeFactor;
#endif // LE_SW_ENGINE_WINDOW_PRESUM
    std::uint8_t  /*const*/ overlapFactor   ;
    std::uint8_t  /*const*/ numberOfChannels;
    std::uint32_t /*const*/ samplerate      ;

    bool complete() const
    {
        return
            fftSize          &&
        #if LE_SW_ENGINE_WINDOW_PRESUM
            windowSizeFactor &&
        #endif // LE_SW_ENGINE_WINDOW_PRESUM
            overlapFactor    &&
            numberOfChannels &&
            samplerate;
    }

    bool operator==( StorageFactors const & other ) const { return std::memcmp( this, &other, sizeof( *this ) ) == 0; }
}; // struct StorageFactors

#pragma warning( pop )

#ifdef LE_HAS_NT2
////////////////////////////////////////////////////////////////////////////////
///
/// \typedef HeapSharedStorage
///
////////////////////////////////////////////////////////////////////////////////

using HeapSharedStorage = Utility::AlignedHeapBuffer<char>;
#endif // LE_HAS_NT2


////////////////////////////////////////////////////////////////////////////////
///
/// \class SharedStorageFFTBasedBuffer
///
/// Adapts the Utility::SharedStorageBuffer class to produce buffers whose size
/// is dependent on the size of the FFT. The actual size of the buffer is
/// calculated as ( a / b ) * FFTSize + c.
///
////////////////////////////////////////////////////////////////////////////////

using Storage = Utility::Storage;

namespace Detail
{
    LE_NOTHROW LE_CONST_FUNCTION
    std::uint16_t LE_FASTCALL_ABI fftBufferSize
    (
        std::uint8_t  a,
        std::uint8_t  b,
        std::uint8_t  c,
        std::uint8_t  sizeoOfT,
        std::uint16_t fftSize
    );
} // namespace Detail

template <typename T, int a = 1, int b = 1, int c = 0>
class SharedStorageFFTBasedBuffer : public Utility::SharedStorageBuffer<T>
{
public:
    SharedStorageFFTBasedBuffer() {}

    LE_COLD LE_CONST_FUNCTION
    static std::uint16_t LE_FASTCALL requiredStorage( StorageFactors const & factors )
    {
        auto const storageBytes( Engine::Detail::fftBufferSize( a, b, c, sizeof( T ), factors.fftSize ) );
        return storageBytes;
    }
    LE_COLD
    void LE_FASTCALL resize( StorageFactors const & factors, Storage & storage )
    {
        Utility::SharedStorageBuffer<T>::resize( requiredStorage( factors ), storage );
    }

    //...mrmlj...required for automatic reset() member function generation by
    //...mrmlj...the LE_DYNAMIC_CHANNEL_STATE macro...
    void reset() { this->clear(); }
}; // class SharedStorageFFTBasedBuffer

template <typename T = real_t> struct   HalfFFTBuffer : SharedStorageFFTBasedBuffer<T, 1, 2, 1> {};
template <typename T = real_t> struct       FFTBuffer : SharedStorageFFTBasedBuffer<T, 1, 1, 0> {};
template <typename T = real_t> struct DoubleFFTBuffer : SharedStorageFFTBasedBuffer<T, 2, 1, 0> {};

template <typename T = real_t>
class WindowBuffer : public Utility::SharedStorageBuffer<T>
{
public:
    LE_COLD LE_CONST_FUNCTION
    static std::uint16_t LE_FASTCALL requiredStorage( StorageFactors const & factors )
    {
    #if LE_SW_ENGINE_WINDOW_PRESUM
        std::uint8_t const windowSizeFactor( factors.windowSizeFactor );
    #else
        std::uint8_t const windowSizeFactor( 1                        );
    #endif // LE_SW_ENGINE_WINDOW_PRESUM
        auto const storageBytes( Engine::Detail::fftBufferSize( windowSizeFactor, 1, 0, sizeof( T ), factors.fftSize ) );
        return storageBytes;
    }
    LE_COLD
    void LE_FASTCALL resize( Engine::StorageFactors const & factors, Storage & storage )
    {
        Utility::SharedStorageBuffer<T>::resize( requiredStorage( factors ), storage );
    }

    //...mrmlj...required for automatic reset() member function generation by
    //...mrmlj...the LE_DYNAMIC_CHANNEL_STATE macro...
    void reset() { this->clear(); }
}; // class WindowBuffer


// www.lysator.liu.se/c/restrict.html
// http://cellperformance.beyond3d.com/articles/2006/05/demystifying-the-restrict-keyword.html
// http://people.cs.pitt.edu/~mock/papers/clei2004.pdf
using         DataRange = boost::iterator_range<float       * LE_RESTRICT>;
using ReadOnlyDataRange = boost::iterator_range<float const * LE_RESTRICT>;


////////////////////////////////////////////////////////////////////////////////
/// \struct DataPair
////////////////////////////////////////////////////////////////////////////////
/// \todo Document.
///                                           (11.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

struct DataPair
{
    enum Index
    {
        First  = 0,
        Second = 1,

        // AmPh pair
        Amps   = First,
        Phases = Second,

        // ReIm pair
        Reals = First,
        Imags = Second,

        // MainSide pair
        Main = First,
        Side = Second
    };
}; // struct DataPair


////////////////////////////////////////////////////////////////////////////////
///
/// \class DataPairImpl
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Document.
///                                           (11.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    //...mrmlj...this DataPairImpl helper has to be declared outside the class
    //...mrmlj...template so that it can be specialized...reinvestigate and fix
    //...mrmlj...this problematic design...
    template <class Data, class FullData>
    LE_NOTHROWNOALIAS
    Data LE_FASTCALL makeData( FullData & fullData, IndexRange const & workingRange ) { return Data( fullData, workingRange ); }
}

#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

template <class Data>
class DataPairImpl : public DataPair
{
protected:
    using DataArray = std::array<Data, 2>;
    using Impl      = DataPairImpl<Data>;
    using size_type = std::uint16_t;

public:
    size_type numberOfBins() const { return size(); }
    size_type size        () const { return static_cast<size_type>( first().size() ); }

    DataRange         jointView()       { return DataRange        ( first().begin(), second().end() ); }
    ReadOnlyDataRange jointView() const { return ReadOnlyDataRange( first().begin(), second().end() ); }

protected:
    DataPairImpl() {}

    template <class FullData>
    DataPairImpl( FullData & fullData, IndexRange const & workingRange )
        :
        data_( makeData( fullData, workingRange ) )
    {}

    Data       & first ()       { return data()[ First  ]; }
    Data       & second()       { return data()[ Second ]; }

    Data const & first () const { return data()[ First  ]; }
    Data const & second() const { return data()[ Second ]; }

    DataArray       & data()       { return data_; }
    DataArray const & data() const { return data_; }

    template <class T> friend class DataPairImpl;

private:
    template <class FullData>
    DataArray makeData( FullData & fullData, IndexRange const & workingRange )
    {
        DataArray const data =
        {{
            Engine::Detail::makeData<Data>( fullData.data()[ 0 ], workingRange ),
            Engine::Detail::makeData<Data>( fullData.data()[ 1 ], workingRange ),
        }};
        return data;
    }

private:
    DataArray data_;
}; // class DataPairImpl

#pragma warning( pop )


////////////////////////////////////////////////////////////////////////////////
///
/// \class SharedStorageDataPairImpl
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Document.
///                                           (11.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

template <class Data>
class SharedStorageDataPairImpl : public DataPairImpl<Data>
{
private:
#ifndef __APPLE__
    //...mrmlj...seems to give slightly better performance out of our FFT...
    static std::uint8_t const partial_address_aliasing_workaround_offset = 1 * Utility::Constants::vectorAlignment;
#else
    static std::uint8_t const partial_address_aliasing_workaround_offset = 0;
#endif // __APPLE__

public:
    LE_COLD LE_CONST_FUNCTION
    static std::uint32_t LE_FASTCALL requiredStorage( StorageFactors const & factors )
    {
        auto const dataElements        ( std::tuple_size<typename DataPairImpl<Data>::DataArray>::value );
        auto const requiredStorageBytes( Utility::align( Data::requiredStorage( factors ) ) * dataElements );
        return static_cast<std::uint32_t>( requiredStorageBytes + ( partial_address_aliasing_workaround_offset * ( dataElements - 1 ) ) );
    }
    LE_COLD
    void LE_FASTCALL resize( StorageFactors const & factors, Storage & storage )
    {
        //for ( auto & data : this->data() ) data.resize( factors, storage );
        this->data()[ 0 ].resize( factors, storage );
        storage.advance_begin( partial_address_aliasing_workaround_offset );
        this->data()[ 1 ].resize( factors, storage );
    }

    void reset()
    {
        this->data()[ 0 ].reset();
        this->data()[ 1 ].reset();
    }
}; // class SharedStorageDataPairImpl


////////////////////////////////////////////////////////////////////////////////
///
/// \class SubRange
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Document.
///                                           (11.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    template <typename Range> struct makeConst            { using type = Range            ; };
    template <>               struct makeConst<DataRange> { using type = ReadOnlyDataRange; };
}

#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

template <class FullRangeData, class SubRangeHolder>
class SubRange : public DataPairImpl<SubRangeHolder>
{
public:
    using Parent    = FullRangeData               ;
    using Impl      = DataPairImpl<SubRangeHolder>;
    using size_type = typename Impl::size_type    ;

    SubRange( Parent & fullData, IndexRange const & workingRange )
        :
        DataPairImpl<SubRangeHolder>(  fullData, workingRange ),
        pFullData_                  ( &fullData               )
    {}

    //...mrmlj...these will not work for MainSide-subranges (?reinvestigate and document why?)...
    size_type beginBin() const { return static_cast<size_type>( first().begin() - full().first().begin() ); }
    size_type endBin  () const { return static_cast<size_type>( first().end  () - full().first().begin() ); }

    void advance_begin( size_type const sizeToSkip )
    {
        this->data()[ 0 ].advance_begin( sizeToSkip );
        this->data()[ 1 ].advance_begin( sizeToSkip );
    }

    SubRange & operator++()
    {
        advance_begin( 1 );
        return *this;
    }

    FullRangeData       & full()       { return *pFullData_; }
    FullRangeData const & full() const { return *pFullData_; }

    //...mrmlj...these will not work for MainSide-subranges...
    void copySkippedRanges( DataPair::Index sourceData, float * const pTargetBuffer )
    {
        using namespace Math;
        ReadOnlyDataRange const & fullData( full().data()[ sourceData ] );
        ReadOnlyDataRange const & thisData( this-> data()[ sourceData ] );
        copy( fullData.begin(), thisData.begin(), &pTargetBuffer[ 0        ] );
        copy( thisData.end  (), fullData.end  (), &pTargetBuffer[ endBin() ] );
    }

    template <class TargetBuffer>
    void copySkippedRanges( DataPair::Index sourceData, TargetBuffer & targetBuffer )
    {
        BOOST_ASSERT_MSG( unsigned( targetBuffer.size() ) >= this->size(), "Target buffer too small." );
        copySkippedRanges( sourceData, targetBuffer.begin() );
    }

    explicit operator bool() const { return static_cast<bool>( first() ); }

protected:
    using Impl::first ;
    using Impl::second;

    using ReadOnlySubRange = typename Engine::Detail::makeConst<SubRangeHolder>::type;

    ReadOnlySubRange const & first () const { return reinterpret_cast<ReadOnlySubRange const &>( Impl::first () ); }
    ReadOnlySubRange const & second() const { return reinterpret_cast<ReadOnlySubRange const &>( Impl::second() ); }

private:
    FullRangeData * LE_RESTRICT const pFullData_;
}; // class SubRange

#pragma warning( pop )


////////////////////////////////////////////////////////////////////////////////
///
/// \class SharedStorageHalfFFTBufferPair
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Document.
///                                           (11.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

class SharedStorageHalfFFTBufferPair : public SharedStorageDataPairImpl<HalfFFTBuffer<float>>
{
protected: template <class FullRangeData, class SubRangeHolder> friend class SubRange;
    using Impl::first ;
    using Impl::second;

    ReadOnlyDataRange const & first () const { return reinterpret_cast<ReadOnlyDataRange const &>( data()[ First  ] ); }
    ReadOnlyDataRange const & second() const { return reinterpret_cast<ReadOnlyDataRange const &>( data()[ Second ] ); }
}; // class SharedStorageHalfFFTBufferPair


namespace Detail
{
    LE_NOTHROWNOALIAS
    DataRange LE_FASTCALL resize( DataRange const & range, IndexRange const & workingRange );

    template <> inline
    LE_NOTHROWNOALIAS
    DataRange LE_FASTCALL makeData<DataRange, HalfFFTBuffer<float>>( HalfFFTBuffer<float> & fullData, IndexRange const & workingRange )
    {
        return DataRange( resize( fullData, workingRange ) );
    }
} // namespace Detail


////////////////////////////////////////////////////////////////////////////////
///
/// \class MainSide
///
////////////////////////////////////////////////////////////////////////////////
/// \todo Document.
///                                           (11.11.2011.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

template <class Data>
class MainSide : public SharedStorageDataPairImpl<Data>
{
public:
    Data       & main()       { return this->first (); }
    Data const & main() const { return this->first (); }
    Data const & side() const { return this->second(); }

private: friend class ChannelData;
    Data & mutableSide() { return this->second(); }
}; // class MainSide

template <class FullRangeData, class SubRangeHolder>
class MainSide<SubRange<FullRangeData, SubRangeHolder>> : public SubRange<FullRangeData, SubRangeHolder>
{
public:
    using size_type = typename SubRange<FullRangeData, SubRangeHolder>::size_type;

    MainSide( FullRangeData & data, IndexRange const & workingRange )
        :
        SubRange<FullRangeData, SubRangeHolder>( data, workingRange )
    {}

    //...mrmlj...
    size_type beginBin() const { return main().beginBin(); }
    size_type endBin  () const { return main().endBin  (); }

    SubRangeHolder       & main()       { return this->first (); }
    SubRangeHolder const & main() const { return this->first (); }
    SubRangeHolder const & side() const { return this->second(); }

private:
    using SubRange<FullRangeData, SubRangeHolder>::second;
}; // class MainSide<SubRange<FullRangeData, SubRangeHolder>>

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace Engine
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // buffers_hpp
