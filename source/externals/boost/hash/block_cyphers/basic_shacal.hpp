
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_BASIC_SHACAL_HPP
#define BOOST_HASH_BLOCK_CYPHERS_BASIC_SHACAL_HPP

#include <boost/hash/block_cyphers/detail/shacal_policy.hpp>
#include <boost/hash/block_cyphers/detail/shacal1_policy.hpp>
#include <boost/static_assert.hpp>

#ifdef BOOST_HASH_SHOW_PROGRESS
#include <cstdio>
#endif

//
// Encrypt implemented directly from the SHA standard as found at
// http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
//
// Decrypt is a straight-forward inverse
//
// In SHA terminology:
// - plaintext = H^(i-1)
// - cyphertext = H^(i)
// - key = M^(i)
// - schedule = W
//

namespace boost {
namespace hashes {
namespace block_cyphers {

//
// The algorithms for SHA(-0) and SHA-1 are identical apart from the
// key scheduling, so encapsulate that as a class that takes an
// already-prepared schedule.  (Constructor is protected to help keep
// people from accidentally giving it just a key in a schedule.)
//

class basic_shacal {
  public:
    typedef detail::shacal_policy policy_type;

    static unsigned const word_bits = policy_type::word_bits;
    typedef policy_type::word_type word_type;

    static unsigned const key_bits = policy_type::key_bits;
    static unsigned const key_words = policy_type::key_words;
    typedef policy_type::key_type key_type;

    static unsigned const block_bits = policy_type::block_bits;
    static unsigned const block_words = policy_type::block_words;
    typedef policy_type::block_type block_type;

    static unsigned const rounds = policy_type::rounds;
    typedef policy_type::schedule_type schedule_type;

  protected:
    basic_shacal(schedule_type const &s) : schedule(s) {}
  private:
    schedule_type const schedule;

  public:
    block_type
    encypher(block_type const &plaintext) {
        return encypher_block(plaintext);
    }
  private:
    block_type
    encypher_block(block_type const &plaintext) {
        return encypher_block(schedule, plaintext);
    }
    static block_type
    encypher_block(schedule_type const &schedule,
                   block_type const &plaintext) {

#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < block_words; ++t) {
            std::printf(word_bits == 32 ?
                        "H[%d] = %.8x\n" :
                        "H[%d] = %.16lx\n",
                        t, plaintext[t]);
        }
#endif

        // Initialize working variables with block
        word_type a = plaintext[0], b = plaintext[1],
                  c = plaintext[2], d = plaintext[3],
                  e = plaintext[4];

        // Encypher block
#ifdef BOOST_HASH_NO_OPTIMIZATION

        for (unsigned t = 0; t < rounds; ++t) {
            word_type T = policy_type::ROTL<5>(a)
                        + policy_type::f(t,b,c,d)
                        + e
                        + policy_type::constant(t)
                        + schedule[t];

            e = d;
            d = c;
            c = policy_type::ROTL<30>(b);
            b = a;
            a = T;

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf(word_bits == 32 ?
                   "t = %2d: %.8x %.8x %.8x %.8x %.8x\n" :
                   "t = %2d: %.16lx %.16lx %.16lx %.16lx %.16lx\n",
                   t, a, b, c, d, e);
#endif
        }

#else // BOOST_HASH_NO_OPTIMIZATION

#   ifdef BOOST_HASH_SHOW_PROGRESS
#       define BOOST_HASH_SHACAL1_TRANSFORM_PROGRESS \
            printf(word_bits == 32 ? \
                   "t = %2d: %.8x %.8x %.8x %.8x %.8x\n" : \
                   "t = %2d: %.16lx %.16lx %.16lx %.16lx %.16lx\n", \
                   t, a, b, c, d, e);
#   else
#       define BOOST_HASH_SHACAL1_TRANSFORM_PROGRESS
#   endif

#   define BOOST_HASH_SHACAL1_TRANSFORM \
            word_type T = policy_type::ROTL<5>(a) \
                        + policy_type::f(t,b,c,d) \
                        + e \
                        + policy_type::constant(t) \
                        + schedule[t]; \
            e = d; \
            d = c; \
            c = policy_type::ROTL<30>(b); \
            b = a; \
            a = T; \
            BOOST_HASH_SHACAL1_TRANSFORM_PROGRESS

        BOOST_STATIC_ASSERT(rounds == 80);
        BOOST_STATIC_ASSERT(rounds % block_words == 0);
        for (unsigned t =  0; t < 20; ) {
            for (int n = block_words; n--; ++t) {
                BOOST_HASH_SHACAL1_TRANSFORM
            }
        }
        for (unsigned t = 20; t < 40; ) {
            for (int n = block_words; n--; ++t) {
                BOOST_HASH_SHACAL1_TRANSFORM
            }
        }
        for (unsigned t = 40; t < 60; ) {
            for (int n = block_words; n--; ++t) {
                BOOST_HASH_SHACAL1_TRANSFORM
            }
        }
        for (unsigned t = 60; t < 80; ) {
            for (int n = block_words; n--; ++t) {
                BOOST_HASH_SHACAL1_TRANSFORM
            }
        }

#endif

        block_type cyphertext = {{a, b, c, d, e}};
        return cyphertext;
    }

  public:
    block_type
    decypher(block_type const &plaintext) {
        return decypher_block(plaintext);
    }
  private:
    block_type
    decypher_block(block_type const &plaintext) {
        return decypher_block(schedule, plaintext);
    }
    static block_type
    decypher_block(schedule_type const &schedule,
                   block_type const &cyphertext) {

#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < block_words; ++t) {
            std::printf(word_bits == 32 ?
                        "H[%d] = %.8x\n" :
                        "H[%d] = %.16lx\n",
                        t, cyphertext[t]);
        }
#endif

        // Initialize working variables with block
        word_type a = cyphertext[0], b = cyphertext[1],
                  c = cyphertext[2], d = cyphertext[3],
                  e = cyphertext[4];

        // Decypher block
        for (unsigned t = rounds; t--; ) {
            word_type T = a;

            a = b;
            b = policy_type::ROTR<30>(c);
            c = d;
            d = e;
            e = T
              - policy_type::ROTL<5>(a)
              - policy_type::f(t, b, c, d)
              - policy_type::constant(t)
              - schedule[t];

#ifdef BOOST_HASH_SHOW_PROGRESS
            std::printf(word_bits == 32 ?
                        "t = %2d: %.8x %.8x %.8x %.8x %.8x\n" :
                        "t = %2d: %.16lx %.16lx %.16lx %.16lx %.16lx\n",
                        t, a, b, c, d, e);
#endif
        }

        block_type plaintext = {{a, b, c, d, e}};
        return plaintext;
    }

};

} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_BASIC_SHACAL_HPP
