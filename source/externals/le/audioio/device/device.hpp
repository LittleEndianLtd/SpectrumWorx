////////////////////////////////////////////////////////////////////////////////
///
/// \file device.hpp
/// ----------------
///
/// Hardware IO.
///
/// Copyright (c) 2013 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef device_hpp__1B86D709_0F0F_4D5C_A587_5ABFC3EAE42D
#define device_hpp__1B86D709_0F0F_4D5C_A587_5ABFC3EAE42D
#pragma once
//------------------------------------------------------------------------------
#include "le/utility/abi.hpp"
#include "le/utility/pimpl.hpp"

#if defined( __ANDROID__ )
    #include <functional>
#elif defined( _MSC_VER )
    #include <boost/function.hpp>
    #include <memory>
#endif // target

#include <cstdint>
#include <type_traits>
#include <utility>
//------------------------------------------------------------------------------
#if defined( _MSC_VER ) && !defined( LE_SDK_NO_AUTO_LINK )
    #ifdef _WIN64
        #pragma comment( lib, "LE_AudioIO_SDK_Win64_x86-64_SSE3.lib" )
    #else // _WIN32
        #pragma comment( lib, "LE_AudioIO_SDK_Win32_x86-32_SSE2.lib" )
    #endif // _WIN32/64
