////////////////////////////////////////////////////////////////////////////////
///
/// plugin.cpp
/// ----------
///
/// Copyright (c) 2012 - 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin.hpp"

#include "boost/simd/preprocessor/malloc.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

namespace Detail
{
    static FMOD_MEMORY_ALLOC_CALLBACK   pAlloc  ;
    static FMOD_MEMORY_REALLOC_CALLBACK pRealloc;
    static FMOD_MEMORY_FREE_CALLBACK    pFree   ;

    void setMemoryCallbacks( FMOD_DSP_STATE_SYSTEMCALLBACKS const & callbacks )
    {
        pAlloc   = callbacks.alloc  ;
        pRealloc = callbacks.realloc;
        pFree    = callbacks.free   ;
    }
}


#pragma warning( push )
#pragma warning( disable : 4702 ) // Unreachable code.
bool FMODHostProxy::reportNewNumberOfIOChannels( std::uint_fast8_t const inputs, std::uint_fast8_t const sideInputs, std::uint_fast8_t const outputs )
{
    BOOST_ASSERT( sideInputs == 0 );
    BOOST_ASSERT( inputs == outputs ); (void)inputs;(void)sideInputs;(void)outputs;
    LE_UNREACHABLE_CODE();
    return false;
}
#pragma warning( pop )

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

namespace boost
{
namespace simd
{
    LE_NOTHROW   BOOST_SIMD_MALLOC   void * custom_malloc_fn (                       std::size_t /*const*/ sz   ) { return LE::Plugins::Detail::pAlloc  (      static_cast<unsigned int>( sz ), FMOD_MEMORY_PLUGIN, nullptr ); }
    LE_NOTHROW /*BOOST_SIMD_MALLOC*/ void * custom_realloc_fn( void * /*const*/ ptr, std::size_t /*const*/ sz  , std::size_t  ) { return LE::Plugins::Detail::pRealloc( ptr, static_cast<unsigned int>( sz ), FMOD_MEMORY_PLUGIN, nullptr ); }
    LE_NOTHROW                       void   custom_free_fn   ( void * /*const*/ ptr, std::size_t /*const   sz*/ ) { return LE::Plugins::Detail::pFree   ( ptr,                                  FMOD_MEMORY_PLUGIN, nullptr ); }
} // namespace simd
} // namespace boost
