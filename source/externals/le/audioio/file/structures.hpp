////////////////////////////////////////////////////////////////////////////////
///
/// \file structures.hpp
/// --------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef structures_hpp__9A3097F6_D278_42A0_AEDA_DBF5E7A9FF35
#define structures_hpp__9A3097F6_D278_42A0_AEDA_DBF5E7A9FF35
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include <cstdint>
#include <cstring>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

namespace Detail
{
#ifdef _MSC_VER
    using WORD  = unsigned short;
    using DWORD = unsigned long ;
#else
    using WORD  = std::uint16_t;
    using DWORD = std::uint32_t;
#endif // _MSC_VER

    #pragma pack( push, 1 )

    ////////////////////////////////////////////////////////////////////////////
    /// \struct GUID
    ////////////////////////////////////////////////////////////////////////////

#ifdef GUID_DEFINED
    using GUID = ::_GUID;
#else
    struct GUID
    {
        std::uint32_t Data1;
        std::uint16_t Data2;
        std::uint16_t Data3;
        std::uint8_t  Data4[ 8 ];

        bool LE_FASTCALL operator==( GUID const & other ) const { return std::memcmp( this, &other, sizeof( *this ) ) == 0; }
        bool LE_FASTCALL operator!=( GUID const & other ) const { return !( *this == other )                              ; }
    }; // struct GUID
#endif // GUID_DEFINED

    ////////////////////////////////////////////////////////////////////////////
    /// \struct WAVEFORMATEX
    ////////////////////////////////////////////////////////////////////////////

    struct WAVEFORMATEX
    {
        WORD  wFormatTag;      /* format type */
        WORD  nChannels;       /* number of channels (i.e. mono, stereo...) */
        DWORD nSamplesPerSec;  /* sample rate */
        DWORD nAvgBytesPerSec; /* for buffer estimation */
        WORD  nBlockAlign;     /* block size of data */
        WORD  wBitsPerSample;  /* Number of bits per sample of mono data */
        WORD  cbSize;          /* The count in bytes of the size of extra information (after cbSize) */
    }; // struct WAVEFORMATEX


    ////////////////////////////////////////////////////////////////////////////
    /// \struct WAVEFORMATEXTENSIBLE
    ////////////////////////////////////////////////////////////////////////////

    struct WAVEFORMATEXTENSIBLE
    {
        WAVEFORMATEX    Format;
        union {
            WORD wValidBitsPerSample;       /* bits of precision  */
            WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
            WORD wReserved;                 /* If neither applies, set to zero. */
        } Samples;
        DWORD           dwChannelMask;      /* which channels are present in stream  */
        GUID            SubFormat;
     }; // struct WAVEFORMATEXTENSIBLE


    #pragma warning( push )
    #pragma warning( disable : 4512 ) // Assignment operator could not be generated.

    ////////////////////////////////////////////////////////////////////////////
    /// \struct FixedValue
    ////////////////////////////////////////////////////////////////////////////

    template <typename T, T ValidValue>
    struct FixedValue
    {
        T const value;

        static T BOOST_CONSTEXPR_OR_CONST validValue = ValidValue;

        FixedValue() : value( validValue ) {}
        bool invalid() const { return value != validValue; }
    }; // struct FixedValue


    ////////////////////////////////////////////////////////////////////////////
    /// \struct ChunkHeader
    ////////////////////////////////////////////////////////////////////////////

    template <DWORD tagValue>
    struct ChunkHeader
    {
        using Header = ChunkHeader<       tagValue>;
        using Tag    = FixedValue <DWORD, tagValue>;

        Tag   tag ;
        DWORD size;
        
        DWORD headerSize() const { return sizeof( *this )          ; }
        DWORD dataSize  () const { return size                     ; }
        DWORD totalSize () const { return headerSize() + dataSize(); }

        bool  requiresPaddingByte() const { return ( dataSize() % 2 ) != 0; }
        bool  invalid            () const { return tag.invalid()          ; }
    }; // struct ChunkHeader


