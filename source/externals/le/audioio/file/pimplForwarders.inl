////////////////////////////////////////////////////////////////////////////////
///
/// pimplForwarders.inl
/// -------------------
///
/// Shared pimpl forwarders for all AudioIO::File implementations
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
#ifdef pimplForwarders_inl__EDC3CCE9_5449_466E_9CF3_2EEEAD2FF0CD
#error This file should be included only once per module
#else
#define pimplForwarders_inl__EDC3CCE9_5449_466E_9CF3_2EEEAD2FF0CD
#endif // pimplForwarders_inl
//------------------------------------------------------------------------------
#include "inputWaveFile.hpp"

#include "le/utility/pimplPrivate.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility { template <> struct Implementation<AudioIO::File> { using type = AudioIO::FileImpl; }; }
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( File          const & left, File          const & right ) { return left.numberOfChannels() == right.numberOfChannels() && left.sampleRate() == right.sampleRate(); }
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( InputWaveFile const & left, InputWaveFile const & right ) { return left.numberOfChannels() == right.numberOfChannels() && left.sampleRate() == right.sampleRate(); }
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( File          const & left, InputWaveFile const & right ) { return left.numberOfChannels() == right.numberOfChannels() && left.sampleRate() == right.sampleRate(); }
LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI equalFormats( InputWaveFile const & left, File          const & right ) { return equalFormats( right, left ); }

////////////////////////////////////////////////////////////////////////////////
// PImpl forwarders
////////////////////////////////////////////////////////////////////////////////

static void verifyOpened( File const & file ) { BOOST_ASSERT_MSG( file, "No file opened." ); (void)file; }

LE_NOTHROW std::uint32_t File::read( float * const pOutput, std::uint32_t const samples ) const { verifyOpened( *this ); return impl( *this ).read( pOutput, samples ); }

LE_COLD LE_NOTHROW LE_PURE_FUNCTION std::uint8_t  File::numberOfChannels     () const { verifyOpened( *this ); return impl( *this ).numberOfChannels     (); }
LE_COLD LE_NOTHROW LE_PURE_FUNCTION std::uint32_t File::sampleRate           () const { verifyOpened( *this ); return impl( *this ).sampleRate           (); }
LE_COLD LE_NOTHROW LE_PURE_FUNCTION std::uint32_t File::remainingSampleFrames() const { verifyOpened( *this ); return impl( *this ).remainingSampleFrames(); }
LE_COLD LE_NOTHROW LE_PURE_FUNCTION std::uint32_t File::lengthInSampleFrames () const { verifyOpened( *this ); return impl( *this ).lengthInSampleFrames (); }

LE_COLD LE_NOTHROW LE_PURE_FUNCTION std::uint32_t File::getTimePosition  (                                            ) const { return impl( *this ).getTimePosition  (                        ); }
LE_COLD LE_NOTHROW LE_PURE_FUNCTION std::uint32_t File::getSamplePosition(                                            ) const { return impl( *this ).getSamplePosition(                        ); }
LE_COLD LE_NOTHROW                  void          File::setTimePosition  ( std::uint32_t const positionInMilliseconds )       { return impl( *this ).setTimePosition  ( positionInMilliseconds ); }
LE_COLD LE_NOTHROW                  void          File::setSamplePosition( std::uint32_t const positionInSampleFrames )       { return impl( *this ).setSamplePosition( positionInSampleFrames ); }
LE_COLD LE_NOTHROW                  void          File::restart          (                                            )       { return impl( *this ).restart          (                        ); }
LE_COLD LE_NOTHROW                  void          File::close            (                                            )       { return impl( *this ).close            (                        ); }

LE_COLD LE_NOTHROW LE_PURE_FUNCTION               File::operator bool() const { return impl( *this ).operator bool(); }

LE_COLD LE_NOTHROWNOALIAS File:: File() {}
LE_COLD LE_NOTHROW        File::~File() {}
LE_COLD LE_NOTHROW        File:: File( File && other ) : ConcreteStackPimpl( std::move( other ) ) {}

LE_NOTHROW bool LE_FASTCALL_ABI File::readLooped( float * pOutput, std::uint32_t numberOfSampleFrames ) const
{
    while ( numberOfSampleFrames )
    {
        auto const readSamples( read( pOutput, numberOfSampleFrames ) );
        LE_ASSUME( readSamples <= numberOfSampleFrames );
        if ( readSamples )
        {
            numberOfSampleFrames -= readSamples;
            pOutput              += readSamples * numberOfChannels();
        }
        else
            /*...mrmlj...*/const_cast<File &>( *this ).restart();
    }
    return true;
}

LE_NOTHROW bool LE_FASTCALL_ABI File::readSilencePadded( float * pOutput, std::uint32_t numberOfSampleFrames ) const
{
    auto const readSamples( read( pOutput, numberOfSampleFrames ) );
    if ( !readSamples ) return false;
    numberOfSampleFrames -= readSamples;
    pOutput              += readSamples * numberOfChannels();
    std::memset( pOutput, 0, numberOfSampleFrames * numberOfChannels() * sizeof( *pOutput ) );
    return true;
}

template <Utility::SpecialLocations parentDirectory>
LE_COLD error_msg_t File::open( char const * const fileName ) { return impl( *this ).open<parentDirectory>( fileName ); }

template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::AbsolutePath   >( char const * );
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::AppData        >( char const * );
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::Documents      >( char const * );
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::Library        >( char const * );
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::Resources      >( char const * );
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::ExternalStorage>( char const * );
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::Temporaries    >( char const * );
#ifdef __ANDROID__
template LE_COLD error_msg_t LE_FASTCALL_ABI File::open<Utility::ToolOutput     >( char const * );
#endif // __ANDROID__

#ifdef _WIN32
template <Utility::SpecialLocations parentDirectory>
error_msg_t File::open( wchar_t const * const fileName ) { return impl( *this ).open<parentDirectory>( fileName ); }

template error_msg_t LE_FASTCALL_ABI File::open<Utility::AbsolutePath   >( wchar_t const * );
template error_msg_t LE_FASTCALL_ABI File::open<Utility::AppData        >( wchar_t const * );
template error_msg_t LE_FASTCALL_ABI File::open<Utility::Documents      >( wchar_t const * );
template error_msg_t LE_FASTCALL_ABI File::open<Utility::Library        >( wchar_t const * );
template error_msg_t LE_FASTCALL_ABI File::open<Utility::Resources      >( wchar_t const * );
template error_msg_t LE_FASTCALL_ABI File::open<Utility::ExternalStorage>( wchar_t const * );
template error_msg_t LE_FASTCALL_ABI File::open<Utility::Temporaries    >( wchar_t const * );
#endif // _WIN32

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
