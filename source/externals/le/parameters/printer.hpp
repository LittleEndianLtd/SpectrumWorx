////////////////////////////////////////////////////////////////////////////////
///
/// \file printer.hpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef printer_hpp__272E159A_8E28_48C6_9775_6C7199CEA3CE__mrmlj
#define printer_hpp__272E159A_8E28_48C6_9775_6C7199CEA3CE__mrmlj
#pragma once
//------------------------------------------------------------------------------
#include "printer_fwd.hpp"

#include "boolean/printer.hpp"
#include "dynamic/printer.hpp"
#include "enumerated/printer.hpp"
#include "linear/printer.hpp"
#include "powerOfTwo/printer.hpp"
#include "symmetric/printer.hpp"
#include "trigger/printer.hpp"

#ifdef __GNUC__ //...ugh...mrmlj...for Plugins::*AutomatedParameter...clean this up...
#include "le/plugins/plugin.hpp"
#endif // __GNUC__

#include "le/utility/platformSpecifics.hpp"
#include "le/utility/tchar.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

LE_OPTIMIZE_FOR_SIZE_BEGIN()

////////////////////////////////////////////////////////////////////////////////
//
// print()
// -------
//
////////////////////////////////////////////////////////////////////////////////

template <class Parameter, typename Source>
char const * LE_COLD LE_FASTCALL print
(
    Source                    const parameterValue,
    SW::Engine::Setup const &       engineSetup,
    PrintBuffer       const &       buffer
)
{
    char const * const valueString( Detail::print<Parameter>( parameterValue, engineSetup, buffer, typename Parameter::Tag() ) );
    BOOST_ASSERT( ( valueString != buffer.begin() ) || ( std::strlen( valueString ) < unsigned( buffer.size() ) ) );
    return valueString;
}


#pragma warning( push )
#pragma warning( disable : 4510 ) // Default constructor could not be generated.
#pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.

struct PrinterBase
{
    typedef char const * result_type;
    template <class Parameter, typename Source>
    result_type LE_COLD LE_FASTCALL operator()( Source const parameterValue ) const { return Parameters::print<Parameter>( parameterValue, engineSetup, buffer ); }
    Parameters::PrintBuffer         const buffer     ;
    SW::Engine::Setup       const &       engineSetup;
}; // struct PrinterBase

struct ParameterPrinter
{
    using result_type = char const *;
    template <class Parameter>
    result_type LE_COLD LE_FASTCALL operator()() const { return printer.operator()<Parameter>( parameterValue ); }
    float       const parameterValue;
    PrinterBase const printer;
}; // struct ParameterPrinter

struct Printer
{
    using result_type = ParameterPrinter::result_type;

    template <class Parameter>
    result_type LE_COLD LE_FASTCALL operator()( Parameter const & parameter ) const
    {
        char const * const pString
        (
            pValue_
                ? printer.operator()<Parameter>( *pValue_             )
                : printer.operator()<Parameter>( parameter.getValue() )
        );
        return pString;
    }

    float const * LE_RESTRICT const pValue_;
    
    PrinterBase printer;
}; // struct Printer

struct AutomatedParameterPrinter
{
    using result_type = ParameterPrinter::result_type;

    enum ValueSource : std::uint8_t { NormalisedLinear, Linear, Unchanged, Internal };

    template <class Parameter>
    LE_COLD result_type LE_FASTCALL operator()( Parameter const & parameter ) const
    {
        switch ( valueSource )
        {
            case Internal: return printer.operator()<Parameter>( parameter.getValue() );
            default      : return this  ->operator()<Parameter>(                      );
        }
    }

    template <class Parameter>
    LE_COLD result_type LE_FASTCALL operator()() const
    {
        float const automationValue( this->automationValue );
        Parameter parameterValue;
        /// \note An ad hoc double-dispatch implementation to account for both
        /// the different automation/parameter marshaling ABIs as well as for
        /// different parameter types (with different printing logic).
        /// To be cleaned up, decoupled, .....
        ///                                   (09.12.2014.) (Domagoj Saric)
        switch ( valueSource )
        {
            case NormalisedLinear: parameterValue.setValue( Plugins::NormalisedAutomatedParameter::convertAutomationToParameterValue<Parameter>( automationValue ) ); break;
            case Linear          : parameterValue.setValue( Plugins::FullRangeAutomatedParameter ::convertAutomationToParameterValue<Parameter>( automationValue ) ); break;
            case Unchanged       : parameterValue.setValue( Math::convert<typename Parameter::value_type>                                      ( automationValue ) ); break;
                                 //return printer.operator()<Parameter>( automationValue );
            LE_DEFAULT_CASE_UNREACHABLE();
        }
        return printer.operator()<Parameter>( parameterValue.getValue() );
    }

    mutable float       automationValue;
    mutable ValueSource valueSource    ;
            PrinterBase printer        ;
}; // struct AutomatedParameterPrinter

#pragma warning( pop )

LE_OPTIMIZE_FOR_SIZE_END()

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // printer_hpp
