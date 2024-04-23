////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin.hpp
/// ----------------
///
/// Unity 5 backend for the LE Plugin framework.
///
/// Copyright (c) 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin_hpp__BC50B92D_D69A_4F55_B824_8BC8FC9FB27F
#define plugin_hpp__BC50B92D_D69A_4F55_B824_8BC8FC9FB27F
#pragma once
//------------------------------------------------------------------------------
#include "le/plugins/plugin.hpp"
#include "tag.hpp"

#include "le/math/conversion.hpp"
#include "le/parameters/powerOfTwo/tag.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/parameters/runtimeInformation.hpp"
#include "le/utility/countof.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/parentFromMember.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/typeTraits.hpp"

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4201 ) // Nonstandard extension used : nameless struct/union.
#endif // _MSC_VER
#ifdef __APPLE__
    #define SInt32_defined
    #define UInt32_defined
#endif // __APPLE__
#include "AudioPluginInterface.h"
#undef  SInt32_defined
#undef  UInt32_defined
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

#include <boost/mpl/string.hpp>
#include <boost/predef/os.h>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

/// \note Detect targets that can also run the Unity Editor (so that UI-only
/// code can be excluded from builds for other targets).
///                                           (14.10.2015.) (Domagoj Saric)
#if !( ( BOOST_OS_MACOS && __LP64__ ) || ( BOOST_OS_WINDOWS && defined( _WIN64 ) ) )
    #ifndef LE_NO_PARAMETER_STRINGS
        #define LE_NO_PARAMETER_STRINGS 1
    #endif // LE_NO_PARAMETER_STRINGS
#endif // editor OS detection

LE_OPTIMIZE_FOR_SIZE_BEGIN()

////////////////////////////////////////////////////////////////////////////////
///
/// \class UnityHostProxy
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class UnityHostProxy
{
public: // Host capabilities.
    static bool const threadSafeEventSystem          = false;
    static bool const pluginCanChangeIOConfiguration = false;

    template <HostCapability capability>
    bool canDo() const { return false; }

public: // Plugin information/state reported/visible to host.
    bool reportNewNumberOfIOChannels( std::uint8_t inputs, std::uint8_t sideInputs, std::uint8_t outputs );
    bool reportNewLatencyInSamples  ( std::uint32_t /*samples*/ ) { return false; }

    static bool automatedParameterChanged( ParameterIndex, float ) { return false; }

    static bool wantsManualDependentParameterNotifications() { return false; }

public:
    static bool getHostProductString( char * const buffer )
    {
        std::strcpy( buffer, "Unity" );
        return true;
    }
}; // class UnityHostProxy


template <class Protocol> class  ParameterInformation;
template <class Protocol> struct ErrorCode;
template <class Protocol> struct AutomatedParameterFor;

template <>
struct ErrorCode<Protocol::Unity>
{
    enum value_type
    {
        Success                  = UNITY_AUDIODSP_OK,
        Unsupported              = UNITY_AUDIODSP_ERR_UNSUPPORTED,
        OutOfMemory              = Unsupported,
        OutOfRange               = Unsupported,
        CannotDoInCurrentContext = Unsupported,
        Unauthorized             = Unsupported,
        FileNotFound             = Unsupported
    };
}; // struct ErrorCode<Protocol::Unity>

template <> struct AutomatedParameterFor<Protocol::Unity> { using type = FullRangeAutomatedParameter; };

////////////////////////////////////////////////////////////////////////////////
///
/// \class UnityPluginBase
///
////////////////////////////////////////////////////////////////////////////////

class UnityPluginBase
    :
    private UnityHostProxy,
    public  ErrorCode<Protocol::Unity>
{
public: // Traits and type definitions.
    using ConstructionParameter = std::nullptr_t;
    using AutomatedParameter    = FullRangeAutomatedParameter;
    using HostProxy             = UnityHostProxy;
    using ParameterSelector     = ParameterIndex;

    using SupportsTimingInformation = std::false_type;

    using Protocol             = Protocol::Unity;
    using ErrorCode            = ErrorCode           <Protocol>::value_type;
    using ParameterInformation = ParameterInformation<Protocol>;

    static DSPGUISeparation const dspGUISeparation = Mandatory;

public: // String buffer definitions.
    typedef char NameBuf           [ 16 ];
    typedef char LabelBuf          [ 16 ];
    typedef char ParameterStringBuf[ 16 ];

public:
    HostProxy       & host()       { return *this; }
    HostProxy const & host() const { return const_cast<UnityPluginBase &>( *this ).host(); }

protected:
     UnityPluginBase( ConstructionParameter ) {}
    ~UnityPluginBase() {}
}; // class UnityPluginBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class UnityPluginBase::ParameterInformation
///
////////////////////////////////////////////////////////////////////////////////

