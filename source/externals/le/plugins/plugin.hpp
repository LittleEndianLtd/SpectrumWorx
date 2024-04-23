////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin.hpp
/// ----------------
///
///   The idea of the LE Plugin architecture is to provide a single abstraction,
/// a single interface specification, that enables plugin code to be written
/// once and to compile for different plugin protocols (with as little 
/// target-protocol specific branching in the user code as possible ). It also
/// tries to automate as much of the boiler-plate setup code as possible as well
/// as insulate any bad, ugly and/or error-prone design of the original
/// interfaces/specifications.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin_hpp__22f9B1AE_714E_4EAf_A7EB_3F0282871940
#define plugin_hpp__22f9B1AE_714E_4EAf_A7EB_3F0282871940
#pragma once
//------------------------------------------------------------------------------
#include "le/parameters/conversion.hpp" //...mrmlj...
#include "le/utility/cstdint.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/mpl/set_c.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters { struct RuntimeInformation; }
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

///   The Capability enum is not really 'done by the book' in the sense
/// that it mentions functionality/capabilities from derived classes (for
/// example ReceiveVSTEvents). This was done on purpose/as a 'lesser evil'
/// because C++ enums do not support 'inheritance'/'extending' so using new
/// 'Capability' enums in more derived classes would, in practice,
/// complicate/'uglify' things even more (for example each plugin would have
/// to supply multiple 'Capabilities' specification/typedefs, multiple or
/// more complicated queryImplementationCapability() member template
/// functions...).
///
/// \todo Try to devise a smarter method to detect plugin/implementation
/// class capabilities from the (non)existence of appropriate member
/// functions (required by those capabilities) as was done for the
/// "process double" capability with the HasProcessDoubleReplacing<>
/// metafunction.
///                                           (24.07.2009.) (Domagoj Saric)
enum PluginCapability
{
    ReceiveMIDIEvent     , ///< plug-in can receive MIDI events from Host
    SendMIDIEvent        , ///< plug-in will send MIDI events to Host
    Bypass               , ///< plug-in supports function #setBypass ()
    MixDryWet            ,
    NonRealTimeProcessing,
    UsesAFixedSizeGUI    ,
    AsInsert             ,
    AsSend               ,
    Ch1in1out            ,
    Ch1in2out            ,
    Ch2in1out            ,
    Ch2in2out            ,
    Ch2in4out            ,
    Ch4in2out            ,
    Ch4in4out            ,
    Ch4in8out            , ///< 4:2 matrix to surround bus 
    Ch8in4out            , ///< surround bus to 4:2 matrix 
    Ch8in8out            ,

    // VST specific.
    OfflineProcessing    , ///< plug-in supports offline functions (#offlineNotify, #offlinePrepare, #offlineRun)
    ReceiveVSTEvents     , ///< plug-in can receive VST events from Host
    SendVSTEvents        , ///< plug-in will send VST events to Host
    ReceiveVSTTimeInfo   , ///< plug-in can receive Time info from Host
    MidiProgramNames       ///< plug-in supports function #getMidiProgramName ()
};

