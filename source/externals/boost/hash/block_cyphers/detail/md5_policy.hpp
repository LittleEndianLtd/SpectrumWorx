
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_DETAIL_MD5_POLICY_HPP
#define BOOST_HASH_BLOCK_CYPHERS_DETAIL_MD5_POLICY_HPP

#include <boost/array.hpp>
#include <boost/hash/block_cyphers/detail/basic_functions.hpp>

namespace boost {
namespace hashes {
namespace block_cyphers {
namespace detail {

struct md5_functions : detail::basic_functions<32> {
    static word_type ff(word_type x, word_type y, word_type z) {
        return (x & y) | (~x & z);
    }
    static word_type gg(word_type x, word_type y, word_type z) {
        return (x & z) | (y & ~z);
        // return F(z, x, y);
    }
    static word_type hh(word_type x, word_type y, word_type z) {
        return x ^ y ^ z;
    }
    static word_type ii(word_type x, word_type y, word_type z) {
        return y ^ (x | ~z);
    }
};

struct md5_policy : md5_functions {

    static unsigned const block_bits = 128;
    static unsigned const block_words = block_bits/word_bits;
    typedef array<word_type, block_words> block_type;

    static unsigned const key_words = 16;
    static unsigned const key_bits = key_words*word_bits;
    typedef array<word_type, key_words> key_type;

    static unsigned const rounds = 64;
    typedef array<word_type, rounds> constants_type;
    typedef array<unsigned, rounds> key_indexes_type;

    static word_type constant(unsigned t) {
        static constants_type const constants = {{
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
            0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
            0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
            0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,

            0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
            0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
            0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,

            0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
            0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
            0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,

            0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
            0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
            0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
        }};
        return constants[t];
    }

    static unsigned key_index(unsigned t) {
        static key_indexes_type const key_indexes = {{
            0,  1,  2,  3,
            4,  5,  6,  7,
            8,  9,  10, 11,
            12, 13, 14, 15,

            1,  6,  11, 0,
            5,  10, 15, 4,
            9,  14, 3,  8,
            13, 2,  7,  12,

            5,  8,  11, 14,
            1,  4,  7,  10,
            13, 0,  3,  6,
            9,  12, 15, 2,

            0,  7,  14, 5,
            12, 3,  10, 1,
            8,  15, 6,  13,
            4,  11, 2,  9,
        }};
        return key_indexes[t];
    }
};

} // namespace detail
} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_DETAIL_MD5_POLICY_HPP
