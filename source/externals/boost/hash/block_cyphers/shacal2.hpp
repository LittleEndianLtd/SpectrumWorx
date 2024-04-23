
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_SHACAL2_HPP
#define BOOST_HASH_BLOCK_CYPHERS_SHACAL2_HPP

#include <boost/hash/block_cyphers/detail/shacal2_policy.hpp>
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

template <unsigned Version>
class shacal2 {
  public:
    static unsigned const version = Version;
    typedef detail::shacal2_policy<version> policy_type;

    static unsigned const word_bits = policy_type::word_bits;
    typedef typename policy_type::word_type word_type;

    static unsigned const key_bits = policy_type::key_bits;
    static unsigned const key_words = policy_type::key_words;
    typedef typename policy_type::key_type key_type;

    static unsigned const block_bits = policy_type::block_bits;
    static unsigned const block_words = policy_type::block_words;
    typedef typename policy_type::block_type block_type;

    static unsigned const rounds = policy_type::rounds;
    typedef typename policy_type::schedule_type schedule_type;

  public:
    shacal2(key_type const &key)
     : schedule(build_schedule(key)) {}
    shacal2(schedule_type s)
     : schedule((prepare_schedule(s), s)) {}
  private:
    static schedule_type
    build_schedule(key_type const &key) {
        // Copy key into beginning of schedule
        schedule_type schedule;
        for (unsigned t = 0; t < key_words; ++t) {
            schedule[t] = key[t];
        }
        prepare_schedule(schedule);
        return schedule;
    }
    static void
    prepare_schedule(schedule_type &schedule) {
#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < key_words; ++t) {
            std::printf(word_bits == 32 ?
                        "W[%2d] = %.8x\n" :
                        "W[%2d] = %.16lx\n",
                        t, schedule[t]);
        }
#endif

        for (unsigned t = key_words; t < rounds; ++t) {
            schedule[t] = policy_type::sigma_1(schedule[t-2])
                        + schedule[t-7]
                        + policy_type::sigma_0(schedule[t-15])
                        + schedule[t-16];
        }
    }
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
                  e = plaintext[4], f = plaintext[5],
                  g = plaintext[6], h = plaintext[7];

        // Encypher block
#ifdef BOOST_HASH_NO_OPTIMIZATION

        for (unsigned t = 0; t < rounds; ++t) {
            word_type T1 = h
                         + policy_type::Sigma_1(e)
                         + policy_type::Ch(e, f, g)
                         + policy_type::constant(t)
                         + schedule[t];
            word_type T2 = policy_type::Sigma_0(a)
                         + policy_type::Maj(a, b, c);

            h = g;
            g = f;
            f = e;
            e = d + T1;
            d = c;
            c = b;
            b = a;
            a = T1 + T2;

#ifdef BOOST_HASH_SHOW_PROGRESS
            std::printf(word_bits == 32 ?
                        "t = %2d: %.8x %.8x %.8x %.8x"
                                " %.8x %.8x %.8x %.8x\n" :
                        "t = %2d: %.16lx %.16lx %.16lx %.16lx"
                                " %.16lx %.16lx %.16lx %.16lx\n",
                        t, a, b, c, d,
                           e, f, g, h);
#endif
        }

#else // BOOST_HASH_NO_OPTIMIZATION

        BOOST_STATIC_ASSERT(rounds % block_words == 0);
        for (unsigned t = 0; t < rounds; ) {
            for (int n = block_words; n--; ++t) {
                word_type T1 = h
                             + policy_type::Sigma_1(e)
                             + policy_type::Ch(e, f, g)
                             + policy_type::constant(t)
                             + schedule[t];
                word_type T2 = policy_type::Sigma_0(a)
                             + policy_type::Maj(a, b, c);

                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;

#ifdef BOOST_HASH_SHOW_PROGRESS
                std::printf(word_bits == 32 ?
                            "t = %2d: %.8x %.8x %.8x %.8x"
                                    " %.8x %.8x %.8x %.8x\n" :
                            "t = %2d: %.16lx %.16lx %.16lx %.16lx"
                                    " %.16lx %.16lx %.16lx %.16lx\n",
                            t, a, b, c, d,
                               e, f, g, h);
#endif
            }
        }

#endif

        block_type cyphertext = {{a, b, c, d, e, f, g, h}};
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
                  e = cyphertext[4], f = cyphertext[5],
                  g = cyphertext[6], h = cyphertext[7];

        // Decypher block
        for (unsigned t = rounds; t--; ) {
            word_type T2 = policy_type::Sigma_0(b)
                         + policy_type::Maj(b, c, d);
            word_type T1 = a - T2;

            a = b;
            b = c;
            c = d;
            d = e - T1;
            e = f;
            f = g;
            g = h;
            h = T1
              - policy_type::Sigma_1(e)
              - policy_type::Ch(e, f, g)
              - policy_type::constant(t)
              - schedule[t];

#ifdef BOOST_HASH_SHOW_PROGRESS
            std::printf(word_bits == 32 ?
                        "t = %2d: %.8x %.8x %.8x %.8x"
                                " %.8x %.8x %.8x %.8x\n" :
                        "t = %2d: %.16lx %.16lx %.16lx %.16lx"
                                " %.16lx %.16lx %.16lx %.16lx\n",
                        t, a, b, c, d,
                           e, f, g, h);
#endif
        }

        block_type plaintext = {{a, b, c, d, e, f, g, h}};
        return plaintext;
    }

};

} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_SHACAL2_HPP