#endif // _MSC_VER && !LE_SDK_NO_AUTO_LINK
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
/// Root namespace for all things AudioIO
namespace AudioIO
{
//------------------------------------------------------------------------------

/// \addtogroup AudioIO
/// @{

typedef char const * error_msg_t;

////////////////////////////////////////////////////////////////////////////////
///
/// \class Device
///
/// \brief Models a hardware audio IO device (e.g. a soundcard, builtin audio
/// hardware of a mobile phone or tablet) capable of capturing audio from an
/// input source (e.g. a microphone) and/or playing audio data through an output
/// device (e.g. headphones).
///
/// \note Currently there is no way to choose the actual underlying device or
/// the low-level API used to access it. The default built-in device is
/// automatically used which should cover most mobile platform use cases.
///
/// \nosubgrouping
///
////////////////////////////////////////////////////////////////////////////////

class Device
#ifdef DOXYGEN_ONLY
#elif defined( __ANDROID__ ) && __LP64__
    : public Utility::StackPImpl<Device, 25 * sizeof( void * ) + 9 * sizeof( int ) + 2 * 36, 16>
#elif defined( __ANDROID__ )
    : public Utility::StackPImpl<Device, 23 * sizeof( void * ) + 9 * sizeof( int )         ,  8>
#elif defined( __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ )
    : public Utility::StackPImpl<Device,  2 * sizeof( void * )>
#elif defined( _WIN32 )
    : public Utility::StackPImpl<Device,  1 * sizeof( void * ) + 2 * sizeof( void (Device::*)() ) + 2 * sizeof( int ), sizeof( double )>
#else
    : public Utility::StackPImpl<Device, 10 * sizeof( void * ) + 2 * sizeof( int )>
#endif // DOXYGEN_ONLY
{
public:
    LE_NOTHROW  Device();
    LE_NOTHROW ~Device(); ///< Calls stop().

    /// Convenience global instance for apps that use only a single Device
    /// instance and would otherwise use a static variable.
    static LE_NOTHROWNOALIAS Device & LE_FASTCALL_ABI singleton();


    /// \name Streaming control
    /// @{

    /// <B>Effect:</B> Starts the streaming process on the device. The function
    /// is non-blocking: data notifications and processing are performed
    /// asynchronously using the callback given in the last call to
    /// setCallback().<BR>
    /// <B>Preconditions:</B>
    ///     - a successful call to the setCallback() member function
    ///     - the device must be in the stopped state.
    LE_NOTHROW void LE_FASTCALL_ABI start();
    /// \details
    /// <B>Effect:</B> Stops the streaming (if any) on the device.<BR>
    LE_NOTHROW void LE_FASTCALL_ABI stop ();

    /// @}

    /// \name Configuration
    /// @{

    /// <B>Effect:</B> Configures the device instance to consume and/or produce
    /// audio data in the given format.<BR>
    /// A Device instance may be (re)configured any number of times.<BR>
    /// <B>Preconditions:</B> The Device instance must be in the stopped state.
    /// \note
    /// - the actual data type used to transfer samples across the API is always
    ///   float
    /// - the <VAR>latencyInSamples</VAR> parameter is treated merely as a hint
    /// - zero can be passed for any of the three parameters to let the library
    ///   heuristically choose the optimum value for the current device and OS
    /// - Android specific: <a href="http://en.wikipedia.org/wiki/Latency_(audio)">
    ///   latency</a> is still very much a moving target and the platform offers
    ///   acceptable latencies only for playback and only on latest OS versions
    ///   (4.1+) and selected devices. Improved recording latency is only
    ///   available with Android 5 (and selected devices).
    ///   Disabling sensors and related background services during sensitive low
    ///   latency playback is known to sometimes help with 'crackling' (buffer
    ///   underruns due to unfavourable scheduling of the audio processing
    ///   thread). For use cases that do not require the minimum latency
    ///   possible, using a higher value may help with buffer underrun problems
    ///   and even with battery draining to some degree.
    LE_NOTHROW error_msg_t LE_FASTCALL_ABI setup( std::uint8_t numberOfChannels, std::uint32_t sampleRate, std::uint16_t latencyInSamples = 0 );

    /// <B>Preconditions:</B>
    /// - a successful setup() call
    LE_NOTHROWNOALIAS std::uint8_t  LE_FASTCALL_ABI numberOfChannels() const;
    /// <B>Preconditions:</B>
    /// - a successful setup() call
    LE_NOTHROWNOALIAS std::uint32_t LE_FASTCALL_ABI sampleRate      () const;

    /// \return A pair of latency related values:
    ///         - first = approximate total latency in samples
    ///         - second = processing buffer size in samples (the value of
    ///           <VAR>numberOfSampleFrames</VAR> in the callback call will
    ///           never be larger than this value, always less than or equal to
    ///           the first latency value).
    /// <B>Preconditions:</B> a successful setup() call (the 'first' value,
    ///           total latency, is only reliable after a successful call to
    ///           setCallback(), after which the IO configuration is known).
    typedef std::pair<std::uint16_t, std::uint16_t> LatencyAndBufferSize;
    LE_NOTHROWNOALIAS LatencyAndBufferSize LE_FASTCALL_ABI latency() const;

    /// @}

#ifdef DOXYGEN_ONLY
    /// \name Audio buffers
    /// \note <B>Advanced</B>: the predefined audio buffer types (Input, Output,
    /// InputOutput, InterleavedInput, InterleavedOutput,
    /// InterleavedInputOutput) are not actually separate structs. Rather they
    /// are typedefs for specialisations of the Device::Audio struct template
    /// which is parameterised by the data layout (ChannelLayout) and the
    /// "streaming direction" (InputOutputLayout). If this more flexible
    /// approach better suits your needs or coding style please see the header/
    /// class definition for more details.
    /// @{

    struct Input                  { /**\brief input data*/        float const * const * pInput      ; /**\brief number of sample frames*/ std::uint16_t numberOfSampleFrames; }; ///< Audio buffer for input/recording only operation (with non-interleaved/separated channel data).
    struct Output                 { /**\brief output data*/       float       * const * pOutput     ; /**\brief number of sample frames*/ std::uint16_t numberOfSampleFrames; }; ///< Audio buffer for output/playback only operation (with non-interleaved/separated channel data).
    struct InputOutput            { /**\brief input&output data*/ float       * const * pInputOutput; /**\brief number of sample frames*/ std::uint16_t numberOfSampleFrames; }; ///< Audio buffer for in-place full-duplex (input/recording + output/playback) operation (with non-interleaved/separated channel data).

    struct InterleavedInput       { /**\brief input data*/        float const *         pInput      ; /**\brief number of sample frames*/ std::uint16_t numberOfSampleFrames; }; ///< Audio buffer for input/recording only operation (with interleaved channel data).
    struct InterleavedOutput      { /**\brief output data*/       float       *         pOutput     ; /**\brief number of sample frames*/ std::uint16_t numberOfSampleFrames; }; ///< Audio buffer for output/playback only operation (with interleaved channel data).
    struct InterleavedInputOutput { /**\brief input&output data*/ float       *         pInputOutput; /**\brief number of sample frames*/ std::uint16_t numberOfSampleFrames; }; ///< Audio buffer for in-place full-duplex (input/recording + output/playback) operation (with interleaved channel data).

    /// @}

    /// \name Callbacks
    /// @{

    /// <B>Effect:</B> Registers <VAR>callback</VAR> to be called periodically
    /// when the device is started in order to produce and/or consume data.<BR>
    /// The type (signature) of the passed callback function/functor determines
    /// both the desired layout of audio data (interleaved or
    /// non-interleaved/separated channels) as well as the streaming direction
    /// (input, output or input and output aka full duplex).<BR>
    /// A different callback can be set any number of times.<BR>
    /// AudioIO::Device is very flexible about the ways in which it allows users
    /// to register a callback/rendering procedure:
    /// - C-style function pointers with a context instance
    /// - generic functors or C++ lambdas
    /// - blocks (on platforms which support them, i.e. OSX and iOS).
    /// .
    /// The signature for all possible callback types always follows this simple
    /// pattern: void ( <VAR>Audio</VAR> data ), with the exception of C-style
    /// callbacks that of course have the additional context parameter:
    /// void ( <VAR>Context</VAR> * context, Audio data ) where
    /// <VAR>Context</VAR> can be of any type.
    /// <VAR>Audio</VAR> on the other hand is the type that makes the
    /// distinction between individual possible callbacks in terms of data
    /// layout and the "streaming direction", it can be any one of the six
    /// documented/supported audio buffer types (Input, Output, InputOutput,
    /// InterleavedInput, InterleavedOutput, InterleavedInputOutput).
    ///
    /// \param callback  A function object which satisfies the above described
    ///                  criteria.
    ///
    /// <B>Preconditions:</B>
    /// - a successful setup() call
    /// - the Device instance must be in the stopped state.<BR>
    ///
    /// \remark Check the LE SoundEffects SDK example app for usage examples.
    ///
    /// \note <B>Advanced</B>: the predefined audio buffer types (Input, Output,
    /// InputOutput, InterleavedInput, InterleavedOutput,
    /// InterleavedInputOutput) are not actually separate structs. Rather they
    /// are typedefs for specialisations of the Device::Audio struct template
    /// which is parameterised by the data layout (ChannelLayout) and the
    /// "streaming direction" (InputOutputLayout). If this more flexible
    /// approach better suits your needs or coding style please see the header/
    /// class definition for more details.
    template <class Callback>
    error_msg_t setCallback( Callback & callback );

    /// Overload for C-style callbacks
    /// \param pCallback  A pointer to a function with a signature that matches
    ///                   void ( <VAR>Context</VAR> * context, <VAR>Audio</VAR>
    ///                   data ) where <VAR>Context</VAR> is any user type and
    ///                   <VAR>Audio</VAR> is one of one of the six
    ///                   documented/supported audio buffer types (Input,
    ///                   Output, InputOutput, InterleavedInput,
    ///                   InterleavedOutput, InterleavedInputOutput).
    /// \param context    A pointer to a user data structure that holds the
    ///                   callback state (can be null).
    ///
    /// <B>Preconditions:</B>
    /// - a successful setup() call
    /// - the Device instance must be in the stopped state.<BR>
    ///
    /// \remark Check the LE SoundEffects SDK example app for usage examples.<BR>
    template <typename Context, typename Callback>
    error_msg_t setCallback( Context * context, Callback * pCallback ); ///< \overload

    /// @}

#endif // DOXYGEN_ONLY

#ifndef DOXYGEN_ONLY
private: /// \internal
    template <typename F, typename Signature, typename DataLayout>
    struct EnableOverloadForImpl
        :
        std::enable_if
        <
            std::is_same<Signature, void (F::*)( DataLayout ) const>::value ||
            std::is_same<Signature, void (F::*)( DataLayout )      >::value,
            error_msg_t
        >
    {};

    template <typename F, typename DataLayout>
    struct EnableOverloadFor : EnableOverloadForImpl<F, decltype( &F::operator() ), DataLayout> {};

    template <typename F, typename DataLayout>
    struct EnableOverloadFor<F &, DataLayout> : EnableOverloadFor<F, DataLayout> {};

    template <typename F, typename DataLayout>
    struct EnableOverloadFor<std::reference_wrapper<F>, DataLayout> : EnableOverloadFor<F, DataLayout>{};

    // enable overloads for plain function pointers with the correct signature
    template <typename DataLayout                       > struct EnableOverloadFor<void (*)( DataLayout  ), DataLayout > : std::enable_if<true , error_msg_t>{};
    template <typename DataLayout1, typename DataLayout2> struct EnableOverloadFor<void (*)( DataLayout1 ), DataLayout2> : std::enable_if<false, error_msg_t>{};

public:
    template <class DataLayout>
    struct Callback
    {
    #if defined( __APPLE__ )
        typedef void (^type)(DataLayout);
        typedef type parameter_type;
    #elif defined( _MSC_VER )
        typedef boost::function<void (DataLayout)> type;
        typedef type const & parameter_type;
    #else
        typedef std::function<void( DataLayout )> type;
        typedef type && parameter_type;
    #endif // __APPLE__
    }; // struct Callback

    enum ChannelLayout
    {
        InterleavedChannels,
        SeparatedChannels
    };

    enum InputOutputLayout
    {
        InputOnly,
        OutputOnly,
        Inplace,
        SeparatedInputOutput
    };

    template <ChannelLayout, typename T> struct Signal                         { typedef T * LE_RESTRICT const * LE_RESTRICT type; };
    template <               typename T> struct Signal<InterleavedChannels, T> { typedef T * LE_RESTRICT                     type; };

    typedef Signal<SeparatedChannels  , float const>::type InputSignal            ;
    typedef Signal<SeparatedChannels  , float      >::type OutputSignal           ;
    typedef Signal<InterleavedChannels, float const>::type InterleavedInputSignal ;
    typedef Signal<InterleavedChannels, float      >::type InterleavedOutputSignal;

    /// \brief Audio signal/data layout abstraction
    template <ChannelLayout channelLayout, InputOutputLayout>
    struct Audio
    {
        typename Signal<channelLayout, float const>::type pInput ; ///< member present only for input  or input&output callbacks
        typename Signal<channelLayout, float      >::type pOutput; ///< member present only for output or input&output callbacks
    };

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4510 ) // Default constructor could not be generated.
    #pragma warning( disable : 4610 ) // Class can never be instantiated - user-defined constructor required.
#endif // _MSC_VER

