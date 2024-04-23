////////////////////////////////////////////////////////////////////////////////
///
/// \file windows.hpp
/// -----------------
///
/// Copyright (c) 2011 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef windows_hpp__1A80D559_4313_4EBB_89BA_4740817A0C44
#define windows_hpp__1A80D559_4313_4EBB_89BA_4740817A0C44
#pragma once
//------------------------------------------------------------------------------
#include "le/spectrumworx/engine/configuration.hpp"

#include "le/utility/platformSpecifics.hpp"

#include "boost/range/iterator_range_core.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

using DataRange = boost::iterator_range<float * LE_RESTRICT>;

void LE_FASTCALL calculateWindow( DataRange const & window, LE::SW::Engine::Constants::Window );

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // windows_hpp
