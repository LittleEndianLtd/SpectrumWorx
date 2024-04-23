////////////////////////////////////////////////////////////////////////////////
///
/// \file deviceImpl.hpp
/// --------------------
///
/// Copyright (c) 2014 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef deviceImpl_hpp__8669AAEA_1F19_4768_A1F2_40DF2D98E69F
#define deviceImpl_hpp__8669AAEA_1F19_4768_A1F2_40DF2D98E69F
#pragma once
//------------------------------------------------------------------------------
#ifdef _MSC_VER
    #pragma warning( disable : 4502 ) // Decorated name length exceeded, name was truncated.
#endif // _MSC_VER
#include "device.hpp"

#include "le/utility/platformSpecifics.hpp"

#ifdef LE_SDK_DEMO_BUILD
    #include "le/utility/demoLimiter.hpp"
#endif // LE_SDK_DEMO_BUILD

#ifdef __APPLE__
#include <Block.h>
#endif // __APPLE__
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace AudioIO
{
//------------------------------------------------------------------------------

using ChannelLayout     = Device::ChannelLayout;
using InputOutputLayout = Device::InputOutputLayout;
template <class DataLayout>
using Callback          = Device::Callback<DataLayout>;

#ifdef __APPLE__
//...mrmlj...use blocks on apple platforms...
// http://clang.llvm.org/docs/LanguageExtensions.html#blocks
// http://clang.llvm.org/docs/Block-ABI-Apple.html
//__has_extension( blocks ) __OBJC__ implicit

template <typename Signature>
struct SmartBlock;
template <typename ReturnType, typename ... Parameters>
struct SmartBlock<ReturnType( Parameters... )>
{
    typedef ReturnType ( Signature)( Parameters ... );
    typedef ReturnType (^Block    )( Parameters ... );

    SmartBlock() noexcept : callable( nullptr ) {}
    SmartBlock( Block      const   block ) noexcept : callable( Block_copy( block          ) ) {}
    SmartBlock( SmartBlock const & other ) noexcept : callable( Block_copy( other.callable ) ) {}
    SmartBlock( SmartBlock      && other ) noexcept : callable(             other.callable   ) { other.callable = nullptr; }
    //template <class Functor> SmartBlock( Functor && functor ) { *this = static_cast<Functor &&>( functor ); }
    ~SmartBlock() noexcept { Block_release( callable ); }

    ReturnType operator()( Parameters && ... arguments ) const noexcept { return callable( static_cast<Parameters &&>( arguments )... ); }

    SmartBlock & operator=( Block const otherBlock ) noexcept
    {
        Block_release( callable );
        this->callable = Block_copy( otherBlock );
        return *this;
    }

    template <class Functor>
    SmartBlock & operator=( Functor && functor ) noexcept
    {
        Block_release( callable );
    #if defined( __OBJC__ )
        callable = static_cast<Functor &&>( functor );
    #else
        Functor const localFunctor ( static_cast<Functor &&>( functor ) );
        auto    const localSmartBlock( ^( Parameters ... arguments ) { localFunctor( arguments ... ); } );
        callable = Block_copy( localSmartBlock );
    #endif // ObjC
        return *this;
    }

    SmartBlock & operator=( SmartBlock const & other ) noexcept
    {
        Block_release( callable );
        this->callable = Block_copy( other.callable );
        return *this;
    }

    SmartBlock & operator=( SmartBlock && other ) noexcept
    {
        Block_release( callable );
        this->callable = std::move( other.callable );
        other.callable = nullptr;
        return *this;
    }

    operator Block const &() const { return callable; }

    explicit operator bool() const { return callable; }

    Block callable;
}; // struct SmartBlock

template <class DataLayout>
struct CallbackImpl { using type = SmartBlock<void(DataLayout)>; };
#else
    template <class DataLayout>
    using CallbackImpl = Callback<DataLayout>;
#endif // __APPLE__

/// \note To avoid the compile-time and runtime overhead of Boost.Variant we
/// (ab)use the fact that CallbackImpl types on all platforms (i.e. boost/std::
/// functions and Apple Blocks) are already type-erased function objects which
/// have the same layout and use the same type-agnostic functions for destroying
/// existing callbacks ('vtables' in boost/std::functions and _Block_release()
/// for blocks) regardless of signature.
///                                           (04.02.2016.) (Domagoj Saric)
using PublicCallback = CallbackImpl<std::nullptr_t>::type;
#if 0
    boost::variant
    <
        CallbackImpl<Device::Input                 >::type,
        CallbackImpl<Device::Output                >::type,
        CallbackImpl<Device::InputOutput           >::type,
        CallbackImpl<Device::InterleavedInput      >::type,
        CallbackImpl<Device::InterleavedOutput     >::type,
        CallbackImpl<Device::InterleavedInputOutput>::type
    >;
#endif

template <class DataLayout>
void storeCallback
(
             PublicCallback                       & currentCallback,
    typename Callback<DataLayout>::parameter_type   newCallback
)
{
    using TargetFunction = typename CallbackImpl<DataLayout>::type;
    static_assert( sizeof( TargetFunction ) == sizeof( currentCallback ), "Internal inconsistency" );
    reinterpret_cast<typename CallbackImpl<DataLayout>::type &>( currentCallback ) = std::move( newCallback );
}

template <class DataLayout>
typename Callback<DataLayout>::type const &
getCallback( PublicCallback const & callback )
{
    //auto const pFunction( boost::get<typename CallbackImpl<DataLayout>::type>( &callback ) );
    using TargetFunction = typename CallbackImpl<DataLayout>::type;
    static_assert( sizeof( TargetFunction ) == sizeof( PublicCallback ), "Internal inconsistency" );
    auto const & function( reinterpret_cast<TargetFunction const &>( callback ) );
    return function;
}

template <class DataLayout>
void invoke( PublicCallback const & callback, DataLayout const dataLayout )
{
    auto const & actualCallback( getCallback<DataLayout>( callback ) );
    BOOST_ASSERT( actualCallback );
    actualCallback( dataLayout );
}

template <typename Buffer> struct CallbackInfo;

template <ChannelLayout channelLayoutParam, InputOutputLayout ioLayoutParam>
struct CallbackInfo<Device::Audio<channelLayoutParam, ioLayoutParam>> { static ChannelLayout const channelLayout = channelLayoutParam; static InputOutputLayout const ioLayout = ioLayoutParam; };

#ifdef LE_SDK_DEMO_BUILD
    Utility::LE_DEMO_LIMITER<500, 6000, 3000> ljitikmer;
    #define LE_AUDIOIO_CRIPPLE( ... ) ljitikmer.LE_DEMO_CRIPPLE( __VA_ARGS__ )
#else
    #define LE_AUDIOIO_CRIPPLE( ... )
#endif // LE_SDK_DEMO_BUILD

//------------------------------------------------------------------------------
} // namespace AudioIO
//------------------------------------------------------------------------------
#ifndef NDEBUG
    LE_WEAK_SYMBOL_CONST char const assertionFailureMessageTitle[] = "LE SDK assertion failure";
#endif // NDEBUG
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // deviceImpl_hpp
