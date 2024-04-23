
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_DETAIL_MD4_POLICY_HPP
#define BOOST_HASH_BLOCK_CYPHERS_DETAIL_MD4_POLICY_HPP

#include <boost/array.hpp>
#include <boost/hash/block_cyphers/detail/basic_functions.hpp>

namespace boost {
namespace hashes {
namespace block_cyphers {
namespace detail {

struct md4_functions : basic_functions<32> {
    static word_type ff(word_type x, word_type y, word_type z) {
        return (x & y) | (~x & z);
    }
    static word_type gg(word_type x, word_type y, word_type z) {
        return (x & y) | (x & z) | (y & z);
    }
    static word_type hh(word_type x, word_type y, word_type z) {
        return x ^ y ^ z;
    }
};

struct md4_policy : md4_functions {

    static unsigned const block_bits = 128;
    static unsigned const block_words = block_bits/word_bits;
    typedef array<word_type, block_words> block_type;

    static unsigned const key_words = 16;
    static unsigned const key_bits = key_words*word_bits;
    typedef array<word_type, key_words> key_type;

    static unsigned const rounds = 48;
    typedef array<unsigned, rounds> key_indexes_type;

    static unsigned key_index(unsigned t) {
        static key_indexes_type const key_indexes = {{
            0, 1, 2, 3,
            4, 5, 6, 7,
            8, 9, 10, 11,
            12, 13, 14, 15,

            0, 4, 8, 12,
            1, 5, 9, 13,
            2, 6, 10, 14,
            3, 7, 11, 15,

            0, 8, 4, 12,
            2, 10, 6, 14,
            1, 9, 5, 13,
            3, 11, 7, 15,
        }};
        return key_indexes[t];
    }
};

} // namespace detail
} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_DETAIL_MD4_POLICY_HPP
