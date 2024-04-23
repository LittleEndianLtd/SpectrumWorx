////////////////////////////////////////////////////////////////////////////////
///
/// sampleWin.cpp
/// -------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//
// http://msdn.microsoft.com/en-us/library/dd377513(VS.85).aspx
// http://msdn.microsoft.com/en-us/library/dd389098(VS.85).aspx
// http://support.microsoft.com/kb/316992
// http://www.google.com/search?hl=hr&rls=com.microsoft:hr&q=CLSID_FilterGraphNoThread&start=10&sa=N
// http://social.msdn.microsoft.com/Forums/en-US/windowsdirectshowdevelopment/thread/dfa87cfd-ad30-47c4-89a9-d0df79ea5f1e/
// http://msdn.microsoft.com/en-us/library/dd375786(VS.85).aspx
// http://www.codeproject.com/KB/mobile/samplegrabberfilter-wm6.aspx
// http://www.codeproject.com/KB/audio-video/dshowencoder.aspx
// http://www.codeproject.com/KB/audio-video/Tanvon_DirectShowFilters.aspx
// http://doc.51windows.net/Directx9_SDK/?url=/Directx9_SDK/htm/grabbersamplefiltersample.htm
// http://www.codeproject.com/KB/windows/samplegrabberfilter-wm6.aspx
// http://social.msdn.microsoft.com/Forums/en-US/windowsdirectshowdevelopment/thread/dfa87cfd-ad30-47c4-89a9-d0df79ea5f1e
//
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "sample.hpp"

#include "le/math/conversion.hpp"
#include "le/math/math.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <boost/assert.hpp>

#include <windows.h>

#include <tchar.h>

#include <array>

//...mrmlj...testing...work in progress...
// http://www.indexcom.com/tech/WindowsAudioSRC
//#define LE_USE_RESAMPLER_DMO

#pragma warning( push )
#pragma warning( disable : 4201 ) // Nonstandard extension used : nameless struct/union.
#include "mmreg.h"
#pragma warning( pop )
#include "strmif.h"

#pragma warning( push )
#pragma warning( disable : 4201 ) // Nonstandard extension used : nameless struct/union.
#include "austream.h"
#include "mmstream.h"

#include "amstream.h"
#include "control.h"
#include "dshow.h"
#include "uuids.h"
#ifdef LE_USE_RESAMPLER_DMO
    #include "dmoreg.h" // DMOCATEGORY_AUDIO_EFFECT
    #include "dmodshow.h" // DMO wrapper
    #include "mftransform.h"
    #include "wmcodecdsp.h" // Audio Resampler DSP
#endif // LE_USE_RESAMPLER_DMO
#pragma warning( pop )

#ifdef LE_USE_RESAMPLER_DMO
    #pragma comment( lib, "dmoguids.lib" ) // CLSID_DMOWrapperFilter
    #pragma comment( lib, "mfuuid.lib" )
    GUID const CLSID_CResamplerMediaObject = { 0xF447B69E, 0x1884, 0x4A7E, 0x80, 0x55, 0x34, 0x6F, 0x74, 0xD6, 0xED, 0xB3 };
#endif // LE_USE_RESAMPLER_DMO


#pragma comment( lib, "Strmiids.lib" )
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

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
    COMPtr(              ) : pT_( 0  ) {}
    COMPtr( T * const pT ) : pT_( pT ) {}

    LE_NOTHROWNOALIAS
    COMPtr( IUnknown & source )
    {
        BOOST_VERIFY( source.QueryInterface( __uuidof( T ), reinterpret_cast<void * *>( &pT_ ) ) == S_OK );
    }

    LE_NOTHROW COMPtr( COMPtr       && other ) : pT_( other.pT_ ) { other.pT_ = 0; }
    LE_NOTHROW COMPtr( COMPtr const &  other ) : pT_( other.pT_ ) { if ( pT ) pT_->AddRef(); }

    LE_NOTHROWNOALIAS
    ~COMPtr() { if ( pT_ ) pT_->Release(); }

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

    void reset()
    {
        if ( pT_ )
        {
            pT_->Release();
            pT_ = 0;
        }
    }

private:
    T * LE_RESTRICT pT_;
}; // class COMPtr<>