/// Helper macro for declaring plugin implementation capabilities.
#define DECLARE_PLUGIN_CAPABILITIES( ... ) typedef boost::mpl::set_c<unsigned int/*::LE::Plugins::PluginCapability*/, __VA_ARGS__> Capabilities


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin
///
/// \brief Base class for the Little Endian audio plugin framework.
///
///   The Plugin class template itself primarily only specifies the basic
/// requirements that both the plugin implementation classes (those that users
/// of the LE Plugin framework write, e.g. SpectrumWorx) and the protocol
/// implementation classes (that are part of the LE Plugin framework, e.g.
/// AudioUnit, FMOD, VST24...) must follow and fulfill. The protocol
/// implementation classes are in fact just specializations of the Plugin class
/// template.
///
///   In the current stage only the VST 2.4 and FMOD protocols are actually
/// supported so the Plugin class is still somewhat thin and, possibly,
/// "under-specified" and, as such, is subject to change. Once further research
/// is done on other plugin protocols (AU, DX/DMO/MF...) it will probably be
/// refactored and more shared functionality extracted from the VST classes into
/// it.
///
///   The framework's design is based on the CRTP:
/// http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
/// http://www.ddj.com/cpp/184401962
/// http://accu.org/index.php/journals/296
/// http://stackoverflow.com/questions/356294/is-partial-class-template-specialization-the-answer-to-this-design-problem.
/// In other words, if we, for example, knew that our plugin class, say APlugin,
/// will only be used to implement VST 2.4 plugins we could simply write:
///   class APlugin : public Plugin<APlugin, Protocol::VST24> {...};
/// However, in case we want to support/target multiple plugin protocols we
/// would have to declare our APlugin class like this:
///   template <template Protocol>
///   class APlugin : public Plugin<APlugin<Protocol>, Protocol> {...};
///
///   The LE Plugin framework expects that functions from the concrete user
/// plugin classes that get called by the framework do not throw any exceptions
/// (this includes constructors and destructors).
///
////////////////////////////////////////////////////////////////////////////////
///
/// Implementation note:
///   The requirements and specifications are outlined with actual class members
/// (as opposed to specifing them in the class comment/description) for better
/// readability and visualization as well as easier/automatic Doxygen
/// documentation generation. Members (be it data, functions or types) that only
/// serve for documentation/specification purposes are only declared (and not
/// defined). Documenting member data and functions are also placed in private
/// sections so that their accidental usage (for example when not providing your
/// own proper definition) would cause compile-time (private access) errors
/// instead of (waiting for) link-time (missing symbol definition) errors.
///                                           (19.08.2009.) (Domagoj Saric)
///
/// \todo Reconsider the above described specification method.
///                                           (19.08.2009.) (Domagoj Saric)
///
/// \todo Research and implement other potential protocols (VST 3.5, DX/DMO/MF,
/// AAX...):
/// - for DX/DMO/MF look into the following interfaces and/or keywords: 
///   - IMediaObject
///   - IMediaObjectInPlace
///   - ISpecifyPropertyPages
///   - IPropertyPage
///   - IPersistStream
///   - IMediaParamInfo
///   - IMediaParams
///   - CLSID/plugin identification
///                                           (19.08.2009.) (Domagoj Saric)
///
////////////////////////////////////////////////////////////////////////////////

class Plugin2HostActiveController
{
public:
    class HostProxy;

protected: // Required function interface (provided by the PluginProtocol).
    HostProxy & host();

    //notify()/update()
};


class Plugin2HostPassiveController
{
private: // Required traits.
    static unsigned int const maxNumberOfParameters   ;
    static unsigned int const maxNumberOfInputs       ;
    static unsigned int const maxNumberOfOutputs      ;
    static unsigned int const maxLatency              ;
    static unsigned int const maxLookAhead            ; ///< how much input data will the plugin require before it can produce output, @see IMediaObject::GetInputSizeInfo().
    static unsigned int const maxTailSize             ; ///< @see AudioEffectX::getGetTailSize().
    static unsigned int const minimumProcessBufferSize; ///< @see IMediaObject::GetInputSizeInfo()/GetOutputSizeInfo().
    static unsigned int const requiredBufferAlignment ; ///< @see IMediaObject::GetInputSizeInfo()/GetOutputSizeInfo().
    static unsigned int const version                 ; ///< Plugin specific version.
    static char         const name                  []; ///< Friendly name.
    static char         const productString         [];

public: // Capabilities.
    typedef boost::mpl::set_c<PluginCapability> Capabilities;

public:
    /// @see AudioEffectX::getParameter()/setParameter().
    typedef void AutomatedParameter;
    AutomatedParameter getAutomatedParameter( unsigned int /*index*/ );
}; // class Plugin2HostPassiveController

class Host2PluginController
{
public:
    /// @see AudioEffectX::getParameter()/setParameter().
    struct AutomatedParameter;
    void setAutomatedParameter( unsigned int /*index*/, AutomatedParameter /*value*/ );
}; // class Host2PluginController

