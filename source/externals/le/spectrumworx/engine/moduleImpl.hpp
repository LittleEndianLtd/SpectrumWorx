////////////////////////////////////////////////////////////////////////////////
///
/// \file moduleImpl.hpp
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef moduleImpl_hpp__0876E0CC_6291_428B_AC32_93B28EDD1251
#define moduleImpl_hpp__0876E0CC_6291_428B_AC32_93B28EDD1251
#pragma once
//------------------------------------------------------------------------------
#include "configuration.hpp"
#include "module.hpp"

#ifndef LE_NO_LFOs
#include "le/parameters/lfo.hpp"
#endif // !LE_NO_LFOs
#ifndef LE_SW_SDK_BUILD
#include "le/plugins/plugin.hpp" //...ugh...mrmlj...for Plugins::*AutomatedParameter usage in printer.hpp...clean this up...
#include "le/parameters/parametersUtilities.hpp"
#include "le/parameters/printer.hpp" // printers for module parameters
#include "le/parameters/uiElements.hpp"
#endif // LE_SW_SDK_BUILD
#include "le/parameters/fusionAdaptors.hpp"
#include "le/parameters/trigger/tag.hpp"
#include "le/spectrumworx/effects/effects.hpp"
#include "le/spectrumworx/engine/channelData.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"
#include "le/utility/buffers.hpp"
#include "le/utility/platformSpecifics.hpp"

#include <boost/range/iterator_range_core.hpp>

