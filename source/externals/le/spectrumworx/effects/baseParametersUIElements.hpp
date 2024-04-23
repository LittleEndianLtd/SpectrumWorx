////////////////////////////////////////////////////////////////////////////////
///
/// \file  baseParametersUIElements.hpp
/// -----------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
// Implementation note:
//   This separation/extraction of UIElements definitions is required for the
// SW SDK because the parameter printing functionality is not provided by nor
// needed for the SDK so we do not want to leak its details unnecessarily.
//                                            (12.04.2011.) (Domagoj Saric)
//////////////////////////////////////////////////////////////////////////////// 
//------------------------------------------------------------------------------
#ifndef baseParametersUIElements_hpp__AC83F587_990D_454A_BD11_5BC1045DF3C2
#define baseParametersUIElements_hpp__AC83F587_990D_454A_BD11_5BC1045DF3C2
#pragma once
//------------------------------------------------------------------------------
#include "baseParameters.hpp"

#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
namespace SW
{
LE_IMPL_NAMESPACE_BEGIN( Engine )
    class Setup;
LE_IMPL_NAMESPACE_END( Engine )
} // namespace SW
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

template <>
struct DisplayValueTransformer<SW::Effects::BaseParameters::StartFrequency>
{
    static float transform( float const & value, SW::Engine::Setup const & );

    typedef boost::mpl::string<' Hz'> Suffix;
};

template <>
struct DisplayValueTransformer<SW::Effects::BaseParameters::StopFrequency>
    : DisplayValueTransformer<SW::Effects::BaseParameters::StartFrequency>{};


/// \note Clang complains about "too late" explicit instantiations in unity
/// builds.
///                                           (12.12.2012.) (Domagoj Saric)
#ifdef __GNUC__
    template<> char const Name<SW::Effects::BaseParameters::Bypass        >::string_[];
    template<> char const Name<SW::Effects::BaseParameters::Gain          >::string_[];
    template<> char const Name<SW::Effects::BaseParameters::Wet           >::string_[];
    template<> char const Name<SW::Effects::BaseParameters::StartFrequency>::string_[];
    template<> char const Name<SW::Effects::BaseParameters::StopFrequency >::string_[];
#endif // __GNUC__

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // baseParametersUIElements_hpp
