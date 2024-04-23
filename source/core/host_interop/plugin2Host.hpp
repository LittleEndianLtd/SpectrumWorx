////////////////////////////////////////////////////////////////////////////////
///
/// \file plugin2Host.hpp
/// ---------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef plugin2Host_hpp__8B5D322C_926B_4B2E_8504_8F1124B6598F
#define plugin2Host_hpp__8B5D322C_926B_4B2E_8504_8F1124B6598F
#pragma once
//------------------------------------------------------------------------------
#include "parameters.hpp"

#include "core/parameterID.hpp"

#include "le/parameters/printer.hpp"
#include "le/parameters/parametersUtilities.hpp" // IndexOf, Clang
#include "le/plugins/plugin.hpp"
#if ( defined( _WIN32 ) || defined( __APPLE__ ) ) && !defined( LE_SW_FMOD ) //...mrmlj...
#include "le/plugins/vst/2.4/plugin.hpp"
#endif // VST2.4

#include "le/utility/cstdint.hpp"

#include "boost/smart_ptr/intrusive_ptr.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
    //...mrmlj...required to be in the header only for getParameterProperties() and EditorKnob::paint()...

    // Implementation note:
    //   PowerOfTwo parameters do not currently support/use the
    // DisplayValueTransformer functionality so we use a direct print()
    // specialization to print the overlap amount in percentages.
    //                                        (03.05.2011.) (Domagoj Saric)
    namespace Detail
    {
        template <class Parameter>
        char const * print
        (
            unsigned int              parameterValue,
            SW::Engine::Setup const &               ,
            PrintBuffer       const &               ,
            PowerOfTwoParameterTag
        );
        template <>
        char const * print<SW::Engine::OverlapFactor>
        (
            unsigned int              parameterValue,
            SW::Engine::Setup const &               ,
            PrintBuffer       const &               ,
            PowerOfTwoParameterTag
        );
        template <class Parameter>
        char const * print
        (
            float             const & parameterValue,
            SW::Engine::Setup const &               ,
            PrintBuffer       const &               ,
            PowerOfTwoParameterTag
        );
        template <>
        char const * print<SW::Engine::OverlapFactor>
        (
            float             const & parameterValue,
            SW::Engine::Setup const &               ,
            PrintBuffer       const &               ,
            PowerOfTwoParameterTag
        );
    } // namespace Detail
} // namespace Parameters
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
    
namespace Engine { class ModuleParameters; }
class AutomatedModuleChain;
class Program;

namespace GUI { LE_NOTHROWNOALIAS bool isThisTheGUIThread(); }

