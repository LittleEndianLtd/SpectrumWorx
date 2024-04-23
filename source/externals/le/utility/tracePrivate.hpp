////////////////////////////////////////////////////////////////////////////////
///
/// \file trace.hpp
/// ---------------
///
/// Copyright (c) 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef tracePrivate_hpp__363A7DB6_02EA_48C5_93CC_D38F9B5EE959
#define tracePrivate_hpp__363A7DB6_02EA_48C5_93CC_D38F9B5EE959
#pragma once
//------------------------------------------------------------------------------
#include "trace.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Utility
{
//------------------------------------------------------------------------------

#ifndef NDEBUG
    /// \note Ad-hoc solution for logging we do not want to appear in/spam the
    /// GUI of SDK example apps.
    ///                                       (28.01.2016.) (Domagoj Saric)
    extern void * LE_FASTCALL Tracer_setUserMessageMethod( void * );
    #define LE_TRACE_LOGONLY( formatString, ... )                                       \
    {                                                                                   \
        auto const __callback__( LE::Utility::Tracer_setUserMessageMethod( nullptr ) ); \
        LE::Utility::Tracer::message( formatString, ##__VA_ARGS__ );                    \
        LE::Utility::Tracer_setUserMessageMethod( __callback__ );                       \
    }
#else
    #define LE_TRACE_LOGONLY( formatString, ... )
#endif // _DEBUG

//------------------------------------------------------------------------------
} // namespace Utility
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // tracePrivate_hpp
