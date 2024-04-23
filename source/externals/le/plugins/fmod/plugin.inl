////////////////////////////////////////////////////////////////////////////////
///
/// plugin.inl
/// ----------
///
/// Copyright (c) 2012 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/utility/countof.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tchar.hpp"
#include "le/utility/trace.hpp"

#include "boost/array.hpp"
#include "boost/assert.hpp"
//------------------------------------------------------------------------------
template <class Impl> struct FMODFactoryInjector;

#ifdef LE_SW_FMOD_SHARED_BUILD
extern "C" F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION * F_API FMODGetDSPDescription();
#endif // LE_SW_FMOD_SHARED_BUILD
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

#if defined( _MSC_VER ) || ( defined( __GNUC__ ) && !defined( __clang__ ) )
    #ifndef HAS_FRIEND_INJECTION
        #define HAS_FRIEND_INJECTION
    #endif // HAS_FRIEND_INJECTION
#endif


#ifndef NDEBUG
    template <class Impl>
    unsigned int Plugin<Impl, Protocol::FMOD>::pluginInstanceCount_( 0 );
#endif


template <class Impl>
Plugin<Impl, Protocol::FMOD>::Plugin( ConstructionParameter const constructionParameter )
    :
    FMODPluginBase( constructionParameter )
#ifndef NDEBUG
    ,pluginInstanceID_( pluginInstanceCount_++ )
#endif
{
#ifndef NDEBUG
    std::printf( "---- %s FMOD instance %d created ----\n", Impl::name, pluginInstanceID_ );
#endif

#if defined( LE_SW_FMOD_SHARED_BUILD ) && !defined( HAS_FRIEND_INJECTION )
    #if defined( _MSC_VER ) && _MSC_VER <= 1500
        template class ::FMODFactoryInjector<Impl>;
    #else
        ::FMODFactoryInjector<Impl> dummy;
        FMOD_DSP_DESCRIPTION * (F_API * pFMODGetDSPDescription)() = &::FMODGetDSPDescription;
        boost::ignore_unused_variable_warning( dummy                  );
        boost::ignore_unused_variable_warning( pFMODGetDSPDescription );
    #endif
#endif // LE_SW_FMOD_SHARED_BUILD && !HAS_FRIEND_INJECTION
}


template <class Impl>
LE_NOTHROW void Plugin<Impl, Protocol::FMOD>::getParameterValueString( unsigned int const index, char * const pBuffer ) const
{
    if ( pBuffer )
    {
    #if defined( LE_SW_PURE_FMOD ) && !defined( _DEBUG )
        boost::ignore_unused_variable_warning( index );
        LE_TRACE( "FMOD requested parameter value printing in a low-level-only build." );
        pBuffer[ 0 ] = 0;
    #else
        pBuffer[  0 ] = 0;
        pBuffer[ 15 ] = 0; //...mrmlj...
        impl().getParameterDisplay( ParameterIndex( index ), *reinterpret_cast<ParameterStringBuf *>( pBuffer ), nullptr );
        BOOST_ASSERT( std::strlen( pBuffer ) < 16 );
    #endif // LE_SW_PURE_FMOD
    }
}


////////////////////////////////////////////////////////////////////////////////
//
// FMODPlugin<>::create()
// ----------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::create( FMOD_DSP_STATE * LE_RESTRICT const pDSP )
{
    BOOST_ASSERT( pDSP            );
    BOOST_ASSERT( pDSP->instance  );
    BOOST_ASSERT( pDSP->callbacks );
    BOOST_ASSERT( pDSP->sidechaindata     == nullptr );
    BOOST_ASSERT( pDSP->sidechainchannels == 0       );

             int samplerate;
    unsigned int blockSize;
    {
        FMOD_DSP_STATE_SYSTEMCALLBACKS const & callbacks( *pDSP->callbacks );
        BOOST_VERIFY( callbacks.getsamplerate( pDSP, &samplerate ) == FMOD_OK );
        BOOST_VERIFY( callbacks.getblocksize ( pDSP, &blockSize  ) == FMOD_OK );
        Detail::setMemoryCallbacks( callbacks );
    }

    pDSP->plugindata = nullptr;

    std::auto_ptr<Impl> pImpl( new (std::nothrow) Impl( nullptr ) );
    if
    (
        pImpl.get() &&
        Impl::makeErrorCode( pImpl->initialise   (                                  ) ) == Success &&
        Impl::makeErrorCode( pImpl->setSampleRate( static_cast<float>( samplerate ) ) ) == Success &&
        Impl::makeErrorCode( pImpl->setBlockSize ( blockSize                        ) ) == Success
    )
    {
        pDSP->plugindata = pImpl.release();
        return FMOD_OK;
    }
    else
    {
        return FMOD_ERR_MEMORY;
    }
}


template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::release( FMOD_DSP_STATE * const pDSP )
{
    delete &impl( pDSP );
    return FMOD_OK;
}


template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::reset( FMOD_DSP_STATE * const pDSP )
{
    impl( pDSP ).suspend();
    impl( pDSP ).resume ();
    return FMOD_OK;
}


