
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_CUBEHASH_HPP
#define BOOST_HASH_CUBEHASH_HPP

#include <boost/hash/detail/cubehash_policy.hpp>
#include <boost/hash/merkle_damgard_block_hash.hpp>
#include <boost/hash/stream_preprocessor.hpp>

// As of 2009-07-15, the submission to NIST for SHA-3 is CubeHash16/32
// http://cubehash.cr.yp.to/submission/tweak.pdf
#ifndef BOOST_HASH_CUBEHASH_DEFAULT_R
#define BOOST_HASH_CUBEHASH_DEFAULT_R 16
#endif
#ifndef BOOST_HASH_CUBEHASH_DEFAULT_B
#define BOOST_HASH_CUBEHASH_DEFAULT_B 32
#endif

namespace boost {
namespace hashes {

template <unsigned r, unsigned b, unsigned h>
struct cubehash_compressor {
  public:
    typedef detail::cubehash_policy<r, b, h> policy_type;

    static unsigned const word_bits = policy_type::word_bits;
    typedef typename policy_type::word_type word_type;

    static unsigned const state_bits = policy_type::state_bits;
    static unsigned const state_words = policy_type::state_words;
    typedef typename policy_type::state_type state_type;

    static unsigned const block_bits = policy_type::block_bits;
    static unsigned const block_words = policy_type::block_words;
    typedef typename policy_type::block_type block_type;

  public:
    void
    operator()(state_type &state,
               block_type const &block) {
        process_block(state, block);
    }

  private:
    static void
    process_block(state_type &state,
                  block_type const &block) {
#ifdef BOOST_HASH_SHOW_PROGRESS
        printf("Xoring the following block to the state:\n");
        for (unsigned i = 0; i < block.size(); ++i) {
            printf("%.8x%c", block[i], (i+1) != block.size() ? ' ' : '\n');
        }
#endif
        for (unsigned i = 0; i < block_words; ++i) {
            state[i] ^= block[i];
        }
        policy_type::transform_r(state);
    }

};

template <unsigned r, unsigned b, unsigned h>
struct cubehash_finalizer {
    typedef detail::cubehash_policy<r, b, h> policy_type;
    typedef typename policy_type::state_type state_type;
    void operator()(state_type &state) const {
        state[31] ^= 1;
        policy_type::transform_10r(state);
    }
};

//
// If the second and third parameters are unspecified (or left 0), then
// the first parameter is the number of bits in the digest, and
// r and b will be set to defaults.
//
// Otherwise the three parameters are r, b, and h respectively.
//

template <unsigned, unsigned = 0, unsigned = 0>
struct cubehash;

template <unsigned r, unsigned b, unsigned h>
struct cubehash {
  private:
    typedef detail::cubehash_policy<r, b, h> policy_type;
  public:
    typedef merkle_damgard_block_hash<
                stream_endian::little_octet_big_bit,
                policy_type::digest_bits,
                typename policy_type::iv_generator,
                cubehash_compressor<r, b, h>,
                cubehash_finalizer<r, b, h>
            > block_hash_type_;
#ifdef BOOST_HASH_NO_HIDE_INTERNAL_TYPES
    typedef block_hash_type_ block_hash_type;
#else
    struct block_hash_type : block_hash_type_ {};
#endif
    template <unsigned value_bits>
    struct stream_hash {
        typedef stream_preprocessor<
                    stream_endian::little_octet_big_bit,
                    value_bits,
                    0, // No length padding!
                    block_hash_type
                > type_;
#ifdef BOOST_HASH_NO_HIDE_INTERNAL_TYPES
        typedef type_ type;
#else
        struct type : type_ {};
#endif
    };
    typedef typename block_hash_type::digest_type digest_type;
};

template <unsigned h>
struct cubehash<h, 0, 0>
 : cubehash<BOOST_HASH_CUBEHASH_DEFAULT_R,
            BOOST_HASH_CUBEHASH_DEFAULT_B,
            h> {};

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_CUBEHASH_HPP
