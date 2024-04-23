////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleParameters.hpp
/// --------------------------
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleParameters_hpp__F444F629_3EBC_4840_BFA4_817E1218B10D
#define moduleParameters_hpp__F444F629_3EBC_4840_BFA4_817E1218B10D
#pragma once
//------------------------------------------------------------------------------
#include "moduleNode.hpp"

#include "le/parameters/lfoImpl.hpp"
#if !LE_NO_PARAMETER_STRINGS
#include "le/parameters/printer_fwd.hpp" //...mrmlj...GetParameterValueString
#endif // !LE_NO_PARAMETER_STRINGS
#include "le/parameters/runtimeInformation.hpp"
#include "le/spectrumworx/effects/baseParameters.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/range/iterator_range_core.hpp"

#include <array>
#include <cstdint>
#include <utility>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters { struct RuntimeInformation; }
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
using ParameterInfo = Parameters::RuntimeInformation;

#ifndef LE_NO_PRESETS
class ParametersLoader;
#ifndef LE_SW_SDK_BUILD
class ParametersSaver ;
#endif // !LE_SW_SDK_BUILD
#endif // !LE_NO_PRESETS
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

using parameter_value_t = Parameters::LFOImpl::value_type;

class LE_NOVTABLE ModuleParameters : public ModuleNode
{
public:
    using BaseParameters = Effects::BaseParameters::Parameters;

    static std::uint8_t const numberOfBaseParameters       = BaseParameters::static_size;// ModuleDSP::Parameters::static_size;
    static std::uint8_t const numberOfNonLFOBaseParameters = 1 /* Bypass */;
    static std::uint8_t const numberOfLFOBaseParameters    = numberOfBaseParameters - numberOfNonLFOBaseParameters;

public:
    LE_PURE_FUNCTION std::uint8_t numberOfParameters              () const { return numberOfEffectSpecificParameters() + numberOfBaseParameters   ; }
    LE_PURE_FUNCTION std::uint8_t numberOfEffectSpecificParameters() const { return metaData_.numberOfExtraParameters;                              }
    LE_PURE_FUNCTION std::uint8_t numberOfLFOControledParameters  () const { return numberOfEffectSpecificParameters() + numberOfLFOBaseParameters; }

    static std::uint8_t effectSpecificParameterIndex( std::uint8_t parameterIndex );

    ParameterInfo const &               parameterInfo( std::uint8_t parameterIndex ) const;
    ParameterInfo const & effectSpecificParameterInfo( std::uint8_t parameterIndex ) const;

    std::uint8_t effectTypeIndex() const { return metaData_.typeIndex_; }

public:
    bool bypass() const;

    BaseParameters       & baseParameters()       { return baseParameters_; }
    BaseParameters const & baseParameters() const { return baseParameters_; }

    // Implementation note:
    //   If chunks are not used/supported and/or the host generated UI is used,
    // the host application "manually" iterates over all parameters to
    // save/restore them so we must explicitly handle the case(s) of parameters
    // that do not actually exist (i.e. requested parameter index is greater
    // than the number of parameters provided by the effect).
    //                                        (26.06.2009.) (Domagoj Saric)
#if LE_SW_GUI //...mrmlj...for loadPresetParameters()
    #define LE_AUX_VIRTUAL_SET virtual
    #ifdef LE_SW_SEPARATED_DSP_GUI
        #define LE_AUX_VIRTUAL_GET virtual
    #else
        #define LE_AUX_VIRTUAL_GET
    #endif // LE_SW_SEPARATED_DSP_GUI
#else
    #define LE_AUX_VIRTUAL_GET
    #define LE_AUX_VIRTUAL_SET
#endif
                       LE_NOTHROW float LE_FASTCALL getBaseParameter  ( std::uint8_t   baseParameterIndex              ) const;
    LE_AUX_VIRTUAL_SET LE_NOTHROW float LE_FASTCALL setBaseParameter  ( std::uint8_t   baseParameterIndex, float value )      ;

    LE_AUX_VIRTUAL_GET LE_NOTHROW float LE_FASTCALL getEffectParameter( std::uint8_t effectParameterIndex              ) const;
    LE_AUX_VIRTUAL_SET LE_NOTHROW float LE_FASTCALL setEffectParameter( std::uint8_t effectParameterIndex, float value )      ;
#undef LE_AUX_VIRTUAL_GET
#undef LE_AUX_VIRTUAL_SET
public: // LFO section
    /// \note Until FMOD timing info functionality settles in, all LFO stuff
    /// will be crammed here.
    ///                                       (14.03.2014.) (Domagoj Saric)
    using LFO  = LE::Parameters::LFOImpl;
    using LFOs = boost::iterator_range<LFO * LE_RESTRICT>;

    void updateLFOs( LFO::Timer::TimingInformationChange );

    LFO       & lfo( std::uint8_t lfoableParameterIndex )      ;
    LFO const & lfo( std::uint8_t lfoableParameterIndex ) const;

    LFO       & baseLFO( std::uint8_t index )       { LE_ASSUME( index < numberOfLFOBaseParameters ); return lfos()[ index ]; }
    LFO const & baseLFO( std::uint8_t index ) const { return const_cast<ModuleParameters &>( *this ).baseLFO( index ); }

