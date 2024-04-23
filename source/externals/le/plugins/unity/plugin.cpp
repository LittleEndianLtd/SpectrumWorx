////////////////////////////////////////////////////////////////////////////////
///
/// plugin.cpp
/// ----------
///
/// Copyright (c) 2015. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "plugin.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace Plugins
{
//------------------------------------------------------------------------------

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4702 ) // Unreachable code.
#endif // _MSC_VER
bool UnityHostProxy::reportNewNumberOfIOChannels( std::uint8_t const inputs, std::uint8_t const sideInputs, std::uint8_t const outputs )
{
    BOOST_ASSERT( sideInputs == 0 );
    BOOST_ASSERT( inputs == outputs ); (void)inputs;(void)sideInputs;(void)outputs;
    LE_UNREACHABLE_CODE();
    return false;
}
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER

//------------------------------------------------------------------------------
} // namespace Plugins
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------

#if 0 // clang (3.6&Xcode7) cannot handle friend injection from templates before instantiation...
extern "C" LE_DLL_EXPORT
int UnityGetAudioEffectDefinitions( UnityAudioEffectDefinition * * * const ppDefinitions )
{
    return LE::Plugins::UnityGetAudioEffectDefinitionsImpl( const_cast<UnityAudioEffectDefinition const * const * *>( ppDefinitions ) );
}
#endif // _DEBUG

//------------------------------------------------------------------------------