    template <ChannelLayout channelLayout>
    struct Audio<channelLayout, SeparatedInputOutput>
    {
        typename Signal<channelLayout, float const>::type const pInput ;
        typename Signal<channelLayout, float      >::type const pOutput;

        std::uint16_t const numberOfSampleFrames;
    };

    template <ChannelLayout channelLayout>
    struct Audio<channelLayout, Inplace>
    {
        typename Signal<channelLayout, float      >::type const pInputOutput;

        std::uint16_t const numberOfSampleFrames;
    };

    template <ChannelLayout channelLayout>
    struct Audio<channelLayout, InputOnly>
    {
        typename Signal<channelLayout, float const>::type const pInput ;

        std::uint16_t const numberOfSampleFrames;
    };

    template <ChannelLayout channelLayout>
    struct Audio<channelLayout, OutputOnly>
    {
        typename Signal<channelLayout, float      >::type const pOutput;

        std::uint16_t const numberOfSampleFrames;
    };
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

    typedef Audio<SeparatedChannels  , InputOnly > Input                 ;
    typedef Audio<SeparatedChannels  , OutputOnly> Output                ;
    typedef Audio<SeparatedChannels  , Inplace   > InputOutput           ;

    typedef Audio<InterleavedChannels, InputOnly > InterleavedInput      ;
    typedef Audio<InterleavedChannels, OutputOnly> InterleavedOutput     ;
    typedef Audio<InterleavedChannels, Inplace   > InterleavedInputOutput;

#ifndef __OBJC__
#ifdef __APPLE__
private:
    template <typename DataType, typename Functor>
    error_msg_t setCallbackMoveable( Functor && callback, std::true_type ) { return setCallback( ^( DataType const data ) { callback( data ); } ); }
    template <typename DataType, typename Functor>
    error_msg_t setCallbackMoveable( Functor && functor, std::false_type )
    {
        /// \note "ObjC++ attempts to copy lambdas, preventing capture of
        /// move-only types". https://llvm.org/bugs/show_bug.cgi?id=20534
        ///                                   (14.01.2016.) (Domagoj Saric)
        __block Functor callback( std::move( functor ) );
        return setCallback( ^( DataType const data ) { callback( data ); } );
    }
public:
    template <typename DataLayout1, typename DataLayout2>
    struct EnableOverloadFor<void( ^ )( DataLayout1 ), DataLayout2> : std::enable_if<false, error_msg_t>{};
#else
private:
    template <typename DataType, typename Functor>
    error_msg_t setCallbackMoveable( Functor && functor, std::true_type ) { return setCallback( typename Callback<DataType>::type( std::forward<Functor>( functor ) ) ); }
    template <typename DataType, typename Functor>
    error_msg_t setCallbackMoveable( Functor && functor, std::false_type )
    {
        auto pFunctor( std::make_shared<Functor>( std::move( functor ) ) ); // no support for move-only functors in either boost or std function (... https://github.com/psiha/function)
        return setCallbackMoveable<DataType>( [=]( DataType const data ){ (*pFunctor)( data ); }, std::true_type() );
    }
public:
#endif // __APPLE__
    #if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
        #define is_nothrow_copy_constructible has_nothrow_copy_constructor
    #endif // old MSVC

