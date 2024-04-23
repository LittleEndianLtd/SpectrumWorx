////////////////////////////////////////////////////////////////////////////////
///
/// channelData.cpp
/// ---------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "channelData.hpp"

#ifndef NDEBUG
    #include "le/math/math.hpp"
#else //...mrmlj...
    #ifndef LE_LOCALLY_DISABLE_FPU_EXCEPTIONS
        #define LE_LOCALLY_DISABLE_FPU_EXCEPTIONS()
    #endif // LE_LOCALLY_DISABLE_FPU_EXCEPTIONS
    #ifndef LE_MATH_VERIFY_VALUES
        #define LE_MATH_VERIFY_VALUES( fpClasses, range, valueName )
    #endif // LE_MATH_VERIFY_VALUES
#endif // NDEBUG
#include "le/math/conversion.hpp"
#include "le/math/dft/domainConversion.hpp"
#include "le/math/dft/fft.hpp"
#include "le/math/vector.hpp"
#include "le/utility/platformSpecifics.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

ChannelData::ChannelData()
    :
    amphDataFreshness_( 0 ),
    dftDataFreshness_ ( 0 )
{
}


void ChannelData::setNewTimeDomainData
(
    float                   const * const mainChannel,
    float                   const * const sideChannel,
    Math::FFT_float_real_1D const &       fft        ,
    ReadOnlyDataRange       const &       window     ,
    std::uint8_t                    const windowSizeFactor
)
{
    amphDataFreshness_ = 0;
    dftDataFreshness_  = 0;
    BOOST_ASSERT( fftSize() == fft.size() );

    dftAndTimeData_.setToDFTDomain();
    time2DFT
    (
        mainChannel,
        dftData().main(),
        window,
        fft,
        windowSizeFactor
    );
    dftDataFreshness_ = 1;

    if ( sideChannel )
    {
        time2DFT
        (
            sideChannel,
            dftData().mutableSide(),
            window,
            fft,
            windowSizeFactor
        );

        dft2AmPh
        (
            dftData ().mutableSide(),
            amphData().mutableSide()
        );
    }
#if 0 //...mrmlj...synth...
    else
    {
        BOOST_ASSERT_MSG
        (
            Math::max( amphData_.side().jointView() ) == 0,
            "Side channel data not cleared."
        );
    }
#endif // disabled

#ifndef NDEBUG
    auto const sideRealsSize( dftData().side().reals().size() );
    auto const sideImagsSize( dftData().side().imags().size() );
    auto const mainRealsSize( dftData().main().reals().size() );
    BOOST_ASSERT_MSG( sideRealsSize == sideImagsSize, "Buffer sizes mismatched." );
    BOOST_ASSERT_MSG( sideRealsSize == mainRealsSize, "Buffer sizes mismatched." );
#endif // NDEBUG
}


float const * ChannelData::getNewTimeDomainData( Math::FFT_float_real_1D const & fft, bool const fftShift )
{
    updateReImData();
#ifdef LE_PURE_REAL_FFT_TEST
    fft.inverseTransform( dftData().main().reals(), dftData().main().imags() );
    dftAndTimeData_.setToTimeDomain();
#else
    ReadOnlyDataRange const & imaginarySubRange( dftData().main().imags() );
    dftAndTimeData_.setToTimeDomain();
    fft.inverseTransform( dftAndTimeData_.timeDomainData(), imaginarySubRange, fftShift );
#endif // LE_PURE_REAL_FFT_TEST
    return dftAndTimeData_.timeDomainData();
}


void ChannelData::clearSideChannelData()
{
    dftAndTimeData_.setToDFTDomain();
    Math::clear( amphData().mutableSide().jointView() );
    Math::clear( dftData ().mutableSide().jointView() );
}


LE_NOTHROW
void ChannelData::time2DFT
(
    float                   const * const pInputData,
    FullChannelData_ReIm          &       dftData,
    ReadOnlyDataRange       const &       window,
    Math::FFT_float_real_1D const &       fft,
    std::uint8_t                          windowSizeFactor
)
{
#if !LE_SW_ENGINE_WINDOW_PRESUM
    LE_ASSUME( windowSizeFactor == 1 );
#endif // LE_SW_ENGINE_WINDOW_PRESUM

    auto const frameSize( fft.size() );

    BOOST_ASSERT_MSG( frameSize < dftData.size() * 2, "Buffer size incorrect." );

    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, ReadOnlyDataRange( pInputData, pInputData + fft.size() ), "time domain" );
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, window                                                  , "window"      );

