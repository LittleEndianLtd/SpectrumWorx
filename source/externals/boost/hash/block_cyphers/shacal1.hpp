
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_SHACAL1_HPP
#define BOOST_HASH_BLOCK_CYPHERS_SHACAL1_HPP

#include <boost/hash/block_cyphers/basic_shacal.hpp>

//
// Implemented directly from the SHA standard as found at
// http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
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

class shacal1 : public basic_shacal {
  public:
    shacal1(key_type const &k)
     : basic_shacal(build_schedule(k)) {}
    shacal1(schedule_type s)
     : basic_shacal((prepare_schedule(s), s)) {}
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
            schedule[t] = schedule[t-3]
                        ^ schedule[t-8]
                        ^ schedule[t-14]
                        ^ schedule[t-16];
            schedule[t] = policy_type::ROTL<1>(schedule[t]);
        }
    }
};

} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_SHACAL1_HPP
