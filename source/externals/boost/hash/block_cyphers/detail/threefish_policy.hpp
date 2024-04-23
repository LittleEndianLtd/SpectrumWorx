
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_DETAIL_THREEFISH_POLICY_HPP
#define BOOST_HASH_BLOCK_CYPHERS_DETAIL_THREEFISH_POLICY_HPP

#include <boost/array.hpp>
#include <boost/hash/block_cyphers/detail/basic_functions.hpp>

namespace boost {
namespace hashes {
namespace block_cyphers {
namespace detail {

template <unsigned Version>
struct basic_threefish_policy : basic_functions<64> {

    static unsigned const block_bits = Version;
    static unsigned const block_words = block_bits/word_bits;
    typedef array<word_type, block_words> block_type;

    static unsigned const key_bits = Version;
    static unsigned const key_words = key_bits/word_bits;
    typedef array<word_type, key_words> key_type;
    typedef array<word_type, key_words+1> key_schedule_type;

    static unsigned const tweak_bits = 128;
    static unsigned const tweak_words = tweak_bits/word_bits;
    typedef array<word_type, tweak_words> tweak_type;
    typedef array<word_type, tweak_words+1> tweak_schedule_type;

    typedef array<unsigned, block_words> permutations_type;
    typedef array<array<unsigned, block_words/2>, 8>
            rotations_type;

};

template <unsigned Version>
struct threefish_policy;

template <>
struct threefish_policy<256> : basic_threefish_policy<256> {

    static unsigned const rounds = 72;
    typedef array<word_type, rounds> constants_type;

    static unsigned permutation(unsigned i) {
        static permutations_type const permutations = {{
            0, 3, 2, 1,
        }};
        return permutations[i];
    }

    static unsigned rotation(unsigned d, unsigned j) {
        static rotations_type const rotations = {{
#ifdef BOOST_HASH_THREEFISH_OLD_ROTATION_CONSTANTS
            {{ 5, 56}},
            {{36, 28}},
            {{13, 46}},
            {{58, 44}},
            {{26, 20}},
            {{53, 35}},
            {{11, 42}},
            {{59, 50}},
#else
            {{14, 16}},
            {{52, 57}},
            {{23, 40}},
            {{ 5, 37}},
            {{25, 33}},
            {{46, 12}},
            {{58, 22}},
            {{32, 32}},
#endif
        }};
        return rotations[d][j];
    }

};

template <>
struct threefish_policy<512> : basic_threefish_policy<512> {

    static unsigned const rounds = 72;
    typedef array<word_type, rounds> constants_type;

    static unsigned permutation(unsigned i) {
        static permutations_type const permutations = {{
            2, 1, 4, 7, 6, 5, 0, 3,
        }};
        return permutations[i];
    }

    static unsigned rotation(unsigned d, unsigned j) {
        static rotations_type const rotations = {{
#ifdef BOOST_HASH_THREEFISH_OLD_ROTATION_CONSTANTS
            {{38, 30, 50, 53}},
            {{48, 20, 43, 31}},
            {{34, 14, 15, 27}},
            {{26, 12, 58,  7}},
            {{33, 49,  8, 42}},
            {{39, 27, 41, 14}},
            {{29, 26, 11,  9}},
            {{33, 51, 39, 35}},
#else
            {{46, 36, 19, 37}},
            {{33, 27, 14, 42}},
            {{17, 49, 36, 39}},
            {{44,  9, 54, 56}},
            {{39, 30, 34, 24}},
            {{13, 50, 10, 17}},
            {{25, 29, 39, 43}},
            {{ 8, 35, 56, 22}},
#endif
        }};
        return rotations[d][j];
    }

};

template <>
struct threefish_policy<1024> : basic_threefish_policy<1024> {

    static unsigned const rounds = 80;
    typedef array<word_type, rounds> constants_type;

    static unsigned permutation(unsigned i) {
        static permutations_type const permutations = {{
            0, 9, 2, 13, 6, 11, 4, 15, 10, 7, 12, 3, 14, 5, 8, 1,
        }};
        return permutations[i];
    }

    static unsigned rotation(unsigned d, unsigned j) {
        static rotations_type const rotations = {{
#ifdef BOOST_HASH_THREEFISH_OLD_ROTATION_CONSTANTS
            {{55, 43, 37, 40, 16, 22, 38, 12}},
            {{25, 25, 46, 13, 14, 13, 52, 57}},
            {{33,  8, 18, 57, 21, 12, 32, 54}},
            {{34, 43, 25, 60, 44,  9, 59, 34}},
            {{28,  7, 47, 48, 51,  9, 35, 41}},
            {{17,  6, 18, 25, 43, 42, 40, 15}},
            {{58,  7, 32, 45, 19, 18,  2, 56}},
            {{47, 49, 27, 58, 37, 48, 53, 56}},
#else
            {{24, 13,  8, 47,  8, 17, 22, 37}},
            {{38, 19, 10, 55, 49, 18, 23, 52}},
            {{33,  4, 51, 13, 34, 41, 59, 17}},
            {{ 5, 20, 48, 41, 47, 28, 16, 25}},
            {{41,  9, 37, 31, 12, 47, 44, 30}},
            {{16, 34, 56, 51,  4, 53, 42, 41}},
            {{31, 44, 47, 46, 19, 42, 44, 25}},
            {{ 9, 48, 35, 52, 23, 31, 37, 20}},
#endif
        }};
        return rotations[d][j];
    }

};

} // namespace detail
} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_DETAIL_THREEFISH_POLICY_HPP