template <class Impl, typename Protocol>
class Plugin;

template <class Impl, typename Protocol>
class Processor
{
public: // Required type definitions (possibly documented separately).
        // (provided by the protocol specialization)

    ///   A specialized Plugin<> class might require that a parameter be
    /// passed through to its constructor. It should specify its type with a
    /// public member type called ConstructionParameter so that user classes can
    /// be written in a generic way (their constructor takes a
    /// ConstructionParameter and simply passes it down to the constructor of
    /// the base class). If a specialized Plugin<> class does not require such a
    /// parameter it should not typedef it to void as that would cause
    /// compile errors, instead it should just define it as an empty struct.
    struct ConstructionParameter;

    class AutomatedParameter;
    class MIDIEvent; ///< Yet to be researched and properly documented.

public: // Required type definitions (possibly documented separately).
        // (provided by the plugin implementation)
    class Editor;

private: // Required function interface (provided by the Plugin).
    Editor & gui();

    /// @see AudioEffectX::resume()/suspend() and IMediaObject::Flush().
    //void resume ();
    //void suspend();

    //void process( float  * * inputs, float  * * outputs, unsigned int sampleFrames ); ///< VSTSDK AudioEffectX::processReplacing() equivalent.
    //void process( double * * inputs, double * * outputs, unsigned int sampleFrames ); ///< optional, VSTSDK AudioEffectX::processDoubleReplacing() equivalent.

public: // Optional function interface.
    /// @see AudioEffectX::startProcess()/stopProcess() and
    /// IMediaObject::AllocateStreamingResources()/FreeStreamingResources().
    bool preStreamingInitialization() { return false; }
    bool postStreamingCleanup      () { return false; }

protected:
     Processor() {}
    ~Processor() {}
}; // class Plugin

template <typename Protocol>
class EditorInterface
{
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin::AutomatedParameter
///
/// \brief Abstracts the specifics of automated parameters for different
/// protocols.
///
////////////////////////////////////////////////////////////////////////////////
#if 0
template <class Impl, typename Protocol>
class Plugin<Impl, Protocol>::AutomatedParameter
{
    ///   Provides the protocol-specific type used for transferring automated
    /// parameter values.
    struct value_type;

    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class Getter
    ///
    /// \brief Gets/converts an automated parameter value to its protocol
    /// specific type and value.
    ///
    ////////////////////////////////////////////////////////////////////////////

    class Getter
    {
        typedef typename AutomatedParameter::value_type result_type;

        template <class Parameter>
        result_type operator()( Parameter const & ) const;
    };


    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class Setter
    ///
    /// \brief Sets the protocol specific value to an automated parameter.
    ///
    ////////////////////////////////////////////////////////////////////////////

    class Setter
    {
        Setter( typename AutomatedParameter::value_type value );
        typedef void result_type;

        template <class Parameter> 
        void operator()( Parameter & );
    };
}; // class Plugin<Impl, Protocol>::AutomatedParameter


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin::Editor
///
/// \brief Base, interface specifying, class for the plugin editor/UI class.
///
////////////////////////////////////////////////////////////////////////////////

template <class Impl, typename Protocol>
class Plugin<Impl, Protocol>::Editor
{
private: // Required protocol-specific type definitions (provided by protocol
         // classes).
    struct Rect;
    struct WindowHandle;

private: // Required function interface (provided by user classes).
    bool open ( WindowHandle );
    void close();
}; // class Plugin<Impl, Protocol>::Editor
#endif // 0

/// @see PluginCapability note.
enum HostCapability
{
    SendMidiEvent                 , ///< Host supports send of MIDI events to plug-in
    ReceiveMidiEvent              , ///< Host can receive MIDI events from plug-in
    SendTimeInfo                  , ///< Host supports send of VstTimeInfo to plug-in
    ReportConnectionChanges       , ///< Host will indicates the plug-in when something change in plug-in´s routing/connections with #suspend/#resume/#setSpeakerArrangement
    AcceptIOChanges               , ///< Host supports #ioChanged ()
    SizeWindow                    , ///< used by VSTGUI
    Offline                       , ///< Host supports offline feature
    OpenFileSelector              , ///< Host supports function #openFileSelector ()
    CloseFileSelector             , ///< Host supports function #closeFileSelector ()
    StartStopProcess              , ///< Host supports functions #startProcess () and #stopProcess ()

