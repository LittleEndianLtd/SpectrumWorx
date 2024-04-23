
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_MD5_HPP
#define BOOST_HASH_BLOCK_CYPHERS_MD5_HPP

#include <boost/hash/block_cyphers/detail/md5_policy.hpp>

#ifdef BOOST_HASH_SHOW_PROGRESS
#include <cstdio>
#endif

//
// Encrypt implemented directly from the RFC as found at
// http://www.faqs.org/rfcs/rfc1321.html
//
// Decrypt is a straight-forward inverse
//
// In MD5 terminology:
// - plaintext = AA, BB, CC, and DD
// - cyphertext = A, B, C, and D
// - key = M^(i) and X
//

namespace boost {
namespace hashes {
namespace block_cyphers {

class md5 {
  public:
    typedef detail::md5_policy policy_type;

    static unsigned const word_bits = policy_type::word_bits;
    typedef policy_type::word_type word_type;

    static unsigned const key_bits = policy_type::key_bits;
    static unsigned const key_words = policy_type::key_words;
    typedef policy_type::key_type key_type;

    static unsigned const block_bits = policy_type::block_bits;
    static unsigned const block_words = policy_type::block_words;
    typedef policy_type::block_type block_type;

  public:
    md5(key_type const &k) : key(k) {
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
#define BOOST_HASH_MD5_ENCYPHER_STEP(aa, bb, cc, dd, fun, k, s, i) \
            { \
                word_type T = aa \
                            + policy_type::fun(bb,cc,dd) \
                            + key[policy_type::key_index(k)] \
                            + policy_type::constant(i-1); \
                aa = bb + policy_type::ROTL<s>(T); \
            }
        for (unsigned t =  0; t < 16; t += 4) {
            BOOST_HASH_MD5_ENCYPHER_STEP(a, b, c, d, ff, t+0,  7, t+1)
            BOOST_HASH_MD5_ENCYPHER_STEP(d, a, b, c, ff, t+1, 12, t+2)
            BOOST_HASH_MD5_ENCYPHER_STEP(c, d, a, b, ff, t+2, 17, t+3)
            BOOST_HASH_MD5_ENCYPHER_STEP(b, c, d, a, ff, t+3, 22, t+4)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 1: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 16; t < 32; t += 4) {
            BOOST_HASH_MD5_ENCYPHER_STEP(a, b, c, d, gg, t+0,  5, t+1)
            BOOST_HASH_MD5_ENCYPHER_STEP(d, a, b, c, gg, t+1,  9, t+2)
            BOOST_HASH_MD5_ENCYPHER_STEP(c, d, a, b, gg, t+2, 14, t+3)
            BOOST_HASH_MD5_ENCYPHER_STEP(b, c, d, a, gg, t+3, 20, t+4)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 2: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 32; t < 48; t += 4) {
            BOOST_HASH_MD5_ENCYPHER_STEP(a, b, c, d, hh, t+0,  4, t+1)
            BOOST_HASH_MD5_ENCYPHER_STEP(d, a, b, c, hh, t+1, 11, t+2)
            BOOST_HASH_MD5_ENCYPHER_STEP(c, d, a, b, hh, t+2, 16, t+3)
            BOOST_HASH_MD5_ENCYPHER_STEP(b, c, d, a, hh, t+3, 23, t+4)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 3: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 48; t < 64; t += 4) {
            BOOST_HASH_MD5_ENCYPHER_STEP(a, b, c, d, ii, t+0,  6, t+1)
            BOOST_HASH_MD5_ENCYPHER_STEP(d, a, b, c, ii, t+1, 10, t+2)
            BOOST_HASH_MD5_ENCYPHER_STEP(c, d, a, b, ii, t+2, 15, t+3)
            BOOST_HASH_MD5_ENCYPHER_STEP(b, c, d, a, ii, t+3, 21, t+4)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 4: %.8x %.8x %.8x %.8x\n",
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
#define BOOST_HASH_MD5_DECYPHER_STEP(aa, bb, cc, dd, fun, k, s, i) \
            { \
                word_type T = policy_type::ROTR<s>(aa - bb); \
                aa = T \
                   - policy_type::fun(bb,cc,dd) \
                   - key[policy_type::key_index(k)] \
                   - policy_type::constant(i-1); \
            }
        for (unsigned t = 64; t -= 4, t >= 48; ) {
            BOOST_HASH_MD5_DECYPHER_STEP(b, c, d, a, ii, t+3, 21, t+4)
            BOOST_HASH_MD5_DECYPHER_STEP(c, d, a, b, ii, t+2, 15, t+3)
            BOOST_HASH_MD5_DECYPHER_STEP(d, a, b, c, ii, t+1, 10, t+2)
            BOOST_HASH_MD5_DECYPHER_STEP(a, b, c, d, ii, t+0,  6, t+1)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 4: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 48; t -= 4, t >= 32; ) {
            BOOST_HASH_MD5_DECYPHER_STEP(b, c, d, a, hh, t+3, 23, t+4)
            BOOST_HASH_MD5_DECYPHER_STEP(c, d, a, b, hh, t+2, 16, t+3)
            BOOST_HASH_MD5_DECYPHER_STEP(d, a, b, c, hh, t+1, 11, t+2)
            BOOST_HASH_MD5_DECYPHER_STEP(a, b, c, d, hh, t+0,  4, t+1)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 3: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 32; t -= 4, t >= 16; ) {
            BOOST_HASH_MD5_DECYPHER_STEP(b, c, d, a, gg, t+3, 20, t+4)
            BOOST_HASH_MD5_DECYPHER_STEP(c, d, a, b, gg, t+2, 14, t+3)
            BOOST_HASH_MD5_DECYPHER_STEP(d, a, b, c, gg, t+1,  9, t+2)
            BOOST_HASH_MD5_DECYPHER_STEP(a, b, c, d, gg, t+0,  5, t+1)

#ifdef BOOST_HASH_SHOW_PROGRESS
            printf("Round 2: %.8x %.8x %.8x %.8x\n",
                   a, b, c, d);
#endif
        }
        for (unsigned t = 16; t -= 4, t < 16; ) {
            BOOST_HASH_MD5_DECYPHER_STEP(b, c, d, a, ff, t+3, 22, t+4)
            BOOST_HASH_MD5_DECYPHER_STEP(c, d, a, b, ff, t+2, 17, t+3)
            BOOST_HASH_MD5_DECYPHER_STEP(d, a, b, c, ff, t+1, 12, t+2)
            BOOST_HASH_MD5_DECYPHER_STEP(a, b, c, d, ff, t+0,  7, t+1)

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

#endif // BOOST_HASH_BLOCK_CYPHERS_MD5_HPP
