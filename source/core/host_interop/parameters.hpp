////////////////////////////////////////////////////////////////////////////////
///
/// \file parameters.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef parameters_hpp__4BD9593B_D1BA_479C_B068_64F56F997500
#define parameters_hpp__4BD9593B_D1BA_479C_B068_64F56F997500
#pragma once
//------------------------------------------------------------------------------
#include "configuration/constants.hpp"

#include "le/spectrumworx/engine/parameters.hpp"
#include "le/utility/cstdint.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
//------------------------------------------------------------------------------
{
namespace ParameterCounts
{
#if LE_SW_SEPARATED_DSP_GUI || !LE_SW_GUI
    static std::uint8_t const lfoExportedParameters = 7; //...mrmlj...LFO::Parameters::static_size;
#else
    // Implementation note:
    //   The SyncTypes and Waveform parameters are not yet exported for
    // automation. Consider solving this cleaner with separate Parameters<> and
    // ExtendedParameters<> instantiations.
    //                                        (18.02.2011.) (Domagoj Saric)
    static std::uint8_t const lfoExportedParameters = 5;
#endif // LE_SW_SEPARATED_DSP_GUI

    static std::uint8_t  const lfoParametersPerModule = lfoExportedParameters * ( Constants::maxNumberOfParametersPerModule - 1 );
    static std::uint16_t const lfoParameters          = lfoParametersPerModule * Constants::maxNumberOfModules;

    static std::uint16_t const maxNumberOfParameters = GlobalParameters::Parameters::static_size
                                                             +
                                                         Constants::maxNumberOfModules
                                                             +
                                                         Constants::maxNumberOfModuleParameters
                                                             +
                                                         lfoParameters;

} //namespace ParameterCounts

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------

//...mrmlj...required to be in the header only for getParameterProperties() and EditorKnob::paint()...
namespace SW { namespace Engine { class Setup; } }
namespace Parameters
{
    template <class Parameter> struct DisplayValueTransformer;

    template <> struct DisplayValueTransformer<SW::GlobalParameters::InputGain>
    {
        template <typename Source>
        static Source transform( Source const & value, SW::Engine::Setup const & ) { return Math::normalisedLinear2dB( value ); }
        typedef boost::mpl::string<'dB'> Suffix;
    };

    template <> struct DisplayValueTransformer<SW::GlobalParameters::OutputGain> : DisplayValueTransformer<SW::GlobalParameters::InputGain> {};

    template <> struct DisplayValueTransformer<SW::GlobalParameters::MixPercentage>
    {
        template <typename Source>
        static Source transform( Source const & value, SW::Engine::Setup const & ) { return Math::normalisedLinear2Percentage( value ); }
        typedef boost::mpl::string<'%'> Suffix;
    };

    template <> struct DisplayValueTransformer<SW::GlobalParameters::OverlapFactor>
    {
        static float transform( unsigned int const parameterValue, SW::Engine::Setup const & )
        {
            float const value     ( Math::convert<float>( parameterValue ) );
            float const percentage( ( value - 1 ) * 100 / value            );
            return percentage;
        }
        typedef boost::mpl::string<'%'> Suffix;
    };
} // namespace Parameters

//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameters_hpp
