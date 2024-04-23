////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin.hpp
/// ----------------
///
/// FMOD implementation for the LE Plugin framework.
///
/// Copyright (c) 2012 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin_hpp__AE78EB83_EA20_45D7_8243_4FFD23F2E6B6
#define plugin_hpp__AE78EB83_EA20_45D7_8243_4FFD23F2E6B6
#pragma once
//------------------------------------------------------------------------------
#include "le/plugins/plugin.hpp"
//#include "tag.hpp" //...mrmlj...fmod name collision...

#include "le/math/conversion.hpp"
#include "le/parameters/boolean/tag.hpp"
#include "le/parameters/dynamic/tag.hpp"
#include "le/parameters/enumerated/tag.hpp"
#include "le/parameters/linear/tag.hpp"
#include "le/parameters/powerOfTwo/tag.hpp"
#include "le/parameters/uiElements.hpp"
#include "le/parameters/runtimeInformation.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/parentFromMember.hpp"
#include "le/utility/platformSpecifics.hpp"
#include "le/utility/typeTraits.hpp"

#ifdef _MSC_VER
    #pragma warning( disable : 4100 ) // Unreferenced formal parameter (fmod_dsp.h).
    #pragma warning( disable : 4505 ) // Unreferenced local function has been removed (fmod_errors.h).
#endif // _MSC_VER
#include "fmod.hpp"
#include "fmod_dsp.h"
#include "fmod_errors.h"

#include "tag.hpp"

#include "boost/optional/optional.hpp" // Boost sandbox

#include "boost/mpl/string.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class FMODHostProxy
///
/// None of its member functions may throw.
///
////////////////////////////////////////////////////////////////////////////////

class FMODHostProxy
{
public: // Host capabilities.
    static bool const threadSafeEventSystem          = false;
    static bool const pluginCanChangeIOConfiguration = false;

    template <HostCapability capability>
    bool canDo() const { return false; }

public: // Plugin information/state reported/visible to host.
    bool reportNewNumberOfIOChannels( std::uint_fast8_t inputs, std::uint_fast8_t sideInputs, std::uint_fast8_t outputs );
    bool reportNewLatencyInSamples  ( unsigned int /*samples*/ ) { return false; }

    static bool automatedParameterChanged( ParameterIndex, float ) { return false; }

    static bool wantsManualDependentParameterNotifications() { return false; }

public:
    static bool getHostProductString( char * const buffer )
    {
        std::strcpy( buffer, "FMOD" );
        return true;
    }
}; // class FMODHostProxy


template <class Protocol> class  ParameterInformation;
template <class Protocol> struct ErrorCode;
template <class Protocol> struct AutomatedParameterFor;

template <>
struct ErrorCode<Protocol::FMOD>
{
    enum value_type
    {
        Success                  = FMOD_OK,
        OutOfMemory              = FMOD_ERR_MEMORY,
        //...mrmlj...quick-fix: FMOD Studio wants to needlessly force everything to default on initialization...
        OutOfRange               = FMOD_OK, //FMOD_ERR_INVALID_PARAM,
        CannotDoInCurrentContext = FMOD_ERR_NOTREADY,
        Unauthorized             = FMOD_ERR_NOTREADY,
        Unsupported              = FMOD_ERR_UNSUPPORTED,
        FileNotFound             = FMOD_ERR_FILE_NOTFOUND
    };
}; // struct ErrorCode<Protocol::FMOD>

template <> struct AutomatedParameterFor<Protocol::FMOD> { typedef FullRangeAutomatedParameter type; };

////////////////////////////////////////////////////////////////////////////////
///
/// \class FMODPluginBase
///
////////////////////////////////////////////////////////////////////////////////

class FMODPluginBase
    :
    private FMODHostProxy,
    public  ErrorCode<Protocol::FMOD>
{
public: // Traits and type definitions.
    typedef std::nullptr_t              ConstructionParameter;
    typedef FullRangeAutomatedParameter AutomatedParameter   ;
    typedef FMODHostProxy               HostProxy            ;
    typedef ParameterIndex              ParameterSelector    ;

    typedef std::false_type SupportsTimingInformation;

    typedef Protocol::FMOD                             Protocol;
    typedef ErrorCode           <Protocol>::value_type ErrorCode;
    typedef ParameterInformation<Protocol>             ParameterInformation;

    static DSPGUISeparation const dspGUISeparation = Mandatory;

public: // FMOD string buffer definitions.
    typedef char NameBuf           [ 16 ];
    typedef char LabelBuf          [ 16 ];
    typedef char ParameterStringBuf[ 16 ];

public:
    FMODHostProxy       & host()       { return *this; }
    FMODHostProxy const & host() const { return const_cast<FMODPluginBase &>( *this ).host(); }

protected:
     FMODPluginBase( ConstructionParameter ) {}
    ~FMODPluginBase() {}
}; // class FMODPluginBase


////////////////////////////////////////////////////////////////////////////////
///
/// \class FMODPluginBase::ParameterInformation
///
////////////////////////////////////////////////////////////////////////////////

