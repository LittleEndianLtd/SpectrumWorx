////////////////////////////////////////////////////////////////////////////////
///
/// \file profiler.hpp
/// ------------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef profiler_hpp__DCFEE47B_8584_452B_9041_224807F9A268
#define profiler_hpp__DCFEE47B_8584_452B_9041_224807F9A268
#pragma once
//------------------------------------------------------------------------------
#include "abi.hpp"

#if !( defined( _MSC_VER ) && ( _MSC_VER < 1700 ) )
#include <chrono>
#endif // old MSVC
#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

class DSPProfiler
{
public:
    LE_NOTHROWNOALIAS DSPProfiler();

    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI setSignalSampleRate( std::uint32_t sampleRate );
    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI reset();

    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI beginInterval();

    LE_NOTHROWNOALIAS void LE_FASTCALL_ABI endInterval( std::uint32_t intervalLengthInSampleFrames );

    LE_NOTHROWNOALIAS float LE_FASTCALL_ABI cpuUsagePercentage() const;

    static DSPProfiler & singleton() { return singleton_; }

private:
    std::uint32_t totalSamples_;
    float         sampleRate_  ;

#if defined( _MSC_VER ) && ( _MSC_VER < 1700 )
    std::uint64_t totalCPUTime_;
    std::uint64_t lastTime_    ;
#else
    std::chrono::steady_clock::duration   totalCPUTime_;
    std::chrono::steady_clock::time_point lastTime_    ;
#endif // MSVC version

    static DSPProfiler singleton_;
}; // class DSPProfiler

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // profiler_hpp
