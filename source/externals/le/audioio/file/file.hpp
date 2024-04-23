////////////////////////////////////////////////////////////////////////////////
///
/// \file file.hpp
/// --------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef file_hpp__6A9131FE_0E31_467F_9A30_C207F86E591E
#define file_hpp__6A9131FE_0E31_467F_9A30_C207F86E591E
//------------------------------------------------------------------------------
#include "le/utility/abi.hpp"
#ifndef _MSC_VER
#include "le/utility/filesystem.hpp"
#endif // _MSC_VER
#include "le/utility/pimpl.hpp"

#include <cstdint>
#include <vector>
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
/// \class File
///
/// \brief Reads audio files in different formats from the filesystem. The list
/// of supported audio formats depends on the underlying OS.
///
/// \note <B>Android specific:</B> Use of this class requires Android 4.0 (API
/// level 14) or later.
///
////////////////////////////////////////////////////////////////////////////////

class File
#ifndef DOXYGEN_ONLY
    :
    public Utility::StackPImpl
    <
        File,
    #if defined( __ANDROID__ )
        8 * sizeof( int ) + 6 * sizeof( void * )
        #if __LP64__
            + 36 + 2 * 44
        #endif
    #elif defined( _WIN32 )
        22 * sizeof( void * ) + 26 * sizeof( int ) + sizeof( bool )
    #else
        5 * sizeof( void * )
    #endif // OS
    >