template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::read
(
    FMOD_DSP_STATE * LE_RESTRICT const pDSP      ,
    float          * LE_RESTRICT const inBuffer  ,
    float          * LE_RESTRICT const outBuffer ,
    unsigned int                 const length    ,
             int                 const inChannels,
             int   * LE_RESTRICT const pOutChannels
)
{
    Impl & impl( Plugin::impl( pDSP ) );

    LE_ASSUME( inChannels == *pOutChannels );

    BOOST_ASSERT_MSG( length <= impl.processBlockSize(), "FMOD changed maximum processing block size in the middle of processing." );

    //pDSP->channelmask        == FMOD_CHANNELMASK_STEREO
    //pDSP->source_speakermode == FMOD_SPEAKERMODE_STEREO
    auto const numberOfMainChannels( inChannels              );
    auto const numberOfSideChannels( pDSP->sidechainchannels );
    if ( ( numberOfSideChannels != 0 ) && ( numberOfSideChannels != numberOfMainChannels ) )
    {
        LE_TRACE( "Different main and side signal channel counts - not yet supported" );
        return FMOD_ERR_UNIMPLEMENTED;
    }

    if ( unsigned( numberOfMainChannels ) != impl.numberOfChannels() )
    {
        LE_TRACE( "FMOD changed the channel configuration in the middle of processing (%u -> %u).", impl.numberOfChannels(), numberOfMainChannels );
        if ( !impl.setNumberOfChannels( numberOfMainChannels + numberOfSideChannels, numberOfMainChannels ) )
            return FMOD_ERR_MEMORY;
    }

    impl.process( inBuffer, pDSP->sidechaindata, outBuffer, length );
    return FMOD_OK;
}


template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::setTimePosition( FMOD_DSP_STATE * const pDSP, unsigned int const positionInSamples )
{
    impl( pDSP ).setPosition( positionInSamples );
    return FMOD_OK;
}