template <>
class ParameterInformation<Protocol::FMOD> : private FMOD_DSP_PARAMETER_DESC
{
public:
    typedef void result_type;

    //...mrmlj...version for dynamic (module) parameters...no power-of-two parameter support...
    void set( AutomatedParameter::Info const & info )
    {
        /// \todo Respect FMOD Studio guidelines for plugin parameter units:
        ///  - "Hz" for frequency or cut-off
        ///  - "ms" for duration, time offset or delay
        ///  - "st" (semitones) for pitch
        ///  - "dB" for gain, threshold or feedback
        ///  - "%" for mix, depth, feedback, quality, probability, multiplier or generic 'amount'
        ///  - "Deg" for angle or angular spread.
        ///                                   (11.07.2013.) (Domagoj Saric)
        switch ( info.type )
        {
            case AutomatedParameter::Info::Boolean:
                type                = FMOD_DSP_PARAMETER_TYPE_BOOL;
                booldesc.defaultval = info.default_ != 0;
                BOOST_ASSERT( booldesc.valuenames == nullptr );
                break;
            case AutomatedParameter::Info::Integer:
            case AutomatedParameter::Info::Enumerated:
                type               = FMOD_DSP_PARAMETER_TYPE_INT;
                intdesc.min        = Math::convert<int>( info.minimum  );
                intdesc.max        = Math::convert<int>( info.maximum  );
                intdesc.defaultval = Math::convert<int>( info.default_ );
                intdesc.goestoinf  = false;
                intdesc.valuenames = const_cast<char **>( info.enumeratedValueStrings );
                break;
            case AutomatedParameter::Info::FloatingPoint:
                type                   = FMOD_DSP_PARAMETER_TYPE_FLOAT;
                floatdesc.min          = info.minimum ;
                floatdesc.max          = info.maximum ;
                floatdesc.defaultval   = info.default_;
                floatdesc.mapping.type = FMOD_DSP_PARAMETER_FLOAT_MAPPING_TYPE_AUTO;
                break;
            LE_DEFAULT_CASE_UNREACHABLE();
        }
        std::strcpy( label, info.unit );
    }

    template <class Parameter>
    result_type operator()() //...mrmlj...kill the duplication with set()...
    {
        setValues<Parameter>( typename Parameter::Tag() );
        setLabel( boost::mpl::c_str<typename LE::Parameters::DisplayValueTransformer<Parameter>::Suffix>::value );
    }

    FMODPluginBase::ParameterStringBuf * nameBuffer() { return &name; }
    void                                 nameSet   () {}

    static void markAsMeta() {}

    void clear()
    {
        BOOST_ASSERT( type        == 0 );
        BOOST_ASSERT( name [ 0 ]  == 0 );
        BOOST_ASSERT( label[ 0 ]  == 0 );
        BOOST_ASSERT( description == 0 );
    }

    FMOD_DSP_PARAMETER_DESC const & fmodStructure() const { return *this; }
    FMOD_DSP_PARAMETER_DESC       & fmodStructure()       { return *this; }

private:
    template <class Parameter>
    void setValues( Parameters::BooleanParameterTag )
    {
        type                = FMOD_DSP_PARAMETER_TYPE_BOOL;
        booldesc.defaultval = Parameter::default_();
        BOOST_ASSERT( booldesc.valuenames == nullptr );
    }

    template <class Parameter>
    void setValues( Parameters::LinearFloatParameterTag )
    {
        type                   = FMOD_DSP_PARAMETER_TYPE_FLOAT;
        floatdesc.min          = Parameter::minimum ();
        floatdesc.max          = Parameter::maximum ();
        floatdesc.defaultval   = Parameter::default_();
        floatdesc.mapping.type = FMOD_DSP_PARAMETER_FLOAT_MAPPING_TYPE_AUTO;
    }

    template <class Parameter>
    void setValues( Parameters::LinearIntegerParameterTag )
    {
        type               = FMOD_DSP_PARAMETER_TYPE_INT;
        intdesc.min        = Parameter::minimum ();
        intdesc.max        = Parameter::maximum ();
        intdesc.defaultval = Parameter::default_();
        BOOST_ASSERT( intdesc.valuenames == nullptr );
    }

    template <class Parameter>
    void setValues( Parameters::EnumeratedParameterTag )
    {
        setValues<Parameter>( LE::Parameters::LinearIntegerParameterTag() );
        intdesc.valuenames  = const_cast<char **>( LE::Parameters::DiscreteValues<Parameter>::strings );
    }

    template <class Parameter>
    void setValues( LE::Parameters::PowerOfTwoParameterTag )
    {
        //setValues<Parameter>( Parameters::LinearIntegerParameterTag() );
        //...mrmlj...workaround for having no way to specify discrete/valid values...

        std::uint8_t const minimumExponent( boost::static_log2<Parameter::unscaledMinimum>::value );
        std::uint8_t const maximumExponent( boost::static_log2<Parameter::unscaledMaximum>::value );
        std::uint8_t const defaultExponent( boost::static_log2<Parameter::unscaledDefault>::value );

        type               = FMOD_DSP_PARAMETER_TYPE_INT;
        intdesc.min        = 0                                ;
        intdesc.max        = maximumExponent - minimumExponent;
        intdesc.defaultval = defaultExponent - minimumExponent;
        BOOST_ASSERT( intdesc.valuenames == nullptr );
    }

