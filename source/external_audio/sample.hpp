////////////////////////////////////////////////////////////////////////////////
///
/// \file sample.hpp
/// ----------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef sample_hpp__A94590F9_6645_4380_8512_060CF57872FA
#define sample_hpp__A94590F9_6645_4380_8512_060CF57872FA
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/platformSpecifics.hpp"

#include <juce/juce_core/juce_core.h>

#include <boost/range/iterator_range_core.hpp>
#include <boost/smart_ptr/scoped_array.hpp>
//------------------------------------------------------------------------------

#ifdef _WIN32
namespace boost { namespace signals2 { class mutex; } }
#endif // _WIN32

namespace LE
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class Sample
///
/// \brief Loads and streams a sample from memory, looped, always stereo.
///
////////////////////////////////////////////////////////////////////////////////

namespace Utility
{
#ifdef _WIN32
    using CriticalSection = boost::signals2::mutex;
#else
    class CriticalSection;
#endif // _WIN32
} // namespace Utility

class Sample
{
public:
    using ChannelData = boost::iterator_range<float const * LE_RESTRICT>;

public:
    char const * load( juce::File const & sampleFile, unsigned int desiredSampleRate, Utility::CriticalSection & );

    void clear();

    ChannelData channel( unsigned int index ) const;

    ChannelData channel1() const;
    ChannelData channel2() const;

    unsigned int & samplePosition();
    void restart();

    juce::File   const & sampleFile    () const { return sampleFile_; }
    juce::String const & sampleFileName() const { return sampleFile().getFullPathName(); }

    static juce::String supportedFormats();

    explicit operator bool() const;

private:
    struct DataHolder
    {
        bool recreate( std::size_t newSizeInSamplesPerChannel );

        void takeDataFrom( DataHolder & other );

        boost::scoped_array<float> pBuffer;
        float * pChannel1End      ;
        float * pChannel2Beginning;
        float * pChannel2End      ;
    };

    class Impl;

private:
    // To be implemented for each platform separately.
    LE_NOTHROWNOALIAS
    static char const * doLoad( juce::String const & sampleFileName, unsigned int desiredSampleRate, DataHolder & data );

private:
    DataHolder data_;

    unsigned int samplePosition_;

    juce::File sampleFile_;

    static unsigned int const fixedNumberOfChannels = 2;
}; // class Sample

//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // sample_hpp
