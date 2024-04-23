////////////////////////////////////////////////////////////////////////////////
///
/// \file lfo.hpp
/// -------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef lfo_hpp__43E00321_1DA0_4093_9688_EC409F3DE554
#define lfo_hpp__43E00321_1DA0_4093_9688_EC409F3DE554
#pragma once
//------------------------------------------------------------------------------
#include "le/parameters/enumerated/parameter.hpp"
#include "le/utility/abi.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Parameters
{
//------------------------------------------------------------------------------

/// \addtogroup Parameters
/// Parameter abstractions and utilities used across LE SDKs
/// @{

////////////////////////////////////////////////////////////////////////////////
///
/// \class LFO
///
/// \brief <a href="http://en.wikipedia.org/wiki/Low-frequency_oscillation">
/// Low frequency oscillator</a> for automatic modulation of parameters
///
/// \details A first time encounter with LFOs might be overwhelming (especially
/// for those without a musical and/or mathematical background). In such cases,
/// climbing the learning curve might be easier by first playing with the SW
/// plugin (and consulting the topic on LFOs in its manual) as the hands-on,
/// realtime, GUI experience should provide an intuitive understanding.
///
////////////////////////////////////////////////////////////////////////////////

class LFO
{
public:
#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
    #pragma warning( push )
    #pragma warning( disable : 4480 )
#endif // _MSC_VER
#ifdef DOXYGEN_ONLY
    /// \brief Types of possible LFO synchronizations to tempo
    /// \details Most often the best or desired effect of modulating a parameter
    /// with an LFO is achieved by synchronizing the LFO's frequency or period
    /// with the rhythm (i.e. the tempo and the <a
    /// href="http://en.wikipedia.org/wiki/Meter_(music)">meter</a>) of a song
    /// or sound.<BR>
    /// The ("synchronized") relationship between a rhythm's beat and the LFO's
    /// period need not be a straightforward 1-to-1 mapping (i.e. setting the
    /// period to the length of one quarter note), all sorts of interesting,
    /// syncopated rhythms can be achieved and this is what this parameter aims
    /// to ease: set the allowed 'relationships' between the LFO's period and
    /// the master tempo and then tweak the period parameter until the desired
    /// effect is achieved (the LFO object will 'snap' the period parameter to
    /// the closest value allowed by the set SyncTypes).
    enum SyncType
    {
        Free   , ///< Allow free/unconstrained period adjustment
        Quarter, ///< Allow syncing to (parts or multiples) of <a href="http://en.wikipedia.org/wiki/Quarter_note">quarter notes</a> (beats in most often used meters)
        Triplet, ///< Allow syncing to (parts or multiples) of <a href="http://en.wikipedia.org/wiki/Tuplet#Triplets">triplet notes</a>
        Dotted , ///< Allow syncing to (parts or multiples) of <a href="http://en.wikipedia.org/wiki/Dotted_note">dotted notes</a>
        All      ///< Allow all supported synchronization types
    }; // enum SyncType
#else // !DOXYGEN_ONLY
    enum SyncType : std::uint8_t
    {
        Free    = 0,
        Quarter = 1 << 0,
        Triplet = 1 << 1,
        Dotted  = 1 << 2,

        All = ( Quarter | Triplet | Dotted )
    }; // enum SyncType
#endif // DOXYGEN_ONLY

    /// Supported <a href="http://en.wikipedia.org/wiki/Waveform">waveforms</a>
    enum Waveform
    #ifndef DOXYGEN_ONLY
        : std::uint8_t
    #endif // DOXYGEN_ONLY
    {
        Sine           , ///< <a href="http://en.wikipedia.org/wiki/Sine_wave">sine wave</a>
        Triangle       , ///< <a href="http://en.wikipedia.org/wiki/Triangle_wave">triangle waveform</a>
        Sawtooth       , ///< <a href="http://en.wikipedia.org/wiki/Sawtooth_wave">sawtooth waveform</a>
        ReverseSawtooth, ///< <a href="http://en.wikipedia.org/wiki/Sawtooth_wave">reverse sawtooth waveform</a>
        Square         , ///< <a href="http://en.wikipedia.org/wiki/Square_wave">square wave</a>
        Exponent       , ///< exponential rise and fall oscillations
        RandomHold     , ///< generate a random number and hold it until the next period
        RandomSlide    , ///< generate a random number every period and linearly slide to it from the previous the previous value
        Whacko         , ///< generate a new random number on every query (i.e. this "waveform" is independent of the set period)
        Dirac          , ///< a simple <a href="http://en.wikipedia.org/wiki/Dirac_delta_function">pulse</a>
        dIRAC          , ///< the ("graphical vertical") inverse of the Dirac waveform

        NumberOfWaveforms
    }; // enum Waveform
#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
    #pragma warning( pop )
#endif // _MSC_VER

public:
    LE_NOTHROW void LE_FASTCALL_ABI setEnabled   ( bool     value       ); ///< \brief enable/disable the LFO
    LE_NOTHROW void LE_FASTCALL_ABI setPhase     ( float    phase       ); ///< \brief set the (waveform's) phase offset (as a normalised percentage value [-0.5,+0.5])
	LE_NOTHROW void LE_FASTCALL_ABI setWaveform  ( Waveform             ); ///< \brief set the desired waveform
     /// \brief constrain the allowed range of values the LFO can set a parameter to
     /// \details a normalised [0, 1] value later automatically mapped to the range of the parameter the LFO is connected to
     /// \return true if the upper bound had to be adjusted in order to make sure that the upper-bound >= lower-bound condition always holds
    LE_NOTHROW bool LE_FASTCALL_ABI setLowerBound( float    lowerBound  );
    /// \brief constrain the allowed range of values the LFO can set a parameter to
    /// \details a normalised [0, 1] value later automatically mapped to the range of the parameter the LFO is mapped to
    /// \return true if the lower bound had to be adjusted in order to make sure that the upper-bound >= lower-bound condition always holds
    LE_NOTHROW bool LE_FASTCALL_ABI setUpperBound( float    upperBound  );

    LE_NOTHROW std::uint16_t LE_FASTCALL_ABI setPeriodInMilliseconds( std::uint16_t periodInMilliseconds ); ///< set the LFO period in milliseconds \return the actually applied value (autoadjusted/'clamped' to the values allowed by the currently configured sync types)
    LE_NOTHROW float         LE_FASTCALL_ABI setPeriodInSeconds     ( float         periodInSeconds      ); ///< set the LFO period in seconds \return the actually applied value (autoadjusted/'clamped' to the values allowed by the currently configured sync types)<BR> \overload

    LE_NOTHROWNOALIAS bool         LE_FASTCALL_ABI enabled   () const; ///< is LFO enabled?
    LE_NOTHROWNOALIAS float        LE_FASTCALL_ABI period    () const; ///< retrieve the current period
    LE_NOTHROWNOALIAS float        LE_FASTCALL_ABI phase     () const; ///< retrieve the "phase offset" parameter
    LE_NOTHROWNOALIAS float        LE_FASTCALL_ABI lowerBound() const; ///< retrieve the "lower bound" parameter
    LE_NOTHROWNOALIAS float        LE_FASTCALL_ABI upperBound() const; ///< retrieve the "upper bound" parameter
    LE_NOTHROWNOALIAS Waveform     LE_FASTCALL_ABI waveForm  () const; ///< retrieve the "Waveform" parameter
    LE_NOTHROWNOALIAS std::uint8_t LE_FASTCALL_ABI syncTypes () const; ///< retrieve the syncTypes

    LE_NOTHROW        void LE_FASTCALL_ABI addSyncType   ( SyncType );       ///< add/enable a specific SyncType
    LE_NOTHROW        void LE_FASTCALL_ABI removeSyncType( SyncType );       ///< remove/disable a specific SyncType
    LE_NOTHROWNOALIAS bool LE_FASTCALL_ABI hasEnabledSync( SyncType ) const; ///< is a specific SyncType enabled?

protected:
#if _MSC_VER < 1800
     LFO() {}
     LFO( LFO const & );
    ~LFO() {}
#else
     LFO()              = default;
     LFO( LFO const & ) = delete;
    ~LFO()              = default;
#endif // _MSC_VER
}; // class LFO

/// @} // group Parameters

//------------------------------------------------------------------------------
} // namespace Parameters
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // lfo_hpp
