
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_STREAM_ENDIAN_HPP
#define BOOST_HASH_STREAM_ENDIAN_HPP

#include <boost/static_assert.hpp>

#include <climits>

namespace boost {
namespace hashes {

namespace stream_endian {

    // General versions; There should be no need to use these directly

    template <int UnitBits>
    struct big_unit_big_bit {};
    template <int UnitBits>
    struct little_unit_little_bit {};
    template <int UnitBits>
    struct big_unit_little_bit {};
    template <int UnitBits>
    struct little_unit_big_bit {};
    template <int UnitBits>
    struct host_unit { BOOST_STATIC_ASSERT(UnitBits % CHAR_BIT == 0); };

    // Typical, useful instantiations

    typedef big_unit_big_bit<1> big_bit;
    typedef big_unit_big_bit<CHAR_BIT> big_byte_big_bit;
    typedef big_unit_big_bit<8> big_octet_big_bit;

    typedef little_unit_little_bit<1> little_bit;
    typedef little_unit_little_bit<CHAR_BIT> little_byte_little_bit;
    typedef little_unit_little_bit<8> little_octet_little_bit;

    typedef big_unit_little_bit<CHAR_BIT> big_byte_little_bit;
    typedef big_unit_little_bit<8> big_octet_little_bit;

    typedef little_unit_big_bit<CHAR_BIT> little_byte_big_bit;
    typedef little_unit_big_bit<8> little_octet_big_bit;

    typedef host_unit<CHAR_BIT> host_byte;

}

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_STREAM_ENDIAN_HPP