    ////////////////////////////////////////////////////////////////////////////
    /// \struct RIFFHeader
    ////////////////////////////////////////////////////////////////////////////

    struct RIFFHeader : ChunkHeader<'FFIR'>
    {
        FixedValue<DWORD, 'EVAW'> waveTag;
    }; // struct RIFFHeader


    ////////////////////////////////////////////////////////////////////////////
    /// \struct Format
    ////////////////////////////////////////////////////////////////////////////

    struct Format : ChunkHeader<' tmf'>
    {
        WAVEFORMATEXTENSIBLE fmt;
    }; // struct Format


    ////////////////////////////////////////////////////////////////////////////
    /// \struct DataHeader
    ////////////////////////////////////////////////////////////////////////////

    using DataHeader = ChunkHeader<'atad'>;

    #pragma warning( pop )
    #pragma pack   ( pop )

#ifndef WAVE_FORMAT_IEEE_FLOAT
    enum WaveFormatExFormatTags
    {
        WAVE_FORMAT_PCM        =      1,
        WAVE_FORMAT_IEEE_FLOAT =      3,
        WAVE_FORMAT_EXTENSIBLE = 0xFFFE,
    }; // enum WaveFormatExFormatTags

    enum WaveFormatExtensibleSpeakerPositions
    {
        SPEAKER_FRONT_LEFT            = 0x1    ,
        SPEAKER_FRONT_RIGHT           = 0x2    ,
        SPEAKER_FRONT_CENTER          = 0x4    ,
        SPEAKER_LOW_FREQUENCY         = 0x8    ,
        SPEAKER_BACK_LEFT             = 0x10   ,
        SPEAKER_BACK_RIGHT            = 0x20   ,
        SPEAKER_FRONT_LEFT_OF_CENTER  = 0x40   ,
        SPEAKER_FRONT_RIGHT_OF_CENTER = 0x80   ,
        SPEAKER_BACK_CENTER           = 0x100  ,
        SPEAKER_SIDE_LEFT             = 0x200  ,
        SPEAKER_SIDE_RIGHT            = 0x400  ,
        SPEAKER_TOP_CENTER            = 0x800  ,
        SPEAKER_TOP_FRONT_LEFT        = 0x1000 ,
        SPEAKER_TOP_FRONT_CENTER      = 0x2000 ,
        SPEAKER_TOP_FRONT_RIGHT       = 0x4000 ,
        SPEAKER_TOP_BACK_LEFT         = 0x8000 ,
        SPEAKER_TOP_BACK_CENTER       = 0x10000,
        SPEAKER_TOP_BACK_RIGHT        = 0x20000,
    }; // enum WaveFormatExtensibleSpeakerPositions

    LE_WEAK_SYMBOL_CONST GUID const KSDATAFORMAT_SUBTYPE_PCM        = { 0x00000001, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
    LE_WEAK_SYMBOL_CONST GUID const KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000, 0x0010, { 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71 } };
#endif // WAVE_FORMAT_IEEE_FLOAT

    /// \note The numberOfBytes parameter has to be signed to cover cases of
    /// files containing only the WAVEFORMATEX structure w/o the cbSize member
    /// when the expression RIFFHeader::fmtSize - sizeof( RIFFHeader::fmt )
    /// yields a negative number (and thus the pointer needs to be moved
    /// backwards).
    ///                                       (27.11.2012.) (Domagoj Saric)
    template <typename T>
    T * LE_RESTRICT advance( T * LE_RESTRICT & pointer, int const numberOfBytes )
    {
        return pointer = reinterpret_cast<T *>( const_cast<char *>( reinterpret_cast<char const *>( pointer ) ) + numberOfBytes );
    }

    template <class Structure, typename T>
    Structure * LE_RESTRICT extractStructureAndAdvance( T * LE_RESTRICT & pointer )
    {
        Structure * LE_RESTRICT const pStructure( reinterpret_cast<Structure * LE_RESTRICT>( pointer ) );
        advance( pointer, sizeof( *pStructure ) );
        return pStructure;
    }
} // namespace Detail

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // structures_hpp
