////////////////////////////////////////////////////////////////////////////////
///
/// \file automatedModuleChain.hpp
/// ------------------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef automatedModuleChain_hpp__46188EA4_1820_4042_A387_537f2C9BEB85
#define automatedModuleChain_hpp__46188EA4_1820_4042_A387_537f2C9BEB85
#pragma once
//------------------------------------------------------------------------------
#include "modules/factory.hpp"

#include "configuration/constants.hpp"
#include "core/host_interop/parameters.hpp" //...mrmlj...for Program

#include "le/parameters/linear/parameter.hpp"
#include "le/parameters/parameter.hpp"
#include "le/spectrumworx/effects/configuration/constants.hpp"
#include "le/spectrumworx/engine/moduleChainImpl.hpp"
#include "le/spectrumworx/engine/moduleParameters.hpp"
#include "le/utility/cstdint.hpp"
#include "le/utility/trace.hpp"

#include "boost/smart_ptr/intrusive_ptr.hpp"

#include <array>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------

//...mrmlj...defined outside of the AutomatedModuleChain class to enable forward declarations to speed up compilation...clean this up...
std::int8_t BOOST_CONSTEXPR_OR_CONST noModule = -1;

LE_DEFINE_PARAMETER
(
    ( ModuleChainParameter                             )
    ( Parameters::LinearSignedInteger                  )
    ( Minimum<noModule>                                )
    ( Maximum<Effects::Constants::numberOfEffects - 1> )
    ( Default<noModule>                                )
);


////////////////////////////////////////////////////////////////////////////////
///
/// \class AutomatedModuleChain
///
////////////////////////////////////////////////////////////////////////////////

class AutomatedModuleChain : public Engine::ModuleChainImpl
{
public:
    enum { noModule = SW::noModule };

    //...mrmlj...GUI only chains don't hold ModuleDSPs...
    //using ModulePtr  = Engine::ModuleChainImpl::      pointer;
    //using ModuleCPtr = Engine::ModuleChainImpl::const_pointer;
    using Module     = Engine::ModuleParameters;
    using ModulePtr  = boost::intrusive_ptr<Module      >;
    using ModuleCPtr = boost::intrusive_ptr<Module const>;

    static std::uint8_t const maximumSize = Constants::maxNumberOfModules;

public:
#if defined( _MSC_VER ) && _MSC_VER < 1900
    AutomatedModuleChain() {}
    AutomatedModuleChain( AutomatedModuleChain && other ) : Engine::ModuleChainImpl( std::forward<Engine::ModuleChainImpl>( other ) ) {}
#else
    using Engine::ModuleChainImpl::ModuleChainImpl;
#endif // _MSC_VER
    using ModuleChainImpl::operator =;

    ModuleChainParameter getParameterForIndex( std::uint8_t moduleIndex ) const;

    LE_NOTHROWNOALIAS ModulePtr  LE_FASTCALL module( std::uint8_t index )      ;
    LE_NOTHROWNOALIAS ModuleCPtr LE_FASTCALL module( std::uint8_t index ) const;

    template <class ActualModule>
    LE_NOTHROWNOALIAS boost::intrusive_ptr<ActualModule> LE_FASTCALL moduleAs( std::uint8_t const index )
    {
        auto const pModule( ModuleChainImpl::module( index ) );
        return ( !isEnd( pModule ) )
            ? &Engine::actualModule<ActualModule>( *pModule )
            : nullptr;
    }

    template <class ActualModule>
    LE_NOTHROWNOALIAS boost::intrusive_ptr<ActualModule const> LE_FASTCALL moduleAs( std::uint8_t const index ) const
    {
        auto const pModule( ModuleChainImpl::module( index ) );
        return ( !isEnd( pModule ) )
            ? &Engine::actualModule<typename std::remove_const<ActualModule>::type /*const*/>( *pModule )
            : nullptr;
    }

    template <class ModuleInitialiser>
    std::pair
    <
        boost::intrusive_ptr<typename ModuleInitialiser::Module>,
        std::int8_t
    > LE_NOTHROW LE_FASTCALL setParameter
    (
        std::uint8_t      const   moduleIndex,
        std:: int8_t      const   newValue,
        ModuleInitialiser const & initialise
    )
    {
        using Module = typename ModuleInitialiser::Module;
        using Engine::actualModule;

        auto const effectIndex( newValue );

        auto const pCurrentModuleNode( ModuleChainImpl::module( moduleIndex ) );
        auto const pCurrentModule    ( !isEnd( pCurrentModuleNode ) ? &actualModule<Module>( *pCurrentModuleNode ) : nullptr  );
        auto const currentEffect     ( !isEnd( pCurrentModuleNode ) ? pCurrentModule->effectTypeIndex()            : noModule );
        if ( currentEffect == newValue )
        {
            LE_TRACE( "\tSW Trying to insert an already existing module in the same slot." );
            return std::make_pair( pCurrentModule, currentEffect );
        }
        else
        if ( effectIndex == noModule )
        {
            this->remove( *pCurrentModuleNode );
            return std::make_pair( nullptr, noModule );
        }

        auto const pNewModule( ModuleFactory::create<Module>( effectIndex ) );

        if ( pNewModule && initialise( *pNewModule, moduleIndex ) )
        {
            insertAtAndReplace( pCurrentModuleNode, Engine::node( *pNewModule ) );
            return std::make_pair( pNewModule, effectIndex );
        }

        return std::make_pair( pCurrentModule, currentEffect );
    }
}; // class AutomatedModuleChain


class Program
{
public:
    using Parameters  = GlobalParameters::Parameters;
    using ModuleChain = AutomatedModuleChain        ;
    using Name        = std::array<char, 24>        ; //...mrmlj...kVstMaxProgNameLen

    Parameters       & parameters()       { return parameters_; }
    Parameters const & parameters() const { return const_cast<Program &>( *this ).parameters(); }

    ModuleChain       & moduleChain()       { return moduleChain_; }
    ModuleChain const & moduleChain() const { return const_cast<Program &>( *this ).moduleChain(); }

    Name       & name()       { return name_; }
    Name const & name() const { return const_cast<Program &>( *this ).name(); }

private:
    ModuleChain moduleChain_;
    Parameters  parameters_ ;
    Name        name_       ;
}; // class Program

//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // automatedModuleChain_hpp