template <>
class ParameterInformation<Protocol::Unity> : private UnityAudioParameterDefinition
{
public:
    using result_type = void;

    //...mrmlj...version for dynamic (module) parameters...no power-of-two parameter support...
    void set( AutomatedParameter::Info const & info )
    {
        /// \note Unity 5.2.1 crashes if the description ptr is left null.
        ///                                   (16.10.2015.) (Domagoj Saric)
        description = "";
    #if !LE_NO_PARAMETER_STRINGS
      //std::strcpy( name, info.name );
        std::strcpy( unit, info.unit );

        displayscale    = 1;
        displayexponent = 1;
   #endif // !LE_NO_PARAMETER_STRINGS

        switch ( info.type )
        {
            case AutomatedParameter::Info::Boolean:
                min        = false;
                max        = true ;
                defaultval = info.default_;
                break;
            case AutomatedParameter::Info::Integer:
            case AutomatedParameter::Info::Enumerated:
            case AutomatedParameter::Info::FloatingPoint:
                min        = info.minimum ;
                max        = info.maximum ;
                defaultval = info.default_;
                break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }
    }

    template <class Parameter>
    result_type operator()() //...mrmlj...kill the duplication with set()...
    {
        description = "";
    #if !LE_NO_PARAMETER_STRINGS
        displayscale    = 1;
        displayexponent = 1;
        setLabel( boost::mpl::c_str<typename LE::Parameters::DisplayValueTransformer<Parameter>::Suffix>::value );
   #endif // !LE_NO_PARAMETER_STRINGS
        setValues<Parameter>( typename Parameter::Tag() );
    }

    UnityPluginBase::ParameterStringBuf * nameBuffer() { return &name; }
    void                                  nameSet   () {}

    static void markAsMeta() {}

    void clear()
    {
        BOOST_ASSERT( name[ 0 ]   == 0       );
        BOOST_ASSERT( unit[ 0 ]   == 0       );
        BOOST_ASSERT( description == nullptr );
    }

    UnityAudioParameterDefinition const & abiStructure() const { return *this; }
    UnityAudioParameterDefinition       & abiStructure()       { return *this; }

private:
    template <class Parameter, typename Tag>
    void setValues( Tag )
    {
        min        = Parameter::minimum ();
        max        = Parameter::maximum ();
        defaultval = Parameter::default_();
    }

    template <class Parameter>
    void setValues( LE::Parameters::PowerOfTwoParameterTag )
    {
        //setValues<Parameter>( Parameters::LinearIntegerParameterTag() );
        //...mrmlj...workaround for having no way to specify discrete/valid values...

        std::uint8_t const minimumExponent( boost::static_log2<Parameter::unscaledMinimum>::value );
        std::uint8_t const maximumExponent( boost::static_log2<Parameter::unscaledMaximum>::value );
        std::uint8_t const defaultExponent( boost::static_log2<Parameter::unscaledDefault>::value );

        min        = 0                                ;
        max        = maximumExponent - minimumExponent;
        defaultval = defaultExponent - minimumExponent;
    }

    template <class Parameter> void setValues( LE::Parameters::DynamicRangeParameterTag ) { setValues<Parameter>( LE::Parameters::LinearFloatParameterTag() ); }

#if !LE_NO_PARAMETER_STRINGS
    template <unsigned int N>
    void setLabel( char const (&unit)[ N ] )
    {
        std::memcpy( this->unit, unit, std::min<unsigned int>( N, sizeof( this->unit ) - 1 ) );
    }
#endif // !LE_NO_PARAMETER_STRINGS
}; // class UnityPluginBase::ParameterInformation


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin<Unity>
///
/// \brief Provides the Unity Ex protocol implementation.
///
////////////////////////////////////////////////////////////////////////////////

/// \note From the Unity Native Audio SDK:
/// "This entire structure must be a multiple of 16 bytes (and and instance 16
/// byte aligned) for PS3 SPU DMA requirements".
///                                           (09.09.2015.) (Domagoj Saric)

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4324 ) // structure was padded due to __declspec(align()).
#endif // _MSC_VER