#include <array>
#include <cstdint>
#include <utility> // std::(make_)index_sequence
//------------------------------------------------------------------------------
#ifdef _MSC_VER // msvc12 does not have std::make_index_sequence
__if_not_exists( std::make_index_sequence )
{
namespace std
{
    template <std::uint16_t...> struct index_sequence { using type = index_sequence; };

    template <class S1, class S2> struct concat_impl;
    template <std::uint16_t... I1, std::uint16_t... I2>
    struct concat_impl<index_sequence<I1...>, index_sequence<I2...>> : index_sequence<I1..., ( sizeof...(I1)+I2 )...>{};

    template <class S1, class S2>
    using concatenate = typename concat_impl<S1, S2>::type;

    template <std::uint16_t N> struct make_index_sequence_impl;
    template <std::uint16_t N>
    struct make_index_sequence_impl : concatenate<typename make_index_sequence_impl<N / 2>::type, typename make_index_sequence_impl<N - N / 2>::type> {};

    template <> struct make_index_sequence_impl<0> : index_sequence< > {};
    template <> struct make_index_sequence_impl<1> : index_sequence<0> {};

    template<std::uint16_t N>
    using make_index_sequence = typename make_index_sequence_impl<N>::type;
} // namespace std
} // __if_not_exists( std::make_index_sequence )
#endif
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
    template <class Parameter> struct DiscreteValues;
    template <class Parameter> struct DisplayValueTransformer;
    template <class Parameter> struct Name;

    struct PowerOfTwoParameterTag;
} // namespace Parameters
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine
{
//------------------------------------------------------------------------------

struct StorageFactors;

namespace Detail ///< \internal
{
    using offset_t      = std::uint8_t;
    using index_t       = std::size_t;
    using ParameterInfo = Parameters::RuntimeInformation;

    //...mrmlj...ugh...cleanup and document this...
    //...mrmlj...C++11 prototypes/experiments at creating non-trivial
    //...variadic-length arrays w/o macros and limited maximum lengths...
    template <typename Parameters, typename Data, typename Indices>
    struct array_aux;
#if 1 // the shorter version below does not yet work even with Clang 3.5
    template <typename Parameters, index_t... Indices>
    struct array_aux<Parameters, ParameterInfo, std::index_sequence<Indices...>>
    {
        static std::uint16_t const N = sizeof...( Indices );
        using DataArray = std::array<ParameterInfo const, N>;
        static DataArray const data;
    }; // struct array_aux

    template <typename Parameters, index_t... Indices>
    struct array_aux<Parameters, offset_t, std::index_sequence<Indices...>>
    {
        static std::uint16_t const N = sizeof...( Indices );
        using DataArray = std::array<offset_t const, N>;
        static DataArray const data;
    }; // struct array_aux

    // terminating partial specialisations
    template <typename Parameters>
    struct array_aux<Parameters, ParameterInfo, std::index_sequence<>>
    {
        static BOOST_CONSTEXPR ParameterInfo const * const data
        #ifndef BOOST_NO_CXX11_CONSTEXPR
            = nullptr
        #endif // BOOST_NO_CXX11_CONSTEXPR
        ;
    }; // struct array_aux
#ifdef BOOST_NO_CXX11_CONSTEXPR
    template <typename Parameters>
    ParameterInfo const * const array_aux<Parameters, ParameterInfo, std::index_sequence<>>::data = nullptr;
#endif // BOOST_NO_CXX11_CONSTEXPR
    template <typename Parameters>
    struct array_aux<Parameters, offset_t, std::index_sequence<>>
    {
        static BOOST_CONSTEXPR offset_t const * const data
        #ifndef BOOST_NO_CXX11_CONSTEXPR
            = nullptr
        #endif // BOOST_NO_CXX11_CONSTEXPR
        ;
    }; // struct array_aux
#ifdef BOOST_NO_CXX11_CONSTEXPR
    template <typename Parameters>
    offset_t const * const array_aux<Parameters, offset_t, std::index_sequence<>>::data = nullptr;
#endif // BOOST_NO_CXX11_CONSTEXPR
#else // disabled/does not work
    template <typename Parameters, typename Data, index_t... Indices>
    struct array_aux<Parameters, Data, std::index_sequence<Indices...>>
    {
        static std::uint16_t const N = sizeof...( Indices );
        using DataArray = std::array<Data const, N>;

        static DataArray const data;
    }; // struct array_aux

    // terminating partial specialisation
    template <typename Parameters, typename Data>
    struct array_aux<Parameters, Data, std::index_sequence<>>
    {
        using DataArray = std::array<Data const, 0>;
        static DataArray const data;
    }; // struct array_aux
    template <typename Parameters, typename Data>
    typename array_aux<Parameters, Data, std::index_sequence<>>::DataArray BOOST_CONSTEXPR_OR_CONST array_aux<Parameters, Data, std::index_sequence<>>::data;
#endif

    template <class Parameters, unsigned index>
    offset_t valueOffsetGetter()
    {
        //...mrmlj...this is actually UB and sooner or later Clang will start
        //...mrmlj...miscompiling this...fix it ASAP...
        // http://stackoverflow.com/questions/35028438/getting-the-offset-of-a-member-variable-via-casting-a-nullptr
        auto const address
        (
            &(
                static_cast<Parameters const *>( nullptr )-> template get<typename Parameters:: template ParameterAt<index>::type>()
            )
        );
        return static_cast<offset_t>( reinterpret_cast<std::size_t>( address ) );
    }
    template <typename Parameters, index_t... Indices>
    typename array_aux<Parameters, offset_t, std::index_sequence<Indices...>>::DataArray const
        array_aux<Parameters, offset_t, std::index_sequence<Indices...>>::data = {{ valueOffsetGetter<Parameters, Indices>()... }};

    //...mrmlj...cleanup and document this...

    template <typename ParameterTypeTag> struct ParameterType;

    template <> struct ParameterType<Parameters::BooleanParameterTag         > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Boolean      ; };
    template <> struct ParameterType<Parameters::TriggerParameterTag         > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Trigger      ; };
    template <> struct ParameterType<Parameters::LinearIntegerParameterTag   > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Integer      ; };
  //template <> struct ParameterType<Parameters::LinearSignedInteger         > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Integer      ; };
  //template <> struct ParameterType<Parameters::LinearUnsignedInteger       > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Integer      ; };
    template <> struct ParameterType<Parameters::SymmetricIntegerParameterTag> { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Integer      ; };
    template <> struct ParameterType<Parameters::EnumeratedParameterTag      > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::Enumerated   ; };
    template <> struct ParameterType<Parameters::LinearFloatParameterTag     > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::FloatingPoint; };
    template <> struct ParameterType<Parameters::SymmetricFloatParameterTag  > { static ParameterInfo::Type BOOST_CONSTEXPR_OR_CONST type = ParameterInfo::FloatingPoint; };

    struct NonEnumeratedParameter
    {
        static BOOST_CONSTEXPR char const * LE_RESTRICT const * LE_RESTRICT const strings
        #ifndef BOOST_NO_CXX11_CONSTEXPR
            = nullptr
        #endif // BOOST_NO_CXX11_CONSTEXPR
            ;
    };
    template <class Parameter, typename Tag> struct EnumeratedValueStrings                                                { using type = NonEnumeratedParameter                   ; };
    template <class Parameter              > struct EnumeratedValueStrings<Parameter, Parameters::EnumeratedParameterTag> { using type = LE::Parameters::DiscreteValues<Parameter>; };

