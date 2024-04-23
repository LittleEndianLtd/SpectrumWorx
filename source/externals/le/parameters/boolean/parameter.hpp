////////////////////////////////////////////////////////////////////////////////
///
/// \file boolean/parameter.hpp
/// ---------------------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( __clang__ ) && ( _MSC_VER < 1800 )
#include "le/parameters/cpp03/boolean/parameter.hpp"
#endif // old MSVC
//------------------------------------------------------------------------------ 
#ifndef parameter_hpp__D017E02E_44C0_40FE_BC6B_4493C53BF028
#define parameter_hpp__D017E02E_44C0_40FE_BC6B_4493C53BF028
#pragma once
//------------------------------------------------------------------------------
#include "tag.hpp"

#include "le/parameters/parameter.hpp" //...mrmlj...for Default

#include "boost/assert.hpp"
#include "boost/concept_check.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

namespace Traits { template <int stringLiteralPart1, int stringLiteralPart2> struct Unit; }

template <typename... Traits> struct TraitPack;

namespace Detail ///< \internal
{
    template <bool defaultValue = false>
    struct BooleanParameterTraits
    {
    public: // Types.
        using Tag = BooleanParameterTag;

        using value_type = bool;
        using param_type = value_type;

        using Defaults = TraitPack<Traits::Unit<0, 0>>;
        using Traits   = TraitPack<Traits::Default<defaultValue>>; //...mrmlj...FMOD param info...

    public:
        static bool BOOST_CONSTEXPR_OR_CONST unscaledMinimum = false;
        static bool BOOST_CONSTEXPR_OR_CONST unscaledMaximum = true ;
        static bool BOOST_CONSTEXPR_OR_CONST unscaledDefault = defaultValue;

        static unsigned char BOOST_CONSTEXPR_OR_CONST rangeValuesDenominator = 1;

    public: // Values.
        static bool minimum () { return unscaledMinimum; }
        static bool maximum () { return unscaledMaximum; }
        static bool default_() { return unscaledDefault; }

        static unsigned char const discreteValueDistance = 1;

        static bool isValidValue( value_type const value )
        {
            BOOST_ASSERT( ( value == false ) || ( value == true ) );
            boost::ignore_unused_variable_warning( value );
            return true;
        }

    protected:
        static void increment( value_type & value ) { value = true ; }
        static void decrement( value_type & value ) { value = false; }
    };
} // namespace Detail

template <class Traits> class Parameter;

////////////////////////////////////////////////////////////////////////////////
/// \internal
/// \typedef Boolean
////////////////////////////////////////////////////////////////////////////////

struct Boolean : Parameter<Detail::BooleanParameterTraits<false>>
{
private:
    using Base = Parameter<Detail::BooleanParameterTraits<false>>;

public:
    template <class... NewTraits>
    struct Modify;

public:
    Boolean( param_type const initialValue = false ) : Base( initialValue ) {}
}; // struct Boolean

template <> struct Boolean::Modify<>                                   { using type = Detail::BooleanParameterTraits<     >; };
template <> struct Boolean::Modify<Parameters::Traits::Default<false>> { using type = Detail::BooleanParameterTraits<false>; };
template <> struct Boolean::Modify<Parameters::Traits::Default<true >> { using type = Detail::BooleanParameterTraits<true >; };

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // parameter_hpp
