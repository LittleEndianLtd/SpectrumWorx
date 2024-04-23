////////////////////////////////////////////////////////////////////////////////
///
/// pimplForwarders.inl
/// -------------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
#ifdef pimplForwarders_inl__B71FD4A6_B1E2_4DE0_9E85_1B9F6D13F7BC
#error This file should be included only once per module
#else
#define pimplForwarders_inl__B71FD4A6_B1E2_4DE0_9E85_1B9F6D13F7BC
#endif // pimplForwarders_inl
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility { template <> struct Implementation<AudioIO::Device> { using type = AudioIO::DeviceImpl; }; }
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
// PImpl forwarders
////////////////////////////////////////////////////////////////////////////////

LE_NOTHROW LE_COLD Device:: Device() {}
LE_NOTHROW LE_COLD Device::~Device() {}

LE_NOTHROW LE_COLD char const * Device::setup( std::uint8_t const numberOfChannels, std::uint32_t const sampleRate, std::uint16_t const latencyInSamples )
{
#ifdef LE_SDK_DEMO_BUILD
    ljitikmer.setup( sampleRate, numberOfChannels );
#endif // LE_SDK_DEMO_BUILD
    return impl( *this ).setup( numberOfChannels, sampleRate, latencyInSamples );
}

LE_NOTHROW error_msg_t LE_COLD LE_FASTCALL_ABI Device::setCallback( Callback<Device::Input                 >::parameter_type callback ) { return impl( *this ).setCallback<Device::Input                 >( std::move( callback ) ); }
LE_NOTHROW error_msg_t LE_COLD LE_FASTCALL_ABI Device::setCallback( Callback<Device::Output                >::parameter_type callback ) { return impl( *this ).setCallback<Device::Output                >( std::move( callback ) ); }
LE_NOTHROW error_msg_t LE_COLD LE_FASTCALL_ABI Device::setCallback( Callback<Device::InputOutput           >::parameter_type callback ) { return impl( *this ).setCallback<Device::InputOutput           >( std::move( callback ) ); }
LE_NOTHROW error_msg_t LE_COLD LE_FASTCALL_ABI Device::setCallback( Callback<Device::InterleavedInput      >::parameter_type callback ) { return impl( *this ).setCallback<Device::InterleavedInput      >( std::move( callback ) ); }
LE_NOTHROW error_msg_t LE_COLD LE_FASTCALL_ABI Device::setCallback( Callback<Device::InterleavedOutput     >::parameter_type callback ) { return impl( *this ).setCallback<Device::InterleavedOutput     >( std::move( callback ) ); }
LE_NOTHROW error_msg_t LE_COLD LE_FASTCALL_ABI Device::setCallback( Callback<Device::InterleavedInputOutput>::parameter_type callback ) { return impl( *this ).setCallback<Device::InterleavedInputOutput>( std::move( callback ) ); }


LE_COLD void Device::start() { impl( *this ).start(); }
LE_COLD void Device::stop ()
{
#ifdef LE_SDK_DEMO_BUILD
    ljitikmer.reset();
#endif // LE_SDK_DEMO_BUILD
    impl( *this ).stop();
}

LE_NOTHROWNOALIAS LE_COLD std::uint8_t                 Device::numberOfChannels() const { return impl( *this ).numberOfChannels(); }
LE_NOTHROWNOALIAS LE_COLD std::uint32_t                Device::sampleRate      () const { return impl( *this ).sampleRate      (); }
LE_NOTHROWNOALIAS LE_COLD Device::LatencyAndBufferSize Device::latency         () const { return impl( *this ).latency         (); }

#ifdef _WIN32
LE_NOTHROWNOALIAS Device & LE_FASTCALL_ABI Device::singleton()
{
    /// \note "Static initialization order fiasco" (due to globals in
    /// RtAudio.cpp) workaround.
    ///                                       (04.02.2016.) (Domagoj Saric)
    static Device singletonInstance;
    return singletonInstance;
}
#else // implementation
LE_WEAK_SYMBOL Device Device::singleton_;
#endif // implementation

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