    template <class Parameter> void setValues( LE::Parameters::DynamicRangeParameterTag ) { setValues<Parameter>( LE::Parameters::LinearFloatParameterTag() ); }

    template <unsigned int N>
    void setLabel( char const (&unit)[ N ] )
    {
        std::memcpy( label, unit, std::min<unsigned int>( N, sizeof( label ) - 1 ) );
    }
}; // class FMODPluginBase::ParameterInformation


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin<FMOD>
///
/// \brief Provides the FMOD Ex protocol implementation.
///
////////////////////////////////////////////////////////////////////////////////

template <class ImplParam>
class Plugin<ImplParam, Protocol::FMOD>
    :
    public FMODPluginBase
{
protected:
    Plugin( ConstructionParameter );

#ifndef NDEBUG
    ~Plugin()
    {
        std::printf( "---- %s FMOD instance %d destroyed ----\n", Impl::name, pluginInstanceID_ );
        --pluginInstanceCount_;
    }
#endif

protected:
    typedef ImplParam Impl;

protected: // Functions (FMOD host -> plugin events/callbacks) that must be
           // implemented by the user Impl class in order to support the VST 2.4
           // platform (in other words no default implementations are provided).

    // void setSampleRate( float sampleRate );

protected: // Default implementations for host -> plugin events/callbacks (user
           // Impl classes are free to override them and properly implement
           // them).

    void getParameterLabel  ( ParameterIndex, ParameterStringBuf & label ) const { label[ 0 ] = '\0'; }
    void getParameterDisplay( ParameterIndex, ParameterStringBuf & text  ) const { text [ 0 ] = '\0'; }
    void getParameterName   ( ParameterIndex, ParameterStringBuf & name  ) const { name [ 0 ] = '\0'; }

    bool getParameterProperties( unsigned int /*index*/, ParameterInformation & ) const { return false; }

private:
    void getParameterValueString( unsigned int index, char * pBuffer ) const;

protected:
    Impl & impl()
    {
        LE_ASSUME( this != 0 );
        return static_cast<Impl &>( *this );
    }

    Impl const & impl() const { return const_cast<Plugin &>( *this ).impl(); }

    static Impl & impl( FMOD_DSP_STATE * const pDSP )
    {
        LE_ASSUME( pDSP );
        void * const pState( pDSP->plugindata );
        LE_ASSUME( pState );
        return *static_cast<Impl * LE_RESTRICT>( pState );
    }

private: // Callbacks.
    LE_NOTHROW static FMOD_RESULT F_CALLBACK create         ( FMOD_DSP_STATE * );
    LE_NOTHROW static FMOD_RESULT F_CALLBACK release        ( FMOD_DSP_STATE * );
    LE_NOTHROW static FMOD_RESULT F_CALLBACK reset          ( FMOD_DSP_STATE * );
    LE_NOTHROW static FMOD_RESULT F_CALLBACK read           ( FMOD_DSP_STATE *, float * inBuffer, float * outBuffer, unsigned int length, int inChannels, int * pOutChannels );
    LE_NOTHROW static FMOD_RESULT F_CALLBACK setTimePosition( FMOD_DSP_STATE *, unsigned int pos );
    LE_NOTHROW static FMOD_RESULT F_CALLBACK canProcess     ( FMOD_DSP_STATE *, FMOD_BOOL inputidle, unsigned int length, FMOD_CHANNELMASK, int inChannels, FMOD_SPEAKERMODE );

    template <typename T>
    LE_NOTHROW static FMOD_RESULT F_CALLBACK getParameter   ( FMOD_DSP_STATE *, int index, T * value, char * valuestr );
    template <typename T>
    LE_NOTHROW static FMOD_RESULT F_CALLBACK setParameter   ( FMOD_DSP_STATE *, int index, T value );

    LE_NOTHROW static FMOD_RESULT F_CALLBACK getParameter   ( FMOD_DSP_STATE *, int index, void * * data, unsigned int * length, char * valuestr );
    LE_NOTHROW static FMOD_RESULT F_CALLBACK setParameter   ( FMOD_DSP_STATE *, int index, void *   data, unsigned int   length                  );

public:
    LE_NOTHROW static FMOD_DSP_DESCRIPTION * getDescription();

#ifndef NDEBUG
private:
    static unsigned int pluginInstanceCount_;
    unsigned int const pluginInstanceID_;
#endif
}; // class Plugin<ImplParam, Protocol::FMOD>


namespace Detail { void setMemoryCallbacks( FMOD_DSP_STATE_SYSTEMCALLBACKS const & ); }

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "plugin.inl"

#endif // plugin_hpp