#ifndef BOOST_NO_CXX11_CONSTEXPR //...mrmlj...msvc12 creates dynamic initialisers if the info<>() function template is used...
    template <class Parameter>
    BOOST_CONSTEXPR ParameterInfo info()
    {
        static_assert
        (
            !std::is_same<typename Parameter::Tag, LE::Parameters::PowerOfTwoParameterTag>::value,
            "Meaningful only for linear parameters." //...mrmlj...
        );

        using Tag = typename Parameter::Tag;

        return
        {
            ParameterType<Tag>::type,

            static_cast<float>( Parameter::unscaledMinimum ) / Parameter::rangeValuesDenominator,
            static_cast<float>( Parameter::unscaledMaximum ) / Parameter::rangeValuesDenominator,
            static_cast<float>( Parameter::unscaledDefault ) / Parameter::rangeValuesDenominator,

            Parameters::Name<Parameter>::string_, //...mrmlj...parameter names are required for presets
        #ifdef LE_NO_PARAMETER_STRINGS
            nullptr, nullptr
        #else
            boost::mpl::c_str<typename Parameters::DisplayValueTransformer<Parameter>::Suffix>::value,
            EnumeratedValueStrings<Parameter, Tag>::type::strings
        #endif // LE_NO_PARAMETER_STRINGS
        };
    }