////////////////////////////////////////////////////////////////////////////////
//
// FMODPlugin::getParameter()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
template <typename T>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::getParameter( FMOD_DSP_STATE * const pDSP, int const index, T * const pValue, char * const pValueString )
{
    BOOST_ASSERT( pValue );
    Impl & effect( impl( pDSP ) );
    *pValue = /*...mrmlj...*/ Math::convert<T>( effect.getParameter( ParameterIndex( index ) ) );
    effect.getParameterValueString( index, pValueString );
    return FMOD_OK;
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::getParameter( FMOD_DSP_STATE * const pDSP, int const index, void * * const data, unsigned int * const length, char * const pValueString )
{
    boost::ignore_unused_variable_warning( pDSP && index && pValueString );
    BOOST_ASSERT_MSG( index == Impl::maxNumberOfParameters, "Unexpected index for data parameter" );
    BOOST_ASSERT_MSG( !pValueString                       , "FMOD requested parameter value printing for the side chain parameter." );
    static FMOD_DSP_PARAMETER_SIDECHAIN const parameter = { true };
    *data   = &const_cast<FMOD_DSP_PARAMETER_SIDECHAIN &>( parameter );
    *length = sizeof( parameter );
    return FMOD_OK;
}


////////////////////////////////////////////////////////////////////////////////
//
// FMODPlugin::setParameter()
// --------------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class Impl>
template <typename T>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::setParameter( FMOD_DSP_STATE * const pDSP, int const index, T const value )
{
    return static_cast<FMOD_RESULT>( impl( pDSP ).setParameter( ParameterIndex( index ), /*...mrmlj...*/ Math::convert<float>( value ) ) );
}

template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::setParameter( FMOD_DSP_STATE * const pDSP, int const index, void * const data, unsigned int const length )
{
    boost::ignore_unused_variable_warning( pDSP && index && data && length );
    return FMOD_ERR_UNIMPLEMENTED;
}


template <class Impl>
LE_NOTHROW FMOD_RESULT F_CALLBACK Plugin<Impl, Protocol::FMOD>::canProcess( FMOD_DSP_STATE * /*pDSP*/, FMOD_BOOL /*inputidle*/, unsigned int /*length*/, FMOD_CHANNELMASK, int /*inChannels*/, FMOD_SPEAKERMODE )
{
    return FMOD_OK;
}

#if LE_SW_GUI
extern FMOD_DSP_DESCRIPTION const * LE_RESTRICT pUgh;
#endif
template <class Impl>
LE_NOTHROW FMOD_DSP_DESCRIPTION * Plugin<Impl, Protocol::FMOD>::getDescription()
{
    bool const useSideChain( true );

    std::uint16_t const numberOfParameters( Impl::maxNumberOfParameters + useSideChain /*side chain specification parameter*/ );
    typedef boost::array<ParameterInformation           , numberOfParameters> ParameterDescriptions       ;
    typedef boost::array<FMOD_DSP_PARAMETER_DESC const *, numberOfParameters> ParameterDescriptionPointers;

    static ParameterDescriptions        parameterDescriptions        ;
    static ParameterDescriptionPointers parameterDescriptionsPointers;

    {
        FMOD_DSP_PARAMETER_DESC const * * pDescriptionPointer = parameterDescriptionsPointers.begin();
        ParameterInformation    const *   pDescription        = parameterDescriptions        .begin();
        while ( pDescriptionPointer != parameterDescriptionsPointers.end() )
        {
            *pDescriptionPointer++ = &pDescription++->fmodStructure();
        }
    }

    static FMOD_DSP_DESCRIPTION /*const*/ description =
    {
        FMOD_PLUGIN_SDK_VERSION,
        ""/*Impl::name*/,
        ( Impl::versionMajor * 0x10000 ) | ( Impl::versionMinor * 0x1000 ) | ( Impl::versionPatch * 0x100 ), // version 0xAAAABBBB   A = major, B = minor.
        1,
        1,
        &Plugin::create,
        &Plugin::release,
        &Plugin::reset,
        &Plugin::read,
        nullptr,
        &Plugin::setTimePosition,
        numberOfParameters,
        const_cast<FMOD_DSP_PARAMETER_DESC * *>( parameterDescriptionsPointers.elems ),
        &Plugin::setParameter<float    >,
        &Plugin::setParameter<int      >,
        &Plugin::setParameter<FMOD_BOOL>,
    #ifdef NDEBUG
        nullptr,
    #else
        &Plugin::setParameter,
    #endif // NDEBUG
        &Plugin::getParameter<float    >,
        &Plugin::getParameter<int      >,
        &Plugin::getParameter<FMOD_BOOL>,
        &Plugin::getParameter,
        &Plugin::canProcess,
        nullptr
    };

    if ( !*description.name )
    {
        std::strncpy( description.name, Impl::name, _countof( description.name ) - 1 );

        ParameterIndex index( 0 );
        for ( auto const & parameterInfo : boost::make_iterator_range( parameterDescriptions.begin(), parameterDescriptions.end() - useSideChain ) )
        {
            Impl::getParameterProperties( index, LE_MSVC_SPECIFIC( const_cast<ParameterInformation &> ) ( parameterInfo ), nullptr );
        #ifdef _DEBUG
            FMOD_DSP_PARAMETER_DESC const & info( parameterInfo.fmodStructure() );
            std::printf
            (
                "Parameter %u, %s: type %d min %.2f, max %.2f, default %.2f, unit (%s).\n",
                index.value,
                info.name,
                info.type,
                ( info.type == FMOD_DSP_PARAMETER_TYPE_FLOAT ) ? info.floatdesc.min        : static_cast<float>( info.intdesc.min        ),
                ( info.type == FMOD_DSP_PARAMETER_TYPE_FLOAT ) ? info.floatdesc.max        : static_cast<float>( info.intdesc.max        ),
                ( info.type == FMOD_DSP_PARAMETER_TYPE_FLOAT ) ? info.floatdesc.defaultval : static_cast<float>( info.intdesc.defaultval ),
                info.label
            );
        #endif // _DEBUG
            index.value++;
        }
        if ( useSideChain )
        {
            auto & sideChain( parameterDescriptions.back().fmodStructure() );
            sideChain.type              = FMOD_DSP_PARAMETER_TYPE_DATA;
            sideChain.name [ 0 ]        = '\0';
            sideChain.label[ 0 ]        = '\0';
            sideChain.description       = nullptr;
            sideChain.datadesc.datatype = FMOD_DSP_PARAMETER_DATA_TYPE_SIDECHAIN;
        }
    #ifdef _DEBUG
        for ( auto const & parameterInfo : parameterDescriptions )
        {
        #ifdef _MSC_VER
            ParameterInformation & parameterInfo( const_cast<ParameterInformation &>( parameterInfo ) );
        #endif // _MSC_VER
            ParameterInformation const * const pDulpicateParameter
            (
                std::find_if
                (
                    &parameterInfo + 1,
                    boost::end( parameterDescriptions ),
                    [&]( ParameterInformation & info ) mutable { return std::strcmp( *info.nameBuffer(), *parameterInfo.nameBuffer() ) == 0; }
                )
            );
            BOOST_ASSERT_MSG
            (
                pDulpicateParameter == boost::end( parameterDescriptions ),
                "Duplicate parameter name generated."
            );
        }
    #endif // _DEBUG
    }
#if LE_SW_GUI
    pUgh = &description;
#endif
    return const_cast<FMOD_DSP_DESCRIPTION *>( &description );
}

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

template <class Impl>
struct FMODFactoryInjector
{
    ////////////////////////////////////////////////////////////////////////////
    //
    // FMODGetDSPDescription()
    // -----------------------
    //
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief The FMOD "factory method"/entry point function.
    ///
    ////////////////////////////////////////////////////////////////////////////

#ifdef HAS_FRIEND_INJECTION
    friend
#else
    static
#endif // HAS_FRIEND_INJECTION
    F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION * F_API FMODGetDSPDescription()
    {
        return LE::Plugins::Plugin<Impl, LE::Plugins::Protocol::FMOD>::getDescription();
    }
}; // struct FMODFactoryInjector

//------------------------------------------------------------------------------