    LFO       & effectLFO( std::uint8_t index )       { return lfos()[ numberOfLFOBaseParameters + index ]; }
    LFO const & effectLFO( std::uint8_t index ) const { return const_cast<ModuleParameters &>( *this ).effectLFO( index ); }

    static LFO::value_type   LE_FASTCALL normalisedToParameterValue( parameter_value_t normalisedValue, ParameterInfo const & );
    static parameter_value_t LE_FASTCALL parameterToNormalisedValue( LFO::value_type   parameterValue , ParameterInfo const & );

protected:
    using LFOPlaceholder = std::aligned_storage<sizeof( LFO ), __alignof( LFO )>::type;
    LFOs lfos() const;

public: //...mrmlj...
    using ParameterInfos = std::array<ParameterInfo const, BaseParameters::static_size>;
    static ParameterInfos const & parameterInfos();

public: //...mrmlj...AutomatedModuleImpl & getParameterValueString...
#if !LE_NO_PARAMETER_STRINGS
    using ParameterPrinter = LE::Parameters::AutomatedParameterPrinter;
#endif // !LE_NO_PARAMETER_STRINGS
    struct EffectMetaData;
    EffectMetaData const & metaData() const { return metaData_; }
public:
#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
#endif // _MSC_VER
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wignored-attribute"
#endif // __clang__
    struct EffectMetaData
    {
    #if !LE_NO_PARAMETER_STRINGS
        typedef char const * (LE_GNU_SPECIFIC( /*mrmlj clang crash*/__fastcall ) LE_MSVC_SPECIFIC( LE_FASTCALL ) GetParameterValueString)( std::uint8_t index, ParameterPrinter const & ) /*noexcept*/;
    #endif // !LE_NO_PARAMETER_STRINGS

        std::uint8_t                      const numberOfExtraParameters;
        std::uint8_t                      const typeIndex_             ;
        ParameterInfo const * LE_RESTRICT const pParameterInfos        ;
    #if !LE_NO_PARAMETER_STRINGS
        GetParameterValueString &               getParameterValueString;
    #endif // !LE_NO_PARAMETER_STRINGS

        EffectMetaData( EffectMetaData const & ) = delete;
    }; // struct EffectMetaData
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

protected:
    LE_NOTHROW
    ModuleParameters
    (
      //std::uint8_t moduleSlotIndex,
        EffectMetaData const & metadata
    #ifndef LE_NO_LFOs
       ,LFOPlaceholder       * pLFOStorage
    #endif
    );

#ifndef LE_NO_PRESETS
public: // Presets
    void LE_FASTCALL loadPresetParameters( ParametersLoader const & )      ;
#ifndef LE_SW_SDK_BUILD
    void LE_FASTCALL savePresetParameters( ParametersSaver  const & ) const;
#endif // LE_SW_SDK_BUILD
#endif // !LE_NO_PRESETS

#ifndef LE_NO_LFOs
protected:
    void LE_FASTCALL updateBaseParametersFromLFOs  ( LFO::Timer const & );
    void LE_FASTCALL updateEffectParametersFromLFOs( LFO::Timer const & );

    //...mrmlj...make ModuleDSP::preProcess() call SW::Module::set*ParameterFromLFO()...
    //...mrmlj...and make ModuleChainImpl::preProcessAll() non template...
    parameter_value_t LE_FASTCALL setBaseParameterFromLFOAux  ( std::uint8_t parameterIndex, LFO::value_type );
    parameter_value_t LE_FASTCALL setEffectParameterFromLFOAux( std::uint8_t parameterIndex, LFO::value_type );

private:
#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
    #define LE_AUX_VIRTUAL virtual
#else
    #define LE_AUX_VIRTUAL
#endif
    LE_AUX_VIRTUAL LE_NOTHROW void LE_FASTCALL setBaseParameterFromLFO  ( std::uint8_t const parameterIndex, LFO::value_type const value ) { setBaseParameterFromLFOAux  ( parameterIndex, value ); }
    LE_AUX_VIRTUAL LE_NOTHROW void LE_FASTCALL setEffectParameterFromLFO( std::uint8_t const parameterIndex, LFO::value_type const value ) { setEffectParameterFromLFOAux( parameterIndex, value ); }
#undef LE_AUX_VIRTUAL

    LE_NOTHROWNOALIAS LFO * LE_FASTCALL constructLFOs( LFOPlaceholder * ) const;
#endif // LE_NO_LFOs

private:
    /// \note We need to have local storage for caching shared parameters in
    /// order to support the current preset loading implementation (where a
    /// module is created and its parameters set before being inserted into the
    /// target chain - which in the separated DSP&GUI situation means that the
    /// parent/editor is not set for the ModuleUI instance in the moment
    /// setBaseParameter() is called during preset loading and so it cannot
    /// propagate the value being set upstream).
    ///                                       (20.10.2014.) (Domagoj Saric)
    //...mrmlj...svn r10423

  //std::uint8_t                             moduleSlotIndex_;
    EffectMetaData const &                   metaData_;
    BaseParameters                           baseParameters_;
#ifndef LE_NO_LFOs
    LFO                  * LE_RESTRICT const pLFOs_;
#endif
}; // class ModuleParameters

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // automatedModule_hpp