#endif // !BOOST_NO_CXX11_CONSTEXPR

    template <typename Parameters, index_t... Indices>
    typename array_aux<Parameters, ParameterInfo, std::index_sequence<Indices...>>::DataArray const
        array_aux<Parameters, ParameterInfo, std::index_sequence<Indices...>>::data =
    {{
    #ifndef BOOST_NO_CXX11_CONSTEXPR
        info<typename boost::fusion::result_of::value_at_c<Parameters, Indices>::type>()...
    #else
        {
            ParameterType<typename boost::fusion::result_of::value_at_c<Parameters, Indices>::type::Tag>::type,
            static_cast<float>(    boost::fusion::result_of::value_at_c<Parameters, Indices>::type::unscaledMinimum ) / boost::fusion::result_of::value_at_c<Parameters, Indices>::type::rangeValuesDenominator,
            static_cast<float>(    boost::fusion::result_of::value_at_c<Parameters, Indices>::type::unscaledMaximum ) / boost::fusion::result_of::value_at_c<Parameters, Indices>::type::rangeValuesDenominator,
            static_cast<float>(    boost::fusion::result_of::value_at_c<Parameters, Indices>::type::unscaledDefault ) / boost::fusion::result_of::value_at_c<Parameters, Indices>::type::rangeValuesDenominator,
            LE::Parameters::Name<typename boost::fusion::result_of::value_at_c<Parameters, Indices>::type>::string_,
        #ifdef LE_NO_PARAMETER_STRINGS
            nullptr, nullptr
        #else
            boost::mpl::c_str<typename LE::Parameters::DisplayValueTransformer<typename boost::fusion::result_of::value_at_c<Parameters, Indices>::type>::Suffix>::value,
            EnumeratedValueStrings<typename boost::fusion::result_of::value_at_c<Parameters, Indices>::type, typename boost::fusion::result_of::value_at_c<Parameters, Indices>::type::Tag>::type::strings,
        #endif // LE_NO_PARAMETER_STRINGS
        }...
    #endif
    }};

    template <class Parameters>
    struct ParametersInformation : array_aux<Parameters, ParameterInfo, std::make_index_sequence<Parameters::static_size>> {};

    BOOST_MPL_HAS_XXX_TRAIT_DEF( ChannelState );

    using ChannelDataProxy = Engine::ModuleDSP::ChannelDataProxy;

    struct MakeChannelStateHolder
    {
        template <class Effect>
        struct ChannelStates
        {
            using ChannelState      = typename Effect::ChannelState;
            using ChannelStateRange = boost::iterator_range<ChannelState * LE_RESTRICT>;

            void LE_FORCEINLINE LE_HOT callProcess
            (
                Effect           const &       effect,
                std::uint8_t             const channel,
                ChannelDataProxy const &       data,
                Engine::Setup    const &       setup
            ) const
            {
                effect.process( channelStates_[ channel ], data, setup );
            }

            LE_OPTIMIZE_FOR_SIZE_BEGIN()

            LE_FORCEINLINE void LE_COLD callReset()
            {
            #ifndef _MSC_VER
                LE_DISABLE_LOOP_UNROLLING()
                LE_DISABLE_LOOP_VECTORIZATION()
            #endif // _MSC_VER
                for ( auto & channelState : channelStates_ )
                    channelState.reset();
            }

            static std::uint16_t const sizeOfChannelState = sizeof( ChannelState );
            static std::uint32_t channelStateRequiredStorage( Engine::StorageFactors const & factors ) { return ChannelState::requiredStorage( factors ); }

            LE_FORCEINLINE LE_NOTHROWNOALIAS LE_COLD
            void LE_FASTCALL resize( Engine::Storage storage, Engine::StorageFactors const & factors )
            {
                LE_ASSUME( factors.numberOfChannels <= 16 );
                char * const pChannelStatesBegin( storage                                                                   .begin() );
                char * const pChannelStatesEnd  ( storage.advance_begin( sizeof( ChannelState ) * factors.numberOfChannels ).begin() );
                channelStates_ = ChannelStateRange
                (
                    reinterpret_cast<ChannelState *>( pChannelStatesBegin ),
                    reinterpret_cast<ChannelState *>( pChannelStatesEnd   )
                );
                BOOST_ASSERT( unsigned( channelStates_.size() ) == factors.numberOfChannels );
            #ifndef _MSC_VER
                LE_DISABLE_LOOP_UNROLLING()
                LE_DISABLE_LOOP_VECTORIZATION()
            #endif // _MSC_VER
                for ( auto & channelState : channelStates_ )
                {
                    BOOST_ASSERT( reinterpret_cast<char *>( &channelState ) < storage.end() );
                    ChannelState * LE_RESTRICT const pNewChannelState( new ( &channelState ) ChannelState );
                    LE_ASSUME( pNewChannelState );
                    pNewChannelState->resize( factors, storage );
                }
            }

            LE_OPTIMIZE_FOR_SIZE_END()

            ChannelStateRange channelStates_;
        }; // struct ChannelStates
    }; // struct MakeChannelStateHolder

    struct MakeEmptyChannelStateHolder
    {
        template <class Effect>
        struct ChannelStates
        {
            void LE_FORCEINLINE LE_HOT callProcess
            (
                Effect           const & effect,
                std::uint8_t           /*channel*/,
                ChannelDataProxy const & data,
                Engine::Setup    const & setup
            ) const
            {
                effect.process( data, setup ); ((void)effect);
            }

            static void callReset() {}

            static std::uint8_t const sizeOfChannelState = 0;
            static std::uint8_t channelStateRequiredStorage( Engine::StorageFactors const & ) { return 0; }

            static void resize( Engine::Storage const &, Engine::StorageFactors const & ) {}
        }; // struct ChannelStates
    }; // struct MakeEmptyChannelStateHolder

