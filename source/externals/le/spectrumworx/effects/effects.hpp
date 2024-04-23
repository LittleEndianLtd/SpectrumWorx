////////////////////////////////////////////////////////////////////////////////
///
/// \file effects.hpp
/// -----------------
///
/// Base header for LittleEndianLibrary effects, documenting design decisions
/// and guidelines and the requirements imposed on effect classes. It also acts
/// as an 'incubator' for shared functionality until it matures and is extracted
/// into separate modules.
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef effects_hpp__552EB9F4_DD3A_45AD_B1A1_C50269CF6C59
#define effects_hpp__552EB9F4_DD3A_45AD_B1A1_C50269CF6C59
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/effects/channelStateStatic.hpp"
#include "le/spectrumworx/engine/channelData_fwd.hpp"

#include "boost/fusion/support/category_of.hpp"

#include <cstdint>
#include <utility>
//------------------------------------------------------------------------------
namespace LE
{
namespace Parameters { struct Tag; }
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
    class Setup;
LE_IMPL_NAMESPACE_END( Engine )
LE_IMPL_NAMESPACE_BEGIN( Effects )
    class IndexRange;
LE_IMPL_NAMESPACE_END( Effects )
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Frequency domain effect class requirements
// ------------------------------------------
//
//  Each frequency domain effect (named "AnEffect") must:
//
// * define a base AnEffect class that:
//  - (optional - if the effect has parameters intended for the end user)
//    defines all of the effect's public/UI parameters and provides a public
//    member type called "Parameters" that contains all the parameters and
//    models a Boost.Fusion associative container (either the parameters factory
//    macros or the Parameters and Parameter class templates can be used)
//  - defines a boolean 'usesSideChannel' static constant
//  - defines 'title' and 'description' static C strings
//
// * define an AnEffectImpl implementation class that
//  - derives from (NoParameters)EffectImpl<AnEffect>:
//    - whether the EffectImpl or NoParametersEffectImpl helper class template
//      is used depends on whether or not AnEffect defines/has any parameters
//    - the purpose of the helper class template is to reduce verbosity and
//      provide a uniform interface for accessing an effect's parameters: it
//      will hold an instance of AnEffect::Parameters (or a dummy empty
//      parameters object) and provide access to them through its parameters()
//      member function
//  - is default constructible
//  - (optional - if the effect needs to maintain audio-data-specific state
//    between calls to its process() member function):
//    provide a public member type called "ChannelState" that holds all the
//    state data for a single channel of audio for the particular effect. This
//    way the effect itself can be decoupled from the idea of processing
//    multiple audio channels so that the knowledge and functionality regarding
//    multichannel processing does not have to be duplicated in each effect.
//    Rather, the user and/or the wrapping class is responsible for creating as
//    many ChannelState instances as it wants/needs and then to pass the
//    appropriate instance in a call to the effect's process function(s)
//  - (optional - if the effect provides the ChannelState type):
//    the ChannelState member must have a "reset()" member function (that resets
//    it to an initial state) in addition to the resize() and requiredStorage()
//    member functions that all shared storage classes must have
//  - (at least) have the following member functions:
//      - void setup
//        (
//            IndexRange const &,
//            Engine::Setup const &
//        )
//      - void process
//        (
//            (optional - see above) ChannelState &,
//            Engine::*ChannelData_*
//            Engine::Setup const &
//        ) const
//  - the above functions must be no-fail/must not throw exceptions (and so no
//    exception specification is necessary in their documentation)
//  - the purpose of the setup() member function is to allow effects which need
//    to do nontrivial preprocessing of their parameters to cache those
//    preprocessed results in member variables in the setup() call (to avoid
//    constant recalculation in the process() function)
//  - the IndexRange setup() parameter holds the effect's current
//    working range (precalculated from the base Start/StopFrequency parameters)
//  - the effect's Parameters instance and the Engine::Setup instance passed to
//    the process() function are guaranteed to be unchanged from the previous
//    setup() call
//  - each effect implementation class is free to choose to work with any of the
//    various ChannelData classes (it makes the choice by simply declaring its
//    process() function to use the desired type).
//
//  The Base-Impl separation is required to facilitate easier extraction of
// effects into the SW SDK without duplication and without disclosing
// internal implementation details. To ensure ABI compatibility all headers that
// contain the "base" effect class definitions (that are therefore part of
// the SDK) must include "boost/config/abi_prefix.hpp" and
// "boost/config/abi_suffix.hpp" headers. The abi_prefix.hpp header must appear
// after all other includes and before any code while the abi_suffix.hpp header
// must appear at the end of the file (before the closing endif).
//
//  All processing is done in-place, side-channel data is considered read only.
//
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
///
/// \class EffectImpl
///
////////////////////////////////////////////////////////////////////////////////

template <class EffectBase>
class EffectImpl : public EffectBase
{
public:
    typename EffectBase::Parameters       & parameters()       { return parameters_; }
    typename EffectBase::Parameters const & parameters() const { return parameters_; }

private:
    typename EffectBase::Parameters parameters_;
};


////////////////////////////////////////////////////////////////////////////////
///
/// \class NoParametersEffectImpl
///
////////////////////////////////////////////////////////////////////////////////

namespace Detail
{
    struct EmptyParameters
    {
        static std::size_t const static_size = 0;
        struct category : boost::fusion::forward_traversal_tag, boost::fusion::associative_tag {};
        using fusion_tag = LE::Parameters::Tag                             ;
        using tag        = boost::fusion::fusion_sequence_tag              ;
        using is_view    = std::false_type                                 ;
        using size       = std::integral_constant<std::size_t, static_size>;
        template <unsigned int   , int dummy = 0> struct ParameterAt;
        template <class Parameter, int dummy = 0> struct IndexOf : size {};
        operator boost::fusion::detail::from_sequence_convertible_type() const;
    };
} // namespace Detail

template <class EffectBase>
class NoParametersEffectImpl : public EffectBase
{
public:
    using Parameters = Detail::EmptyParameters;

