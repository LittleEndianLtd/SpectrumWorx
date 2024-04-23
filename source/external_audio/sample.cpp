////////////////////////////////////////////////////////////////////////////////
///
/// sample.cpp
/// ----------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "sample.hpp"

#ifndef NDEBUG
    #include "le/math/vector.hpp"
#endif // NDEBUG
#include "le/utility/criticalSection.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------

char const * Sample::load( juce::File const & sampleFile, unsigned int const desiredSampleRate, Utility::CriticalSection & criticalSection )
{
    DataHolder newData;
    char const * const pErrorString( doLoad( sampleFile.getFullPathName(), desiredSampleRate, newData ) );
    if ( !pErrorString )
    {
        Utility::CriticalSectionLock const lock( criticalSection );
        this->sampleFile_ = sampleFile;
        this->data_.takeDataFrom( newData );
        // Implementation note:
        //   The sample position has to be cleared while we still hold the lock
        // otherwise race conditions (and thus crashes) occur.
        //                                    (18.06.2010.) (Domagoj Saric)
        /// \todo Think of a cleaner solution where the Sample class knows
        /// nothing about critical sections and threading issues.
        ///                                   (22.09.2010.) (Domagoj Saric)
        samplePosition_ = 0;

        // Assert something was actually read.
        BOOST_ASSERT( !channel1().empty() );

        // Assert all channels are of equal size.
        BOOST_ASSERT( channel1().size() == channel2().size() );

        /// \todo The Mac loader seems to return out-of-dynamic-range samples
        /// when loading MP3 files. Investigate.
        ///                                   (29.09.2010.) (Domagoj Saric)
        float const maximumAbsoluteValue
        (
            #ifdef __APPLE__
                1.15f
            #else
                1.00f
            #endif // __APPLE__
        );

        // Assert data was correctly read (all values are in the normalised range).
        BOOST_ASSERT( Math::max( channel1() ) <= +maximumAbsoluteValue );
        BOOST_ASSERT( Math::max( channel2() ) <= +maximumAbsoluteValue );
        BOOST_ASSERT( Math::min( channel1() ) >= -maximumAbsoluteValue );
        BOOST_ASSERT( Math::min( channel2() ) >= -maximumAbsoluteValue );

        boost::ignore_unused_variable_warning( maximumAbsoluteValue );
    }
    return pErrorString;
}


void Sample::clear()
{
    data_.pBuffer.reset();
    //samplePosition_ = 0;
    sampleFile_ = juce::File::nonexistent;
}


Sample::ChannelData Sample::channel( unsigned int const index ) const
{ //...mrmlj...think of a smarter solution (to provide true indexed channel access)...
    switch ( index )
    {
        case 0: return channel1();
        case 1: return channel2();
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


Sample::ChannelData Sample::channel1() const
{
    return ChannelData( data_.pBuffer.get(), data_.pChannel1End );
}


Sample::ChannelData Sample::channel2() const
{
    return ChannelData( data_.pChannel2Beginning, data_.pChannel2End );
}


unsigned int & Sample::samplePosition()
{
    BOOST_ASSERT( *this ); return samplePosition_;
}


void Sample::restart()
{
    samplePosition_ = 0;
}


bool Sample::DataHolder::recreate( std::size_t const newSizeInSamplesPerChannel )
{
    pBuffer.reset( new (std::nothrow) float[ newSizeInSamplesPerChannel * fixedNumberOfChannels ] );
    pChannel1End       = pBuffer.get();
    pChannel2Beginning = pChannel1End + newSizeInSamplesPerChannel;
    pChannel2End       = pChannel2Beginning;

    return pBuffer.get() != NULL;
}


void Sample::DataHolder::takeDataFrom( DataHolder & other )
{
    pBuffer.swap( other.pBuffer );
    pChannel1End       = other.pChannel1End      ;
    pChannel2Beginning = other.pChannel2Beginning;
    pChannel2End       = other.pChannel2End      ;

    // Catch (incorrect) subsequent usage of 'other' (now in undefined state).
    #ifndef NDEBUG
        other.pBuffer.reset();
        other.pChannel1End       = 0;
        other.pChannel2Beginning = 0;
        other.pChannel2End       = 0;
    #endif // NDEBUG
}


Sample::operator bool() const { return data_.pBuffer.get() != nullptr; }

//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