#if !LE_NO_PARAMETER_STRINGS
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wignored-attribute"
#endif // __clang__
    template <class Parameters>
    struct EffectParameterPrinter
    {
        LE_NOTHROWNOALIAS LE_COLD static
        char const * LE_GNU_SPECIFIC( /*mrmlj clang crash*/__fastcall ) LE_MSVC_SPECIFIC( LE_FASTCALL )
        print( std::uint8_t const parameterIndex, LE::Parameters::AutomatedParameterPrinter const & printer )
        {
            LE_ASSUME( parameterIndex < Parameters::static_size );
            return LE::Parameters::invokeFunctorOnIndexedParameter<Parameters>
            (
                parameterIndex,
                std::forward<LE::Parameters::AutomatedParameterPrinter const>( printer )
            );
        }
    }; // class EffectParameterPrinter
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__
#endif // !LE_NO_PARAMETER_STRINGS

    template <class Effect, typename TypeIndex>
    struct MakeEffectMetaData { static ModuleParameters::EffectMetaData const data; };

    template <class Effect, typename TypeIndex>
    ModuleParameters::EffectMetaData const MakeEffectMetaData<Effect, TypeIndex>::data =
    {
        Effect::Parameters::static_size,
        TypeIndex::value,
        &ParametersInformation <typename Effect::Parameters>::data[ 0 ],
    #if !LE_NO_PARAMETER_STRINGS
        EffectParameterPrinter<typename Effect::Parameters>::print
    #endif // !LE_NO_PARAMETER_STRINGS
    };

    ////////////////////////////////////////////////////////////////////////////
    // EffectParameterOffsets<Effect>
    ////////////////////////////////////////////////////////////////////////////

    template <class Effect>
    struct EffectParameterOffsets
    {
        /// \note
        /// Up to revision 5812 a different approach was used where the
        /// ModuleDSP class template was also parameterized with an
        /// AutomatedParameter class and used that information to internally
        /// provide all conversion to and from automation values as well as
        /// preset loading and saving. This was replaced with the type-erased
        /// ParameterInfo approach in order to remove automation and preset
        /// related knowledge from the ModuleDSP class template so that it would
        /// be easier to provide SpectrumWorx versions without preset
        /// functionality.
        ///                                   (06.02.2012.) (Domagoj Saric)
        //typedef std::array<std::uint8_t const, Effect::Parameters::static_size> ParameterOffsets;
        using ParameterOffsets = std::uint8_t const * const;
        static ParameterOffsets const parameterOffsets;
    }; // class EffectParameterOffsets

    template <class Effect>
    typename EffectParameterOffsets<Effect>::ParameterOffsets const EffectParameterOffsets<Effect>::parameterOffsets =
    &array_aux
    <
        typename Effect::Parameters,
        offset_t,
        std::make_index_sequence<Effect::Parameters::static_size>
    >::data[ 0 ];
} // namespace Detail

////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleEffectImpl ...mrmlj...cleanup this naming mess...
///
/// \brief Implements the ModuleDSP interface(s) for a given effect.
///
/// Provides the following:
/// - implements the Module interface
/// - holds multiple channel state/data
/// - holds the effect specific UI components (parameter widgets).
///
////////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4127 ) // Conditional expression is constant.
#pragma warning( disable : 4373 ) // Previous versions of the compiler did not override when parameters only differed by const/volatile qualifiers.

