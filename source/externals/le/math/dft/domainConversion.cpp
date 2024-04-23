////////////////////////////////////////////////////////////////////////////////
///
/// domainConversion.cpp
/// --------------------
///
/// Copyright (c) 2010 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "domainConversion.hpp"

#include "le/math/vector.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Math )
//------------------------------------------------------------------------------

void LE_FASTCALL reim2AmPh
(
    float const * const reals     , float const * const imags ,  // input
    float       * const amplitudes, float       * const phases,  // output
    std::uint16_t const numberOfSamples
)
{
    BOOST_ASSERT( reals && imags && amplitudes && phases );

    rectangular2polar( reals, imags, amplitudes, phases, numberOfSamples );
}


void LE_FASTCALL amph2ReIm
(
    float const * const amplitudes, float const * const phases,  // input
    float       * const reals     , float       * const imags ,  // output
    std::uint16_t const numberOfSamples
)
{
    BOOST_ASSERT( amplitudes && phases && reals && imags );

    polar2rectangular( amplitudes, phases, reals, imags, numberOfSamples );
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Math )
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