    // VST specific.
    SendVstEvents                 , ///< Host supports send of Vst events to plug-in
    ReceiveVstEvents              , ///< Host can receive Vst events from plug-in
    ShellCategory                 , ///< 'shell' handling via uniqueID. If supported by the Host and the Plug-in has the category #kPlugCategShell
    SendVstMidiEventFlagIsRealtime  ///< Host supports flags for #VstMidiEvent
    // ...mrmlj... deprecated/pre 2.4 VST host can-dos ... temporarily left
    // here for documentation and investigation purposes...
    // "supplyIdle"
    // "supportShell"
    // "editFile"
    // "getChunkFile"
}; // enum HostCapability


////////////////////////////////////////////////////////////////////////////////
///
/// \class HostProxy
///
/// \brief Base, interface specifying, class the plugin editor/UI class.
///
////////////////////////////////////////////////////////////////////////////////
#if 0
template <class Impl, typename Protocol>
class Plugin<Impl, Protocol>::HostProxy
{
private: // Required protocol-specific type definitions (provided by protocol
         // specialized classes).
    struct PathSpec;

public: // Required function interface (can be overriden by protocol classes).
    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \brief Returns whether the host is capable of the requested Capability.
    /// \throws nothing
    ///
    ////////////////////////////////////////////////////////////////////////////

    template <HostCapability capability>
    bool canDo() const { return false; }

protected:
     HostProxy() {}
    ~HostProxy() {}
}; // class Plugin<Impl, Protocol>::HostProxy
#endif // 0

struct ParameterIndex
{
    using value_type = std::uint16_t;
    value_type value;
    operator value_type() const { return value; }
}; // struct ParameterIndex

struct ParameterID
{
    using value_type = std::uint32_t;
    value_type value;
}; // struct ParameterID

struct AutomatedParameter
{
    using value_type = float;
    using Info       = Parameters::RuntimeInformation;

    template <class Impl>
    struct Getter
    {
        using result_type = value_type;
        template <class Parameter>
        result_type operator()( Parameter const & parameter ) const { return Impl:: template convertParameterToAutomationValue<Parameter>( parameter ); }
    }; // struct Getter

    ////////////////////////////////////////////////////////////////////////////
    ///
    /// \class Setter
    ///
    /// \brief Sets the value of an automated parameter.
    ///
    ///   None of its member functions may throw.
    ///
    ////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
#endif // _MSC_VER
    template <class Impl>
    struct Setter
    {
    #ifdef _MSC_VER //...mrmlj..."error C2797: list initialization inside member initializer list is not implemented" in LFOParameterSetter
        Setter( value_type const value ) : automationValue( value ) {}
    #endif // _MSC_VER
        using result_type = void;

        template <class Parameter>
        void operator()( Parameter & parameter ) const { parameter.setValue( Impl:: template convertAutomationToParameterValue<Parameter>( automationValue ) ); }

        value_type const automationValue;
    }; // class Setter
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER
}; // struct AutomatedParameter


using AutomatedParameterValue = AutomatedParameter::value_type;

struct NormalisedAutomatedParameter : AutomatedParameter
{
    static bool const normalised = true;

    template <class Parameter> static AutomatedParameterValue        convertParameterToAutomationValue( Parameter const &       );
    template <class Parameter> static typename Parameter::value_type convertAutomationToParameterValue( AutomatedParameterValue );

    using Getter = AutomatedParameter::Getter<NormalisedAutomatedParameter>;
    using Setter = AutomatedParameter::Setter<NormalisedAutomatedParameter>;
}; // struct NormalisedAutomatedParameter


struct FullRangeAutomatedParameter : AutomatedParameter
{
public:
    static bool const normalised = false;

