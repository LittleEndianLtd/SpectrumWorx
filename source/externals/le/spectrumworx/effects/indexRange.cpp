////////////////////////////////////////////////////////////////////////////////
///
/// indexRange.cpp
/// --------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#include "indexRange.hpp"

#include "boost/assert.hpp"
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Effects )
//------------------------------------------------------------------------------

IndexRange::IndexRange( value_type const begin, value_type const end )
    :
    Pair( begin, end )
{
    BOOST_ASSERT_MSG( isValid(), "Invalid range" );
}


void IndexRange::setFirst( value_type const newBeginning )
{
    BOOST_ASSERT_MSG( isValid( newBeginning, end() ), "Invalid range" );
    Pair::first = newBeginning;
}


void IndexRange::setLast( value_type const newLast )
{
    BOOST_ASSERT_MSG( isValid( first(), newLast + 1 ), "Invalid range" );
    Pair::second = newLast + 1;
}


void IndexRange::setNewRange( value_type const newBeginning, value_type const newLast )
{
    BOOST_ASSERT_MSG( isValid( newBeginning, newLast + 1 ), "Invalid range" );
    Pair::first  = newBeginning    ;
    Pair::second = newLast      + 1;
}


bool IndexRange::isValid( value_type const first, value_type const end )
{
    return first <= end;
}

bool IndexRange::isValid() const
{
    return isValid( first(), end() );
}

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Effects )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
