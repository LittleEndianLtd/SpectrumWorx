////////////////////////////////////////////////////////////////////////////////
///
/// \file inputWaveFileImpl.hpp
/// ---------------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef inputWaveFileImpl_hpp__8B0AAB74_7E4B_4DDE_9CC8_8498FD30A480
#define inputWaveFileImpl_hpp__8B0AAB74_7E4B_4DDE_9CC8_8498FD30A480
#pragma once
//------------------------------------------------------------------------------
#include "inputWaveFile.hpp"
#include "structures.hpp"

#include "le/utility/filesystem.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"

#ifdef __ANDROID__
#include <boost/variant/variant.hpp>
#endif // __ANDROID__

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

class InputWaveFileImpl
{
public:
    InputWaveFileImpl() : pFormat_( nullptr ) {}
    InputWaveFileImpl( InputWaveFileImpl && );

    template <Utility::SpecialLocations rootLocation>
    error_msg_t LE_FASTCALL open( char const * relativePathToFile );

    void close() { mappedFile_ = InputWaveFileImpl::MappedWAVE(); }

    std::uint32_t LE_FASTCALL read( float * pOutput, std::uint32_t samples ) const;

    std::uint8_t  numberOfChannels() const;
    std::uint32_t sampleRate      () const;
    std::uint32_t remainingBytes  () const;
    std::uint32_t remainingSamples() const;
    std::uint32_t lengthInSamples () const;

    LE_COLD
    void restart() { pData_ = pDataBegin_; }
    LE_COLD
    void setTimePosition( std::uint32_t const positionInMilliseconds ) { setSamplePosition( milliseconds2samples( positionInMilliseconds ) ); }
    LE_COLD
    void setSamplePosition( std::uint32_t const positionInSampleFrames ) { pData_ = pDataBegin_ + numberOfChannels() * positionInSampleFrames; }
    LE_COLD
    std::uint32_t getTimePosition() const { return samples2milliseconds( getSamplePosition() ); }
    LE_COLD
    std::uint32_t getSamplePosition() const
    {
        BOOST_ASSERT_MSG( pFormat_->Format.nBlockAlign == ( pFormat_->Format.wBitsPerSample / 8 * numberOfChannels() ), "Unexpected sample frame size" );
        return static_cast<std::uint32_t>( pData_ - pDataBegin_ ) / pFormat_->Format.nBlockAlign;
    }

    explicit operator bool() const { return pFormat_ != nullptr; }

private:
    char const * open( void const * mappedFileData, std::uint32_t fileSize );

    std::uint32_t remainingSampleFramesFrom( char const * const pDataBegin ) const
    {
        BOOST_ASSERT_MSG( pFormat_->Format.nBlockAlign == ( pFormat_->Format.wBitsPerSample / 8 * numberOfChannels() ), "Unexpected sample frame size" );
        return static_cast<std::uint32_t>( pDataEnd_ - pDataBegin ) / pFormat_->Format.nBlockAlign;
    }

    //...mrmlj...copied from fileAndroid.cpp
    LE_COLD std::uint32_t LE_FASTCALL milliseconds2samples( std::uint32_t const milliseconds ) const { return static_cast<std::uint32_t>( std::uint64_t( milliseconds ) * sampleRate() / 1000 ); }
    LE_COLD std::uint32_t LE_FASTCALL samples2milliseconds( std::uint32_t const sampleFrames ) const { return static_cast<std::uint32_t>( std::uint64_t( sampleFrames ) * 1000 / sampleRate() ); }

private:
            Detail::WAVEFORMATEXTENSIBLE const * LE_RESTRICT pFormat_   ;
    mutable char                         const *             pData_     ;
            char                         const * LE_RESTRICT pDataBegin_;
            char                         const * LE_RESTRICT pDataEnd_  ;

#ifdef __ANDROID__
    using MappedWAVE = boost::variant
    <
        Utility::File        ::MemoryMapping,
        Utility::ResourceFile::MemoryMapping
    >;
#else
    using MappedWAVE = Utility::File::MemoryMapping;
#endif // __ANDROID__

    MappedWAVE mappedFile_;
}; // class InputWaveFileImpl

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // inputWaveFileImpl_hpp