    #define LE_AUX_MAKECALLBACK( dataType, callback )   \
        setCallbackMoveable<dataType>( std::forward<Functor>( callback ), std::is_nothrow_copy_constructible<typename std::remove_reference<Functor>::type>() )

    #define LE_AUX_SETCALLBACK_OVERLOAD( dataType )                                                                       \
        template <typename Functor>                                                                                       \
        typename EnableOverloadFor<Functor, dataType>::type LE_NOTHROW LE_FASTCALL_ABI setCallback( Functor && callback ) \
        {                                                                                                                 \
            return LE_AUX_MAKECALLBACK( dataType, callback );                                                             \
        }

    LE_AUX_SETCALLBACK_OVERLOAD( Input                  )
    LE_AUX_SETCALLBACK_OVERLOAD( Output                 )
    LE_AUX_SETCALLBACK_OVERLOAD( InputOutput            )
    LE_AUX_SETCALLBACK_OVERLOAD( InterleavedInput       )
    LE_AUX_SETCALLBACK_OVERLOAD( InterleavedOutput      )
    LE_AUX_SETCALLBACK_OVERLOAD( InterleavedInputOutput )

    #undef LE_AUX_MAKECALLBACK
    #undef LE_AUX_SETCALLBACK_OVERLOAD
#endif // __OBJC__