////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin2HostInteropControler
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE Plugin2HostInteropControler
{
public:
    using Parameters = GlobalParameters::Parameters;
    using Module     = Engine::ModuleParameters;

    template <class Parameter>
    void LE_FASTCALL globalParameterChanged( Parameter const parameter, bool const asDiscreteGesture )
    {
        //...mrmlj...In and Out gain parameters only appear to be linear 'by chance' (because of the chosen minimum and maximum values)...
        globalParameterChanged
        (
            LE::Parameters::IndexOf<GlobalParameters::Parameters, Parameter>::value,
            Plugins::FullRangeAutomatedParameter ::convertParameterToAutomationValue( parameter ),
            Plugins::NormalisedAutomatedParameter::convertParameterToAutomationValue( parameter ),
            asDiscreteGesture
        );
    }

    void LE_FASTCALL automatedParameterChanged( ParameterID::LFO, float value ) const;
    void LE_FASTCALL automatedParameterChanged( Module               const &, std::uint8_t moduleIndex, std::uint8_t moduleParameterIndex, float parameterValue ) const;
    void LE_FASTCALL modulesChanged           ( AutomatedModuleChain const &, std::uint8_t firstModuleIndex, std::uint8_t lastModuleIndex                       ) const;
    void LE_FASTCALL moduleChangedByUser      ( std::uint8_t chainParameterIndex, Module const * ) const;

    static bool canParameterBeAutomated( Plugins::ParameterIndex, void const * /*pContext*/ ) { return true; }
    static bool canParameterBeAutomated( Plugins::ParameterID   , void const * /*pContext*/ ) { return true; }

    struct ParameterValueForAutomation
    {
        using value_type = Plugins::AutomatedParameter::value_type;

        value_type fullRange ;
        value_type normalised;
    }; // ParameterValueForAutomation

    template <typename TargetParameterSelector> static TargetParameterSelector make( ParameterID );


public: // Protocol specific functionality to be implemented by derived classes.
    // notifications
    virtual LE_NOTHROW void LE_FASTCALL automatedParameterBeginEdit( ParameterID                              ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL automatedParameterEndEdit  ( ParameterID                              ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL gestureBegin               ( char const * description                 ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL gestureEnd                 (                                          ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL automatedParameterChanged  ( ParameterID, ParameterValueForAutomation ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL moduleChanged              ( std::uint8_t moduleIndex, Module const * ) const = 0;
    virtual LE_NOTHROW bool LE_FASTCALL parameterListChanged       (                                          ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL presetChangeBegin          (                                          ) const = 0;
    virtual LE_NOTHROW void LE_FASTCALL presetChangeEnd            (                                          ) const = 0;
    virtual LE_NOTHROW bool LE_FASTCALL latencyChanged             (                                          )       = 0;

    // queries
#if LE_SW_ENGINE_INPUT_MODE >= 2
public:
    virtual LE_NOTHROW        bool LE_FASTCALL hostTryIOConfigurationChange      ( std::uint8_t numberOfMainChannels, std::uint8_t numberOfSideChannels )       = 0;
    virtual LE_NOTHROWNOALIAS bool LE_FASTCALL hostSupportsIOConfigurationChanges(                                                                      ) const = 0;
#endif // LE_SW_ENGINE_INPUT_MODE >= 2

private:
    void LE_FASTCALL globalParameterChanged
    (
        std::uint8_t                            index,
        ParameterValueForAutomation::value_type fullRange,
        ParameterValueForAutomation::value_type normalised,
        bool asDiscreteGesture //....mrmlj...ugh cleanup....for distinction between knobs and comboboxes
    );
}; // class Plugin2HostInteropControler


////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin2HostPassiveInteropController
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE Plugin2HostPassiveInteropController
{
#if ( defined( _WIN32 ) || defined( __APPLE__ ) ) && !defined( LE_SW_FMOD ) //...mrmlj...
public: // VST 2.4 protocol required traits.
    static std::uint8_t      const maxNumberOfPrograms = Constants::numberOfPrograms;
    static ::VstPlugCategory const category            = kPlugCategEffect;
    static ::VstInt32        const vstUniqueID         = 'LESW';
#endif // VST2.4

public:
    ////////////////////////////////////////////////////////////////////////////
    // Parameters
    ////////////////////////////////////////////////////////////////////////////

    public: // Parameter info
      //static LE_NOALIAS void LE_FASTCALL getParameterDisplay( ParameterID, boost::iterator_range<char                 *> text , Engine::Setup const &, Plugins::AutomatedParameterValue const * pOptionalValue, Program const & );
        static LE_NOALIAS void LE_FASTCALL getParameterLabel  ( ParameterID, boost::iterator_range<char                 *> label,                                                                                 Program const * );
        static LE_NOALIAS void LE_FASTCALL getParameterName   ( ParameterID, boost::iterator_range<char                 *> name ,                                                                                 Program const * );
        static LE_NOALIAS void LE_FASTCALL getParameterIDs    (              boost::iterator_range<Plugins::ParameterID *> ids  ,                                                                                 Program const * );

        static LE_NOALIAS std::uint16_t LE_FASTCALL numberOfParameters( Program const * );

    public: // Indexed parameter functors.
        /// \note
        ///   Up to revision 5763 a different approach for "printing" parameter
        /// values was used. Until then the parameter value to string conversion
        /// functionality was only needed for GUI code so each (parameter)
        /// widget stored a simple callback that directly converted a parameter
        /// value into its string representation. When extended support for
        /// parameter metadata/"generic UIs" (which includes exporting
        /// parameter values as strings through the plugin APIs) was added this
        /// was replaced with using the new "plugin parameter metadata"
        /// functionality.
        ///                                   (27.01.2012.) (Domagoj Saric)
        struct ParameterLabelGetter      ;
        struct ParameterValueStringGetter;
        struct ParameterNameGetter       ;
}; // class Plugin2HostPassiveInteropController

////////////////////////////////////////////////////////////////////////////////
///
/// \class Plugin2HostInteropControler::ParameterLabelGetter
///
////////////////////////////////////////////////////////////////////////////////

struct Plugin2HostPassiveInteropController::ParameterLabelGetter
{
    using result_type = char const * LE_RESTRICT const;

    result_type operator()( ParameterID::Global     , Program const * ) const;
    result_type operator()( ParameterID::ModuleChain, Program const * ) const { return nullptr; }
    result_type operator()( ParameterID::Module     , Program const * ) const;
    result_type operator()( ParameterID::LFO        , Program const * ) const;
}; // struct Plugin2HostPassiveInteropController::ParameterLabelGetter


////////////////////////////////////////////////////////////////////////////////
///
/// \struct Plugin2HostPassiveInteropController::ParameterValueStringGetter
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
struct Plugin2HostPassiveInteropController::ParameterValueStringGetter
{
    using result_type = char const *;

    result_type operator()( ParameterID::Global     , Program const * ) const;
    result_type operator()( ParameterID::ModuleChain, Program const * ) const;
  //result_type operator()( ParameterID::Module     , Program const * ) const;
    result_type operator()( ParameterID::LFO        , Program const * ) const;

    mutable Parameters::AutomatedParameterPrinter printer;
}; // struct ParameterValueStringGetter
#pragma warning( pop )

//...mrmlj...MSVC12u5: bad codegen if we move these functions into the .cpp file...
template <> LE_FORCEINLINE Plugins::ParameterID    Plugin2HostInteropControler::make<Plugins::ParameterID   >( SW::ParameterID const selector ) { return                           { selector.binaryValue }; }
template <> LE_FORCEINLINE Plugins::ParameterIndex Plugin2HostInteropControler::make<Plugins::ParameterIndex>( SW::ParameterID const selector ) { return parameterIndexFromBinaryID( selector.binaryValue ); }

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // plugin2Host_hpp
