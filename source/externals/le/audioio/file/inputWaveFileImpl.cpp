////////////////////////////////////////////////////////////////////////////////
///
/// inputWaveFileImpl.cpp
/// ---------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "inputWaveFileImpl.hpp"

#include "le/math/vector.hpp"

#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/trace.hpp"

#include "boost/mmap/mappble_objects/file/utility.hpp" // Boost sandbox
#include "boost/simd/sdk/config/arch.hpp"

#include <algorithm>

#if defined( __ANDROID__ )
    #include "android/asset_manager.h"
#elif defined( __APPLE__ )
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #import "Foundation/Foundation.h"
    #endif // iOS
#endif // __APPLE__

#include <cstdint>
#include <utility> // rvalues
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

template <> struct Implementation<AudioIO::InputWaveFile> { using type = AudioIO::InputWaveFileImpl; };

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

InputWaveFileImpl::InputWaveFileImpl( InputWaveFileImpl && other )
    :
    pFormat_   (            other.pFormat_      ),
    pData_     (            other.pData_        ),
    pDataBegin_(            other.pDataBegin_   ),
    pDataEnd_  (            other.pDataEnd_     ),
    mappedFile_( std::move( other.mappedFile_ ) )
{
    other.pFormat_    = nullptr;
    other.pData_      = nullptr;
    other.pDataBegin_ = nullptr;
    other.pDataEnd_   = nullptr;
}

namespace
{
    //[]( char const & position ) { return reinterpret_cast<DWORD const &>( position ) == DataHeader::Tag::validValue; }
    using Detail::DWORD;
    template <DWORD value> struct Finder { bool operator()( char const & position ) const { return reinterpret_cast<DWORD const &>( position ) == value; } };
} // anonymous namespace

LE_NOTHROW
char const * LE_COLD InputWaveFileImpl::open( void const * LE_RESTRICT const mappedFileData, std::uint32_t const fileSize )
{
    char const * LE_RESTRICT       pData   ( static_cast<char const *>( mappedFileData ) );
    char const * LE_RESTRICT const pDataEnd( pData + fileSize / sizeof( *pData )         );

    // http://en.wikipedia.org/wiki/Resource_Interchange_File_Format
    // https://ccrma.stanford.edu/courses/422/projects/WaveFormat
    // http://www.sonicspot.com/guide/wavefiles.html
    // http://home.roadrunner.com/~jgglatt/tech/wave.htm
    // http://www.dragonwins.com/domains/getteched/wav/index.htm
    // http://tech.ebu.ch/docs/tech/tech3285.pdf (BWF, 'fact' chunk)

    using namespace Detail;

    RIFFHeader const * LE_RESTRICT const pRIFFHeader( extractStructureAndAdvance<RIFFHeader const>( pData ) );

    pData = std::find_if( pData, pDataEnd, Finder<Format::Tag::validValue>() );
    if ( pData == pDataEnd ) return "Not a WAVE file";
    Format const * LE_RESTRICT const pFormat( extractStructureAndAdvance<Format const>( pData ) );
    advance( pData, pFormat->size - sizeof( pFormat->fmt ) );

    pData = std::find_if( pData, pDataEnd, Finder<DataHeader::Tag::validValue>() );
    if ( pData == pDataEnd ) return "Unrecognized WAVE file format";
    DataHeader const * LE_RESTRICT const pDataHeader( extractStructureAndAdvance<DataHeader const>( pData ) );

    unsigned int const sizeRemainingForRIFFChunk(                            fileSize - pRIFFHeader->headerSize()                                                                                                     );
    unsigned int const sizeRemainingForDataChunk( static_cast<unsigned int>( fileSize - pDataHeader->headerSize() - ( reinterpret_cast<char const *>( pDataHeader ) - static_cast<char const *>( mappedFileData ) ) ) );

    BOOST_ASSERT( !pFormat    ->tag.invalid() );
    BOOST_ASSERT( !pDataHeader->tag.invalid() );
    // http://www.dsprelated.com/showmessage/58062/1.php
    LE_TRACE_IF( ( pRIFFHeader->size < sizeRemainingForRIFFChunk ), "WAVE with trailing data after the RIFF chunk" );

    bool const extendedFormat( pFormat->fmt.Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE );
    enum DataType { Integer, Float, Unknown };
    DataType dataType;
    if
    (
        ( extendedFormat && pFormat->fmt.SubFormat         == KSDATAFORMAT_SUBTYPE_PCM ) ||
        (                   pFormat->fmt.Format.wFormatTag == WAVE_FORMAT_PCM          )
    )
    { dataType = Integer; }
    else
    if
    (
        ( extendedFormat && pFormat->fmt.SubFormat         == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) ||
        (                   pFormat->fmt.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT          )
    )
    { dataType = Float; }
    else
    { dataType = Unknown; }

    if
    (
        ( pRIFFHeader->tag    .invalid()                ) ||
        ( pRIFFHeader->waveTag.invalid()                ) ||
        ( pRIFFHeader->size > sizeRemainingForRIFFChunk ) ||
        ( pDataHeader->size > sizeRemainingForDataChunk ) ||
        (
            pFormat->size != sizeof( pFormat->fmt        )                                        &&
            pFormat->size != sizeof( pFormat->fmt.Format )                                        &&
            pFormat->size != sizeof( pFormat->fmt.Format ) - sizeof( pFormat->fmt.Format.cbSize )
        ) ||
        (
            dataType == Unknown
        ) ||
        (
            dataType == Integer &&
            pFormat->fmt.Format.wBitsPerSample != 16 &&
            pFormat->fmt.Format.wBitsPerSample != 24
        ) ||
        (
            dataType == Float &&
            pFormat->fmt.Format.wBitsPerSample != 32
        )
    )
    {
        return "Unrecognized WAVE file format";
    }

    pFormat_    = &pFormat->fmt;
    pDataBegin_ = pData;
    pDataEnd_   = pData + pDataHeader->size;
    pData_      = pData;

    restart();

    return nullptr;
}


