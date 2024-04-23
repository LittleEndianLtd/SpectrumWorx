////////////////////////////////////////////////////////////////////////////////
///
/// fileWindows.cpp
/// ---------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//
// DirectShow
// https://msdn.microsoft.com/en-us/library/jj556183.aspx DirectShow Decoder Filter for Windows Embedded Compact
// http://msdn.microsoft.com/en-us/library/dd377513(VS.85).aspx
// http://msdn.microsoft.com/en-us/library/dd389098(VS.85).aspx
// http://support.microsoft.com/kb/316992
// http://www.google.com/search?hl=hr&rls=com.microsoft:hr&q=CLSID_FilterGraphNoThread&start=10&sa=N
// http://social.msdn.microsoft.com/Forums/en-US/windowsdirectshowdevelopment/thread/dfa87cfd-ad30-47c4-89a9-d0df79ea5f1e/
// http://msdn.microsoft.com/en-us/library/dd375786(VS.85).aspx
// http://www.codeproject.com/KB/mobile/samplegrabberfilter-wm6.aspx
// http://social.msdn.microsoft.com/Forums/windowsdesktop/en-US/d0197f45-540d-4281-8b35-4a9e66723c56/isamplegrabber-deprecated-where-can-i-find-alternatives?forum=windowsdirectshowdevelopment
// http://www.codeproject.com/KB/audio-video/dshowencoder.aspx
// http://www.codeproject.com/KB/audio-video/Tanvon_DirectShowFilters.aspx
// http://doc.51windows.net/Directx9_SDK/?url=/Directx9_SDK/htm/grabbersamplefiltersample.htm
// http://social.msdn.microsoft.com/Forums/en-US/windowsdirectshowdevelopment/thread/dfa87cfd-ad30-47c4-89a9-d0df79ea5f1e
//
// MediaFoundation
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd940436(v=vs.85).aspx
// http://blogs.msdn.com/b/chuckw/archive/2010/08/13/quot-who-moved-my-windows-media-cheese-quot.aspx :/
//
// COM
// The Rules of the Component Object Model https://msdn.microsoft.com/en-us/library/ms810016.aspx
// https://groups.google.com/forum/#!topic/microsoft.public.win32.programmer.directx.video/eEVFGIBCwlI
// https://web.archive.org/web/20050311085154/http://msdn.microsoft.com/library/en-us/dnesscom/html/objectsinterfacesapartments.asp
// https://web.archive.org/web/20031010111526/http://msdn.microsoft.com/library/en-us/dnesscom/html/c5interfaceimplementationrevisited.asp
//
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "file.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"

#include "le/utility/conditionVariable.hpp"
#include "le/utility/filesystem.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <boost/assert.hpp>

#include <windows.h>
#undef min
#undef max

#include <tchar.h>

//...mrmlj...testing...work in progress...
// http://www.indexcom.com/tech/WindowsAudioSRC
//#define LE_USE_RESAMPLER_DMO

#pragma warning( push )
#pragma warning( disable : 4201 ) // Nonstandard extension used : nameless struct/union.
#include <mmreg.h>
#pragma warning( pop )
#include <strmif.h>

#pragma warning( push )
#pragma warning( disable : 4201 ) // Nonstandard extension used : nameless struct/union.
#include <austream.h>
#include <mmstream.h>

#include <amstream.h>
#include <control.h>
#include <dshow.h>
#include <uuids.h>
#ifdef LE_USE_RESAMPLER_DMO
    #include <dmoreg.h> // DMOCATEGORY_AUDIO_EFFECT
    #include <dmodshow.h> // DMO wrapper
    #include <mftransform.h>
    #include <wmcodecdsp.h> // Audio Resampler DSP
#endif // LE_USE_RESAMPLER_DMO
#pragma warning( pop )

#include <array>
#ifndef NDEBUG
#include <atomic>
#endif // NDEBUG
#include <cstdint>
//------------------------------------------------------------------------------
#ifdef LE_USE_RESAMPLER_DMO
    #pragma comment( lib, "dmoguids.lib" ) // CLSID_DMOWrapperFilter
    #pragma comment( lib, "mfuuid.lib"   )
    GUID const CLSID_CResamplerMediaObject = { 0xF447B69E, 0x1884, 0x4A7E, 0x80, 0x55, 0x34, 0x6F, 0x74, 0xD6, 0xED, 0xB3 };
#endif // LE_USE_RESAMPLER_DMO

#pragma comment( lib, "strmiids.lib" )
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \class COMPtr
/// \brief A smart COM pointer wrapper with scoped_ptr semantics. Used instead
/// of the ATL provided one to avoid the dependency on ATL (not available in the
/// express Visual Studio editions).
////////////////////////////////////////////////////////////////////////////////

template <class T>
class COMPtr : boost::noncopyable
{
private:
    class LE_NOVTABLE NoAddRefReleaseProxy : public T
    {
    private:
        ULONG STDMETHODCALLTYPE AddRef () LE_OVERRIDE;
        ULONG STDMETHODCALLTYPE Release() LE_OVERRIDE;
    };

public:
             COMPtr(              ) : pT_( nullptr ) {}
    explicit COMPtr( T * const pT ) : pT_( pT      ) {}

    LE_NOTHROWNOALIAS COMPtr( IUnknown     &  source ) { BOOST_VERIFY( source.QueryInterface( __uuidof( T ), reinterpret_cast<void * *>( &pT_ ) ) == S_OK ); }
    LE_NOTHROWNOALIAS COMPtr( COMPtr       && other  ) : pT_( other.pT_ ) { other.pT_ = nullptr; }
    LE_NOTHROWNOALIAS COMPtr( COMPtr const &  other  ) : pT_( other.pT_ ) { if ( pT ) pT_->AddRef(); }

    LE_NOTHROWNOALIAS ~COMPtr() { release(); }

    NoAddRefReleaseProxy * operator->() const
    {
        BOOST_ASSERT( pT_ != nullptr );
        return static_cast<NoAddRefReleaseProxy *>( pT_ );
    }

    T * * operator&()
    {
        BOOST_ASSERT( pT_ == nullptr );
        return &pT_;
    }

    operator T *() const { return pT_; }

    HRESULT createInstance( CLSID const & classID )
    {
        BOOST_ASSERT( pT_ == nullptr );
        return ::CoCreateInstance( classID, nullptr, CLSCTX_ALL, __uuidof( T ), reinterpret_cast<void * *>( &pT_ ) );
    }

    template <class Q>
    HRESULT queryInterface( COMPtr<Q> & pTarget ) const
    {
        BOOST_ASSERT( pT_ != nullptr );
        return pT_->QueryInterface( __uuidof( Q ), reinterpret_cast<void * *>( &pTarget ) );
    }

    void reset( T * const pNewObject = nullptr )
    {
        release();
        pT_ = pNewObject;
    }

    explicit operator bool() const { return pT_ != nullptr; }

private:
    void release() { if ( pT_ ) pT_->Release(); }

private:
    T * LE_RESTRICT pT_;
}; // class COMPtr<>


////////////////////////////////////////////////////////////////////////////////
///
/// \class FileImpl
///
/// \brief Loads a sample file using DirectShow.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   The current implementation implements all required interfaces in a single
// class instead of properly modeling the design with different classes/objects.
// This is maybe slightly more efficient but is rather ugly and should be fixed
// when the time comes.
//                                            (18.06.2010.) (Domagoj Saric)
////////////////////////////////////////////////////////////////////////////////

namespace
{
    ////////////////////////////////////////////////////////////////////////////
    /// \internal
    /// \class ConnectedOutputPin
    /// \brief Utility wrapper to verify that a (possibly) connected output pin
    /// was properly disconnected before a FileImpl instance gets destroyed.
    ////////////////////////////////////////////////////////////////////////////

    class ConnectedOutputPin
    {
    public:
        ConnectedOutputPin() : pConnectedOutputPin_( 0 ) {}
       ~ConnectedOutputPin() { BOOST_ASSERT( !pConnectedOutputPin_ ); }
        ConnectedOutputPin( ConnectedOutputPin const & ) = delete;
        ConnectedOutputPin( ConnectedOutputPin && other ) : pConnectedOutputPin_( other.pConnectedOutputPin_ ) { other.pConnectedOutputPin_ = nullptr; }

        operator IPin * & () { return pConnectedOutputPin_; }

        IPin * operator->() { LE_ASSUME( pConnectedOutputPin_ ); return  pConnectedOutputPin_; }
        IPin & operator *() { LE_ASSUME( pConnectedOutputPin_ ); return *pConnectedOutputPin_; }

        ConnectedOutputPin & operator=( IPin * const other ) { pConnectedOutputPin_ = other; return *this; }

    private:
        IPin * LE_RESTRICT pConnectedOutputPin_;
    }; // ConnectedOutputPin


    char const * errorMessage( HRESULT const errorCode )
    {
        // Possibly investigate AMGetErrorText(), DXGetErrorString(),
        // DXGetErrorDescription()...
        static char buffer[ 512 ] = { 0 };
        HMODULE const quartz( ::LoadLibraryA( "quartz.dll" ) );
        BOOST_ASSERT( quartz );
        BOOST_VERIFY
        (
            ::FormatMessageA
            (
                FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                quartz,
                errorCode,
                MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                buffer,
                _countof( buffer ),
                nullptr
            ) > 0
        );
        BOOST_VERIFY( ::FreeLibrary( quartz ) );
        return buffer;
    }
} // namespace anonymous