#ifdef LE_PURE_REAL_FFT_TEST
    BOOST_ASSERT( windowSizeFactor == 1 );
    fft.transform( pInputData, window.begin(), dftData.reals().begin(), dftData.imags() );
#else
    float * const windowedTimeData( dftData.jointView().begin() );

    Math::multiply( pInputData, &window[ 0 ], windowedTimeData, frameSize );
    bool const needFFTShift( windowSizeFactor == 1 );
    auto position( frameSize );
    while ( --windowSizeFactor )
    {
        Math::addProduct( &pInputData[ position ], &window[ position - 1 ] + 1, windowedTimeData, frameSize );
        position += frameSize;
    }

    fft.transform( windowedTimeData, dftData.imags(), needFFTShift );
#endif // LE_PURE_REAL_FFT_TEST

    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, dftData.reals(), "FFT reals" );
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, dftData.imags(), "FFT imags" );
}


void ChannelData::dft2AmPh
(
    FullChannelData_ReIm const & reImData,
    FullChannelData_AmPh       & amPhData
)
{
    using namespace Math;

    LE_MATH_VERIFY_VALUES( InvalidOrSlow, reImData.reals(), "reals" );
    LE_MATH_VERIFY_VALUES( InvalidOrSlow, reImData.imags(), "imags" );

    reim2AmPh
    (
        reImData.reals ().begin(),
        reImData.imags ().begin(),
        amPhData.amps  ().begin(),
        amPhData.phases().begin(),
        amPhData.numberOfBins()
    );

    LE_MATH_VERIFY_VALUES( InvalidOrSlow | Negative, amPhData.amps  (), "amplitudes" );
    LE_MATH_VERIFY_VALUES( InvalidOrSlow           , amPhData.phases(), "phases"     );

    //...mrmlj...need not hold if input data is amplified/out of range...fix this...
    //BOOST_ASSERT_MSG
    //(
    //    max( amPhData.amps() ) <= FFT_float_real_1D::maximumAmplitude( convert<float>( ( amPhData.numberOfBins() - 1 ) * 2 ) ),
    //    "FFT reports wrong maximum amplitude."
    //);
}


void ChannelData::amph2DFT
(
    FullChannelData_AmPh const & amPhData,
    FullChannelData_ReIm       & reImData
)
{
    using namespace Math;

    LE_MATH_VERIFY_VALUES( InvalidOrSlow | Negative, amPhData.amps  (), "amplitudes" );
    LE_MATH_VERIFY_VALUES( InvalidOrSlow           , amPhData.phases(), "phases"     );

    amph2ReIm
    (
        amPhData.amps  ().begin(),
        amPhData.phases().begin(),
        reImData.reals ().begin(),
        reImData.imags ().begin(),
        amPhData.numberOfBins()
    );

    LE_MATH_VERIFY_VALUES( InvalidOrSlow, reImData.reals(), "reals" );
    LE_MATH_VERIFY_VALUES( InvalidOrSlow, reImData.imags(), "imags" );
}


LE_NOTHROW
void ChannelData::updateAmPhData()
{
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, dftData().main().reals(), "reals" );
    LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow, dftData().main().imags(), "imags" );

    if ( amphDataFreshness_ < dftDataFreshness_ )
    {
        dft2AmPh
        (
            dftData ().main(),
            amphData().main()
        );
        amphDataFreshness_ = dftDataFreshness_;
    }
    else
    {
        LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow | Math::Negative, amphData().main().amps  (), "amplitudes" );
        LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow                 , amphData().main().phases(), "phases"     );
    }
}

LE_NOTHROW
void ChannelData::updateReImData()
{
    if ( dftDataFreshness_ < amphDataFreshness_ )
    {
    #if defined( __ANDROID__ ) && defined( __i386__ ) //...mrmlj...TalkingWind...even though denormals are disabled!?
        LE_MATH_VERIFY_VALUES( Math::Invalid       | Math::Negative, amphData().main().amps  (), "amplitudes" );
    #else
		// Commented out due to customer complaint. (2016-05-18) (Danijel Domazet)
        // LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow | Math::Negative, amphData().main().amps  (), "amplitudes" );
    #endif // __ANDROID__
        LE_MATH_VERIFY_VALUES( Math::InvalidOrSlow                 , amphData().main().phases(), "phases"     );

        amph2DFT
        (
            amphData().main(),
            dftData ().main()
        );
        dftDataFreshness_ = amphDataFreshness_;
    }
}


