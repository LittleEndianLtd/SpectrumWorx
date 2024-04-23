////////////////////////////////////////////////////////////////////////////////
///
/// inserterImpl.cpp
/// ----------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "inserterImpl.hpp"

#include "le/spectrumworx/effects/indexRange.hpp"
#include "le/spectrumworx/engine/channelDataAmPh.hpp"
#include "le/spectrumworx/engine/setup.hpp"
#include "le/math/math.hpp"
#include "le/math/vector.hpp"
#include "le/parameters/uiElements.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
namespace Effects
{
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
//
// Inserter static member definitions.
//
////////////////////////////////////////////////////////////////////////////////

char const Inserter::title      [] = "Inserter";
char const Inserter::description[] = "Insert side channel into main.";


////////////////////////////////////////////////////////////////////////////////
//
// Inserter UIElements definitions.
//
////////////////////////////////////////////////////////////////////////////////

EFFECT_PARAMETER_NAME( Inserter::Destination, "Destination" )
EFFECT_PARAMETER_NAME( Inserter::Source     , "Source"      )
EFFECT_PARAMETER_NAME( Inserter::InsertSize , "Size"        )


////////////////////////////////////////////////////////////////////////////////
//
// InserterImpl::setup()
// ---------------------
//
////////////////////////////////////////////////////////////////////////////////

void InserterImpl::setup( IndexRange const & workingRange, Engine::Setup const & engineSetup )
{
    source_     = std::max( workingRange.begin(), engineSetup.frequencyPercentageToBin( parameters().get<Source     >() ) );
    target_     = std::max( workingRange.begin(), engineSetup.frequencyPercentageToBin( parameters().get<Destination>() ) );
    source_     = std::min( source_, workingRange.end() );
    target_     = std::min( target_, workingRange.end() );
    insertSize_ = std::min
                  (
                    static_cast<IndexRange::value_type>( workingRange.end() - std::max( target_, source_ ) ),
                    engineSetup.frequencyPercentageToBin( parameters().get<InsertSize>() )
                  );

    mode_.unpack( parameters().get<Mode>() );
}


////////////////////////////////////////////////////////////////////////////////
//
// InserterImpl::process()
// -----------------------
//
////////////////////////////////////////////////////////////////////////////////

void InserterImpl::process( Engine::MainSideChannelData_AmPh data, Engine::Setup const & ) const
{
    auto const source    ( source_     );
    auto const target    ( target_     );
    auto const insertSize( insertSize_ );
    if ( mode_.magnitudes() )
    {
        Math::copy
        (
            &data.full().side().amps()[ source ],
            &data.full().main().amps()[ target ],
            insertSize
        );
    }
    if ( mode_.phases() )
    {
        Math::copy
        (
            &data.full().side().phases()[ source ],
            &data.full().main().phases()[ target ],
            insertSize
        );
    }
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
