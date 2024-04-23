////////////////////////////////////////////////////////////////////////////////
///
/// \file indexRange.hpp
/// --------------------
///
/// Copyright (c) 2009 - 2016. Little Endian Ltd. All rights reserved.
///
////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------------------------
#ifndef indexRange_hpp__E1F61CC5_AE07_414C_8EF8_3DDC52F62AD5
#define indexRange_hpp__E1F61CC5_AE07_414C_8EF8_3DDC52F62AD5
#pragma once
//------------------------------------------------------------------------------
#include <cstdint>
#include <utility>
//------------------------------------------------------------------------------
namespace LE
{
//------------------------------------------------------------------------------
namespace SW
{
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Engine )
    class Setup;
LE_IMPL_NAMESPACE_END(   Engine )
//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_BEGIN( Effects )
//------------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
///
/// \class IndexRange
///
/// Models a range of indices. It is therefore not associated with any
/// particular container or (contained) object type, although it is best suited
/// for random-access containers considering its index-based implementation.
///
////////////////////////////////////////////////////////////////////////////////

/// \note std::uint_fast16_t maps to std::uint64_t on 64bit Android/GCC/Clang
/// platforms which is too much.
/// http://stackoverflow.com/questions/4116297/x86-64-why-is-uint-least16-t-faster-then-uint-fast16-t-for-multiplication
///                                           (26.03.2015.) (Domagoj Saric)
class IndexRange : public std::pair<std::uint16_t, std::uint16_t>
{
public:
    using value_type        = first_type;
    using signed_value_type = std::int16_t;
    using Pair              = std::pair<value_type, value_type>;

public:
    IndexRange() {}
    IndexRange( value_type begin, value_type end );

    value_type begin() const { return Pair::first ; }
    value_type end  () const { return Pair::second; }

    value_type first() const { return begin()    ; }
    value_type last () const { return end  () - 1; }

    value_type size() const { return end() - begin(); }

    bool empty() const { return size() == 0; }

    void setFirst( value_type newBeginning );
    void setLast ( value_type newLast      );

    void setNewRange( value_type newBeginning, value_type newLast );

    value_type operator*() const { return first(); }

    explicit operator bool() const { return !empty(); }

private:
    static bool isValid( value_type first, value_type end )      ;
           bool isValid(                                  ) const;
}; // class IndexRange

//------------------------------------------------------------------------------
LE_IMPL_NAMESPACE_END( Effects )
//------------------------------------------------------------------------------
} // namespace SW
//------------------------------------------------------------------------------
} // namespace LE
//------------------------------------------------------------------------------
#endif // indexRange_hpp