    error_msg_t LE_FASTCALL_ABI setCallback( Callback<Input                 >::parameter_type callback );
    error_msg_t LE_FASTCALL_ABI setCallback( Callback<Output                >::parameter_type callback );
    error_msg_t LE_FASTCALL_ABI setCallback( Callback<InputOutput           >::parameter_type callback );
    error_msg_t LE_FASTCALL_ABI setCallback( Callback<InterleavedInput      >::parameter_type callback );
    error_msg_t LE_FASTCALL_ABI setCallback( Callback<InterleavedOutput     >::parameter_type callback );
    error_msg_t LE_FASTCALL_ABI setCallback( Callback<InterleavedInputOutput>::parameter_type callback );

    // overloads for C-style (context + function) callbacks
    #ifdef __APPLE__
        #define LE_AUX_CALLBACKPREFIX() ^
    #else
        #define LE_AUX_CALLBACKPREFIX() [=]
    #endif // __APPLE__
    template <typename Context> LE_NOTHROW error_msg_t setCallback( Context * context, void( *callback )( Context *, Input                  ) ) { return setCallback( LE_AUX_CALLBACKPREFIX()( Input                  const data ){ callback( context, data ); } ); }
    template <typename Context> LE_NOTHROW error_msg_t setCallback( Context * context, void( *callback )( Context *, Output                 ) ) { return setCallback( LE_AUX_CALLBACKPREFIX()( Output                 const data ){ callback( context, data ); } ); }
    template <typename Context> LE_NOTHROW error_msg_t setCallback( Context * context, void( *callback )( Context *, InputOutput            ) ) { return setCallback( LE_AUX_CALLBACKPREFIX()( InputOutput            const data ){ callback( context, data ); } ); }
    template <typename Context> LE_NOTHROW error_msg_t setCallback( Context * context, void( *callback )( Context *, InterleavedInput       ) ) { return setCallback( LE_AUX_CALLBACKPREFIX()( InterleavedInput       const data ){ callback( context, data ); } ); }
    template <typename Context> LE_NOTHROW error_msg_t setCallback( Context * context, void( *callback )( Context *, InterleavedOutput      ) ) { return setCallback( LE_AUX_CALLBACKPREFIX()( InterleavedOutput      const data ){ callback( context, data ); } ); }
    template <typename Context> LE_NOTHROW error_msg_t setCallback( Context * context, void( *callback )( Context *, InterleavedInputOutput ) ) { return setCallback( LE_AUX_CALLBACKPREFIX()( InterleavedInputOutput const data ){ callback( context, data ); } ); }
    #undef LE_AUX_CALLBACKPREFIX
#endif // DOXYGEN_ONLY

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
    Device( Device const  & );
    Device( Device       && );

