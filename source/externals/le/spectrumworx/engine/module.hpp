////////////////////////////////////////////////////////////////////////////////
///
/// \file module.hpp
///
///    Base module interface and implementation.
///
/// Copyright � 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef module_hpp__0A6DD89D_4BD6_4CE8_B0F1_4E83E0F5DABE
#define module_hpp__0A6DD89D_4BD6_4CE8_B0F1_4E83E0F5DABE
#pragma once
//------------------------------------------------------------------------------
#include "channelData_fwd.hpp"
#ifdef LE_SW_SDK_BUILD
#include "moduleBase.hpp"
#endif // LE_SW_SDK_BUILD

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/buffers.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"

#include "le/utility/platformSpecifics.hpp"

#include <cstdint>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Engine { struct StorageFactors; using Storage = boost::iterator_range<char * LE_RESTRICT>; }
LE_IMPL_NAMESPACE_BEGIN( Engine )
//------------------------------------------------------------------------------

class ChannelData;
class Setup;

////////////////////////////////////////////////////////////////////////////////
///
/// \class ModuleDSP
///
////////////////////////////////////////////////////////////////////////////////

class LE_NOVTABLE ModuleDSP
    :
#ifdef LE_SW_SDK_BUILD
    public LE::SW::Engine::ModuleBase,
#endif // LE_SW_SDK_BUILD
    public LE::SW::Engine::ModuleParameters
{
public:
    using Parameters = Effects::BaseParameters::Parameters;
    using LFO        = ModuleParameters::LFO;

#ifdef _MSC_VER
    #pragma warning( push )
    #pragma warning( disable : 4512 ) // Assignment operator could not be generated.
#endif // _MSC_VER
    class ChannelDataProxy
    {
    public:
        ChannelDataProxy( ChannelData &, ModuleDSP const &, bool doBlend, bool & amPh2ReIm );

        LE_NOTHROWNOALIAS LE_FASTCALL operator MainSideChannelData_AmPh() const;
        LE_NOTHROWNOALIAS LE_FASTCALL operator MainSideChannelData_ReIm() const;

        LE_NOTHROWNOALIAS LE_FASTCALL operator ChannelData_AmPh        () const;
        LE_NOTHROWNOALIAS LE_FASTCALL operator ChannelData_ReIm        () const;

        LE_NOTHROWNOALIAS LE_FASTCALL operator ChannelData_AmPh2ReIm   () const;
        LE_NOTHROWNOALIAS LE_FASTCALL operator ChannelData_ReIm2AmPh   () const;

    private:
        ModuleDSP   const &       module_       ;
        ChannelData       &       data_         ;
        bool              &       amPh2ReIm_    ;
        bool                const blendRequired_;
    }; // class ChannelDataProxy
#ifdef _MSC_VER
    #pragma warning( pop )
#endif // _MSC_VER


    template <class Effect> class Impl;

public:
    using ModuleParameters::baseParameters;
    using ModuleParameters::setEffectParameter;

            LE_NOTHROW        void LE_FASTCALL initialise( Setup const & engineSetup ) { setup( engineSetup ); }
            LE_NOTHROW        void LE_FASTCALL preProcess( LFO::Timer const &,                  Setup const & )      ;
            LE_NOTHROW        void LE_FASTCALL process   ( std::uint8_t channel, ChannelData &, Setup const & ) const;

    virtual LE_NOTHROWNOALIAS void LE_FASTCALL reset (                        ) = 0;
    virtual LE_NOTHROWNOALIAS bool LE_FASTCALL resize( StorageFactors const & ) = 0;

protected:
    ModuleDSP
    (
        ModuleParameters::EffectMetaData const &       metaData,
    #ifndef LE_NO_LFOs
        LFOPlaceholder                         * const pLFOs,
    #endif // !LE_NO_LFOs
        std::uint8_t                     const * const pParameterOffsets,
        std::uint16_t                            const parametersBaseOffset
    )
        :
    #ifdef LE_NO_LFOs
        ModuleParameters     ( metaData             ),
    #else
        ModuleParameters     ( metaData, pLFOs      ),
    #endif
        parametersBaseOffset_( parametersBaseOffset ),
        pParameterOffsets_   ( pParameterOffsets    )
    { BOOST_ASSERT( storage_.begin() == nullptr ); }

#ifdef LE_SW_SDK_BUILD //...mrmlj...reinvestigate this...
    public: LE_NOTHROW virtual ~ModuleDSP(); protected:
#else
    LE_NOTHROW ~ModuleDSP();
#endif

    void setup( Setup const & );

    bool LE_FASTCALL allocateStorage( StorageFactors const &, std::uint16_t channelStateSize, std::uint32_t channelStateRequiredStorage );
    Storage const & storage() const { return storage_; }

    Effects::IndexRange const & workingRange() const { return workingRange_; }

private:
    virtual LE_NOTHROWNOALIAS void LE_FASTCALL doPreProcess(                                         Setup const & )       = 0;
    virtual LE_NOTHROWNOALIAS void LE_FASTCALL doProcess   ( std::uint8_t channel, ChannelDataProxy, Setup const & ) const = 0;

#ifdef LE_SW_SDK_BUILD
private: // boost::intrusive_ptr required section
    friend LE_NOTHROWNOALIAS void LE_FASTCALL_ABI intrusive_ptr_add_ref( ModuleBase const * );
    friend LE_NOTHROW        void LE_FASTCALL_ABI intrusive_ptr_release( ModuleBase const * );
#endif // LE_SW_SDK_BUILD

private:
    friend class LE::SW::Engine::ModuleParameters;
    float LE_FASTCALL setEffectParameter( std::uint8_t parameterIndex, float value, ParameterInfo const & cachedInfo );

    LE_CONST_FUNCTION void       * LE_FASTCALL getEffectParameterPtr( std::uint8_t parameterIndex )      ;
    LE_CONST_FUNCTION void const * LE_FASTCALL getEffectParameterPtr( std::uint8_t parameterIndex ) const;

private:
    mutable Effects::IndexRange workingRange_;

    std::uint16_t                     const parametersBaseOffset_;
    std::uint8_t  const * LE_RESTRICT const pParameterOffsets_   ;

    HeapSharedStorage storage_;
}; // class ModuleDSP

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Engine )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // module_hpp