////////////////////////////////////////////////////////////////////////////////
///
/// \class Sample::Impl
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
    /// was properly disconnected before a Sample::Impl instance gets destroyed.
    ////////////////////////////////////////////////////////////////////////////

    class ConnectedOutputPin
    {
    public:
         ConnectedOutputPin() : pConnectedOutputPin_( 0 ) {}
        ~ConnectedOutputPin() { BOOST_ASSERT( !pConnectedOutputPin_ ); }

        operator IPin * & () { return pConnectedOutputPin_; }

        IPin * operator->() { BOOST_ASSERT( pConnectedOutputPin_ ); return pConnectedOutputPin_; }

        IPin * & operator=( IPin * const other ) { pConnectedOutputPin_ = other; return pConnectedOutputPin_; }

    private:
        IPin * pConnectedOutputPin_;
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

class Sample::Impl
    :
    private IPin,
    private IMemInputPin,
    private IBaseFilter,
    private IEnumMediaTypes,
    private IEnumPins
{
public:
    using ChannelData = Sample::ChannelData;

public:
    explicit Impl( std::uint32_t allowedSamplerate );
    ~Impl();

    LE_NOTHROWNOALIAS
    char const * load( juce::String const & sampleFileName, DataHolder & data );

private:
    // Implementation note:
    //   std::pair<> forces value initialization.
    //                                        (23.08.2011.) (Domagoj Saric)
    struct DataType
    {
        WAVEFORMATEX  format;
        AM_MEDIA_TYPE type  ;
    };

    unsigned int remainingTypes() const { return static_cast<unsigned int>( types_.size() - currentType_ ); }

    static void fillType( DataType &, unsigned int sampleRate, unsigned short resolution );

#ifdef _DEBUG
    DWORD_PTR logHandle() const { return reinterpret_cast<DWORD_PTR>( hLog_ ); }
#endif // _DEBUG

private: // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface( IID const &, void * * ppvObject ) override;

    ULONG STDMETHODCALLTYPE AddRef () override { return 1; }
    ULONG STDMETHODCALLTYPE Release() override { return 1; }

public: // IPersist
    HRESULT STDMETHODCALLTYPE GetClassID( CLSID * ) override { return E_FAIL; }

public: // IMediaFilter
    HRESULT STDMETHODCALLTYPE Stop () override { state_ = State_Stopped; return S_OK; }
    HRESULT STDMETHODCALLTYPE Pause() override { state_ = State_Paused ; return S_OK; }
    HRESULT STDMETHODCALLTYPE Run  ( REFERENCE_TIME ) override;

    HRESULT STDMETHODCALLTYPE GetState( DWORD dwMilliSecsTimeout, FILTER_STATE * ) override;

    HRESULT STDMETHODCALLTYPE SetSyncSource( IReferenceClock *   ) override;
    HRESULT STDMETHODCALLTYPE GetSyncSource( IReferenceClock * * ) override;

public: // IBaseFilter
    HRESULT STDMETHODCALLTYPE EnumPins( IEnumPins * * ) override;

    HRESULT STDMETHODCALLTYPE FindPin( LPCWSTR /*Id*/, IPin * * ) override { return E_NOTIMPL; }

    HRESULT STDMETHODCALLTYPE QueryFilterInfo( FILTER_INFO * ) override;

    HRESULT STDMETHODCALLTYPE JoinFilterGraph( IFilterGraph *, LPCWSTR pName ) override;

    HRESULT STDMETHODCALLTYPE QueryVendorInfo( LPWSTR * /*pVendorInfo*/ ) override { return E_NOTIMPL; }

private: // IEnumMediaTypes
    HRESULT STDMETHODCALLTYPE Next( ULONG cMediaTypes, AM_MEDIA_TYPE * * ppMediaTypes, ULONG * pcFetched ) override;

    HRESULT STDMETHODCALLTYPE Skip( ULONG cMediaTypes ) override;

    HRESULT STDMETHODCALLTYPE IEnumMediaTypes::Reset() override { currentType_ = 0; return S_OK; }

    HRESULT STDMETHODCALLTYPE Clone( IEnumMediaTypes * * ) override { return E_NOTIMPL; }

public: // IPin
    HRESULT STDMETHODCALLTYPE Connect( IPin *, AM_MEDIA_TYPE const * ) override;

    HRESULT STDMETHODCALLTYPE ReceiveConnection( IPin *, AM_MEDIA_TYPE const * ) override;

    HRESULT STDMETHODCALLTYPE Disconnect() override;

    HRESULT STDMETHODCALLTYPE ConnectedTo( IPin * * ) override;

    HRESULT STDMETHODCALLTYPE ConnectionMediaType( AM_MEDIA_TYPE * ) override;

    HRESULT STDMETHODCALLTYPE QueryPinInfo( PIN_INFO * ) override;

    HRESULT STDMETHODCALLTYPE QueryDirection( PIN_DIRECTION * ) override;

    HRESULT STDMETHODCALLTYPE QueryId    ( LPWSTR * /*Id*/       ) override { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE QueryAccept( AM_MEDIA_TYPE const * ) override { return E_NOTIMPL; }

    HRESULT STDMETHODCALLTYPE EnumMediaTypes( IEnumMediaTypes * * ) override;

    HRESULT STDMETHODCALLTYPE QueryInternalConnections( IPin * *, ULONG * nPin ) override;

    HRESULT STDMETHODCALLTYPE EndOfStream() override;
    HRESULT STDMETHODCALLTYPE BeginFlush () override { return S_OK; }
    HRESULT STDMETHODCALLTYPE EndFlush   () override { return S_OK; }

    HRESULT STDMETHODCALLTYPE NewSegment( REFERENCE_TIME /*tStart*/, REFERENCE_TIME /*tStop*/, double /*dRate*/ ) override { return S_OK; }

private: // IMemInputPin
    HRESULT STDMETHODCALLTYPE GetAllocator( IMemAllocator * * /*ppAllocator*/ ) override;

    HRESULT STDMETHODCALLTYPE NotifyAllocator( IMemAllocator * /*pAllocator*/, BOOL /*bReadOnly*/ ) override { return S_OK; }

    HRESULT STDMETHODCALLTYPE GetAllocatorRequirements( ALLOCATOR_PROPERTIES * /*pProps*/ ) override { return E_NOTIMPL; }

    HRESULT STDMETHODCALLTYPE Receive( IMediaSample * pBufferHolder ) override;

    HRESULT STDMETHODCALLTYPE ReceiveMultiple( IMediaSample * *, long nSamples, long * nSamplesProcessed ) override;

    HRESULT STDMETHODCALLTYPE ReceiveCanBlock() override { return S_FALSE; }

private: // IEnumPins
    HRESULT STDMETHODCALLTYPE Next( ULONG cPins, IPin * * ppPins, ULONG * pcFetched ) override;

    // Shares the IEnumMediaTypes implementation.
    //HRESULT STDMETHODCALLTYPE Skip( ULONG cPins ) override;

    HRESULT STDMETHODCALLTYPE IEnumPins::Reset() override { onlyPinAlreadyEnumerated_ = false; return S_OK; }

    HRESULT STDMETHODCALLTYPE Clone( IEnumPins * * /*ppEnum*/ ) override { return E_NOTIMPL; }

private:
    Sample::DataHolder data_;

    unsigned int resolutionInBytesPerSample_;

    // 'Data type enumerator' members
    unsigned int            currentType_;
#ifdef LE_USE_RESAMPLER_DMO
    std::array<DataType, 4> types_      ;
#else
    std::array<DataType, 3> types_      ;
#endif // LE_USE_RESAMPLER_DMO

    // 'Pin enumerator' members
    bool onlyPinAlreadyEnumerated_;

    // 'Pin' members
    ConnectedOutputPin pConnectedOutputPin_;

    // 'Filter' members
    FILTER_STATE   state_   ;
    IFilterGraph * pMyGraph_;

    COMPtr<IGraphBuilder> pGraphBuilder_;

    COMPtr<IBaseFilter>   pSourceFilter_;

    COMPtr<IMediaControl> pMediaControl_;
    COMPtr<IMediaSeeking> pMediaSeeking_;
    COMPtr<IMediaFilter > pMediaFilter_ ;
    COMPtr<IMediaEventEx> pMediaEvent_  ;

#ifdef _DEBUG
    HANDLE hLog_;
#endif // _DEBUG
}; // Sample::Impl

namespace
{
    #ifdef _DEBUG
    HANDLE openLogFile()
    {
        TCHAR expandedBuffer[ MAX_PATH ];
        BOOST_VERIFY( ::ExpandEnvironmentStrings( _T( "%TEMP%\\SpectrumWorxDirectShow.log" ), expandedBuffer, _countof( expandedBuffer ) ) < _countof( expandedBuffer ) );
        HANDLE const logHandle( ::CreateFile( expandedBuffer, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, nullptr, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr ) );
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
                    BOOST_ASSERT( FAILED( hrTmp ) && pTmp == 0 );
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

Sample::Impl::Impl( std::uint32_t const allowedSamplerate )
    :
    state_                   ( State_Stopped ),
    pMyGraph_                ( 0             ),
    currentType_             ( 0             ),
    onlyPinAlreadyEnumerated_( false         )
    #ifdef _DEBUG
        ,hLog_( openLogFile() )
    #endif // _DEBUG
{
#ifdef _DEBUG
	BOOST_ASSERT( hLog_ );
#endif // _DEBUG

    fillType( types_[ 0 ], allowedSamplerate, 16 );
    fillType( types_[ 1 ], allowedSamplerate, 24 );
    fillType( types_[ 2 ], allowedSamplerate, 32 );

#ifdef LE_USE_RESAMPLER_DMO
    types_[ 3 ] = types_[ 2 ];
    types_[ 3 ].format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
    types_[ 3 ].type.subtype      = MEDIASUBTYPE_IEEE_FLOAT;
#endif // LE_USE_RESAMPLER_DMO
}


Sample::Impl::~Impl()
{
#ifdef _DEBUG
    //...mrmlj...this sometimes fails so it is not currently checked as a (harmless) workaround...
    /*BOOST_VERIFY*/( ::CloseHandle( hLog_ ) );
#endif // _DEBUG
}


void Sample::Impl::fillType( DataType & type, unsigned int const sampleRate, unsigned short const resolution )
{
    WAVEFORMATEX & desiredFormat( type.format );
    desiredFormat.wFormatTag      = WAVE_FORMAT_PCM; //WAVE_FORMAT_IEEE_FLOAT;
    desiredFormat.nChannels       = fixedNumberOfChannels;
    desiredFormat.nSamplesPerSec  = sampleRate;
    desiredFormat.wBitsPerSample  = resolution;
    desiredFormat.nAvgBytesPerSec = desiredFormat.nSamplesPerSec * desiredFormat.wBitsPerSample / 8 * desiredFormat.nChannels;
    desiredFormat.nBlockAlign     = desiredFormat.nChannels      * desiredFormat.wBitsPerSample / 8;
    desiredFormat.cbSize          = 0;

    AM_MEDIA_TYPE & mt( type.type );
    mt.majortype            = MEDIATYPE_Audio;
    mt.subtype              = MEDIASUBTYPE_PCM;
    mt.bFixedSizeSamples    = true;
    mt.bTemporalCompression = false;
    mt.lSampleSize          = desiredFormat.nBlockAlign;
    mt.formattype           = FORMAT_WaveFormatEx;
    mt.pUnk                 = 0;
    mt.cbFormat             = sizeof( desiredFormat );
    mt.pbFormat             = reinterpret_cast<BYTE *>( &desiredFormat );
}


HRESULT STDMETHODCALLTYPE Sample::Impl::QueryInterface( IID const & iid, void * * const ppvObject )
{
    BOOST_ASSERT( ppvObject );

    if ( iid == IID_IMediaSeeking && pConnectedOutputPin_ /*...mrmlj...a temporary quick-fix attempt for issues Danijel encountered with Nero filters...*/ )
    {
        BOOST_ASSERT( pConnectedOutputPin_ );
        return pConnectedOutputPin_->QueryInterface( iid, ppvObject );
    }
    else
    if ( iid == __uuidof( IUnknown ) )
    {
        *ppvObject = static_cast<IUnknown *>( static_cast<IPin *>( this ) );
        return S_OK;
    }
    else
    if ( iid == __uuidof( IPin ) )
    {
        *ppvObject = static_cast<IPin *>( this );
        return S_OK;
    }
    else
    if ( iid == __uuidof( IMemInputPin ) )
    {
        *ppvObject = static_cast<IMemInputPin *>( this );
        return S_OK;
    }
    else
    if ( iid == __uuidof( IBaseFilter ) )
    {
        *ppvObject = static_cast<IBaseFilter *>( this );
        return S_OK;
    }
    else
    if ( iid == __uuidof( IEnumMediaTypes ) )
    {
        *ppvObject = static_cast<IEnumMediaTypes *>( this );
        return S_OK;
    }
    else
    if ( iid == __uuidof( IEnumPins ) )
    {
        *ppvObject = static_cast<IEnumPins *>( this );
        return S_OK;
    }
    else
    {
        *ppvObject = nullptr;
        return E_NOINTERFACE;
    }
}


HRESULT STDMETHODCALLTYPE Sample::Impl::Run( REFERENCE_TIME /*tStart*/ )
{
    BOOST_ASSERT( state_ != State_Running );
    state_ = State_Running;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::GetState( DWORD /*dwMilliSecsTimeout*/, FILTER_STATE * const pState )
{
    BOOST_ASSERT( pState );
    *pState = state_;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::SetSyncSource( IReferenceClock * const pClock )
{
    BOOST_VERIFY( !pClock );
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::GetSyncSource( IReferenceClock * * const pClock )
{
    BOOST_ASSERT( pClock );
    *pClock = 0;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::EnumPins( IEnumPins * * const ppEnum )
{
    BOOST_ASSERT( ppEnum );
    *ppEnum = static_cast<IEnumPins *>( this );
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::QueryFilterInfo( FILTER_INFO * const pInfo )
{
    BOOST_ASSERT( pInfo );
    pInfo->achName[ 0 ] = 0;
    pInfo->pGraph = pMyGraph_;
    pMyGraph_->AddRef();
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::JoinFilterGraph( IFilterGraph * const pGraph, LPCWSTR /*pName*/ )
{
    BOOST_ASSERT( !pGraph || !pMyGraph_ );
    pMyGraph_ = pGraph;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::Next( ULONG const cMediaTypes, AM_MEDIA_TYPE * * const ppMediaTypes, ULONG * const pcFetched )
{
    BOOST_ASSERT( ppMediaTypes );
    *ppMediaTypes = 0;
    unsigned int const numberOfTypes( std::min<unsigned int>( cMediaTypes, remainingTypes() ) );
    unsigned int const targetType   ( currentType_ + numberOfTypes                            );
    AM_MEDIA_TYPE * * ppMediaType = ppMediaTypes;
    while ( currentType_ < targetType )
    {
        AM_MEDIA_TYPE * const pMediaType( static_cast<AM_MEDIA_TYPE *>( ::CoTaskMemAlloc( sizeof( AM_MEDIA_TYPE ) ) ) );
        if ( !pMediaType )
            break;
        *pMediaType = types_[ currentType_ ].type;
        pMediaType->pbFormat = static_cast<BYTE *>( ::CoTaskMemAlloc( pMediaType->cbFormat ) );
        if ( !pMediaType->pbFormat )
        {
            ::CoTaskMemFree( pMediaType );
            break;
        }
        std::memcpy( pMediaType->pbFormat, &types_[ currentType_ ].type.pbFormat, pMediaType->cbFormat );
        *ppMediaType++ = pMediaType;
        ++currentType_;
    }
    unsigned int const written( static_cast<unsigned int>( ppMediaType - ppMediaTypes ) );
    if ( pcFetched )
        *pcFetched = written;
    return ( written == cMediaTypes ) ? S_OK : S_FALSE;
}

#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
HRESULT STDMETHODCALLTYPE Sample::Impl::Skip( ULONG /*cMediaTypes*/ )
{
    LE_UNREACHABLE_CODE();
    return S_FALSE;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::Connect( IPin * /*pReceivePin*/, AM_MEDIA_TYPE const * /*pmt*/ )
{
    LE_UNREACHABLE_CODE();
    return E_NOTIMPL;
}
#pragma warning( pop )

HRESULT STDMETHODCALLTYPE Sample::Impl::ReceiveConnection( IPin * const pConnector, AM_MEDIA_TYPE const * const pmt )
{
    BOOST_ASSERT( !pConnectedOutputPin_ );
    BOOST_ASSERT( state_ == State_Stopped );
    std::size_t const typeRelevantSize  ( offsetof( AM_MEDIA_TYPE, pUnk   ) );
    std::size_t const formatRelevantSize( offsetof( WAVEFORMATEX , cbSize ) );
    for ( auto & type : types_ )
    {
        if
        (
            ( std::memcmp( &type.type  , pmt          , typeRelevantSize   ) == 0 ) &&
            ( std::memcmp( &type.format, pmt->pbFormat, formatRelevantSize ) == 0 )
        )
        {
            #ifndef NDEBUG
                PIN_DIRECTION connectorDirection;
                BOOST_VERIFY( pConnector->QueryDirection( &connectorDirection ) == S_OK );
                BOOST_ASSERT( ( connectorDirection == PINDIR_OUTPUT ) || ( connectorDirection == PINDIR_INPUT ) );
                BOOST_ASSERT( connectorDirection != PINDIR_INPUT );
            #endif //NDEBUG
            pConnectedOutputPin_ = pConnector;
            pConnectedOutputPin_->AddRef();
            BOOST_ASSERT_MSG
            (
                ( type.format.wBitsPerSample / 8 ) == ( type.format.nBlockAlign / type.format.nChannels ),
                "Padded data types (e.g. unpacked 24 bit) not supported."
            );
            resolutionInBytesPerSample_ = type.format.wBitsPerSample / 8;
            return S_OK;
        }
    }

    return VFW_E_TYPE_NOT_ACCEPTED;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::Disconnect()
{
    BOOST_ASSERT( pConnectedOutputPin_ );
    pConnectedOutputPin_->Release();
    pConnectedOutputPin_ = 0;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::ConnectedTo( IPin * * const ppPin )
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
HRESULT STDMETHODCALLTYPE Sample::Impl::ConnectionMediaType( AM_MEDIA_TYPE * const /*pmt*/ )
{
    LE_UNREACHABLE_CODE();
    return E_NOTIMPL;
}
#pragma warning( pop )

HRESULT STDMETHODCALLTYPE Sample::Impl::QueryPinInfo( PIN_INFO * const pInfo )
{
    BOOST_ASSERT( pInfo );
    pInfo->pFilter = static_cast<IBaseFilter *>( this );
    pInfo->dir     = PINDIR_INPUT;
    pInfo->achName[ 0 ] = 0;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::QueryDirection( PIN_DIRECTION * const pPinDir )
{
    BOOST_ASSERT( pPinDir );
    *pPinDir = PINDIR_INPUT;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::EnumMediaTypes( IEnumMediaTypes * * const ppEnum )
{
    BOOST_ASSERT( ppEnum );
    *ppEnum = static_cast<IEnumMediaTypes *>( this );
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::QueryInternalConnections( IPin * * const ppPin, ULONG * const pNPin )
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
HRESULT STDMETHODCALLTYPE Sample::Impl::EndOfStream()
{
    BOOST_ASSERT( pMyGraph_ );
    COMPtr<IMediaEventSink> pEventSink( *pMyGraph_ );
    if ( !pEventSink )
        return S_FALSE;
    BOOST_VERIFY( pEventSink->Notify( EC_COMPLETE, S_OK, reinterpret_cast<LONG_PTR>( static_cast<IUnknown *>( static_cast<IBaseFilter *>( this ) ) ) ) == S_OK );
    return S_OK;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::GetAllocator( IMemAllocator * * /*ppAllocator*/ )
{
    return VFW_E_NO_ALLOCATOR;
}


HRESULT STDMETHODCALLTYPE Sample::Impl::Receive( IMediaSample * const pBufferHolder )
{
    BOOST_ASSERT( pBufferHolder );
    //...mrmlj...BOOST_ASSERT( pBufferHolder->IsDiscontinuity() != S_OK );
    BOOST_ASSERT( pBufferHolder->IsPreroll() != S_OK );
    //...mrmlj...BOOST_ASSERT( pBufferHolder->IsSyncPoint() == S_OK );

    // Implementation note:
    //   When loading MP3s after one has already been loaded, this method
    // (eventually) gets called with a buffer larger than the remaining space
    // left in our buffer so we must explicitly handle this situation.
    //                                        (12.07.2010.) (Domagoj Saric)
    /// \todo Reinvestigate the above issue.
    ///                                       (12.07.2010.) (Domagoj Saric)
    LONG const reportedBufferLen( pBufferHolder->GetActualDataLength() );
    BOOST_ASSERT( reportedBufferLen <= pBufferHolder->GetSize() );
    BOOST_ASSERT( data_.pChannel1End <= data_.pChannel2Beginning );
    unsigned int const bufferSpaceLeftInFloatsPerChannel( static_cast<unsigned int>( data_.pChannel2Beginning - data_.pChannel1End ) );
    unsigned int const bufferSpaceLeft                  ( bufferSpaceLeftInFloatsPerChannel * 2 * resolutionInBytesPerSample_        );
    unsigned int const inputLength                      ( std::min<unsigned int>( reportedBufferLen, bufferSpaceLeft )               );

    bool const endOfStream( inputLength == bufferSpaceLeft );

    BYTE * pBuffer;
    BOOST_VERIFY( pBufferHolder->GetPointer( &pBuffer ) == S_OK );
    BYTE const * const dataEnd( pBuffer + inputLength );

    BOOST_ASSERT( inputLength % ( resolutionInBytesPerSample_ * 2 ) == 0 );

    switch ( resolutionInBytesPerSample_ )
    {
        case 2:
        {
            float         const scale  ( - 1.0f / std::numeric_limits<short>::min() );
            short const *       pSample( reinterpret_cast<short const *>( pBuffer ) );

            while ( pSample < reinterpret_cast<short const *>( dataEnd ) )
            {
                BOOST_ASSERT( data_.pChannel1End < data_.pChannel2Beginning );

                *data_.pChannel1End++ = LE::Math::convert<float>( *pSample++ ) * scale;
                *data_.pChannel2End++ = LE::Math::convert<float>( *pSample++ ) * scale;
            }
            break;
        }

        case 3:
        {
            class Two24bitSamples
            {
            public:
                int firstSample24bit() const { return firstSample_; }

                int firstSample32bit () const { return static_cast<int>( ( firstSample24bit () +  1 ) * 0xFF - 1 ); }
                int secondSample32bit() const { return static_cast<int>(   secondSample24bit() << 8   | 0xFF     ); }

            #ifdef _M_X64
                int secondSample24bit() const { return secondSample_; }
            #else // avoid importing MSVC long long asm routines on 32 bit machines.
                int secondSample24bit() const { return ( secondSampleFirstByte_ & 0xFF ) | ( secondSampleLastTwoBytes_ << 8 ); }
            #endif

            private:
            #ifdef _M_X64
                signed long long firstSample_  : 24;
                signed long long secondSample_ : 24;
            #else
                signed long  firstSample_              : 24;
                signed long  secondSampleFirstByte_    : 8 ;
                signed short secondSampleLastTwoBytes_     ;
            #endif
            };

            float const scale( 1.0f / ( 1U << 23 ) );

            while ( pBuffer < dataEnd )
            {
                BOOST_ASSERT( data_.pChannel1End < data_.pChannel2Beginning );

                Two24bitSamples const & twoSamples( *reinterpret_cast<Two24bitSamples const *>( pBuffer ) );

                *data_.pChannel1End++ = Math::convert<float>( twoSamples.firstSample24bit () ) * scale;
                *data_.pChannel2End++ = Math::convert<float>( twoSamples.secondSample24bit() ) * scale;

                pBuffer += 2 * 24/8;
            }

            break;
        }

        case 4:
        {
            float         const scale  ( - 1.0f / std::numeric_limits<int>::min() );
            int   const *       pSample( reinterpret_cast<int const *>( pBuffer ) );

            while ( pSample < reinterpret_cast<int const *>( dataEnd ) )
            {
                BOOST_ASSERT( data_.pChannel1End < data_.pChannel2Beginning );

                *data_.pChannel1End++ = static_cast<float>( *pSample++ ) * scale;
                *data_.pChannel2End++ = static_cast<float>( *pSample++ ) * scale;
            }
            break;
        }

        LE_DEFAULT_CASE_UNREACHABLE();
    }


    // Implementation note:
    //   In some hosts (Live 7.0.14) we have to manually detect the end of the
    // stream and send the appropriate notification otherwise loading never
    // finishes and finally gets aborted.
    //                                        (18.06.2010.) (Domagoj Saric)
    if ( endOfStream )
    {
        BOOST_VERIFY( Sample::Impl::EndOfStream() == S_OK );
        return S_FALSE;
    }
    else
        return S_OK;
}

#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
HRESULT STDMETHODCALLTYPE Sample::Impl::ReceiveMultiple( IMediaSample * *, long, long * )
{
    LE_UNREACHABLE_CODE();
    return E_NOTIMPL;
}
#pragma warning( pop )

// http://msdn.microsoft.com/en-us/library/windows/desktop/dd376612(v=vs.85).aspx
HRESULT STDMETHODCALLTYPE Sample::Impl::Next( ULONG const cPins, IPin * * const ppPins, ULONG * const pcFetched )
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

    *ppPins = static_cast<IPin *>( this );
    return S_OK;
}


LE_NOTHROWNOALIAS
char const * Sample::Impl::load( juce::String const & sampleFileName, Sample::DataHolder & data )
{
    HRESULT hr;
    if
    (
        ( ( hr = pGraphBuilder_.createInstance( CLSID_FilterGraphNoThread                   ) ) != S_OK ) ||
        #ifdef _DEBUG
        ( ( hr = pGraphBuilder_->SetLogFile   ( logHandle()                                 ) ) != S_OK ) ||
        #endif // _DEBUG
        ( ( hr = pGraphBuilder_->AddFilter    ( static_cast<IBaseFilter *>( this ), nullptr ) ) != S_OK ) ||
        ( ( hr = pGraphBuilder_.queryInterface( pMediaControl_ ) ) != S_OK ) ||
        ( ( hr = pGraphBuilder_.queryInterface( pMediaEvent_   ) ) != S_OK ) ||
        ( ( hr = pGraphBuilder_.queryInterface( pMediaFilter_  ) ) != S_OK ) ||
        ( ( hr = pGraphBuilder_.queryInterface( pMediaSeeking_ ) ) != S_OK )
    )
        return errorMessage( hr );

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
    {
        wchar_t const * const sampleFileNameW( sampleFileName.toWideCharPointer() );
        COMPtr<IFileSourceFilter> pAsfSource;
        if
        (
            ( ( hr = pSourceFilter_ .createInstance( CLSID_WMAsfReader        ) ) != S_OK ) ||
            ( ( hr = pSourceFilter_ .queryInterface( pAsfSource               ) ) != S_OK ) ||
            ( ( hr = pAsfSource    ->Load          ( sampleFileNameW, nullptr ) ) != S_OK ) ||
            ( ( hr = pGraphBuilder_->AddFilter     ( pSourceFilter_ , nullptr ) ) != S_OK )
        )
        {
            pSourceFilter_.reset();
            hr = pGraphBuilder_->AddSourceFilter( sampleFileNameW, nullptr, &pSourceFilter_ );
            if ( hr != S_OK )
                return errorMessage( hr );
        }

        { // Resampling:

            // Resources on resampling:
            // https://ccrma.stanford.edu/~jos/resample/Free_Resampling_Software.html
            // http://music.columbia.edu/cmc/music-dsp/musicdspfaq.html#doupsampling
            // http://www.kvraudio.com/forum/viewtopic.php?t=217232
            // http://msdn.microsoft.com/en-us/site/dd443196
            // http://www.codeforge.com/article/184169
            // http://www.chrisnet.net/code.htm

        #ifdef LE_USE_RESAMPLER_DMO
            //...mrmlj...testing...work in progress...

            // http://msdn.microsoft.com/en-us/library/dd443196(VS.85).aspx
            // http://www.ernzo.com/Mpeg4Dmo.aspx

            COMPtr<IBaseFilter> pDMOWrapperFilter;
            BOOST_VERIFY( pDMOWrapperFilter.createInstance( CLSID_DMOWrapperFilter ) == S_OK );
            COMPtr<IDMOWrapperFilter> pDMOWrapper;
            BOOST_VERIFY( pDMOWrapperFilter.queryInterface( pDMOWrapper ) == S_OK );
            BOOST_VERIFY( pDMOWrapper );
            BOOST_VERIFY( pDMOWrapper->Init( CLSID_CResamplerMediaObject, DMOCATEGORY_AUDIO_EFFECT ) == S_OK );

            COMPtr<IMediaObject> pIMediaObject;
            BOOST_VERIFY( pDMOWrapper.queryInterface( pIMediaObject ) == S_OK );
            BOOST_VERIFY( pGraphBuilder_->AddFilter( pDMOWrapperFilter, L"Resampler" ) == S_OK );
            // http://msdn.microsoft.com/en-us/library/aa451595.aspx
            //hr = pIMediaObject->SetOutputType( 0, reinterpret_cast<DMO_MEDIA_TYPE const *>( &types_[ 3 ] ), 0 );

        #else

            // Implementation note:
            //   As a further workaround for broken DirectShow filter
            // collections/setups here we explicitly add the ACM wrapper
            // (http://msdn.microsoft.com/en-us/library/windows/desktop/dd373410(v=vs.85).aspx)
            // to the graph in order to make the graph builder first consider
            // the built in ACM PCM codec that provides resampling functionality
            // (otherwise long sample loading times were encountered for example
            // on Danijel's WinXP installation where the Wavelab EQ1 filter was
            // tried before the built in PCM codec).
            //   No proper error handling is done here because there is nothing
            // that we can actually do in case of an error except let the graph
            // building to continue and possibly find/construct a different
            // working graph.
            //                                (19.07.2010.) (Domagoj Saric)
            COMPtr<IBaseFilter> pPCMFilter;
            BOOST_VERIFY( pPCMFilter.createInstance( CLSID_ACMWrapper    ) == S_OK );
            BOOST_VERIFY( pGraphBuilder_->AddFilter( pPCMFilter, nullptr ) == S_OK );

        #endif // LE_USE_RESAMPLER_DMO
        }
    }

    BOOST_ASSERT( pSourceFilter_ );
    COMPtr<IPin> const pSource( getOutputPin( *pSourceFilter_, hr ) );
    if ( !pSource )
        return errorMessage( hr );

    hr = pGraphBuilder_->Connect( pSource, static_cast<IPin *>( this ) );
    if ( hr != S_OK )
        return errorMessage( hr );

    BOOST_VERIFY( pMediaEvent_->SetNotifyFlags ( AM_MEDIAEVENT_NONOTIFY ) == S_OK );
    BOOST_VERIFY( pMediaEvent_->SetNotifyWindow( NULL, 0, 0             ) == S_OK );
    HANDLE hEvent;
    BOOST_VERIFY( pMediaEvent_->GetEventHandle( reinterpret_cast<OAEVENT *>( &hEvent ) ) == S_OK );

    long long reftime;
    BOOST_VERIFY( pMediaSeeking_->GetDuration( &reftime ) == S_OK );
    BOOST_ASSERT( reftime );
    #ifndef NDEBUG
    {
        GUID timeFormat;
        BOOST_VERIFY( pMediaSeeking_->GetTimeFormat( &timeFormat ) == S_OK );
        BOOST_ASSERT( timeFormat == TIME_FORMAT_MEDIA_TIME );
    }
    #endif // NDEBUG
    unsigned long long const hundredNanoSecondsPerSecond( 1000 * 1000 * 1000ULL / 100 );
    std::size_t const numberOfSamples
    (
        static_cast<std::size_t>
        (
            LE::Math::roundUpUnsignedIntegerDivision
            (
                static_cast<unsigned long long>( reftime ) * types_.front().format.nSamplesPerSec,
                hundredNanoSecondsPerSecond
            )
        )
    );
    BOOST_ASSERT( ( pMediaSeeking_->GetCurrentPosition( &reftime ) == S_OK ) && ( reftime == 0 ) );

    if ( !data_.recreate( numberOfSamples ) )
        return "Out of memory.";

#ifndef NDEBUG
    FILTER_STATE state;
    BOOST_VERIFY( pMediaControl_->GetState( INFINITE, reinterpret_cast<OAFilterState *>( &state ) ) == S_OK );
    BOOST_ASSERT( state == State_Stopped );
#endif // NDEBUG

    // Set the graph clock.
    BOOST_VERIFY( pMediaFilter_->SetSyncSource( 0 ) == S_OK );

    hr = pMediaControl_->Run();
    if ( ( hr != S_OK ) && ( hr != S_FALSE ) )
        return errorMessage( hr );

#ifndef NDEBUG
    hr = pMediaControl_->GetState( INFINITE, reinterpret_cast<OAFilterState *>( &state ) );
    BOOST_ASSERT( hr == S_OK || hr == VFW_S_STATE_INTERMEDIATE );
    BOOST_ASSERT( state == State_Running );
#endif // NDEBUG

    DWORD const waitResult( ::WaitForSingleObject( hEvent, 10000 ) );
    //...mrmlj...quick-fix for Ableton Live 7...
    //if ( waitResult != WAIT_OBJECT_0 )
    //if ( channel1().empty() )
    if ( data_.pChannel1End == data_.pBuffer.get() )
        return "Failed to read data."; waitResult;

    BOOST_VERIFY( pMediaControl_->Stop() == S_OK );

#ifndef NDEBUG
    BOOST_VERIFY( pMediaControl_->GetState( INFINITE, reinterpret_cast<OAFilterState *>( &state ) ) == S_OK );
    BOOST_ASSERT( state == State_Stopped );
#endif // NDEBUG

    data.takeDataFrom( this->data_ );

    return nullptr;
}


juce::String Sample::supportedFormats()
{
    // http://msdn.microsoft.com/en-us/library/dd407173(v=vs.85).aspx
    // http://support.microsoft.com/kb/316992
    return "*.aif;*.aiff;*.au;*.mpa;*.mp3;*.snd;*.wav;*.wma";
}


LE_NOTHROWNOALIAS
char const * Sample::doLoad( juce::String const & sampleFileName, unsigned int const desiredSampleRate, Sample::DataHolder & data )
{
    {
        // Implementation note:
        //   Because we load samples on dedicated background threads we have to
        // explicitly initialize COM for those ('private') threads.
        //                                    (12.03.2010.) (Domagoj Saric)
        HRESULT const result( ::CoInitializeEx( 0, COINIT_MULTITHREADED | COINIT_DISABLE_OLE1DDE | COINIT_SPEED_OVER_MEMORY ) );
        switch ( result )
        {
            case RPC_E_CHANGED_MODE:
            #ifdef _DEBUG
                std::printf( "Tried to change thread COM concurrency model.\n" );
            #endif // _DEBUG
                LE_UNREACHABLE_CODE();

            case S_FALSE:
            #ifdef _DEBUG
                std::printf( "COM already initialized.\n" );
            #endif // _DEBUG
                LE_UNREACHABLE_CODE();

            case S_OK:
                break;

            default:
                return errorMessage( result );
        }
    }

    class COMUninitializer
    {
    public:
        ~COMUninitializer()
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
    } const comGuard;

    Sample::Impl loader( desiredSampleRate );
    return loader.load( sampleFileName, data );
}

//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
