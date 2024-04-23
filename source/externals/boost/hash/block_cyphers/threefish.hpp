
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_THREEFISH_HPP
#define BOOST_HASH_BLOCK_CYPHERS_THREEFISH_HPP

#include <boost/hash/block_cyphers/detail/threefish_policy.hpp>

#include <boost/cstdint.hpp>

#ifdef BOOST_HASH_SHOW_PROGRESS
#include <cstdio>
#endif

//
// Encrypt implemented directly from the Skein standard as found at
// http://www.skein-hash.info/sites/default/files/skein1.2.pdf
//

namespace boost {
namespace hashes {
namespace block_cyphers {

template <unsigned Version>
class threefish {
  public:
    static unsigned const version = Version;
    typedef detail::threefish_policy<version> policy_type;

    static unsigned const word_bits = policy_type::word_bits;
    typedef typename policy_type::word_type word_type;

    static unsigned const key_bits = policy_type::key_bits;
    static unsigned const key_words = policy_type::key_words;
    typedef typename policy_type::key_type key_type;

    static unsigned const block_bits = policy_type::block_bits;
    static unsigned const block_words = policy_type::block_words;
    typedef typename policy_type::block_type block_type;

    static unsigned const tweak_bits = policy_type::tweak_bits;
    static unsigned const tweak_words = policy_type::tweak_words;
    typedef typename policy_type::tweak_type tweak_type;

    static unsigned const rounds = policy_type::rounds;
    typedef typename policy_type::key_schedule_type key_schedule_type;
    typedef typename policy_type::tweak_schedule_type tweak_schedule_type;

  public:
    threefish(key_type const &key, tweak_type const &tweak = tweak_type()) {
        set_key(key);
        set_tweak(tweak);
    }
  public:
    void
    set_tweak(tweak_type const &t) {
        tweak_schedule[0] = t[0];
        tweak_schedule[1] = t[1];
        tweak_schedule[2] = t[0] ^ t[1];
#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t <= tweak_words; ++t) {
            std::printf("t_%d = %.16lx\n",
                        t, tweak_schedule[t]);
        }
#endif
    }
  private:
    void
    set_key(key_type const &key) {
        word_type k_N_w = UINT64_C(0x5555555555555555);
        for (unsigned t = 0; t < key_words; ++t) {
            key_schedule[t] = key[t];
#ifdef BOOST_HASH_SHOW_PROGRESS
            std::printf("k_%-2d = %.16lx\n",
                        t, key_schedule[t]);
#endif
            k_N_w ^= key[t];
        }
        key_schedule[key_words] = k_N_w;
#ifdef BOOST_HASH_SHOW_PROGRESS
        std::printf("k_%-2d = %.16lx\n",
                    key_words, key_schedule[key_words]);
#endif
    }
    key_schedule_type key_schedule;
    tweak_schedule_type tweak_schedule;
  private:
    word_type k(unsigned s, unsigned i) {
        word_type x = key_schedule[ (s+i) % (key_words+1) ];
        switch (i) {
          default:
            return x;
          case block_words - 3:
            return x + tweak_schedule[ s % 3 ];
          case block_words - 2:
            return x + tweak_schedule[ (s+1) % 3 ];
          case block_words - 1:
            return x + s;
        }
    }

