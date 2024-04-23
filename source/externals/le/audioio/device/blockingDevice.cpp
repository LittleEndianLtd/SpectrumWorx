////////////////////////////////////////////////////////////////////////////////
///
/// blockingDevice.cpp
/// ------------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "device.hpp"

#if defined( _WIN32 ) && ( _WIN32_WINNT < _WIN32_WINNT_VISTA )
    //...mrmlj...ConditionVariable requires Windows Vista or later
    #undef _WIN32_WINNT
    #undef WINVER
    #undef NTDDI_VERSION
    #define _WIN32_WINNT  _WIN32_WINNT_VISTA
    #define WINVER        _WIN32_WINNT_VISTA
    #define NTDDI_VERSION NTDDI_VISTASP2
#endif // Win32 version

#include "le/utility/clear.hpp"
#include "le/utility/conditionVariable.hpp"
#include "le/utility/pimplPrivate.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

#pragma warning( push )
#pragma warning( disable : 4512 ) // Assignment operator could not be generated.

class BlockingDeviceImpl
{
public:
    BlockingDeviceImpl() : pDevice_( nullptr ) {}
#ifndef NDEBUG
    ~BlockingDeviceImpl() { BOOST_ASSERT_MSG( !pDevice_, "Destroying BlockingDevice while still waiting" ); }
#endif // NDEBUG

    void notify()
    {
        waitable_.notify_enter();
        /// \note RtAudio and ASIO-under-Windows/COM seem to have stricter
        /// threading requirements which forbid calling API functions from
        /// different threads. For this reason we defer the call to
        /// Device::stop() to the wait() member function which should presumably
        /// be called from the 'main' thread.
        /// ...mrmlj...this can cause this function to be called a few more
        /// times before the 'main' thread is given a time slice to stop the
        /// Device...
        /// http://mtippach.proboards.com/thread/1400/application-9beta-hangs-asiostop-call?page=1&scrollTo=3907
        /// http://www.un4seen.com/forum/?topic=8805.msg60569#msg60569
        /// http://music.columbia.edu/pipermail/portaudio/2003-October/002370.html
        ///                                   (17.01.2014.) (Domagoj Saric)
    #if !defined( _WIN32 )
        BOOST_ASSERT_MSG( pDevice_, "No one listening." );
        pDevice_->stop();
        pDevice_ = nullptr;
    #endif // !Windows
        waitable_.notify();
        waitable_.notify_exit();
    }

    void startAndWait( Device & device )
    {
        BOOST_ASSERT_MSG( !pDevice_, "Already waiting." );
        pDevice_ = &device;
        waitable_.wait_enter();
        device.start();
        waitable_.wait();
    #if defined( _WIN32 )
        device.stop();
        pDevice_ = nullptr;
    #else
        BOOST_ASSERT( !pDevice_ );
    #endif // !Windows
        waitable_.wait_exit();
    }

    Device * device() { return pDevice_; }

private:
    Utility::Waitable waitable_;

    Device * LE_RESTRICT pDevice_;
}; // class BlockingDeviceImpl

#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////
// PImpl forwarders
////////////////////////////////////////////////////////////////////////////////

namespace Utility
{
//------------------------------------------------------------------------------

template <>
struct Implementation<AudioIO::BlockingDevice> { using type = AudioIO::BlockingDeviceImpl; };

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

LE_NOTHROW LE_COLD BlockingDevice:: BlockingDevice() {}
LE_NOTHROW LE_COLD BlockingDevice::~BlockingDevice() {}

void LE_FASTCALL_ABI BlockingDevice::startAndWait( Device & device ) { return impl( *this ).startAndWait( device ); }
void LE_FASTCALL_ABI BlockingDevice::stop        (                 ) { return impl( *this ).notify      (        ); }

Device * LE_FASTCALL_ABI BlockingDevice::device() { return impl( *this ).device(); }

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
