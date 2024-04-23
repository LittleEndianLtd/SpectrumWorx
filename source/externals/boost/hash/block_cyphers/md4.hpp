
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_MD4_HPP
#define BOOST_HASH_BLOCK_CYPHERS_MD4_HPP

#include <boost/hash/block_cyphers/detail/md4_policy.hpp>

#ifdef BOOST_HASH_SHOW_PROGRESS
#include <cstdio>
#endif

//
// Encrypt implemented directly from the RFC as found at
// http://www.faqs.org/rfcs/rfc1320.html
//
// Decrypt is a straight-forward inverse
//
// In MD4 terminology:
// - plaintext = AA, BB, CC, and DD
// - cyphertext = A, B, C, and D
// - key = M^(i) and X
//

namespace boost {
namespace hashes {
namespace block_cyphers {

class md4 {
  public:
    typedef detail::md4_policy policy_type;

    static unsigned const word_bits = policy_type::word_bits;
    typedef policy_type::word_type word_type;

    static unsigned const key_bits = policy_type::key_bits;
    static unsigned const key_words = policy_type::key_words;
    typedef policy_type::key_type key_type;

    static unsigned const block_bits = policy_type::block_bits;
    static unsigned const block_words = policy_type::block_words;
    typedef policy_type::block_type block_type;

  public:
    md4(key_type const &k) : key(k) {
#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < key_words; ++t) {
            std::printf("X[%2d] = %.8x\n",
                        t, key[t]);
        }
#endif
    }
  private:
    key_type const key;

  public:
    block_type
    encypher(block_type const &plaintext) {
        return encypher_block(plaintext);
    }
  private:
    block_type
    encypher_block(block_type const &plaintext) {
        return encypher_block(key, plaintext);
    }
    static block_type
    encypher_block(key_type const &key,
                   block_type const &plaintext) {

#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < block_words; ++t) {
            std::printf("%c%c = %.8x\n",
                        'A'+t, 'A'+t, plaintext[t]);
        }
#endif

        // Initialize working variables with block
        word_type a = plaintext[0], b = plaintext[1],
                  c = plaintext[2], d = plaintext[3];

        // Encypher block
#define BOOST_HASH_MD4_ENCYPHER_STEP(aa, bb, cc, dd, fun, k, s, val) \
            { \
                word_type T = aa \
                            + policy_type::fun(bb,cc,dd) \
                            + key[policy_type::key_index(k)] \
                            + val; \
                aa = policy_type::ROTL<s>(T); \
            }
        for (unsigned t =  0; t < 16; t += 4) {
            BOOST_HASH_MD4_ENCYPHER_STEP(a, b, c, d, ff, t+0,  3, 0x00000000)
            BOOST_HASH_MD4_ENCYPHER_STEP(d, a, b, c, ff, t+1,  7, 0x00000000)
            BOOST_HASH_MD4_ENCYPHER_STEP(c, d, a, b, ff, t+2, 11, 0x00000000)
            BOOST_HASH_MD4_ENCYPHER_STEP(b, c, d, a, ff, t+3, 19, 0x00000000)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 1: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 16; t < 32; t += 4) {
            BOOST_HASH_MD4_ENCYPHER_STEP(a, b, c, d, gg, t+0,  3, 0x5a827999)
            BOOST_HASH_MD4_ENCYPHER_STEP(d, a, b, c, gg, t+1,  5, 0x5a827999)
            BOOST_HASH_MD4_ENCYPHER_STEP(c, d, a, b, gg, t+2,  9, 0x5a827999)
            BOOST_HASH_MD4_ENCYPHER_STEP(b, c, d, a, gg, t+3, 13, 0x5a827999)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 2: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 32; t < 48; t += 4) {
            BOOST_HASH_MD4_ENCYPHER_STEP(a, b, c, d, hh, t+0,  3, 0x6ed9eba1)
            BOOST_HASH_MD4_ENCYPHER_STEP(d, a, b, c, hh, t+1,  9, 0x6ed9eba1)
            BOOST_HASH_MD4_ENCYPHER_STEP(c, d, a, b, hh, t+2, 11, 0x6ed9eba1)
            BOOST_HASH_MD4_ENCYPHER_STEP(b, c, d, a, hh, t+3, 15, 0x6ed9eba1)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 3: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }

        block_type cyphertext = {{a, b, c, d}};
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
        return decypher_block(key, plaintext);
    }
    static block_type
    decypher_block(key_type const &key,
                   block_type const &cyphertext) {

#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < block_words; ++t) {
            std::printf("%c = %.8x\n",
                        'A'+t, cyphertext[t]);
        }
#endif

        // Initialize working variables with block
        word_type a = cyphertext[0], b = cyphertext[1],
                  c = cyphertext[2], d = cyphertext[3];

        // Decypher block
#define BOOST_HASH_MD4_DECYPHER_STEP(aa, bb, cc, dd, fun, k, s, val) \
            { \
                word_type T = policy_type::ROTR<s>(aa); \
                aa = T \
                   - policy_type::fun(bb,cc,dd) \
                   - key[policy_type::key_index(k)] \
                   - val; \
            }
        for (unsigned t = 48; t -= 4, t >= 32; ) {
            BOOST_HASH_MD4_DECYPHER_STEP(b, c, d, a, hh, t+3, 15, 0x6ed9eba1)
            BOOST_HASH_MD4_DECYPHER_STEP(c, d, a, b, hh, t+2, 11, 0x6ed9eba1)
            BOOST_HASH_MD4_DECYPHER_STEP(d, a, b, c, hh, t+1,  9, 0x6ed9eba1)
            BOOST_HASH_MD4_DECYPHER_STEP(a, b, c, d, hh, t+0,  3, 0x6ed9eba1)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 3: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 32; t -= 4, t >= 16; ) {
            BOOST_HASH_MD4_DECYPHER_STEP(b, c, d, a, gg, t+3, 13, 0x5a827999)
            BOOST_HASH_MD4_DECYPHER_STEP(c, d, a, b, gg, t+2,  9, 0x5a827999)
            BOOST_HASH_MD4_DECYPHER_STEP(d, a, b, c, gg, t+1,  5, 0x5a827999)
            BOOST_HASH_MD4_DECYPHER_STEP(a, b, c, d, gg, t+0,  3, 0x5a827999)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 2: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 16; t -= 4, t < 16; ) {
            BOOST_HASH_MD4_DECYPHER_STEP(b, c, d, a, ff, t+3, 19, 0x00000000)
            BOOST_HASH_MD4_DECYPHER_STEP(c, d, a, b, ff, t+2, 11, 0x00000000)
            BOOST_HASH_MD4_DECYPHER_STEP(d, a, b, c, ff, t+1,  7, 0x00000000)
            BOOST_HASH_MD4_DECYPHER_STEP(a, b, c, d, ff, t+0,  3, 0x00000000)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 1: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }

        block_type plaintext = {{a, b, c, d}};
        return plaintext;
    }

};

} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_MD4_HPP