#endif // DOXYGEN_ONLY
{
public:
    LE_NOTHROWNOALIAS  File();
    LE_NOTHROW         File( File && other );
    LE_NOTHROW        ~File(); ///< \details Implicitly calls close().

    /// <B>Effect:</B> Opens the file pointed to by <VAR>relativePathToFile</VAR> within/relative to <VAR>rootLocation</VAR>.<BR>
    /// The function can be called many times, i.e. one File instance can be used to read as many audio files as desired. Note that the function offers only the basic (error safety) guarantee.<BR>
    /// <B>Postconditions:</B> Unless an error is reported, the file was successfully opened and its header parsed making it ready for audio and metadata extraction.<BR>
    /// \tparam rootLocation The filesystem location at which to look for
    ///         <VAR>relativePathToFile</VAR>. @see Utility::SpecialLocations
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI open( char    const * relativePathToFile );
#ifdef _WIN32
    template <Utility::SpecialLocations rootLocation>
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI open( wchar_t const * relativePathToFile );
#endif // _WIN32

    LE_NOTHROW void LE_FASTCALL_ABI close(); ///< \details <B>Effect:</B> Closes the open file (if any).<BR>

    /// <B>Effect:</B> Reads up to <VAR>numberOfSampleFrames</VAR> * numberOfChannels() interleaved samples into <VAR>pOutput</VAR> and adjusts the current file position (as reported by remainingSamples()).<BR>
    /// <B>Preconditions:</B>
    ///     - a successful open() call
    ///     - the buffer pointed to by pOutput must be large enough to hold <VAR>numberOfSampleFrames</VAR> * numberOfChannels() samples.<BR>
    /// \return Number of sample frames actually read (0 on failure, always <= <VAR>numberOfSampleFrames</VAR>).
    ///
    /// .
    ///
    /// \note
    /// It is important to check the return value of this function because
    /// it may be less than the requested number of sample frames. Beyond the
    /// usual EOF condition this may also happen because the values reported by
    /// the lengthInSamples() and remainingSamples() member functions are only
    /// approximate. This slight uncertainty in the audio file length
    /// calculation is due to two causes:
    /// - the File class supports reading from a multitude of file
    ///   formats some of which do not support efficient yet exact seeking or
    ///   duration extraction (e.g. VBR MP3s)
    /// - rounding errors in conversions between durations/time stamps expressed
    ///   in samples and milliseconds.
    /// This discrepancy between the requested and returned/read
    /// <VAR>numberOfSampleFrames</VAR> should only occur at the very end of a
    /// file (or when reading the entire file at once, of course).
    LE_NOTHROW        std::uint32_t LE_FASTCALL_ABI read             ( float * pOutput, std::uint32_t numberOfSampleFrames ) const;

    /// <B>Effect:</B> Reads (<VAR>numberOfSampleFrames</VAR> *
    /// numberOfChannels()) interleaved samples into <VAR>pOutput</VAR>. If the
    /// file is not long enough, it is looped until the requested number of
    /// samples is retrieved.<BR>
    /// <B>Preconditions:</B> the same as for the read() member function.
    /// \return True for a successful operation, false otherwise.
    LE_NOTHROW        bool          LE_FASTCALL_ABI readLooped       ( float * pOutput, std::uint32_t numberOfSampleFrames ) const;
    /// <B>Effect:</B> Reads (<VAR>numberOfSampleFrames</VAR> *
    /// numberOfChannels()) interleaved samples into <VAR>pOutput</VAR>. If the
    /// file is not long enough, the remaining output is filled with zeroes.<BR>
    /// <B>Preconditions:</B> the same as for the read() member function.
    /// \return True for a successful operation, false otherwise.
    LE_NOTHROW        bool          LE_FASTCALL_ABI readSilencePadded( float * pOutput, std::uint32_t numberOfSampleFrames ) const;

    LE_NOTHROW LE_PURE_FUNCTION std::uint8_t  LE_FASTCALL_ABI numberOfChannels     () const; ///< Number of channels reported by the OS decoder for the audio file. <BR> \details <B>Preconditions:</B> A successful open() call.
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI sampleRate           () const; ///< Sample rate reported by the OS decoder for the audio file. <BR> \details <B>Preconditions:</B> A successful open() call.
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI remainingSampleFrames() const; ///< Approximate number of sample frames remaining/not yet read from the audio file as reported by the OS decoder. <BR> \details <B>Preconditions:</B> A successful open() call.
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI lengthInSampleFrames () const; ///< Approximate total number of sample frames in the currently opened file. <BR> \details <B>Preconditions:</B> A successful open() call.

    LE_NOTHROW                  void          LE_FASTCALL_ABI setTimePosition  ( std::uint32_t positionInMilliseconds ); ///< Seek to a specific position measured in milliseconds. <BR> \details Not guaranteed to be sample accurate (especially on Android).<BR> <B>Preconditions:</B> <BR> - a successful open() call <BR> - <VAR>positionInMilliseconds</VAR> must be valid.
    LE_NOTHROW                  void          LE_FASTCALL_ABI setSamplePosition( std::uint32_t positionInSampleFrames ); ///< Seek to a specific position measured in sample frames. <BR> \details Not guaranteed to be sample accurate (especially on Android).<BR> <B>Preconditions:</B> <BR> - a successful open() call <BR> - <VAR>positionInSampleFrames</VAR> must be valid.

    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getTimePosition  () const; ///< Retrieve current position (in milliseconds).
    LE_NOTHROW LE_PURE_FUNCTION std::uint32_t LE_FASTCALL_ABI getSamplePosition() const; ///< Retrieve current position (in samples).

    LE_NOTHROW void LE_FASTCALL_ABI restart(); ///< (Re)Start reading from the beginning (equivalent to set*Position( 0 )).

#if !( defined( _MSC_VER ) && ( _MSC_VER < 1800 ) )
    explicit
#endif // old MSVC
    LE_NOTHROW LE_PURE_FUNCTION LE_FASTCALL_ABI operator bool() const;

    /// \brief A semicolon separated list of supported audio file formats/containers.
    /// \details
    ///     - Android (4.0+): *.3gp;*.mp4;*.m4a;*.aac;*.ts;*.flac;*.mp3;*.mid;*.xmf;*.mxmf;*.rtttl;*.rtx;*.ota;*.imy;*.ogg;*.mkv;*.wav
    ///     - iOS & OSX     : *.aac;*.ac3;*.adts;*.aif;*.aiff;*.au;*.caf;*.mpa;*.mp3;*.mp4;*.m4a;*.sd2;*.snd;*.wav
    ///     - Windows       : *.aac;*.aif;*.aiff;*.au;*.mpa;*.mp3;*.snd;*.wav;*.wma
    static char const supportedFormats[];

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
     File( File const & );
#else
     File( File const & ) = delete;
#endif // _MSC_VER
}; // class File


class InputWaveFile;
/// \details Compares the audio signal format (sample rate and number of channels)
/// of two audio files.
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( File          const &, File          const & );
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( File          const &, InputWaveFile const & ); ///< \overload
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( InputWaveFile const &, File          const & ); ///< \overload
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( InputWaveFile const &, InputWaveFile const & ); ///< \overload

/// @} // AudioIO group

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // file_hpp
