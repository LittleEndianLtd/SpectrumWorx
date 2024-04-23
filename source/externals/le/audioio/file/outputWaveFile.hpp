////////////////////////////////////////////////////////////////////////////////
///
/// \file outputWaveFile.hpp
/// ------------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef outputWaveFile_hpp__20E1BC50_AD13_436D_B244_5D8FB26FD8EA
#define outputWaveFile_hpp__20E1BC50_AD13_436D_B244_5D8FB26FD8EA
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/abi.hpp"
#ifndef _MSC_VER
#include "le/utility/filesystem.hpp"
#endif // _MSC_VER
#include "le/utility/pimpl.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
#ifdef _MSC_VER
namespace Utility { enum SpecialLocations; }
#endif // _MSC_VER
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

/// \addtogroup AudioIO
/// @{

typedef char const * error_msg_t;

////////////////////////////////////////////////////////////////////////////////
///
/// \class OutputWaveFile
///
/// \brief A lightweight portable class for creating and writing uncompressed
/// WAVE_FORMAT_PCM WAVE files.
///
////////////////////////////////////////////////////////////////////////////////

class OutputWaveFile
#ifndef DOXYGEN_ONLY
    :
    public Utility::StackPImpl<OutputWaveFile, 18 * sizeof( std::uint32_t )>
#endif // DOXYGEN_ONLY
{
public:
    LE_NOTHROW  OutputWaveFile();
    LE_NOTHROW  OutputWaveFile( OutputWaveFile && );
    LE_NOTHROW ~OutputWaveFile(); ///< \details Implicitly calls close().

    /// <B>Effect:</B> Creates a file at <VAR>pathToFile</VAR> for writing.<BR>
    /// The function can be called many times, i.e. one instance of OutputWaveFile can be used to create and write as many WAVE files as desired. Note that the function offers only the basic (error safety) guarantee.<BR>
    /// <B>Postconditions:</B> Unless an error is reported, the file was successfully created with preallocated space for the header/metadata. Implicitly closes any previously possibly open file.<BR>
    /// \tparam rootLocation The filesystem location at which to create the desired file. @see Utility::SpecialLocations
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI create( char const * pathToFile, std::uint8_t numberOfChannels, std::uint32_t sampleRate );

    /// <B>Effect:</B> Writes out the WAVE header and closes the file.<BR>
    LE_NOTHROW void LE_FASTCALL_ABI close();

    /// <B>Effect:</B> Writes interleaved <VAR>numberOfSampleFrames</VAR> * <VAR>numberOfChannels</VAR> samples from <VAR>pInput</VAR> into the underlying file.<BR>
    /// <B>Preconditions:</B>
    ///     - a successful create() call
    ///     - the buffer pointed to by pInput must hold at least <VAR>numberOfSampleFrames</VAR> * <VAR>numberOfChannels</VAR> samples.
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI write( float const * pInput, std::uint32_t numberOfSampleFrames );

    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getTimePosition  () const;
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getSamplePosition() const;

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
    OutputWaveFile( OutputWaveFile const & );
#else
    OutputWaveFile( OutputWaveFile const & ) = delete;
#endif
}; // class OutputWaveFile


////////////////////////////////////////////////////////////////////////////////
///
/// \class OutputWaveFileAsync
///
/// \brief An OutputWaveFile clone for asynchronous file operations
/// \details The write() member function is non-blocking (it copies and
/// schedules the passed data to a background worker thread) and thus safe to
/// use in real time (audio rendering) threads/callbacks.
///
/// \nosubgrouping
///
////////////////////////////////////////////////////////////////////////////////

class OutputWaveFileAsync
#ifndef DOXYGEN_ONLY
    :
    public Utility::StackPImpl
    <
        OutputWaveFileAsync,
        sizeof( OutputWaveFile ) +
        #if defined( __ANDROID__ ) && __LP64__
            33 * sizeof( std::uint32_t ) + 30 * sizeof( void * )
        #elif defined( __ANDROID__ ) && !defined( __i386__ )
            33 * sizeof( std::uint32_t ) + 28 * sizeof( void * )
        #elif defined( __APPLE__ )
             1 * sizeof( std::uint16_t ) +  1 * sizeof( void * )
        #else
            33 * sizeof( std::uint32_t ) + 21 * sizeof( void * )
        #endif
    >
#endif // DOXYGEN_ONLY
{
public:
    LE_NOTHROW  OutputWaveFileAsync();
    LE_NOTHROW  OutputWaveFileAsync( OutputWaveFileAsync && );
    LE_NOTHROW ~OutputWaveFileAsync(); ///< \details Implicitly calls close().

    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI create( char const * pathToFile, std::uint8_t numberOfChannels, std::uint32_t sampleRate );

    /// <B>Effect:</B> Writes out the WAVE header and closes the file
    /// <B>synchronously</B> (the file equivalent of a thread join).<BR>
    /// \return True for a successful operation, false if any of the write()
    /// calls (since the last successfull create() call) asynchronously failed.
    LE_NOTHROW bool LE_FASTCALL_ABI close();

    LE_NOTHROW error_msg_t LE_FASTCALL_ABI write( float const * pInput, std::uint16_t numberOfSampleFrames );

    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getTimePosition  () const;
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getSamplePosition() const;

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
    OutputWaveFileAsync( OutputWaveFileAsync const & );
#else
    OutputWaveFileAsync( OutputWaveFileAsync const & ) = delete;
#endif
}; // class OutputWaveFileAsync

/// @} // group AudioIO

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // outputWaveFile_hpp