template <class EffectParam, class Base>
class LE_NOVTABLE ModuleEffectImpl
    :
    public Base
{
public:
    using Effect = EffectParam;

private:
    using ChannelStatesHolder =
        typename boost::mpl::if_
        <
            Engine::Detail::has_ChannelState<Effect>,
            Engine::Detail::MakeChannelStateHolder,
            Engine::Detail::MakeEmptyChannelStateHolder
        >::type:: template ChannelStates<Effect>;

public:
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wuninitialized"
#endif // __clang__
    template <typename EffectTypeIndex, typename ... T>
    LE_COLD
    ModuleEffectImpl( EffectTypeIndex, T && ... args )
        :
        Base
        (
            std::forward<T>( args )...,
            Engine::Detail::MakeEffectMetaData<Effect, EffectTypeIndex>::data,
        #ifndef LE_NO_LFOs
            &lfos_[ 0 ],
        #endif // !LE_NO_LFOs
            Engine::Detail::EffectParameterOffsets<Effect>::parameterOffsets,
            static_cast<std::uint16_t>
            (
                reinterpret_cast<char const *>( &effect().parameters() )
                    -
                reinterpret_cast<char const *>( static_cast<Base const *>( this ) )
            )
        )
    #ifndef NDEBUG
        ,setupCalled_( false )
    #endif // NDEBUG
    {}
#ifdef __clang__
    #pragma clang diagnostic pop
#endif // __clang__

public:
    Effect       & effect()       { return effect_; }
    Effect const & effect() const { return effect_; }

    ChannelStatesHolder const & channelStatesHolder() const { return channelStatesHolder_; }

protected: // Module process interface implementation.
    LE_NOTHROWNOALIAS LE_FORCEINLINE LE_COLD
    void LE_FASTCALL doPreProcess( Setup const & engineSetup ) LE_OVERRIDE
    {
        effect().setup( ModuleDSP::workingRange(), engineSetup );
    #ifndef NDEBUG
        setupCalled_ = true;
    #endif
    }

    LE_NOTHROWNOALIAS LE_FORCEINLINE LE_HOT
    void LE_FASTCALL doProcess( std::uint8_t const channel, Engine::ModuleDSP::ChannelDataProxy const data, Setup const & setup ) const LE_OVERRIDE
    {
        BOOST_ASSERT( setupCalled_ );
        channelStatesHolder_.callProcess( effect(), channel, data, setup );
    }

LE_OPTIMIZE_FOR_SIZE_BEGIN()
public: //...mrmlj...
    LE_NOINLINE LE_NOTHROWNOALIAS LE_COLD
    void LE_FASTCALL reset() LE_OVERRIDE LE_SEALED { channelStatesHolder_.callReset(); }

private:
    LE_NOTHROWNOALIAS LE_COLD
    bool LE_FASTCALL resize( StorageFactors const & factors ) LE_OVERRIDE LE_SEALED
    {
        //...mrmlj...au uninitialise...BOOST_ASSERT( factors.complete() );

        if ( ChannelStatesHolder::sizeOfChannelState == 0 )
        {
            BOOST_ASSERT( channelStatesHolder_.channelStateRequiredStorage( factors ) == 0 );
            return true;
        }

        if
        ( BOOST_LIKELY(
            this->allocateStorage
            (
                factors,
                channelStatesHolder_.sizeOfChannelState,
                channelStatesHolder_.channelStateRequiredStorage( factors )
            ) )
        )
        {
            channelStatesHolder_.resize( this->storage(), factors );
            ModuleEffectImpl<Effect, Base>::reset();
            return true;
        }

        return false;
    }
LE_OPTIMIZE_FOR_SIZE_END()

private:
    Effect              effect_             ;
    ChannelStatesHolder channelStatesHolder_;

#ifndef LE_NO_LFOs
    /// \note Only allocate the storage for all (base + effect) LFOs in the
    /// implementation class and let the base class construct all the LFOs. The
    /// other approach is to have the base class allocate and construct the
    /// base parameter LFOs and the implementation class the LFOs for the effect
    /// specific parameters. The first approach is better as all the LFOs are in
    /// a single, contiguous location (and can thus be accessed through a single
    /// pointer) and the LFO array construction code is generated only in a
    /// single location (smaller code and faster compiles).
    ///                                       (23.04.2015.) (Domagoj Saric)
    using LFOStorage = std::array<ModuleParameters::LFOPlaceholder, ModuleParameters::numberOfLFOBaseParameters + Effect::Parameters::static_size>;
    LFOStorage lfos_;
#endif // !LE_NO_LFOs

#ifndef NDEBUG
    bool setupCalled_;
#endif
}; // class ModuleEffectImpl

#pragma warning( pop )


template <class Effect>
class ModuleDSP::Impl LE_SEALED
    : public ModuleEffectImpl<Effect, ModuleDSP>
{
public:
#if ( _MSC_VER < 1900 ) && !defined( __clang__ )
    template <typename ... T>
    LE_COLD
    Impl( T && ... args ) : ModuleEffectImpl( std::forward<T>( args )... ) {}
#else
    using ModuleEffectImpl<Effect, ModuleDSP>::ModuleEffectImpl;
#endif // _MSC_VER
}; // class ModuleDSP::Impl

//...mrmlj...assummes single inclusion...
ModuleParameters::ParameterInfos const & ModuleParameters::parameterInfos()
{
    return Engine::Detail::ParametersInformation<ModuleParameters::BaseParameters>::data;
}
#ifdef BOOST_NO_CXX11_CONSTEXPR
LE_WEAK_SYMBOL
char const * LE_RESTRICT const * LE_RESTRICT const Engine::Detail::NonEnumeratedParameter::strings = nullptr;
#endif // BOOST_NO_CXX11_CONSTEXPR

//------------------------------------------------------------------------------
} // namespace Engine
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // moduleImpl_hpp