void ChannelData::saveCurrentReImDataForBlending()
{
    Math::copy( currentReImData().jointView(), currentAmPhData().jointView() );
}


FullMainSideChannelData_AmPh & ChannelData::freshAmPhData( bool const saveForDryWetBlending )
{
    updateAmPhData();
    if ( saveForDryWetBlending )
    {
        updateReImData();
    }
    ++amphDataFreshness_;
    return amphData();
}


FullMainSideChannelData_ReIm & ChannelData::freshReImData( bool const saveForDryWetBlending )
{
    updateReImData();
    if ( saveForDryWetBlending )
    {
        saveCurrentReImDataForBlending();
    }
    ++dftDataFreshness_;
    return dftData();
}


ChannelData::AmPhReImData ChannelData::freshAmPh2ReImData( bool /*saveForDryWetBlending...see the amPh2ReIm blend quick fix in ModuleDSP::process*/ )
{
    updateAmPhData();
    dftDataFreshness_ = amphDataFreshness_ + 1;
    return AmPhReImData( amphData(), dftData() );
}


ChannelData::AmPhReImData ChannelData::freshReIm2AmPhData( bool /*saveForDryWetBlending*/ )
{
    updateReImData();
    amphDataFreshness_ = dftDataFreshness_ + 1;
    return AmPhReImData( amphData(), dftData() );
}


void ChannelData::blendWithPreviousData( float const currentDataWeight, bool const amPh2ReIm )
{
    if ( !amPh2ReIm && reImDataIsFresh() )
    {
        DataRange         const wetData( currentReImData().jointView() );
        ReadOnlyDataRange const dryData( currentAmPhData().jointView() );
        Math::mix( wetData.begin(), dryData.begin(), wetData.begin(), wetData.end(), currentDataWeight );
    }
    else
    {
        FullChannelData_AmPh const & amphData( currentAmPhData() );
        FullChannelData_ReIm       & reImData( currentReImData() );
        Math::mix( amphData.amps(), amphData.phases(), reImData.reals(), reImData.imags(), amPh2ReIm ? ( 1 - currentDataWeight ) : currentDataWeight );
        /// \note The mixed data is stored as ReIm data so dftDataFreshness_ has
        /// to be updated to reflect this.
        ///                                   (29.05.2012.) (Domagoj Saric)
        dftDataFreshness_ = amphDataFreshness_ + 1;
    }
    BOOST_ASSERT( dftDataFreshness_ > amphDataFreshness_ );
}


void ChannelData::amplifyCurrentData( float const gain )
{
    DataRange const data
    (
        reImDataIsFresh()
            ? currentReImData().jointView()
            : currentAmPhData().amps     ()
    );
    Math::multiply( data, gain );
}


std::uint32_t ChannelData::requiredStorage( StorageFactors const & factors )
{
    return
        FullMainSideChannelData_AmPh::requiredStorage( factors )
            +
        InPlaceDFTBuffer            ::requiredStorage( factors );
}


void ChannelData::resize( StorageFactors const & factors, Storage & storage )
{
    amphData_      .resize( factors, storage );
    dftAndTimeData_.resize( factors, storage );
#ifndef NDEBUG
    // Required for "no negative amps assertions".
    Math::clear( amphData_.main().amps() );
#endif // NDEBUG
}


float * ChannelData::InPlaceDFTBuffer::timeDomainData()
{
    BOOST_ASSERT_MSG( isTimeDomainDataValid(), "Incorrect usage or buffer in wrong state." );
    return this->main().jointView().begin();
}


FullMainSideChannelData_ReIm & ChannelData::InPlaceDFTBuffer::dftData()
{
    BOOST_ASSERT_MSG( isDFTDomainDataValid(), "Incorrect usage or buffer in wrong state." );
    return *this;
}

#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
bool ChannelData::InPlaceDFTBuffer::isDFTDomainDataValid() const
{
#ifdef NDEBUG
    LE_UNREACHABLE_CODE();
    return false;
#else
    return dataIsDFTDomain_;
#endif // NDEBUG
}
#pragma warning( pop )

void ChannelData::InPlaceDFTBuffer::setToDFTDomain()
{
#ifndef NDEBUG
    dataIsDFTDomain_ = true;
#endif // NDEBUG
}


void ChannelData::InPlaceDFTBuffer::setToTimeDomain()
{
#ifndef NDEBUG
    dataIsDFTDomain_ = false;
#endif // NDEBUG
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
