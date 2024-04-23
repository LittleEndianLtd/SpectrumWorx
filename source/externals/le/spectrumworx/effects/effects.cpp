////////////////////////////////////////////////////////////////////////////////
///
/// effects.cpp
/// -----------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "effects.hpp"

#include "commonParameters.hpp"
#include "le/utility/platformSpecifics.hpp"
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

void UnpackedMagPhaseMode::unpack( CommonParameters::Mode const & mode )
{
    switch ( mode.getValue() )
    {
        case CommonParameters::Mode::Both:
            magnitudes_ = true;
            phases_     = true;
            break;
        case CommonParameters::Mode::Magnitudes:
            magnitudes_ = true ;
            phases_     = false;
            break;
        case CommonParameters::Mode::Phases:
            magnitudes_ = false;
            phases_     = true ;
            break;
        LE_DEFAULT_CASE_UNREACHABLE();
    }
}


bool UnpackedMagPhaseMode::magnitudes() const
{
    return magnitudes_;
}


bool UnpackedMagPhaseMode::phases() const
{
    return phases_;
}


////////////////////////////////////////////////////////////////////////////////
//
// ModuloCounter::nextValueFor()
// -----------------------------
//
////////////////////////////////////////////////////////////////////////////////
///
/// \return incremented or wrapped-around counter value with the flag specifying
/// whether the counter wrapped-around as a result of the increment.
///
/// \throws nothing
///
////////////////////////////////////////////////////////////////////////////////

std::pair<ModuloCounter::value_type, bool> ModuloCounter::nextValueFor( value_type const moduloValue )
{
    value_type const incrementedCounter( counter_ + 1                      );
    bool       const wrapAround        ( incrementedCounter >= moduloValue );
    value_type const actualCounter
    (
        wrapAround
            ? 0
            : incrementedCounter
    );
    counter_ = actualCounter;
    return std::pair<value_type, bool>( actualCounter, wrapAround );
}

//------------------------------------------------------------------------------
} // namespace Effects
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