std::uint32_t LE_HOT LE_FASTCALL InputWaveFileImpl::read( float * LE_RESTRICT /*const*/ pOutput, std::uint32_t samples ) const
{
    BOOST_ASSERT_MSG( !!*this, "No file open." );

    samples = std::min( samples, remainingSamples() );

    using namespace Detail;

    auto const totalSamples( samples * numberOfChannels() );

    if
    (
        ( pFormat_->Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) ||
        (
            pFormat_->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE &&
            pFormat_->SubFormat         == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT
        )
    )
    {
        float const * LE_RESTRICT pInput( reinterpret_cast<float const *>( pData_ ) );
        BOOST_ASSERT( pFormat_->Format.wBitsPerSample == sizeof( *pInput ) * 8 );
        std::copy( pInput, pInput + totalSamples, pOutput );
        pData_ += totalSamples * sizeof( *pInput );
        return samples;
    }

    switch ( pFormat_->Format.wBitsPerSample )
    {
        case 16:
        {
            auto pInput( reinterpret_cast<std::int16_t const * LE_RESTRICT>( pData_ ) );
            Math::convertSamples( pInput, pOutput, totalSamples );
            pInput += totalSamples;
            pData_ = reinterpret_cast<char const *>( pInput );
            break;
        }

        case 24:
        {
            float const scale( 1.0f / ( 1U << 23 ) );

            char const * pInput( pData_ );
            auto const pOutputEnd( pOutput + totalSamples );
            while ( pOutput < pOutputEnd )
            {
                std::int32_t inputSample;
                std::memcpy( &inputSample, pInput - 1, sizeof( inputSample ) );
                inputSample >>= 8;
                float const outputSample( static_cast<float>( inputSample ) * scale );
                *pOutput++ = outputSample;
                pInput += 3;
            }
            pData_ = pInput;

            break;
        }

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    return samples;
}

std::uint8_t  InputWaveFileImpl::numberOfChannels() const { return pFormat_->Format.nChannels     ; }
std::uint32_t InputWaveFileImpl::sampleRate      () const { return pFormat_->Format.nSamplesPerSec; }
std::uint32_t InputWaveFileImpl::remainingBytes  () const { return static_cast<std::uint32_t>( pDataEnd_ - pData_ ); }
std::uint32_t InputWaveFileImpl::remainingSamples() const { return remainingSampleFramesFrom( pData_      ); }
std::uint32_t InputWaveFileImpl::lengthInSamples () const { return remainingSampleFramesFrom( pDataBegin_ ); }

template <Utility::SpecialLocations parentDirectory>
char const * LE_COLD InputWaveFileImpl::open( char const * const fileName )
{
    using namespace Detail;

    auto mappedFile( Utility::File::map<parentDirectory>( fileName ) );
    if ( !mappedFile )
        return LE_TRACE_RETURN( "Error opening file", "Failed to map file %s (@ %d, errno: %d)", fileName, parentDirectory, errno );

    error_msg_t const error( InputWaveFileImpl::open( mappedFile.begin(), static_cast<unsigned int>( mappedFile.size() ) ) );
    if ( !error )
        mappedFile_ = std::move( mappedFile );
    return error;
}


////////////////////////////////////////////////////////////////////////////////
// PImpl forwarders
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW std::uint32_t InputWaveFile::read( float * const pOutput, std::uint32_t const samples ) const { return impl( *this ).read( pOutput, samples ); }

LE_NOTHROW LE_PURE_FUNCTION std::uint8_t  InputWaveFile::numberOfChannels     () const { return impl( *this ).numberOfChannels(); }
LE_NOTHROW LE_PURE_FUNCTION std::uint32_t InputWaveFile::sampleRate           () const { return impl( *this ).sampleRate      (); }
LE_NOTHROW LE_PURE_FUNCTION std::uint32_t InputWaveFile::remainingSampleFrames() const { return impl( *this ).remainingSamples(); }
LE_NOTHROW LE_PURE_FUNCTION std::uint32_t InputWaveFile::lengthInSampleFrames () const { return impl( *this ).lengthInSamples (); }

LE_NOTHROW                  void          InputWaveFile::setTimePosition  ( std::uint32_t const positionInMilliseconds ) { return impl( *this ).setTimePosition  ( positionInMilliseconds ); }
LE_NOTHROW                  void          InputWaveFile::setSamplePosition( std::uint32_t const positionInSampleFrames ) { return impl( *this ).setSamplePosition( positionInSampleFrames ); }
LE_NOTHROW                  void          InputWaveFile::restart          (                                            ) { return impl( *this ).restart          (                        ); }

LE_NOTHROW LE_PURE_FUNCTION std::uint32_t InputWaveFile::getTimePosition  () const { return impl( *this ).getTimePosition  (); }
LE_NOTHROW LE_PURE_FUNCTION std::uint32_t InputWaveFile::getSamplePosition() const { return impl( *this ).getSamplePosition(); }

LE_NOTHROW LE_PURE_FUNCTION               InputWaveFile::operator bool() const { return impl( *this ).operator bool(); }

LE_NOTHROWNOALIAS InputWaveFile:: InputWaveFile() {}
LE_NOTHROW        InputWaveFile:: InputWaveFile( InputWaveFile && other ) : ConcreteStackPimpl( std::move( other ) ) {}
LE_NOTHROWNOALIAS InputWaveFile::~InputWaveFile() {}

LE_NOTHROW void InputWaveFile::close() { impl( *this ).close(); }

LE_NOTHROW bool InputWaveFile::readLooped( float * pOutput, std::uint32_t numberOfSampleFrames ) const
{
    while ( numberOfSampleFrames )
    {
        auto const readSamples( read( pOutput, numberOfSampleFrames ) );
        numberOfSampleFrames -= readSamples;
        pOutput              += readSamples * numberOfChannels();
        if ( readSamples < numberOfSampleFrames )
            /*...mrmlj...*/const_cast<InputWaveFile &>( *this ).restart();
    }
    return true;
}

LE_NOTHROW bool InputWaveFile::readSilencePadded( float * pOutput, std::uint32_t numberOfSampleFrames ) const
{
    auto const readSamples( read( pOutput, numberOfSampleFrames ) );
    if ( !readSamples ) return false;
    numberOfSampleFrames -= readSamples;
    pOutput              += readSamples * numberOfChannels();
    std::fill_n( pOutput, numberOfSampleFrames * numberOfChannels(), 0.0f );
    return true;
}

template <Utility::SpecialLocations parentDirectory>
error_msg_t InputWaveFile::open( char const * const fileName ) { return impl( *this ).open<parentDirectory>( fileName ); }

template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::AbsolutePath   >( char const * );
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::AppData        >( char const * );
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::Documents      >( char const * );
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::Library        >( char const * );
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::Resources      >( char const * );
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::ExternalStorage>( char const * );
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::Temporaries    >( char const * );
#ifdef __ANDROID__
template error_msg_t LE_FASTCALL_ABI InputWaveFile::open<Utility::ToolOutput     >( char const * );
#endif // __ANDROID__

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