    template <class Parameter> static AutomatedParameterValue        convertParameterToAutomationValue( Parameter const &       );
    template <class Parameter> static typename Parameter::value_type convertAutomationToParameterValue( AutomatedParameterValue );

    using Getter = AutomatedParameter::Getter<FullRangeAutomatedParameter>;
    using Setter = AutomatedParameter::Setter<FullRangeAutomatedParameter>;

private:
    template <class Parameter, typename Tag> struct AutomationRange;
}; // struct FullRangeAutomatedParameter


enum DSPGUISeparation
{
    NotAvailable,
    Optional,
    Mandatory
}; // enum DSPGUISeparation


template <class Protocol> class  ParameterInformation;
template <class Protocol> struct ErrorCode;
template <class Protocol> struct AutomatedParameterFor;

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------

//...mrmlj...figure out a proper place for this Plugins<->Parameters bridge...
namespace Plugins
{
    template <class Parameter>
    AutomatedParameterValue NormalisedAutomatedParameter::convertParameterToAutomationValue( Parameter const & parameter )
    {
        return LE::Parameters::convertParameterValueToLinearValue<AutomatedParameterValue, 0, 1, 1>( parameter );
    }

    template <class Parameter>
    typename Parameter::value_type NormalisedAutomatedParameter::convertAutomationToParameterValue( AutomatedParameterValue const automationValue )
    {
        return LE::Parameters::convertLinearValueToParameterValue<AutomatedParameterValue, 0, 1, 1, Parameter>( automationValue );
    }

    template <class Parameter, typename Tag>
    struct FullRangeAutomatedParameter::AutomationRange
    {
        static int BOOST_CONSTEXPR_OR_CONST unscaledMinimum = Parameter::unscaledMinimum;
        static int BOOST_CONSTEXPR_OR_CONST unscaledMaximum = Parameter::unscaledMaximum;
    };
    template <class Parameter>
    struct FullRangeAutomatedParameter::AutomationRange<Parameter, Parameters::PowerOfTwoParameterTag>
    {
        //...mrmlj...workaround for having no way to specify discrete/valid (power-of-two) values...
        static std::uint8_t BOOST_CONSTEXPR_OR_CONST unscaledMinimum = boost::static_log2<Parameter::unscaledMinimum>::value;
        static std::uint8_t BOOST_CONSTEXPR_OR_CONST unscaledMaximum = boost::static_log2<Parameter::unscaledMaximum>::value;
    };

    template <class Parameter>
    AutomatedParameterValue FullRangeAutomatedParameter::convertParameterToAutomationValue( Parameter const & parameter )
    {
        using Range = AutomationRange<Parameter, typename Parameter::Tag>;
        return LE::Parameters::convertParameterValueToLinearValue<AutomatedParameterValue, Range::unscaledMinimum, Range::unscaledMaximum - Range::unscaledMinimum, Parameter::rangeValuesDenominator>( parameter );
    }

    template <class Parameter>
    typename Parameter::value_type FullRangeAutomatedParameter::convertAutomationToParameterValue( AutomatedParameterValue const automationValue )
    {
        /// \note We can't simply call
        /// Math::convert<typename Parameter::value_type>() here because of
        /// power-of-two parameters that require quantization/clamping to
        /// allowed values.
        ///                                   (01.02.2012.) (Domagoj Saric)
        //using namespace Math;
        //std::uint8_t const minimumExponent( boost::static_log2<Parameter::unscaledMinimum>::value );
        //return convert<float>( PowerOfTwo::log2( parameter.getValue() ) - minimumExponent );
        using Range = AutomationRange<Parameter, typename Parameter::Tag>;
        return LE::Parameters::convertLinearValueToParameterValue<AutomatedParameterValue, Range::unscaledMinimum, Range::unscaledMaximum - Range::unscaledMinimum, Parameter::rangeValuesDenominator, Parameter>( automationValue );
    }
} // namespace Plugins

} // namespace LE
//------------------------------------------------------------------------------
#endif // plugin_hpp