    #undef is_nothrow_copy_constructible
#else
    Device( Device const  & ) = delete;
    Device( Device       && ) = delete;
#endif // _MSC_VER

#ifndef _WIN32
private:
    static Device singleton_;
#endif // _DEBUG
}; // class Device

#ifndef _WIN32
inline Device & Device::singleton() { return singleton_; }
#endif // _WIN32


////////////////////////////////////////////////////////////////////////////////
///
/// \class BlockingDevice
///
/// \brief Provides synchronized/blocking access to a Device instance. Useful
/// mostly for single-threaded command line applications.
///
////////////////////////////////////////////////////////////////////////////////

class BlockingDevice
#ifdef DOXYGEN_ONLY
#elif defined( __ANDROID__ ) && __LP64__
    : public Utility::StackPImpl<BlockingDevice, 13 * sizeof( void * )>
#elif defined( __ANDROID__ ) || defined( _WIN32 )
    : public Utility::StackPImpl<BlockingDevice,  4 * sizeof( void * )>
#else
    : public Utility::StackPImpl<BlockingDevice, 14 * sizeof( void * ) + 10 * sizeof( int )>
#endif // DOXYGEN_ONLY
{
public:
    /// \details
    /// <B>Effect:</B> Creates the blocking wrapper for the given Device object which must outlive the created BlockingDevice object.
    LE_NOTHROW  BlockingDevice();
    LE_NOTHROW ~BlockingDevice();

    /// <B>Effect:</B> Starts the associated Device instance and blocks until the stop() member function is called (from another thread).<BR>
    /// <B>Preconditions:</B> The same as for Device::start().<BR>
    LE_NOTHROW void LE_FASTCALL_ABI startAndWait( Device & );
    /// <B>Effect:</B> Stops the associated Device and unblocks the thread blocked by the call to startAndWait().<BR>
    /// <B>Preconditions:</B> A prior call to startAndWait().<BR>
    LE_NOTHROW void LE_FASTCALL_ABI stop();

    LE_NOTHROW Device * LE_FASTCALL_ABI device();

#if defined( _MSC_VER ) && ( _MSC_VER < 1800 )
private:
    BlockingDevice( BlockingDevice const  & );
    BlockingDevice( BlockingDevice       && );
#else
    BlockingDevice( BlockingDevice const  & ) = delete;
    BlockingDevice( BlockingDevice       && ) = delete;
#endif // _MSC_VER
}; // class BlockingDevice

/// @} // group AudioIO

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // device_hpp
