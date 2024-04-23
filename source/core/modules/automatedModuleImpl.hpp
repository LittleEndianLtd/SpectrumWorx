////////////////////////////////////////////////////////////////////////////////
///
/// \file automatedModuleImpl.hpp
/// ----------------------------
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef automatedModuleImpl_hpp__4462A949_9FE5_44AF_8252_402F9E776690
#define automatedModuleImpl_hpp__4462A949_9FE5_44AF_8252_402F9E776690
#pragma once
//------------------------------------------------------------------------------
#include "automatedModule.hpp"

#include "le/utility/cstdint.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

template <class InterfaceImpl>
class LE_NOVTABLE AutomatedModuleImpl
{
public: // Parameters
    Plugins::AutomatedParameterValue LE_NOALIAS LE_FASTCALL getSharedAutomatedParameter        ( std::uint8_t parameterIndex              , bool normalised ) const;
    Plugins::AutomatedParameterValue LE_NOALIAS LE_FASTCALL getEffectSpecificAutomatedParameter( std::uint8_t effectSpecificParameterIndex, bool normalised ) const;
    Plugins::AutomatedParameterValue LE_NOALIAS LE_FASTCALL getAutomatedParameter              ( std::uint8_t parameterIndex              , bool normalised ) const;

    void                                        LE_FASTCALL setAutomatedParameter              ( std::uint8_t parameterIndex, Plugins::AutomatedParameterValue, bool normalised );

    LE_NOTHROWNOALIAS char const * getParameterValueString( std::uint8_t parameterIndex, LE::Parameters::AutomatedParameterPrinter const & ) const;

private:
    InterfaceImpl       & impl()       { LE_ASSUME( this ); return *static_cast<InterfaceImpl * LE_RESTRICT>( this ); }
    InterfaceImpl const & impl() const { return const_cast<AutomatedModuleImpl &>( *this ).impl(); }
}; // class AutomatedModuleImpl

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // automatedModuleImpl_hpp