  public:
    block_type
    encypher(block_type const &plaintext) {
        return encypher_block(plaintext);
    }
  private:
    block_type
    encypher_block(block_type const &plaintext) {

        // Initialize working variables with block
        block_type v = plaintext;

#ifdef BOOST_HASH_SHOW_PROGRESS
        for (unsigned t = 0; t < block_words; ++t) {
            std::printf("v_0,%-2d = %.16lx\n",
                        t, v[t]);
        }
#endif

        // Encypher block
        for (unsigned d = 0; d < rounds; ) {
            // Add a subkey (when d%4 == 0)
            for (unsigned i = 0; i < block_words; ++i) {
                v[i] += k(d/4, i);
#ifdef BOOST_HASH_SHOW_PROGRESS
                std::printf("e_%d,%-2d = %.16lx\n",
                            d, i, v[i]);
#endif
            }

            // Unrolling by 4 is also useful as the permutations
            // have a cycle of 2 or 4 (see 8.3)
            for (unsigned q = 0; q < 4; ++q, ++d) {
                block_type f;
                // MIX into f
                for (unsigned j = 0; j < block_words/2; ++j) {
                    word_type x0 = v[2*j+0];
                    word_type x1 = v[2*j+1];
                    word_type y0 = x0 + x1;
                    unsigned r = policy_type::rotation(d%8, j);
                    word_type y1 = policy_type::ROTL(x1, r) ^ y0;
                    f[2*j+0] = y0;
                    f[2*j+1] = y1;
#ifdef BOOST_HASH_SHOW_PROGRESS
                    std::printf("f_%d,%-2d = %.16lx\n",
                                d, 2*j+0, f[2*j+0]);
                    std::printf("f_%d,%-2d = %.16lx\n",
                                d, 2*j+1, f[2*j+1]);
#endif
                }
                // PERMUTE back into v
                for (unsigned i = 0; i < block_words; ++i) {
                    unsigned pi = policy_type::permutation(i);
                    v[i] = f[pi];
#ifdef BOOST_HASH_SHOW_PROGRESS
                    std::printf("v_%d,%-2d = %.16lx\n",
                                d+1, i, v[i]);
#endif
                }
            }
        }

        block_type cyphertext;
        // Add final subkey
        for (unsigned i = 0; i < block_words; ++i) {
            cyphertext[i] = v[i] + k(rounds/4, i);
#ifdef BOOST_HASH_SHOW_PROGRESS
                std::printf("c_%-2d = %.16lx\n",
                            i, cyphertext[i]);
#endif
        }
        return cyphertext;
    }

  public:
    block_type
    decypher(block_type const &plaintext) {
        return decypher_block(plaintext);
    }
  private:
    block_type
    decypher_block(block_type const &cyphertext) {
        for (unsigned i = 0; i < block_words; ++i) {
#ifdef BOOST_HASH_SHOW_PROGRESS
                std::printf("c_%-2d = %.16lx\n",
                            i, cyphertext[i]);
#endif
        }

        block_type v;

        // Remove final subkey
        for (unsigned i = 0; i < block_words; ++i) {
            v[i] = cyphertext[i] - k(rounds/4, i);
#ifdef BOOST_HASH_SHOW_PROGRESS
                std::printf("v_%d,%-2d = %.16lx\n",
                            rounds, i, v[i]);
#endif
        }

        // Decypher block
        for (unsigned d = rounds; d; ) {
            // Unrolling by 4 is also useful as the permutations
            // have a cycle of 2 or 4 (see 8.3)
            for (unsigned q = 4; q--; ) {
                --d;

                block_type f;
                // PERMUTE back into f
                for (unsigned i = 0; i < block_words; ++i) {
                    unsigned pi = policy_type::permutation(i);
                    f[pi] = v[i];
#ifdef BOOST_HASH_SHOW_PROGRESS
                    std::printf("f_%d,%-2d = %.16lx\n",
                                d, pi, f[pi]);
#endif
                }

                // UNMIX into v
                for (unsigned j = 0; j < block_words/2; ++j) {
                    word_type y0 = f[2*j+0];
                    word_type y1 = f[2*j+1];

                    unsigned r = policy_type::rotation(d%8, j);
                    word_type x1 = policy_type::ROTR(y0 ^ y1, r);
                    word_type x0 = y0 - x1;

                    v[2*j+0] = x0;
                    v[2*j+1] = x1;

#ifdef BOOST_HASH_SHOW_PROGRESS
                    std::printf("e_%d,%-2d = %.16lx\n",
                                d, 2*j+0, v[2*j+0]);
                    std::printf("e_%d,%-2d = %.16lx\n",
                                d, 2*j+1, v[2*j+1]);
#endif
                }
            }

            // Remove a subkey (when d%4 == 0)
            for (unsigned i = 0; i < block_words; ++i) {
                v[i] -= k(d/4, i);
#ifdef BOOST_HASH_SHOW_PROGRESS
                std::printf("d_%d,%-2d = %.16lx\n",
                            d, i, v[i]);
#endif
            }
        }

        block_type plaintext = v;
        return plaintext;
    }

};

} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_THREEFISH_HPP
