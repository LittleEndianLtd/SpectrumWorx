////////////////////////////////////////////////////////////////////////////////
///
/// \file inputWaveFile.hpp
/// -----------------------
///
/// Copyright (c) 2012 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef inputWaveFile_hpp__CAE81353_A5A7_45C1_89A5_63EE5B8DF212
#define inputWaveFile_hpp__CAE81353_A5A7_45C1_89A5_63EE5B8DF212
#pragma once
//------------------------------------------------------------------------------
#include <le/utility/abi.hpp>
#ifndef _MSC_VER
#include <le/utility/filesystem.hpp>
#endif // _MSC_VER
#include <le/utility/pimpl.hpp>

#include <cstdint>
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

typedef char const * error_msg_t; ///< A static error message string.

////////////////////////////////////////////////////////////////////////////////
///
/// \class InputWaveFile
///
/// \brief A lightweight portable class for reading uncompressed PCM WAVE files
/// from the filesystem.
///
////////////////////////////////////////////////////////////////////////////////

class InputWaveFile
#ifndef DOXYGEN_ONLY
    :
    public Utility::StackPImpl
    <
        InputWaveFile,
    #ifdef __ANDROID__
        7 * sizeof( void * )
    #else
        6 * sizeof( void * )
    #endif // __ANDROID__
    >
#endif // DOXYGEN_ONLY
{
public:
    LE_NOTHROWNOALIAS  InputWaveFile();
    LE_NOTHROW         InputWaveFile( InputWaveFile && );
    LE_NOTHROWNOALIAS ~InputWaveFile(); ///< \details Implicitly calls close().

    /// <B>Effect:</B> Opens the file pointed to by <VAR>relativePathToFile</VAR> within/relative to <VAR>rootLocation</VAR>.<BR>
    /// The function can be called many times, i.e. one instance of InputWaveFile can be used to read as many WAVE files as desired. Note that the function offers only the basic (error safety) guarantee.<BR>
    /// <B>Postconditions:</B> Unless an error is reported, the file was successfully opened and its header parsed making it ready for audio and metadata extraction.<BR>
    /// <B>Effect:</B> Searches for <VAR>relativePathToFile</VAR> in <VAR>rootLocation</VAR> and, if found, attempts to open the file for reading.<BR>
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI open( char const * relativePathToFile );
#ifdef _WIN32
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI open( wchar_t const * relativePathToFile );
#endif // _WIN32

    /// <B>Effect:</B> Closes the open file (if any).<BR>
    LE_NOTHROW void LE_FASTCALL_ABI close();

    /// <B>Effect:</B> Reads up to <VAR>numberOfSampleFrames</VAR> * numberOfChannels() interleaved samples into <VAR>pOutput</VAR> and adjusts the current file position (as reported by remainingSamples()).<BR>
    /// <B>Preconditions:</B>
    ///     - a successful open() call
    ///     - the buffer pointed to by pOutput must be large enough to hold <VAR>numberOfSampleFrames</VAR> * numberOfChannels() samples.
    /// \return Number of sample frames actually read (always <= <VAR>numberOfSampleFrames</VAR>)
    LE_NOTHROW        std::uint32_t  LE_FASTCALL_ABI read             ( float * pOutput, std::uint32_t numberOfSampleFrames ) const;

    /// <B>Effect:</B> Reads (<VAR>numberOfSampleFrames</VAR> *
    /// numberOfChannels()) interleaved samples into <VAR>pOutput</VAR>. If the
    /// file is not long enough, it is looped until the requested number of
    /// samples is retrieved.<BR>
    /// <B>Preconditions:</B> the same as for the read() member function.
    /// \return True for a successful operation, false otherwise.
    LE_NOTHROW        bool           LE_FASTCALL_ABI readLooped       ( float * pOutput, std::uint32_t numberOfSampleFrames ) const;
    /// <B>Effect:</B> Reads (<VAR>numberOfSampleFrames</VAR> *
    /// numberOfChannels()) interleaved samples into <VAR>pOutput</VAR>. If the
    /// file is not long enough, the remaining output is filled with zeroes.<BR>
    /// <B>Preconditions:</B> the same as for the read() member function.
    /// \return True for a successful operation, false otherwise.
    LE_NOTHROW        bool           LE_FASTCALL_ABI readSilencePadded( float * pOutput, std::uint32_t numberOfSampleFrames ) const;

    LE_NOTHROW LE_PURE_FUNCTION std::uint8_t  LE_FASTCALL_ABI numberOfChannels     () const; ///< Number of channels reported by the WAVE file. <BR> \details <B>Preconditions:</B> A successful open() call.
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI sampleRate           () const; ///< Sample rate reported by the WAVE file. <BR> \details <B>Preconditions:</B> A successful open() call.
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI remainingSampleFrames() const; ///< Number of sample frames remaining/not yet read from the WAVE file. <BR> \details <B>Preconditions:</B> A successful open() call.
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI lengthInSampleFrames () const; ///< Total number of sample frames in the currently opened WAVE file. <BR> \details <B>Preconditions:</B> A successful open() call.

    LE_NOTHROW                  void          LE_FASTCALL_ABI setTimePosition  ( std::uint32_t positionInMilliseconds ); ///< Seek to a specific position measured in milliseconds. <BR> \details <B>Preconditions:</B> <BR> - a successful open() call <BR> - <VAR>positionInMilliseconds</VAR> must be valid.
    LE_NOTHROW                  void          LE_FASTCALL_ABI setSamplePosition( std::uint32_t positionInSampleFrames ); ///< Seek to a specific position measured in sample frames. <BR> \details <B>Preconditions:</B> <BR> - a successful open() call <BR> - <VAR>positionInSampleFrames</VAR> must be valid.

    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getTimePosition  () const; ///< Retrieve current position (in milliseconds).
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getSamplePosition() const; ///< Retrieve current position (in samples).

    LE_NOTHROW void LE_FASTCALL_ABI restart(); ///< (Re)Start reading from the beginning.

#if !( defined( _MSC_VER ) && ( _MSC_VER < 1800 ) )
    explicit
#endif // old MSVC
    LE_NOTHROW LE_PURE_FUNCTION LE_FASTCALL_ABI operator bool() const;

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
    InputWaveFile( InputWaveFile const & );
#else
    InputWaveFile( InputWaveFile const & ) = delete;
#endif
}; // class InputWaveFile

/// @} // AudioIO group

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // inputWaveFile_hpp
