////////////////////////////////////////////////////////////////////////////////
///
/// \file phaseVocoderSynthesis.hpp
/// -------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef phaseVocoderSynthesis_hpp__CEE2AE79_CDE4_4206_83A0_3FFAA1942FBE
#define phaseVocoderSynthesis_hpp__CEE2AE79_CDE4_4206_83A0_3FFAA1942FBE
#if defined( _MSC_VER ) && !defined( DOXYGEN_ONLY )
#pragma once
#endif // MSVC && !Doxygen
//------------------------------------------------------------------------------
#include "boost/config/abi_prefix.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class PhaseVocoderSynthesis
///
/// \ingroup Effects
///
/// \brief Transforms back from the phase-vocoder domain to Fourier transform domain.
///
/// SpectrumWorx draws upon Fourier Transform algorithms to work in the frequency 
/// domain. Fourier transform is used to transform time samples to the frequency 
/// domain, so we end up with various amplitudes and phases of the resulting sine 
/// waves. The sine waves produced by Fourier transform are only approximations; 
/// they don' t represent "true" frequencies present in the input signal. Phase 
/// vocoding, on the other hand, can be used to find the true frequencies. After 
/// the true frequencies are found, we can manipulate amplitudes and frequencies 
/// rather than simply amplitudes and phases. This opens up new territory for 
/// effects processing. For example, accurate pitch shifting is possible since 
/// we know the true frequencies. 
/// 
////////////////////////////////////////////////////////////////////////////////

struct PhaseVocoderSynthesis
{
    static bool const usesSideChannel = false;

    static char const title      [];
    static char const description[];
};

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#include "boost/config/abi_suffix.hpp"

#endif // phaseVocoderSynthesis_hpp