template <class ImplParam>
class LE_ALIGN( 16 ) Plugin<ImplParam, Protocol::Unity>
    :
    public UnityPluginBase
{
protected:
    Plugin( ConstructionParameter );

#ifndef NDEBUG
    ~Plugin()
    {
        std::printf( "---- %s Unity instance %d destroyed ----\n", Impl::name, pluginInstanceID_ );
        --pluginInstanceCount_;
    }
#endif

protected:
    using Impl = ImplParam;

protected: // Default implementations for host -> plugin events/callbacks (user
           // Impl classes are free to override them and properly implement
           // them).

    static void getParameterLabel  ( ParameterIndex, ParameterStringBuf & label ) { label[ 0 ] = '\0'; }
    static void getParameterDisplay( ParameterIndex, ParameterStringBuf & text  ) { text [ 0 ] = '\0'; }
    static void getParameterName   ( ParameterIndex, ParameterStringBuf & name  ) { name [ 0 ] = '\0'; }

    static bool getParameterProperties( unsigned int /*index*/, ParameterInformation & ) { return false; }

protected:
    Impl       & impl()       { LE_MSVC_SPECIFIC( LE_ASSUME( this != 0 ) ); return static_cast<Impl &>( *this ); }
    Impl const & impl() const { return const_cast<Plugin &>( *this ).impl(); }

    static Impl & impl( UnityAudioEffectState * const pState )
    {
        LE_ASSUME( pState );
        void * const pImpl( pState->effectdata );
        LE_ASSUME( pImpl );
        return *static_cast<Impl * LE_RESTRICT>( pImpl );
    }

private: // Callbacks.
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK create      ( UnityAudioEffectState * pState );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK release     ( UnityAudioEffectState * pState );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK reset       ( UnityAudioEffectState * pState );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK process     ( UnityAudioEffectState * pState, float * inbuffer, float * outbuffer, unsigned int length, int inchannels, int outchannels );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK setPosition ( UnityAudioEffectState * pState, unsigned int position );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK setParameter( UnityAudioEffectState * pState, int index, float value );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK getParameter( UnityAudioEffectState * pState, int index, float * value, char * valuestr );
    LE_NOTHROW static UNITY_AUDIODSP_RESULT UNITY_AUDIODSP_CALLBACK getSignal   ( UnityAudioEffectState * pState, char const * name, float * buffer, int numsamples );

private:
    // http://objectmix.com/c/39865-how-fix-program-depends-friend-injection.html
    // http://www.open-std.org/jtc1/sc22/wg21/docs/papers/1995/N0777.pdf
    LE_COLD
    friend int UnityGetAudioEffectDefinitionsImpl( ::UnityAudioEffectDefinition const * const * * const ppDefinitions )
    {
        auto BOOST_CONSTEXPR_OR_CONST numberOfParameters = Impl::maxNumberOfParameters;
        using ParameterDefinitions = std::array<ParameterInformation, numberOfParameters>;
        static ParameterDefinitions parameterDefinitions;

        bool BOOST_CONSTEXPR_OR_CONST usesSideChain( Impl::maxNumberOfInputs == ( Impl::maxNumberOfOutputs * 2 ) );
        static UnityAudioEffectDefinition /*const*/ definition =
        {
            /*structsize       */ sizeof( definition            ),
            /*paramstructsize  */ sizeof( *definition.paramdefs ),
            /*apiversion       */ UNITY_AUDIO_PLUGIN_API_VERSION,
            /*pluginversion    */ Impl::version,
            /*channels         */ 0,
            /*numparameters    */ numberOfParameters,
            /*flags            */ usesSideChain ? UnityAudioEffectDefinitionFlags_IsSideChainTarget : 0,
            /*name[ 32 ]       */ { /*Impl::name*/ },
            /*create           */ &Plugin::create     ,
            /*release          */ &Plugin::release    ,
            /*reset            */ &Plugin::reset      ,
            /*process          */ &Plugin::process    ,
            /*setposition      */ &Plugin::setPosition,
            /*paramdefs        */ const_cast<UnityAudioParameterDefinition *>( &parameterDefinitions[ 0 ].abiStructure() ),
            /*setfloatparameter*/ &Plugin::setParameter,
            /*getfloatparameter*/ &Plugin::getParameter,
            /*getfloatbuffer   */ &Plugin::getSignal
        };
        static UnityAudioEffectDefinition const * const pDefinitions[] = { &definition };

        if ( !*definition.name )
        {
            std::strncpy( definition.name, Impl::name, _countof( definition.name ) - 1 );

            ParameterIndex index{ 0 };
            LE_DISABLE_LOOP_UNROLLING()
            for ( auto & parameterInfo : parameterDefinitions )
            {
                Impl::getParameterProperties( index, parameterInfo, nullptr );
                index.value++;
            }
        }

        *ppDefinitions = pDefinitions;
        return _countof( pDefinitions );
    }

#ifndef NDEBUG
private:
    static unsigned int pluginInstanceCount_;
    unsigned int const pluginInstanceID_;
#endif
}; // class Plugin<ImplParam, Protocol::Unity>

#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

/// \note Clang (3.6&Xcode7) barfs if the declaration is put before the Plugin
/// specialization.
///                                           (25.09.2015.) (Domagoj Saric)
extern int UnityGetAudioEffectDefinitionsImpl( ::UnityAudioEffectDefinition const * const * * ppDefinitions );

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "plugin.inl"

#endif // plugin_hpp