class FileImpl final
    :
    private IPin,
    private IMemInputPin,
    private IBaseFilter,
    private IEnumPins
#if 0 // for testing
   ,private IMediaSeeking
   ,private IMediaPosition
#endif // disabled
{
public:
    LE_NOTHROW  FileImpl();
    LE_NOTHROW  FileImpl( FileImpl && );
    LE_NOTHROW ~FileImpl();

    template <Utility::SpecialLocations parentDirectory>
    char const * LE_COLD open( char const * const fileName ) { return openFullPath( Utility::fullPath<parentDirectory>( fileName ) ); }

    template <Utility::SpecialLocations parentDirectory>
    char const * LE_COLD open( wchar_t const * const wideFileName )
    {
        //...mrmlj...
        std::array<char, MAX_PATH> fileName;
        BOOST_VERIFY( ::WideCharToMultiByte( CP_ACP, 0, wideFileName, -1, &fileName[ 0 ], static_cast<unsigned int>( fileName.size() ), nullptr, nullptr ) > 0 );
        return openFullPath( Utility::fullPath<parentDirectory>( &fileName[ 0 ] ) );
    }
    template <> char const * LE_COLD open<Utility::AbsolutePath>( wchar_t const * const wideFileName ) { return openFullPath( wideFileName ); }

    LE_NOTHROW void LE_COLD close()
    {
        BOOST_ASSERT( outputBuffer_.empty() );
        if ( pGraphBuilder_ )
        {
            state_ = State_Stopped; //...mrmlj...
            wakeAllWaiters();
            BOOST_VERIFY( pGraphBuilder_->Abort() == S_OK );
            BOOST_VERIFY( pMediaControl_->Stop () == S_OK );
            BOOST_ASSERT( state() == State_Stopped );
            BOOST_VERIFY( pGraphBuilder_->RemoveFilter( pSourceFilter_ ) == S_OK || !pSourceFilter_ );
            pSourceFilter_.reset();
        }
    }

    std::uint32_t LE_FASTCALL read( float * pOutput, std::uint32_t samples ) const;

    std::uint8_t  LE_FASTCALL numberOfChannels     () const { return static_cast<std::uint8_t>( dataType().format.Format.nChannels )   ; }
    std::uint32_t LE_FASTCALL sampleRate           () const { return                            dataType().format.Format.nSamplesPerSec; }
    std::uint32_t LE_FASTCALL lengthInSampleFrames () const { return timePosition2SamplePosition( duration() ); }
    std::uint32_t LE_FASTCALL remainingSampleFrames() const
    {
        std::uint32_t const unreadSamples( timePosition2SamplePosition( duration() - currentPosition() ) );
        BOOST_ASSERT( bytesToSkipFromNextBuffer_ % resolutionInBytesPerSample_ == 0 );
        std::uint16_t const samplesToSkipFromNextBuffer( bytesToSkipFromNextBuffer_ / resolutionInBytesPerSample_ );
        BOOST_ASSERT( samplesToSkipFromNextBuffer <= unreadSamples );
        return unreadSamples - samplesToSkipFromNextBuffer;
    }

    void LE_FASTCALL setTimePosition( std::uint32_t const positionInMilliseconds )
    {
        REFERENCE_TIME const position( REFERENCE_TIME( positionInMilliseconds ) * 10 * 1000 );
        setAbsolutePosition( position );
    }

    void LE_FASTCALL setSamplePosition( std::uint32_t const positionInSampleFrames )
    {
        REFERENCE_TIME const position( REFERENCE_TIME( positionInSampleFrames ) * 10 * 1000 * 1000 / sampleRate() );
        setAbsolutePosition( position );
    }

    void restart() { setAbsolutePosition( 0 ); }

    LE_NOTHROW void LE_FASTCALL setAbsolutePosition( REFERENCE_TIME const position )
    {
        verifyTimeFormat();
        REFERENCE_TIME actualPosition( position );
        BOOST_VERIFY( pMediaSeeking_->SetPositions( &actualPosition, AM_SEEKING_AbsolutePositioning | AM_SEEKING_NoFlush, nullptr, AM_SEEKING_NoPositioning | AM_SEEKING_NoFlush ) == S_OK );
        BOOST_ASSERT( actualPosition == position );
    }

    LE_NOTHROW std::uint32_t LE_FASTCALL getSamplePosition() const { return static_cast<std::uint32_t>( getAbsolutePosition() / 10 / 1000 ); }
    LE_NOTHROW std::uint32_t LE_FASTCALL getTimePosition  () const { return timePosition2SamplePosition( getAbsolutePosition() ); }

    LE_NOTHROW REFERENCE_TIME LE_FASTCALL getAbsolutePosition() const
    {
        REFERENCE_TIME position;
        BOOST_VERIFY( pMediaSeeking_->GetCurrentPosition( &position ) == S_OK );
        return position;
    }

    explicit operator bool() const { return static_cast<bool>( pSourceFilter_ ); }

private:
    bool LE_FASTCALL copyAndConvertSamples( IMediaSample & ) const;

    void LE_FASTCALL sendEvent( unsigned int const code, unsigned int const parameter1, void * const pParameter2 ) const
    {
        sendEvent( static_cast<LONG>( code ), static_cast<LONG>( parameter1 ), reinterpret_cast<LONG_PTR>( pParameter2 ) );
    }
    void LE_FASTCALL sendEvent( LONG code, LONG_PTR parameter1, LONG_PTR parameter2 ) const;

    void LE_FASTCALL sendStopEvent()
    {
        endOfStreamEventPending_ = false;
        sendEvent( EC_COMPLETE, S_OK, static_cast<IUnknown *>( static_cast<IBaseFilter *>( this ) ) );
    }

    unsigned int LE_FASTCALL getEvent() const
    {
    #ifdef _DEBUG
        unsigned int const timeOut( INFINITE );
    #else
        unsigned int const timeOut( 500      );
    #endif // _DEBUG
        LONG     event;
        LONG_PTR param1, param2;
        BOOST_VERIFY( pMediaEvent_->GetEvent       ( &event, &param1, &param2, timeOut ) == S_OK );
        BOOST_VERIFY( pMediaEvent_->FreeEventParams(  event,  param1,  param2          ) == S_OK );
        return event;
    }

    LE_NOTHROW REFERENCE_TIME LE_FASTCALL currentPosition() const
    {
        REFERENCE_TIME result;
        BOOST_VERIFY( pMediaSeeking_->GetCurrentPosition( &result ) == S_OK );
        BOOST_ASSERT( result >= 0        );
        BOOST_ASSERT( result <= duration() );
        return result;
    }

    LE_NOTHROW REFERENCE_TIME LE_FASTCALL duration() const
    {
        REFERENCE_TIME result;
        BOOST_VERIFY( pMediaSeeking_->GetDuration( &result ) == S_OK );
        BOOST_ASSERT( result > 0 );
        return result;
    }

    char const * LE_FASTCALL openFullPath( char const * const sampleFileName )
    {
        std::array<wchar_t, MAX_PATH> widePath;
        std::copy_n( sampleFileName, std::strlen( sampleFileName ) + 1, widePath.begin() );
        return openFullPath( &widePath[ 0 ] );
    }

    char const * LE_FASTCALL openFullPath( wchar_t const * sampleFileName );

    // Implementation note:
    //   std::pair<> forces value initialization.
    //                                        (23.08.2011.) (Domagoj Saric)
    struct DataType
    {
        AM_MEDIA_TYPE        type  ;
        WAVEFORMATEXTENSIBLE format;
    }; // struct DataType

    DataType const & dataType() const { return currentType_; }

    void verifyTimeFormat() const
    {
    #ifndef NDEBUG
        GUID timeFormat;
        BOOST_VERIFY( pMediaSeeking_->GetTimeFormat( &timeFormat ) == S_OK );
        BOOST_ASSERT( timeFormat == TIME_FORMAT_MEDIA_TIME );
    #endif // NDEBUG
    }

    std::uint32_t LE_FASTCALL timePosition2SamplePosition( std::uint64_t const time ) const
    {
        verifyTimeFormat();
        std::uint64_t const hundredNanoSecondsPerSecond( 1000 * 1000 * 1000ULL / 100 );
        auto const numberOfSamples
        (
            static_cast<std::uint32_t>
            (
                LE::Math::roundUpUnsignedIntegerDivision
                (
                    time * sampleRate(),
                    hundredNanoSecondsPerSecond
                )
            )
        );
        return numberOfSamples;
    }

    LE_NOTHROWNOALIAS FILTER_STATE state() const
    {
    #ifndef NDEBUG
        if ( pMediaControl_ )
        {
            FILTER_STATE state;
            /// \note Fetching the state with a timeout initiates a wait on an
            /// internal DShow mutex and it seems IMediaControl::Stop() tries to
            /// acquire the same mutex so using an INFINITE timeout here can
            /// cause a deadlock (e.g. in case the IMemInputPin::Receive()
            /// implementation calls this function as part of a response to an
            /// IMediaControl::Stop() call.
            /// http://msdn.microsoft.com/en-us/library/windows/desktop/dd390172(v=vs.85).aspx
            ///                               (05.06.2014.) (Domagoj Saric)
            HRESULT const hr( pMediaControl_->GetState( 0, reinterpret_cast<OAFilterState *>( &state ) ) );
            BOOST_ASSERT( hr == S_OK || hr == VFW_S_STATE_INTERMEDIATE || hr == E_FAIL /*...mrmlj...investigate...*/ );
            BOOST_ASSERT_MSG( state == state_ || hr == E_FAIL, "Filter vs Filter manger state out of sync" );
        }
    #endif // NDEBUG
        return state_;
    }

    LE_NOTHROW LE_NOINLINE
    void wakeAllWaiters()
    {
        /// \note It seems that signaling a condition variable that nobody is
        /// waiting on is a no-op. Needs further research...
        /// (if (!var) return; /* if no one is waiting - nothing to do */)
        /// http://www.winehq.org/pipermail/wine-patches/2013-January/121264.html
        ///                                   (06.06.2014.) (Domagoj Saric)
        //...mrmlj...deadlocks...investigate race conditions...
        //bufferLock_.lock  ();
        bufferNotEmpty_.signal();
        bufferNotFull_ .signal();
        //bufferLock_.unlock();
    }

#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    DWORD_PTR logHandle() const { return reinterpret_cast<DWORD_PTR>( hLog_ ); }
#endif // _DEBUG

    ////////////////////////////////////////////////////////////////////////////
    // DShow interfaces implementations
    ////////////////////////////////////////////////////////////////////////////

private: // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface( IID const &, void * * ppvObject ) final;

    ULONG STDMETHODCALLTYPE AddRef () final { BOOST_ASSERT( refCount_++ <  255 ); return 1; }
    ULONG STDMETHODCALLTYPE Release() final { BOOST_ASSERT( refCount_-- >=   1 ); return 1; }

private: // IPersist
    HRESULT STDMETHODCALLTYPE GetClassID( CLSID * ) final { return E_FAIL; }

private: // IMediaFilter
    HRESULT STDMETHODCALLTYPE Stop () final { state_ = State_Stopped; endOfStreamEventPending_ = false; wakeAllWaiters(); /*BOOST_ASSERT( state_ == State_Stopped );...mrmlj...!?*/ return S_OK; }
    HRESULT STDMETHODCALLTYPE Pause() final { state_ = State_Paused ;                                   wakeAllWaiters(); BOOST_ASSERT( state_ == State_Paused  ); return S_OK; }
    HRESULT STDMETHODCALLTYPE Run  ( REFERENCE_TIME ) final;

    HRESULT STDMETHODCALLTYPE GetState( DWORD dwMilliSecsTimeout, FILTER_STATE * ) final;

    HRESULT STDMETHODCALLTYPE SetSyncSource( IReferenceClock *   ) final;
    HRESULT STDMETHODCALLTYPE GetSyncSource( IReferenceClock * * ) final;

private: // IBaseFilter
    HRESULT STDMETHODCALLTYPE EnumPins( IEnumPins * * ) final;

    HRESULT STDMETHODCALLTYPE FindPin( LPCWSTR /*Id*/, IPin * * ) final { return E_NOTIMPL; }

    HRESULT STDMETHODCALLTYPE QueryFilterInfo( FILTER_INFO * ) final;

    HRESULT STDMETHODCALLTYPE JoinFilterGraph( IFilterGraph *, LPCWSTR pName ) final;

    HRESULT STDMETHODCALLTYPE QueryVendorInfo( LPWSTR * /*pVendorInfo*/ ) final { return E_NOTIMPL; }

private: // IPin
    HRESULT STDMETHODCALLTYPE Connect( IPin *, AM_MEDIA_TYPE const * ) final;

    HRESULT STDMETHODCALLTYPE ReceiveConnection( IPin *, AM_MEDIA_TYPE const * ) final;

    HRESULT STDMETHODCALLTYPE Disconnect() final;

    HRESULT STDMETHODCALLTYPE ConnectedTo( IPin * * ) final;

    HRESULT STDMETHODCALLTYPE ConnectionMediaType( AM_MEDIA_TYPE * ) final;

    HRESULT STDMETHODCALLTYPE QueryPinInfo( PIN_INFO * ) final;

    HRESULT STDMETHODCALLTYPE QueryDirection( PIN_DIRECTION * ) final;

    HRESULT STDMETHODCALLTYPE QueryId       ( LPWSTR * /*Id*/       ) final { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE QueryAccept   ( AM_MEDIA_TYPE const * ) final { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE EnumMediaTypes( IEnumMediaTypes * *   ) final { return E_NOTIMPL; }

    HRESULT STDMETHODCALLTYPE QueryInternalConnections( IPin * *, ULONG * nPin ) final;

    HRESULT STDMETHODCALLTYPE EndOfStream() final;
    HRESULT STDMETHODCALLTYPE BeginFlush () final { flushing_ = true ; endOfStreamEventPending_ = false; bytesToSkipFromNextBuffer_ = 0; wakeAllWaiters(); return S_OK; } // https://msdn.microsoft.com/EN-US/library/windows/desktop/dd390419(v=vs.85).aspx
    HRESULT STDMETHODCALLTYPE EndFlush   () final { flushing_ = false;                                                                                     return S_OK; } // https://msdn.microsoft.com/en-us/library/windows/desktop/dd390424(v=vs.85).aspx

    HRESULT STDMETHODCALLTYPE NewSegment( REFERENCE_TIME /*tStart*/, REFERENCE_TIME /*tStop*/, double /*dRate*/ ) final { return S_OK; }

private: // IMemInputPin
    HRESULT STDMETHODCALLTYPE GetAllocator( IMemAllocator * * /*ppAllocator*/ ) final;

    HRESULT STDMETHODCALLTYPE NotifyAllocator( IMemAllocator * /*pAllocator*/, BOOL /*bReadOnly*/ ) final { return S_OK; }

    HRESULT STDMETHODCALLTYPE GetAllocatorRequirements( ALLOCATOR_PROPERTIES * /*pProps*/ ) final { return E_NOTIMPL; }

    HRESULT STDMETHODCALLTYPE Receive( IMediaSample * pBufferHolder ) final;

    HRESULT STDMETHODCALLTYPE ReceiveMultiple( IMediaSample * *, long nSamples, long * nSamplesProcessed ) final;

    /// \note Reads of arbitrary sizes supported by blocking the decoding
    /// thread.
    ///                                       (06.06.2014.) (Domagoj Saric)
    HRESULT STDMETHODCALLTYPE ReceiveCanBlock() final { return S_OK; }

private: // IEnumPins
    HRESULT STDMETHODCALLTYPE IEnumPins::Next ( ULONG cPins, IPin * * ppPins, ULONG * pcFetched ) final;
    HRESULT STDMETHODCALLTYPE IEnumPins::Skip ( ULONG cPins                                     ) final;
    HRESULT STDMETHODCALLTYPE IEnumPins::Reset() final { onlyPinAlreadyEnumerated_ = false; return S_OK; }
    HRESULT STDMETHODCALLTYPE IEnumPins::Clone( IEnumPins * * /*ppEnum*/ ) final { return E_NOTIMPL; }

#if 0 // disabled/for testing
/// \note See the note(s) in the QueryInterface member funcion.
///                                           (12.02.2016.) (Domagoj Saric)
private: // IMediaSeeking
    template <typename Interface, typename ... T>
    HRESULT forwardAPI( HRESULT (STDMETHODCALLTYPE Interface::*api)( T... ), T ... args )
    {
        return pConnectedOutputPin_ ? (COMPtr<Interface>( *pConnectedOutputPin_ )->*api)( args... ) : E_NOTIMPL;
    }
    #define LE_FWD_SEEKING( API, ... ) return forwardAPI( &IMediaSeeking::API, __VA_ARGS__ )

    STDMETHODIMP GetCapabilities     ( DWORD      * pCapabilities ) { LE_FWD_SEEKING( GetCapabilities     , pCapabilities ); }
    STDMETHODIMP CheckCapabilities   ( DWORD      * pCapabilities ) { LE_FWD_SEEKING( CheckCapabilities   , pCapabilities ); }
    STDMETHODIMP IsFormatSupported   ( GUID const * pFormat       ) { LE_FWD_SEEKING( IsFormatSupported   , pFormat       ); }
    STDMETHODIMP QueryPreferredFormat( GUID       * pFormat       ) { LE_FWD_SEEKING( QueryPreferredFormat, pFormat       ); }
    STDMETHODIMP GetTimeFormat       ( GUID       * pFormat       ) { LE_FWD_SEEKING( GetTimeFormat       , pFormat       ); }
    STDMETHODIMP IsUsingTimeFormat   ( GUID const * pFormat       ) { LE_FWD_SEEKING( IsUsingTimeFormat   , pFormat       ); }
    STDMETHODIMP SetTimeFormat       ( GUID const * pFormat       ) { LE_FWD_SEEKING( SetTimeFormat       , pFormat       ); }
    STDMETHODIMP GetDuration         ( LONGLONG   * pDuration     ) { LE_FWD_SEEKING( GetDuration         , pDuration     ); }
    STDMETHODIMP GetStopPosition     ( LONGLONG   * pStop         ) { LE_FWD_SEEKING( GetStopPosition     , pStop         ); }
    STDMETHODIMP GetCurrentPosition  ( LONGLONG   * pCurrent      ) { LE_FWD_SEEKING( GetCurrentPosition  , pCurrent      ); }
    STDMETHODIMP ConvertTimeFormat   ( LONGLONG   * pTarget  , GUID const * pTargetFormat, LONGLONG Source, GUID const * pSourceFormat ) { LE_FWD_SEEKING( ConvertTimeFormat, pTarget, pTargetFormat, Source, pSourceFormat ); }
    STDMETHODIMP SetPositions        ( LONGLONG   * pCurrent , DWORD dwCurrentFlags, LONGLONG * pStop, DWORD dwStopFlags ) { LE_FWD_SEEKING( SetPositions, pCurrent, dwCurrentFlags, pStop, dwStopFlags ); }
    STDMETHODIMP GetPositions        ( LONGLONG   * pCurrent , LONGLONG * pStop   ) { LE_FWD_SEEKING( GetPositions, pCurrent , pStop   ); }
    STDMETHODIMP GetAvailable        ( LONGLONG   * pEarliest, LONGLONG * pLatest ) { LE_FWD_SEEKING( GetAvailable, pEarliest, pLatest ); }
    STDMETHODIMP SetRate             ( double       dRate        ) { LE_FWD_SEEKING( SetRate   , dRate      ); }
    STDMETHODIMP GetRate             ( double     * pdRate       ) { LE_FWD_SEEKING( GetRate   , pdRate     ); }
    STDMETHODIMP GetPreroll          ( LONGLONG   * pllPreroll   ) { LE_FWD_SEEKING( GetPreroll, pllPreroll ); }

private: // IMediaPosition
    /// \note "Do not implement this method. Implement IMediaSeeking instead. If
    /// your filter supports IMediaSeeking, the Filter Graph Manager
    /// automatically handles calls to IMediaPosition."
    /// https://msdn.microsoft.com/en-us/library/windows/desktop/dd406977(v=vs.85).aspx
    ///                                       (12.02.2016.) (Domagoj Saric)
    #define LE_FWD_POS( API, ... ) return forwardAPI( &IMediaPosition::API, __VA_ARGS__ )

    STDMETHODIMP get_Duration       ( REFTIME * plength          ) { LE_FWD_POS( get_Duration       , plength          ); }
    STDMETHODIMP put_CurrentPosition( REFTIME   llTime           ) { LE_FWD_POS( put_CurrentPosition, llTime           ); }
    STDMETHODIMP get_StopTime       ( REFTIME * pllTime          ) { LE_FWD_POS( get_StopTime       , pllTime          ); }
    STDMETHODIMP put_StopTime       ( REFTIME   llTime           ) { LE_FWD_POS( put_StopTime       , llTime           ); }
    STDMETHODIMP get_PrerollTime    ( REFTIME * pllTime          ) { LE_FWD_POS( get_PrerollTime    , pllTime          ); }
    STDMETHODIMP put_PrerollTime    ( REFTIME   llTime           ) { LE_FWD_POS( put_PrerollTime    , llTime           ); }
    STDMETHODIMP get_Rate           ( double  * pdRate           ) { LE_FWD_POS( get_Rate           , pdRate           ); }
    STDMETHODIMP put_Rate           ( double    dRate            ) { LE_FWD_POS( put_Rate           , dRate            ); }
    STDMETHODIMP get_CurrentPosition( REFTIME * pllTime          ) { LE_FWD_POS( get_CurrentPosition, pllTime          ); }
    STDMETHODIMP CanSeekForward     ( LONG    * pCanSeekForward  ) { LE_FWD_POS( CanSeekForward     , pCanSeekForward  ); }
    STDMETHODIMP CanSeekBackward    ( LONG    * pCanSeekBackward ) { LE_FWD_POS( CanSeekBackward    , pCanSeekBackward ); }

    #define LE_FWD_DISPATCH( API, ... ) return forwardAPI( &IMediaPosition::API, __VA_ARGS__ )

    STDMETHODIMP GetTypeInfoCount( UINT * pctinfo                                                                                                                                   ) { return E_NOTIMPL; }
    STDMETHODIMP GetTypeInfo     ( UINT itinfo, LCID lcid, ITypeInfo ** pptinfo                                                                                                     ) { return E_NOTIMPL; }
    STDMETHODIMP GetIDsOfNames   ( REFIID riid, OLECHAR  ** rgszNames, UINT cNames, LCID lcid, DISPID * rgdispid                                                                    ) { return E_NOTIMPL; }
    STDMETHODIMP Invoke          ( DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS * pdispparams,VARIANT * pvarResult, EXCEPINFO * pexcepinfo, UINT * puArgErr ) { return E_NOTIMPL; }
#endif // disabled
private:
    bool outputFilled() const { return outputBuffer_.empty(); }

private:
    class COMGuard
    {
    public:
         COMGuard() : initialised_( false ) {}
         COMGuard( COMGuard && other ) : initialised_( other.initialised_ ) { other.initialised_ = false; }
        ~COMGuard()
        {
            if ( initialised_ )
            {
                // Implementation note:
                //   Immediate unloading caused Wavelab to crash on exit with an
                // access violation when a last session preset 'containing' a sample
                // was loaded on plugin startup. A value of 1000 milliseconds was
                // found to work properly so we use a 10 second timeout.
                //                                    (03.03.2010.) (Domagoj Saric)
                ::CoFreeUnusedLibrariesEx( 1000, 0 );

                ::CoUninitialize();
            }
        }

        bool initialised() const { return initialised_; }

        HRESULT initialise()
        {
            BOOST_ASSERT( !initialised_ );
            //HRESULT const result( ::CoInitializeEx( 0, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY ) );
            HRESULT const result( ::CoInitialize( nullptr ) );
            switch ( result )
            {
                case RPC_E_CHANGED_MODE:
                    LE_TRACE( "Tried to change thread COM concurrency model." );
                #if 0
                    LE_UNREACHABLE_CODE();
                #else // not using a dedicated thread...
                    initialised_ = true;
                    return S_OK;
                #endif
                case S_FALSE:
                #ifndef LE_PUBLIC_BUILD
                    LE_TRACE( "COM already initialized." );
                #endif // LE_PUBLIC_BUILD
                #if 0
                    LE_UNREACHABLE_CODE();
                #else // not using a dedicated thread...
                    initialised_ = true;
                    return S_OK;
                #endif
                case S_OK:
                    initialised_ = true;
                    break;
                default:
                    break;
            }
            return result;
        }

    private:
        bool initialised_;
    }; // class COMGuard

private:
    DataType currentType_;

    using OutputBuffer = boost::iterator_range<float * LE_RESTRICT>;
    mutable OutputBuffer outputBuffer_;

    mutable Utility::ConditionVariable       bufferNotEmpty_;
    mutable Utility::ConditionVariable       bufferNotFull_ ;
    mutable Utility::ConditionVariable::Lock bufferLock_    ;

    mutable std::uint16_t bytesToSkipFromNextBuffer_;

    COMGuard comGuard_;

    std::uint8_t resolutionInBytesPerSample_;

#ifndef NDEBUG
    std::atomic_uint8_t refCount_;
#endif // NDEBUG

    // 'Pin enumerator' members
    bool onlyPinAlreadyEnumerated_;

    // 'Filter' members
    // https://msdn.microsoft.com/en-us/library/windows/desktop/dd319485%28v=vs.85%29.aspx EC_COMPLETE
    bool                       endOfStreamEventPending_;
    bool                       flushing_               ;
    FILTER_STATE volatile      state_                  ;
    IFilterGraph * LE_RESTRICT pMyGraph_               ; //...mrmlj...check...should be equal to pGraphBuilder_...

    // 'Pin' members
    ConnectedOutputPin pConnectedOutputPin_;

    COMPtr<IGraphBuilder> pGraphBuilder_;

    COMPtr<IBaseFilter>   pSourceFilter_;

    COMPtr<IMediaControl> pMediaControl_;
    COMPtr<IMediaSeeking> pMediaSeeking_;
    COMPtr<IMediaFilter > pMediaFilter_ ;
    COMPtr<IMediaEventEx> pMediaEvent_  ;

#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    HANDLE hLog_;
#endif // _DEBUG
}; // FileImpl

namespace
{
#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    HANDLE openLogFile()
    {
        TCHAR expandedBuffer[ MAX_PATH ];
        BOOST_VERIFY( ::ExpandEnvironmentStrings( _T( "%TEMP%\\SpectrumWorxDirectShow.log" ), expandedBuffer, _countof( expandedBuffer ) ) < _countof( expandedBuffer ) );
        HANDLE const logHandle( ::CreateFile( expandedBuffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr ) );
        BOOST_ASSERT_MSG( logHandle != INVALID_HANDLE_VALUE, "DShow wrapper: failed to open log file." );
        return logHandle;
    }
#endif // _DEBUG

    /// \todo See if a simpler method could be used to fetch the required output
    /// pin. For example, for the WMA read filter we could directly call
    /// pSourceFilter->FindPin( L"Raw Audio 0", &pSourcePin );
    ///                                       (02.07.2010.) (Domagoj Saric)
    LE_NOTHROWNOALIAS
    COMPtr<IPin> LE_FASTCALL getOutputPin( IBaseFilter & filter, HRESULT & hr )
    {
        COMPtr<IEnumPins> pEnumerator;
        hr = filter.EnumPins( &pEnumerator );
        if ( FAILED( hr ) )
            return COMPtr<IPin>();

        IPin * pPin;
        while ( ( hr = pEnumerator->Next( 1, &pPin, nullptr ) ) == S_OK )
        {
            BOOST_ASSERT( pPin );
            PIN_DIRECTION pinDirection;
            BOOST_VERIFY( pPin->QueryDirection( &pinDirection ) == S_OK );
            if ( pinDirection == PINDIR_OUTPUT )
            {
            #ifndef NDEBUG
                IPin * pTmp;
                HRESULT const hrTmp( pPin->ConnectedTo( &pTmp ) );
                BOOST_ASSERT_MSG( FAILED( hrTmp ) && pTmp == 0, "Graph output pin already connected." );
            #endif // NDEBUG
                BOOST_ASSERT( hr == S_OK );
                return COMPtr<IPin>( pPin );
            }
            else
                pPin->Release();
        }
        // Did not find a matching pin.
        return COMPtr<IPin>();
    }
} // namespace anonymous

FileImpl::FileImpl()
    :
    state_                   ( State_Stopped ),
    pMyGraph_                ( nullptr       ),
#ifndef NDEBUG
    refCount_                ( 0             ),
#endif // NDEBUG
    onlyPinAlreadyEnumerated_( false         ),
    endOfStreamEventPending_ ( false         ),
    flushing_                ( false         )
#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    ,hLog_( openLogFile() )
#endif // _DEBUG
{
#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
	BOOST_ASSERT( hLog_ );
#endif // _DEBUG
}


FileImpl::~FileImpl()
{
    /// \note The graph has to be explicitly stopped to avoid a deadlock in
    /// IGraphBuilder::Release().
    ///                                       (05.06.2014.) (Domagoj Saric)
    close();
    BOOST_ASSERT( state_ == State_Stopped ); // state() is recursive here
#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    BOOST_VERIFY( ::CloseHandle( hLog_ ) || hLog_ == INVALID_HANDLE_VALUE );
#endif // _DEBUG

#ifndef NDEBUG
    pGraphBuilder_.reset();
    pSourceFilter_.reset();
    pMediaControl_.reset();
    pMediaSeeking_.reset();
    pMediaFilter_ .reset();
    pMediaEvent_  .reset();
    BOOST_ASSERT( refCount_ == 0 );
#endif // NDEBUG
}


FileImpl::FileImpl( FileImpl && other )
    :
    currentType_               (            other.currentType_                  ),
    outputBuffer_              (            other.outputBuffer_                 ),
    bufferNotEmpty_            ( std::move( other.bufferNotEmpty_             ) ),
    bufferNotFull_             ( std::move( other.bufferNotFull_              ) ),
    bufferLock_                ( std::move( other.bufferLock_                 ) ),
    bytesToSkipFromNextBuffer_ (            other.bytesToSkipFromNextBuffer_    ),
    comGuard_                  ( std::move( other.comGuard_                   ) ),
    resolutionInBytesPerSample_(            other.resolutionInBytesPerSample_   ),
#ifndef NDEBUG
    refCount_                  ( 0                                              ),
#endif // NDEBUG
    onlyPinAlreadyEnumerated_  (            other.onlyPinAlreadyEnumerated_     ),
    endOfStreamEventPending_   (            other.endOfStreamEventPending_      ),
    flushing_                  (            other.flushing_                     ),
    state_                     (            other.state_                        ),
    pMyGraph_                  (            nullptr                             ),

    pGraphBuilder_             ( std::move( other.pGraphBuilder_              ) ),
    pSourceFilter_             ( std::move( other.pSourceFilter_              ) ),
    pMediaControl_             ( std::move( other.pMediaControl_              ) ),
    pMediaSeeking_             ( std::move( other.pMediaSeeking_              ) ),
    pMediaFilter_              ( std::move( other.pMediaFilter_               ) ),
    pMediaEvent_               ( std::move( other.pMediaEvent_                ) )

#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    ,hLog_( other.hLog_ )
#endif // _DEBUG
{
#if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
    other.hLog_ = INVALID_HANDLE_VALUE;
#endif // _DEBUG
    if ( pGraphBuilder_ )
    {
        /// \note We have to break the graph connection with the
        /// previous/old/'other' instance and reconnect the new object.
        ///                                   (12.02.2016.) (Domagoj Saric)
        BOOST_VERIFY( pGraphBuilder_->RemoveFilter( &other          ) == S_OK && other.pMyGraph_ == nullptr        );
        BOOST_VERIFY( pGraphBuilder_->AddFilter   ( this  , nullptr ) == S_OK && this->pMyGraph_ == pGraphBuilder_ );
        if ( other.pConnectedOutputPin_ )
        {
            IPin * const pConnectedOutputPin( other.pConnectedOutputPin_ );
            pConnectedOutputPin->AddRef();
            BOOST_VERIFY( pGraphBuilder_->Disconnect( &other              ) == S_OK );
            BOOST_VERIFY( pGraphBuilder_->Disconnect( pConnectedOutputPin ) == S_OK );
            BOOST_ASSERT( !other.pConnectedOutputPin_ );

            BOOST_VERIFY( pGraphBuilder_->ConnectDirect( pConnectedOutputPin, static_cast<IPin *>( this ), &currentType_.type ) == S_OK );
            BOOST_ASSERT( pConnectedOutputPin_ == pConnectedOutputPin );
            pConnectedOutputPin->Release();
        }
    }
    BOOST_ASSERT( other.refCount_ == 0 );
}


HRESULT STDMETHODCALLTYPE FileImpl::QueryInterface( IID const & iid, void * * const ppvObject )
{
    BOOST_ASSERT( ppvObject );

    /// \note The documentation seems to imply (by the combination of the
    /// "static set of supported interfaces" COM requirement, the "renderers
    /// must implement IMediaSeeking and forward it upstream" DShow requirement
    /// and the source of DShow 'base classes', e.g. CPosPassThru) that we have
    /// to actually provide a 'dummy'/forwarding IMediaSeeking implementation.
    /// However we seem to be able to get away with simply returning the
    /// IMediaSeeking instance that the pConnectedOutputPin_ returns (even with
    /// the below workaround for the AAC decoder).
    /// "Seek support in transform filter" http://www.tech-archive.net/Archive/Development/microsoft.public.win32.programmer.directx.video/2007-07/msg00064.html
    ///                                       (12.02.2016.) (Domagoj Saric)

    /// \note The Microsoft DTV-DVD Audio Decoder (used for AAC files) queries
    /// the IMediaSeeking interface before a connection has been established
    /// with an output pin (tested only on Windows 7).
    ///                                       (25.07.2014.) (Domagoj Saric)
#ifndef LE_PUBLIC_BUILD
    LE_TRACE_IF( iid == __uuidof( IMediaSeeking ) && !pConnectedOutputPin_, "IMediaSeeking interface requested w/o a connected output pin." );
#endif // LE_PUBLIC_BUILD

    FileImpl::AddRef();

         if ( iid == IID_IMediaSeeking  && pConnectedOutputPin_ ) { FileImpl::Release(); return pConnectedOutputPin_->QueryInterface( iid, ppvObject ); }
  //else if ( iid == IID_IMediaPosition ) { *ppvObject = static_cast<IMediaPosition*>(                      this   );                         return S_OK; }
    else if ( iid == IID_IPin           ) { *ppvObject = static_cast<IPin          *>(                      this   );                         return S_OK; }
    else if ( iid == IID_IMemInputPin   ) { *ppvObject = static_cast<IMemInputPin  *>(                      this   );                         return S_OK; }
    else if ( iid == IID_IBaseFilter    ) { *ppvObject = static_cast<IBaseFilter   *>(                      this   );                         return S_OK; }
    else if ( iid == IID_IEnumPins      ) { *ppvObject = static_cast<IEnumPins     *>(                      this   ); /*IEnumPins::*/Reset(); return S_OK; }
    else if ( iid == IID_IUnknown       ) { *ppvObject = static_cast<IUnknown      *>( static_cast<IPin *>( this ) );                         return S_OK; }
    else
    {
        //BOOST_ASSERT( iid != IID_IMediaPosition ); // it still gets queried for
        FileImpl::Release();
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }

    LE_UNREACHABLE_CODE();
}


HRESULT STDMETHODCALLTYPE FileImpl::Run( REFERENCE_TIME /*tStart*/ )
{
    if ( endOfStreamEventPending_ )
    {
        BOOST_ASSERT( state_ == State_Paused ); // state() is recursive here
        sendStopEvent();
    }
    BOOST_ASSERT( state_ != State_Running );
    state_ = State_Running;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::GetState( DWORD /*dwMilliSecsTimeout*/, FILTER_STATE * const pState )
{
    BOOST_ASSERT( pState );
    *pState = state_;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::SetSyncSource( IReferenceClock * const pClock )
{
    BOOST_VERIFY( !pClock );
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::GetSyncSource( IReferenceClock * * const pClock )
{
    BOOST_ASSERT( pClock );
    *pClock = nullptr;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::EnumPins( IEnumPins * * const ppEnum )
{
    BOOST_ASSERT( ppEnum );
    FileImpl::AddRef();
    *ppEnum = static_cast<IEnumPins *>( this );
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::QueryFilterInfo( FILTER_INFO * const pInfo )
{
    BOOST_ASSERT( pInfo     );
    BOOST_ASSERT( pMyGraph_ );
    pInfo->achName[ 0 ] = '\0';
    pInfo->pGraph = pMyGraph_;
    pMyGraph_->AddRef();
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::JoinFilterGraph( IFilterGraph * const pGraph, LPCWSTR /*pName*/ )
{
    BOOST_ASSERT( !pGraph || !pMyGraph_ );
    pMyGraph_ = pGraph;
    return S_OK;
}


#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
HRESULT STDMETHODCALLTYPE FileImpl::Skip   ( ULONG /*cMediaTypes/cPins*/                           ) { LE_UNREACHABLE_CODE(); return S_FALSE  ; }
HRESULT STDMETHODCALLTYPE FileImpl::Connect( IPin * /*pReceivePin*/, AM_MEDIA_TYPE const * /*pMT*/ ) { LE_UNREACHABLE_CODE(); return E_NOTIMPL; }
#pragma warning( pop )


namespace
{
    WAVEFORMATEXTENSIBLE const * waveFormat( AM_MEDIA_TYPE const & mediaType )
    {
        if ( mediaType.majortype != MEDIATYPE_Audio )
            return nullptr;

        // Audio subtypes
        // http://msdn.microsoft.com/en-us/library/windows/desktop/dd317599%28v=vs.85%29.aspx
        if
        (
            ( mediaType.subtype != MEDIASUBTYPE_PCM        ) &&
            ( mediaType.subtype != MEDIASUBTYPE_IEEE_FLOAT )
        )
            return nullptr;

        BOOST_ASSERT( mediaType.bFixedSizeSamples    == TRUE                );
        BOOST_ASSERT( mediaType.bTemporalCompression == FALSE               );
        BOOST_ASSERT( mediaType.formattype           == FORMAT_WaveFormatEx );
        /// \note It was encountered (when decoding some MP3s under Windows 7
        /// SP1 x64) that a DShow filter sets cbFormat to
        /// sizeof( WAVEFORMATEXTENSIBLE ) even though the members/memory after
        /// the Format field seem uninitialised/unused.
        ///                                   (30.05.2014.) (Domagoj Saric)
        BOOST_ASSERT
        (
            mediaType.cbFormat == sizeof( WAVEFORMATEX         ) ||
            mediaType.cbFormat == sizeof( WAVEFORMATEXTENSIBLE )
        );

        auto const & extendedFormat( *reinterpret_cast<WAVEFORMATEXTENSIBLE *>( mediaType.pbFormat ) );
        auto const & format        ( extendedFormat.Format );
        /// \note Bogus pMT->lSampleSize == 1 values were encountered in the
        /// same situation(s) as in the above note for pMT->cbFormat.
        ///                                   (30.05.2014.) (Domagoj Saric)
        BOOST_ASSERT( format.nBlockAlign == mediaType.lSampleSize || mediaType.lSampleSize == 1 );
        BOOST_ASSERT( format.cbSize      == 0  || format.wFormatTag == WAVE_FORMAT_EXTENSIBLE );
        BOOST_ASSERT_MSG
        (
            ( format.wBitsPerSample / 8 ) == ( format.nBlockAlign / format.nChannels ),
            "Padded data types (e.g. unpacked 24 bit) not supported."
        );
        BOOST_ASSERT
        (
            ( mediaType.subtype == MEDIASUBTYPE_PCM        && format.wFormatTag == WAVE_FORMAT_PCM        ) ||
            ( mediaType.subtype == MEDIASUBTYPE_IEEE_FLOAT && format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT ) ||
            (                                                 format.wFormatTag == WAVE_FORMAT_EXTENSIBLE )
        );
        boost::ignore_unused_variable_warning( format );

        return &extendedFormat;
    }
} // anonymous namespace

HRESULT STDMETHODCALLTYPE FileImpl::ReceiveConnection
(
    IPin                * LE_RESTRICT const pConnector,
    AM_MEDIA_TYPE const * LE_RESTRICT const pMT
)
{
    BOOST_ASSERT( pMT                      );
    BOOST_ASSERT( pMT->pUnk     == nullptr );
  //BOOST_ASSERT( pMT->pbFormat != nullptr );
  //BOOST_ASSERT( pMT->cbFormat != 0       );
    BOOST_ASSERT( state_ == State_Stopped  ); // state() is recursive here

    auto const * const pExtendedFormat( waveFormat( *pMT ) );
    if ( !pExtendedFormat )
        return VFW_E_TYPE_NOT_ACCEPTED;
    auto const & format( pExtendedFormat->Format );

    /// \note Reject sample formats which do not match the input file format (if
    /// we were able to detect/extract one). For now, we accept an increase in
    /// bit depth as we unconditionally convert to normalized floats anyway.
    /// See th related note in the openFullPath() member function.
    ///                                       (25.11.2014.) (Domagoj Saric)
    if ( currentType_.type.pbFormat )
    {
        if
        (
            ( format.nChannels      != currentType_.format.Format.nChannels      ) ||
            ( format.nSamplesPerSec != currentType_.format.Format.nSamplesPerSec ) ||
            ( format.wBitsPerSample  < currentType_.format.Format.wBitsPerSample )
        )
            return VFW_E_TYPE_NOT_ACCEPTED;
    }

    std::memcpy( &currentType_.format, &format, sizeof( format ) + format.cbSize );
    currentType_.type          = *pMT;
    currentType_.type.pbFormat = reinterpret_cast<BYTE *>( &currentType_.format );

    resolutionInBytesPerSample_ = static_cast<std::uint8_t>
    (
        ( currentType_.format.Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT )
            ? -1
            : currentType_.format.Format.wBitsPerSample / 8
    );

#ifndef NDEBUG
    PIN_DIRECTION connectorDirection;
    BOOST_VERIFY( pConnector->QueryDirection( &connectorDirection ) == S_OK );
    BOOST_ASSERT( ( connectorDirection == PINDIR_OUTPUT ) || ( connectorDirection == PINDIR_INPUT ) );
    BOOST_ASSERT(   connectorDirection != PINDIR_INPUT );
#endif //NDEBUG

    if ( pConnectedOutputPin_ ) pConnectedOutputPin_->Release();
                                pConnectedOutputPin_ = pConnector;
                                pConnectedOutputPin_->AddRef();

    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::Disconnect()
{
    BOOST_ASSERT( pConnectedOutputPin_ );
    pConnectedOutputPin_->Release();
    pConnectedOutputPin_ = nullptr;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::ConnectedTo( IPin * * const ppPin )
{
    BOOST_ASSERT( ppPin );

    *ppPin = pConnectedOutputPin_;
    if ( pConnectedOutputPin_ )
    {
        pConnectedOutputPin_->AddRef();
        return S_OK;
    }
    else
    {
        BOOST_ASSERT( *ppPin == nullptr );
        return VFW_E_NOT_CONNECTED;
    }
}

#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
HRESULT STDMETHODCALLTYPE FileImpl::ConnectionMediaType( AM_MEDIA_TYPE * const /*pMT*/ ) { LE_UNREACHABLE_CODE(); return E_NOTIMPL; }
#pragma warning( pop )

HRESULT STDMETHODCALLTYPE FileImpl::QueryPinInfo( PIN_INFO * const pInfo )
{
    BOOST_ASSERT( pInfo );
    FileImpl::AddRef();
    pInfo->pFilter = static_cast<IBaseFilter *>( this );
    pInfo->dir     = PINDIR_INPUT;
    pInfo->achName[ 0 ] = 0;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::QueryDirection( PIN_DIRECTION * const pPinDir )
{
    BOOST_ASSERT( pPinDir );
    *pPinDir = PINDIR_INPUT;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::QueryInternalConnections( IPin * * const ppPin, ULONG * const pNPin )
{
    BOOST_ASSERT( pNPin );
    /// \todo Reread the documentation for the IPin::QueryInternalConnections()
    /// and IAMFilterMiscFlags interfaces and rethink this function's
    /// implementation.
    ///                                       (02.07.2010.) (Domagoj Saric)
    //return E_NOTIMPL;
    if ( ppPin )
        *ppPin = nullptr;
    *pNPin = 0;
    return S_OK;
}

LE_NOTHROW
void FileImpl::sendEvent( LONG const code, LONG_PTR const parameter1, LONG_PTR const parameter2 ) const
{
    BOOST_ASSERT( pMyGraph_ );
    COMPtr<IMediaEventSink> const pEventSink( *pMyGraph_ );
    BOOST_ASSERT( pEventSink );
    BOOST_VERIFY( pEventSink->Notify( code, parameter1, parameter2 ) == S_OK );
    ::SwitchToThread();
}

LE_NOTHROW
HRESULT STDMETHODCALLTYPE FileImpl::EndOfStream()
{
    auto const state( state_ ); // state() is recursive here
    BOOST_VERIFY( FileImpl::Stop() == S_OK );
    switch ( state )
    {
        case State_Running: sendStopEvent();                  break;
        case State_Stopped: //...mrmlj...?...intentional fallthrough...
        case State_Paused :  endOfStreamEventPending_ = true; break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
    return S_OK;
}


HRESULT STDMETHODCALLTYPE FileImpl::GetAllocator( IMemAllocator * * /*ppAllocator*/ ) { return VFW_E_NO_ALLOCATOR; }

LE_OPTIMIZE_FOR_SIZE_END()

LE_OPTIMIZE_FOR_SPEED_BEGIN()

LE_NOTHROW
bool LE_HOT FileImpl::copyAndConvertSamples( IMediaSample & bufferHolder ) const
{
    //...mrmlj...BOOST_ASSERT( bufferHolder.IsDiscontinuity() != S_OK );
    BOOST_ASSERT( bufferHolder.IsPreroll() != S_OK );
    //...mrmlj...BOOST_ASSERT( bufferHolder.IsSyncPoint() == S_OK );

    // Implementation note:
    //   When loading MP3s after one has already been loaded, this method
    // (eventually) gets called with a buffer larger than the remaining space
    // left in our buffer so we must explicitly handle this situation.
    //                                        (12.07.2010.) (Domagoj Saric)
    /// \todo Reinvestigate the above issue.
    ///                                       (12.07.2010.) (Domagoj Saric)
    BYTE * LE_RESTRICT pBuffer;
    BOOST_VERIFY( bufferHolder.GetPointer( &pBuffer ) == S_OK );
    auto const reportedBufferLen( static_cast<std::uint32_t>( bufferHolder.GetActualDataLength() ) );
    BOOST_ASSERT( reportedBufferLen <= unsigned( bufferHolder.GetSize() ) );
    pBuffer += bytesToSkipFromNextBuffer_;
    auto const availableData   ( reportedBufferLen - bytesToSkipFromNextBuffer_ );
    auto const outputBufferSize( static_cast<std::uint32_t>( outputBuffer_.size() ) * resolutionInBytesPerSample_ );
    auto const inputLength     ( std::min<std::uint32_t>( availableData, outputBufferSize ) );

    bytesToSkipFromNextBuffer_ += inputLength;

    bool const inputBufferConsumed( inputLength == availableData    );
  //bool const outputBufferFull   ( inputLength == outputBufferSize );

    std::uint32_t const numberOfSamples( inputLength / resolutionInBytesPerSample_ );

    switch ( resolutionInBytesPerSample_ )
    {
        case 2:
        {
            Math::convertSamples( reinterpret_cast<std::int16_t const *>( pBuffer ), outputBuffer_.begin(), numberOfSamples );
            outputBuffer_.advance_begin( numberOfSamples );
            break;
        }

        case 3:
        {
            struct One24bitSample { signed long value: 24; signed long /*unused*/: 8; };

            auto  const maxSample( 1U << 23              );
            float const scale    ( 1.0f / maxSample      );
            auto  const dataEnd  ( pBuffer + inputLength );

            while ( pBuffer < dataEnd )
            {
                auto const & sample( *reinterpret_cast<One24bitSample const *>( pBuffer ) );
                outputBuffer_.front() = LE::Math::convert<float>( sample.value ) * scale;
                outputBuffer_.advance_begin( 1 );
                pBuffer += 3;
            }

            break;
        }

        case 4:
        {
            Math::convertSamples( reinterpret_cast<std::int32_t const *>( pBuffer ), outputBuffer_.begin(), numberOfSamples );
            outputBuffer_.advance_begin( numberOfSamples );
            break;
        }

        case static_cast<std::uint8_t>( -1 ): //...mrmlj...float
        {
            Math::convertSamples( reinterpret_cast<float const *>( pBuffer ), outputBuffer_.begin(), numberOfSamples );
            outputBuffer_.advance_begin( numberOfSamples );
            break;
        }

        LE_DEFAULT_CASE_UNREACHABLE();
    }

    return inputBufferConsumed;
}

LE_OPTIMIZE_FOR_SPEED_END()

LE_OPTIMIZE_FOR_SIZE_BEGIN()

// IMemInputPin::Receive
// https://msdn.microsoft.com/en-us/library/windows/desktop/dd407077%28v=vs.85%29.aspx
HRESULT STDMETHODCALLTYPE FileImpl::Receive( IMediaSample * LE_RESTRICT const pBufferHolder )
{
    LE_ASSUME( pBufferHolder );

    /// \note A producer-consumer implementation.
    /// http://en.wikipedia.org/wiki/Producer–consumer_problem
    /// http://msdn.microsoft.com/en-us/library/windows/desktop/ms686903(v=vs.85).aspx
    /// http://code.msdn.microsoft.com/windowsdesktop/Producer-Consumer-inter-cd30193b
    /// http://technogems.blogspot.com/2012/09/producer-consumer-in-c.html
    /// How To Get Data from a Microsoft DirectShow Filter Graph
    /// https://msdn.microsoft.com/en-us/library/ms867162.aspx#thedirectshowbasicsyouneedtoknow
    ///                                       (04.06.2014.) (Domagoj Saric)
    //...mrmlj...fix this producer-consumer-weird-dshow-synchronization ugliness...
    // https://groups.google.com/forum/#!topic/microsoft.public.win32.programmer.directx.video/FNsMOHLBZ6M
    // http://msdn.microsoft.com/en-us/library/ms867162.aspx
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd376085(v=vs.85).aspx
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd375433(v=vs.85).aspx
    // http://www.gdcl.co.uk/q_and_a.htm#IAsyncReader
    // http://193.68.19.127/install/VS%206%20%20%20%200/cd2/SAMPLES/VC98/SDK/GRAPHICS/DIRECTANIMATION/help/ds/dssd0114.htm
    // http://msdn.microsoft.com/en-us/library/windows/desktop/dd375794(v=vs.85).aspx

    if ( BOOST_UNLIKELY( flushing_ ) )
        return E_UNEXPECTED;

    if ( BOOST_UNLIKELY( endOfStreamEventPending_ ) )
        // https://msdn.microsoft.com/en-us/library/ms923395.aspx
        return E_UNEXPECTED;

#ifndef LE_PUBLIC_BUILD
    LE_TRACE_IF( state_ != State_Running, "DShow calling Receive() in a non running state" ); // state() is recursive here
    LE_TRACE_IF( outputFilled()         , "outputFilled()"                                 );
#endif // LE_PUBLIC_BUILD

    bool sampleConsumed( false );
    do
    {
        bufferLock_.lock();

        BOOST_ASSERT( !flushing_              );
        BOOST_ASSERT( state_ != State_Stopped ); // state() is recursive here

        while ( outputFilled() && state_ /*!= State_Stopped*/ == State_Running )
        {
            // Buffer is full - sleep so consumers can get items.
            bufferNotFull_.wait( bufferLock_ );
        }

        sampleConsumed = copyAndConvertSamples( *pBufferHolder );

        bufferLock_.unlock();

        // If a consumer is waiting, wake it.
        bufferNotEmpty_.signal();
    } while ( ( state_ /*!= State_Stopped*/ == State_Running ) && !sampleConsumed );

    bytesToSkipFromNextBuffer_ = 0;
    return S_OK;
}

#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
HRESULT STDMETHODCALLTYPE FileImpl::ReceiveMultiple( IMediaSample * *, long, long * )
{
    LE_UNREACHABLE_CODE();
    return E_NOTIMPL;
}
#pragma warning( pop )

// http://msdn.microsoft.com/en-us/library/windows/desktop/dd376612(v=vs.85).aspx
HRESULT STDMETHODCALLTYPE FileImpl::Next( ULONG const cPins, IPin * * const ppPins, ULONG * const pcFetched )
{
    BOOST_ASSERT( cPins  );
    BOOST_ASSERT( ppPins );

    if ( onlyPinAlreadyEnumerated_ )
    {
        if ( pcFetched )
            *pcFetched = 0;
        #ifdef LE_USE_RESAMPLER_DMO
            onlyPinAlreadyEnumerated_ = false;
        #endif // LE_USE_RESAMPLER_DMO
        return S_FALSE;
    }

    onlyPinAlreadyEnumerated_ = true;

    if ( pcFetched )
        *pcFetched = 1;
    else
    if ( cPins != 1 )
        return E_POINTER;

    FileImpl::AddRef();
    *ppPins = static_cast<IPin *>( this );
    return S_OK;
}


LE_NOTHROW
char const * FileImpl::openFullPath( wchar_t const * const sampleFileName )
{
    HRESULT hr;
    if ( !comGuard_.initialised() )
    {
        if
        (
            ( ( hr = comGuard_     .initialise    (                           ) ) != S_OK ) ||
            ( ( hr = pGraphBuilder_.createInstance( CLSID_FilterGraphNoThread ) ) != S_OK ) ||
            #if defined( _DEBUG ) && !defined( LE_PUBLIC_BUILD )
            ( ( hr = pGraphBuilder_->SetLogFile   ( logHandle()               ) ) != S_OK ) ||
            #endif // _DEBUG
            ( ( hr = pGraphBuilder_->AddFilter    ( this, nullptr             ) ) != S_OK ) ||
            ( ( hr = pGraphBuilder_.queryInterface( pMediaControl_            ) ) != S_OK ) ||
            ( ( hr = pGraphBuilder_.queryInterface( pMediaEvent_              ) ) != S_OK ) ||
            ( ( hr = pGraphBuilder_.queryInterface( pMediaFilter_             ) ) != S_OK ) ||
            ( ( hr = pGraphBuilder_.queryInterface( pMediaSeeking_            ) ) != S_OK )
        )
            return errorMessage( hr );

        BOOST_VERIFY( pMediaEvent_ ->SetNotifyFlags ( AM_MEDIAEVENT_NONOTIFY ) == S_OK );
        BOOST_VERIFY( pMediaEvent_ ->SetNotifyWindow( NULL, 0, 0             ) == S_OK );
        BOOST_VERIFY( pMediaFilter_->SetSyncSource  ( nullptr                ) == S_OK );
    }
    else
    {
        restart();
        close  ();
    }

    COMPtr<IFileSourceFilter> pFileSource;
    // Implementation note:
    //   As a workaround for issues (access violation crashes) caused by Nero
    // filters we first manually try to load the file using the Windows Media
    // reader filter (this seems to avoid the loading of Nero filters and also
    // enables loading of certain MP3 files that the default Windows DirectShow
    // filter fails to load) and fall back to the
    // IGraphBuilder::AddSourceFilter() method (the WMA filter seems not to be
    // able to load WAV files).
    // http://alax.info/blog/967/comment-page-1
    // http://www.eggheadcafe.com/software/aspnet/30216748/how-to-unregister-nero-6.aspx
    // http://www.riseoftheants.com/mmx/faq.htm
    // http://msdn.microsoft.com/en-us/library/dd389378(v=VS.85).aspx
    // http://groups.google.com/group/microsoft.public.win32.programmer.directx.audio/msg/e1f5ac84ec6e439a
    // http://groups.google.com/group/microsoft.public.win32.programmer.directx.video/browse_thread/thread/46d7e70960ae99bd?pli=1
    //                                        (02.07.2010.) (Domagoj Saric)
    //...mrmlj...deadlocks in the SDK sample app...!?!?
    //if
    //(
    //    ( ( hr = pSourceFilter_ .createInstance( CLSID_WMAsfReader       ) ) != S_OK ) ||
    //    ( ( hr = pSourceFilter_ .queryInterface( pFileSource             ) ) != S_OK ) ||
    //    ( ( hr = pFileSource   ->Load          ( sampleFileName, nullptr ) ) != S_OK ) ||
    //    ( ( hr = pGraphBuilder_->AddFilter     ( pSourceFilter_, nullptr ) ) != S_OK )
    //)
    {
        pSourceFilter_.reset();
        hr = pGraphBuilder_->AddSourceFilter( sampleFileName, nullptr, &pSourceFilter_ );
        if ( hr != S_OK )
            return errorMessage( hr );
        pFileSource.reset();
        BOOST_VERIFY( pSourceFilter_.queryInterface( pFileSource ) == S_OK );
    }

#if 0
    /// \todo Mono AAC decoding seems broken with the default Microsoft decoder
    /// (on Windows 7 x64 SP1 the output is amplified and clipped). Try loading
    /// the FFDShow decoder if present.
    ///                                       (28.11.2014.) (Domagoj Saric)
    GUID const CLSID_FFDShowAudioDecoder = { 0x0F40E1E5, 0x4F79, 0x4988, 0xB1, 0xA9, 0xCC, 0x98, 0x79, 0x4E, 0x6B, 0x55 };
#endif

    BOOST_ASSERT( pSourceFilter_ );
    BOOST_ASSERT( pFileSource    );
    COMPtr<IPin> const pSource( getOutputPin( *pSourceFilter_, hr ) );
    if ( !pSource )
        return errorMessage( hr );

    /// \note Detect the input file format beforehand (before ReceiveConnection
    /// gets called) as a workaround for certain decoder filters
    /// (e.g. Microsoft DTV-DVD Audio Decoder automatically inserted by
    /// GraphBuilder for AAC files on Windows 7 SP1) that try to change the
    /// original sample format (so we, for example, end up reporting 6 channels
    /// to the user for a mono or stereo M4A file).
    ///                                       (25.11.2014.) (Domagoj Saric)
    {
        LPOLESTR pFileName;
        Utility::clear( currentType_ );
        if ( ( hr = pFileSource->GetCurFile( &pFileName, &currentType_.type ) ) != S_OK ) return errorMessage( hr );
        ::CoTaskMemFree( pFileName );
        BOOST_ASSERT( !currentType_.type.pUnk );
        if ( currentType_.type.cbFormat )
        {
            BOOST_ASSERT( currentType_.type.cbFormat <= sizeof( currentType_.format ) );
            std::memcpy( &currentType_.format, currentType_.type.pbFormat, currentType_.type.cbFormat );
            ::CoTaskMemFree( currentType_.type.pbFormat );
            currentType_.type.pbFormat = reinterpret_cast<BYTE *>( &currentType_.format );
        }
        else
        {
            COMPtr<IEnumMediaTypes> pMediaTypesEnumerator;
            if ( ( hr = pSource->EnumMediaTypes( &pMediaTypesEnumerator ) ) != S_OK ) return errorMessage( hr );
            AM_MEDIA_TYPE * pMT;
            currentType_.type.pbFormat = nullptr;
            while
            (
                ( pMediaTypesEnumerator->Next( 1, &pMT, nullptr ) == S_OK    ) &&
                ( currentType_.type.pbFormat                      == nullptr )
            )
            {
                /// \note Here we are only interested in the actual sample
                /// format (not the file encoding format) so we only check the
                /// AM_MEDIA_TYPE::formattype field.
                ///                           (27.11.2014.) (Domagoj Saric)
                if ( pMT->formattype == FORMAT_WaveFormatEx )
                {
                    BOOST_ASSERT( pMT->majortype == MEDIATYPE_Audio               );
                    BOOST_ASSERT( pMT->pbFormat  != nullptr                       );
                    BOOST_ASSERT( pMT->cbFormat  <= sizeof( currentType_.format ) );
                    currentType_.type = *pMT;
                    currentType_.type.pbFormat = reinterpret_cast<BYTE *>( &currentType_.format );
                    std::memcpy( &currentType_.format, pMT->pbFormat, pMT->cbFormat );
                }
                if ( pMT->pbFormat ) ::CoTaskMemFree( pMT->pbFormat );
                                     ::CoTaskMemFree( pMT           );
            }
            BOOST_VERIFY_MSG
            (
                pMediaTypesEnumerator->Next( 1, &pMT, nullptr ) == S_FALSE,
                "File source filter offers more than one output sample format?"
            );
        }
    }

    hr = pGraphBuilder_->Connect( pSource, static_cast<IPin *>( this ) );
    if ( hr != S_OK )
        return errorMessage( hr );

    BOOST_ASSERT( state_ == State_Stopped ); // state() is recursive here

    bytesToSkipFromNextBuffer_ = 0;

    return nullptr;
}

LE_NOTHROW
std::uint32_t FileImpl::read( float * LE_RESTRICT const pOutput, std::uint32_t const sampleFrames ) const
{
    auto const samples( sampleFrames * numberOfChannels() );

    bufferLock_.lock();

    BOOST_ASSERT( outputFilled() );

    outputBuffer_ = OutputBuffer( pOutput, pOutput + samples );

    //...mrmlj...a producer-consumer implementation based only on the IMediaEvent event handle (pre SVN 9239) failed to work properly...
    //HANDLE hEvent;
    //BOOST_VERIFY( pMediaEvent_->GetEventHandle( reinterpret_cast<OAEVENT *>( &hEvent ) ) == S_OK );

    if ( state_ != State_Running ) // state() is recursive here
    {
        HRESULT const hr( pMediaControl_->Run() );
        if ( BOOST_UNLIKELY( ( hr != S_OK ) && ( hr != S_FALSE ) ) )
        {
            LE_TRACE( "File::read() error (%s).", errorMessage( hr ) );
            outputBuffer_ = OutputBuffer();
            bufferLock_.unlock();
            return 0;
        }
    }

    bufferLock_.unlock();
        bufferNotFull_.signal();
    bufferLock_.lock  ();

    while ( !outputFilled() && state_ == State_Running )
    {
        // Buffer is empty/not full - sleep so producers can create items.
        bufferNotEmpty_.wait( bufferLock_ );
    }

    auto const readSampleFrames( static_cast<std::uint32_t>( outputBuffer_.begin() - pOutput ) / numberOfChannels() );
    BOOST_ASSERT( readSampleFrames <= sampleFrames );
    outputBuffer_ = OutputBuffer();

    bufferLock_.unlock();

    return readSampleFrames;
}


// http://msdn.microsoft.com/en-us/library/dd407173(v=vs.85).aspx
// http://support.microsoft.com/kb/316992
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd390676(v=vs.85).aspx
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd757927(v=vs.85).aspx
char const File::supportedFormats[] = "*.aac;*.aif;*.aiff;*.au;*.mpa;*.mp3;*.snd;*.wav;*.wma";

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "pimplForwarders.inl"

LE_OPTIMIZE_FOR_SIZE_END()
