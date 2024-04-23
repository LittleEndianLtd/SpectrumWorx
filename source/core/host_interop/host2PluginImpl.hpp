////////////////////////////////////////////////////////////////////////////////
///
/// \file host2PluginImpl.hpp
/// -------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef host2PluginImpl_hpp__B10401C1_DAAE_4053_83AA_C44B1A7E0657
#define host2PluginImpl_hpp__B10401C1_DAAE_4053_83AA_C44B1A7E0657
#pragma once
//------------------------------------------------------------------------------
#include "host2Plugin.hpp"

#include "le/plugins/plugin.hpp"
#include "le/utility/platformSpecifics.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

template <class Impl, class Protocol>
class LE_NOVTABLE Host2PluginInteropImpl
/// \note No virtual functions so no need to inherit from
/// Host2PluginInteropControler (makes implementing/defining derived classes
/// easier.
///                                           (26.03.2014.) (Domagoj Saric)
{
public:
    using ErrorCode = typename Plugins::ErrorCode<Protocol>::value_type;
    static ErrorCode makeErrorCode( ErrorCode const errorCode ) { return errorCode                                                                                  ; }
    static ErrorCode makeErrorCode( bool      const success   ) { return success ? Plugins::ErrorCode<Protocol>::Success : Plugins::ErrorCode<Protocol>::OutOfMemory; }

public:
    typedef typename Plugins::AutomatedParameterFor<Protocol>::type AutomatedParameter;

public: // Plugin framework interface
    ErrorCode LE_FASTCALL setParameter( ParameterID, Plugins::AutomatedParameterValue );

private: //...mrmlj...spectrumWorxSharedImpl.hpp...
public:
    bool isHost( Protocol, char const * const hostName ) const
    {
        typedef Plugins::Plugin<Impl, Protocol> PluginPlatform;
        typename PluginPlatform::ProductStringBuf buffer; buffer[ 0 ] = 0;
        impl().host().getHostProductString( buffer );
        return std::strstr( buffer, hostName ) != nullptr;
    }

    template <typename OtherProtocol>
    static
    bool isHost( OtherProtocol, char const * /*hostName*/ ) { return false; }

protected:
    Impl       & impl()       { LE_ASSUME( this ); return static_cast<Impl &>( *this );      }
    Impl const & impl() const { return const_cast<Host2PluginInteropImpl &>( *this ).impl(); }

private: // Indexed parameter functors.
    class ParameterSetter;
}; // class Host2PluginInteropImpl

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // host2PluginImpl_hpp
