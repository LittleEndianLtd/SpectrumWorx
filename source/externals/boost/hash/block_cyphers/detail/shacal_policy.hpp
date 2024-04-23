
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_DETAIL_SHACAL_POLICY_HPP
#define BOOST_HASH_BLOCK_CYPHERS_DETAIL_SHACAL_POLICY_HPP

#include <boost/array.hpp>
#include <boost/hash/block_cyphers/detail/shacal_functions.hpp>

namespace boost {
namespace hashes {
namespace block_cyphers {
namespace detail {

struct shacal_policy : shacal_functions {

    static unsigned const block_words = 5;
    static unsigned const block_bits = block_words*word_bits;
    typedef array<word_type, block_words> block_type;

    static unsigned const key_words = 16;
    static unsigned const key_bits = key_words*word_bits;
    typedef array<word_type, key_words> key_type;

    static unsigned const rounds = 80;
    typedef array<word_type, rounds> schedule_type;
    typedef array<word_type, rounds> constants_type;

    static word_type constant(unsigned t) {
        static constants_type const constants = {{
            0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999,
            0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999,
            0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999,
            0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999,
            0x5a827999, 0x5a827999, 0x5a827999, 0x5a827999,

            0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1,
            0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1,
            0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1,
            0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1,
            0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1, 0x6ed9eba1,

            0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc,
            0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc,
            0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc,
            0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc,
            0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc, 0x8f1bbcdc,

            0xca62c1d6, 0xca62c1d6, 0xca62c1d6, 0xca62c1d6,
            0xca62c1d6, 0xca62c1d6, 0xca62c1d6, 0xca62c1d6,
            0xca62c1d6, 0xca62c1d6, 0xca62c1d6, 0xca62c1d6,
            0xca62c1d6, 0xca62c1d6, 0xca62c1d6, 0xca62c1d6,
            0xca62c1d6, 0xca62c1d6, 0xca62c1d6, 0xca62c1d6,
        }};
        return constants[t];
    }

};

typedef shacal_policy shacal0_policy;

} // namespace detail
} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_DETAIL_SHACAL_POLICY_HPP
