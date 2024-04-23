////////////////////////////////////////////////////////////////////////////////
///
/// moduleFactory.cpp
/// -----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "factory.hpp"

#include "configuration/versionConfiguration.hpp"

#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
    #include "core/modules/moduleDSPAndGUI.hpp"
#else
    #include "core/modules/moduleDSP.hpp"
    #if LE_SW_GUI
    #include "core/modules/moduleGUI.hpp"
    #endif
#endif
#include "core/modules/finalImplementations.hpp"

#include "le/spectrumworx/effects/configuration/effectNames.hpp"
#include "le/spectrumworx/effects/configuration/indexToEffectImplMapping.hpp"
#include "le/spectrumworx/effects/configuration/includedEffects.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"
#include "le/utility/rvalueReferences.hpp"
#include "le/utility/switch.hpp"

#if LE_SW_GUI && !defined( LE_SW_FMOD )
    // Implementation note:
    //   Required for the (PVD)PitchMagnet::Target-unnecessarily-quantized
    // workaround. Included here to avoid slowing down the compilation of
    // modules that include gui.hpp.
    //                                        (13.12.2011.) (Domagoj Saric)
    #include "le/spectrumworx/effects/pitch_magnet/pitchMagnet.hpp"
#endif // LE_SW_GUI

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

#if LE_SW_GUI && !defined( LE_SW_FMOD )
namespace GUI
{
    // Implementation note:
    //   The (PVD)PitchMagnet::Target parameter is so far the only parameter
    // that is not handled correctly by the simple ModuleKnob::QuantizationFor
    // logic (which detects quantized parameters simply by their units). It is a
    // Hertz parameter but its precision is not bound by the DFT engine
    // parameters ando so it does not need to be quantized accordingly.
    //                                        (13.12.2011.) (Domagoj Saric)
    /// \todo Think of a cleaner solution.
    ///                                       (13.12.2011.) (Domagoj Saric)
    template <>
    struct ModuleKnob::QuantizationFor<Effects::Detail::PitchMagnetBase::Target>
    {
        static ModuleKnob::Quantization const value = ModuleKnob::Fixed;
    };
} // namespace GUI
#endif // LE_SW_GUI

namespace
{

////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct ModuleSizeGetter
////////////////////////////////////////////////////////////////////////////

template <class ModuleInterface>
struct ModuleSizeGetter
{
    using result_type = std::uint16_t;

    template <class EffectIndex>
    result_type operator()( EffectIndex ) const
    {
        using EffectImplementation = typename Effects::ImplForIndex<EffectIndex::value>::type      ;
        using ModuleImplementation = typename ModuleInterface:: template Impl<EffectImplementation>;
        return sizeof( ModuleImplementation );
    }
}; // struct ModuleSizeGetter


////////////////////////////////////////////////////////////////////////////
/// \internal
/// \struct ModuleConstructor
////////////////////////////////////////////////////////////////////////////

#pragma warning( push )
#pragma warning( disable : 4200 ) // Nonstandard extension used : zero-sized array in struct/union.

template <class ModuleInterface>
struct ModuleConstructor
{
    using result_type = ModuleInterface * LE_RESTRICT;

    template <class EffectIndex>
    LE_NOTHROWNOALIAS
    result_type operator()( EffectIndex )
    {
        using EffectImplementation = typename Effects::ImplForIndex<EffectIndex::value>::type      ;
        using ModuleImplementation = typename ModuleInterface:: template Impl<EffectImplementation>;
        LE_ASSUME( this );
        result_type const result( new ( storage ) ModuleImplementation( EffectIndex() ) );
        LE_ASSUME( result );
        return result;
    }

    // Implementation note:
    //   A "quick-patch" to handle the special case of the Armonizer effect
    // where we want the Wet parameter to default to 50%.
    //                                        (12.12.2011.) (Domagoj Saric)
    using ArmonizerIndex = std::integral_constant<unsigned int, 52>;
    LE_NOTHROWNOALIAS
    result_type operator()( ArmonizerIndex )
    {
        typedef typename Effects::ImplForIndex<ArmonizerIndex::value>::type    EffectImplementation;
        typedef typename ModuleInterface:: template Impl<EffectImplementation> ModuleImplementation;
        static_assert( std::is_same<EffectImplementation, Effects::ArmonizerImpl>::value, "Internal inconsistency" );
        auto const pModule( new ( storage ) ModuleImplementation( ArmonizerIndex() ) );
        LE_ASSUME( pModule );
    #if LE_SW_SEPARATED_DSP_GUI
        pModule->setBaseParameter( LE::Parameters::IndexOf<Effects::BaseParameters::Parameters, Effects::BaseParameters::Wet>::value, 50 );
    #else
        pModule->baseParameters(). template set<Effects::BaseParameters::Wet>( 50.0f );
    #endif // LE_SW_SEPARATED_DSP_GUI
        return pModule;
    }

    char storage[];
}; // struct ModuleConstructor

#pragma warning( pop )
} // anonymous namespace


////////////////////////////////////////////////////////////////////////////////
//
// ModuleFactory::create()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

template <class ModuleInterface>
LE_NOTHROW
boost::intrusive_ptr<ModuleInterface> ModuleFactory::create( std::int8_t const effectIndex )
{
    std::int8_t const noModule( -1 );
    if ( effectIndex == noModule )
        return nullptr;
    LE_ASSUME( effectIndex >= 0 );

    bool const moduleEnabled( Effects::includedEffects[ effectIndex ] );
#ifdef LE_SW_FULL
    LE_ASSUME( moduleEnabled == true );
#endif // LE_SW_FULL
    if ( !moduleEnabled )
    {
    #if LE_SW_GUI
        GUI::warningMessageBox( MB_WARNING " effect not available in this edition.", Effects::effectName( effectIndex ), false );
    #endif // LE_SW_GUI
        return nullptr;
    }

    using namespace boost;

    using SizeGetter = ModuleSizeGetter<ModuleInterface>;
    auto const storageSize
    (
        switch_<Effects::ValidIndices>
        (
            effectIndex,
            SizeGetter(),
            assert_no_default_case<typename SizeGetter::result_type>()
        )
    );
    void * const pStorage( std::malloc( storageSize ) );

    if ( BOOST_UNLIKELY( !pStorage ) )
        return nullptr;

    using Constructor = ModuleConstructor<ModuleInterface>;
    Constructor & moduleConstructor( *static_cast<Constructor *>( pStorage ) );
    return
        switch_<Effects::ValidIndices>
        (
            effectIndex,
            std::forward<Constructor>( moduleConstructor ),
            assert_no_default_case<typename Constructor::result_type>()
        );
}


#if LE_SW_GUI && !LE_SW_SEPARATED_DSP_GUI
    template LE_NOTHROW boost::intrusive_ptr<SW::Module   > LE_FASTCALL ModuleFactory::create( std::int8_t effectIndex );
#else
    template LE_NOTHROW boost::intrusive_ptr<SW::ModuleDSP> LE_FASTCALL ModuleFactory::create( std::int8_t effectIndex );
    #if LE_SW_GUI
    template LE_NOTHROW boost::intrusive_ptr<SW::ModuleGUI> LE_FASTCALL ModuleFactory::create( std::int8_t effectIndex );
    #endif
#endif

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