    Parameters       & parameters()       { static Parameters dummy; return dummy;                             }
    Parameters const & parameters() const { return const_cast<NoParametersEffectImpl &>( *this ).parameters(); }
};


/// \todo Extract this to a separate header.
///                                           (31.01.2011.) (Domagoj Saric)

//...mrmlj...cleanup these duplicated typedefs (also in fft.hpp)...
using         DataRange = boost::iterator_range<float       * LE_RESTRICT>;
using ReadOnlyDataRange = boost::iterator_range<float const * LE_RESTRICT>;


////////////////////////////////////////////////////////////////////////////////
///
/// \class UnpackedMagPhaseMode
///
////////////////////////////////////////////////////////////////////////////////

namespace CommonParameters { class Mode; }

class UnpackedMagPhaseMode
{
public:
    void unpack( CommonParameters::Mode const & );

    bool magnitudes() const;
    bool phases    () const;

private:
    bool magnitudes_;
    bool phases_    ;
}; // class UnpackedMagPhaseMode


////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuloCounter
///
/// A counter that wraps around/resets to zero after going past an, externally
/// specified, value. The wrap-around value is not held by ModuloCounter
/// instances because it is intended for use in ChannelState objects which will
/// usually share a single wrap-around value (because of multiple ChannelStates
/// per effect instances).
///
////////////////////////////////////////////////////////////////////////////////

class ModuloCounter
{
public:
    ModuloCounter() : counter_( 0 ) {}

    using value_type = std::uint16_t;

    std::pair<value_type, bool> nextValueFor( value_type moduloValue );

    void reset() { counter_ = 0; }

             value_type value() const { return counter_; }
    operator value_type      () const { return value() ; }

private:
    value_type counter_;
}; // class ModuloCounter


struct ModuloCounterChannelState : StaticChannelState
{
    ModuloCounter frameCounter;
    void reset() { frameCounter.reset(); }
}; // struct ModuloCounterChannelState

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // effects_hpp
