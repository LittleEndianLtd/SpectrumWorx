
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_DETAIL_SHACAL_FUNCTIONS_HPP
#define BOOST_HASH_BLOCK_CYPHERS_DETAIL_SHACAL_FUNCTIONS_HPP

#include <boost/hash/block_cyphers/detail/basic_functions.hpp>

namespace boost {
namespace hashes {
namespace block_cyphers {
namespace detail {

//
// Implemented directly from the standard as found at
// http://csrc.nist.gov/publications/fips/fips180-2/fips180-2.pdf
//

// Specifically, subsection 4.1

template <unsigned word_bits_>
struct basic_shacal_functions : basic_functions<word_bits_> {
    typedef typename basic_functions<word_bits_>::word_type word_type;

    static word_type Ch(word_type x, word_type y, word_type z) {
        return (x & y) ^ (~x & z);
    }
    static word_type Maj(word_type x, word_type y, word_type z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }
};

struct shacal_functions : public basic_shacal_functions<32> {
    static word_type Parity(word_type x, word_type y, word_type z) {
        return x ^ y ^ z;
    }
    static word_type f(unsigned t, word_type x, word_type y, word_type z) {
        if (t < 40) {
            if (t < 20) return Ch(x, y, z);
        } else {
            if (t < 60) return Maj(x, y, z);
        }
        return Parity(x, y, z);
    }
};

typedef shacal_functions shacal0_functions;
typedef shacal_functions shacal1_functions;

template <unsigned word_bits_>
struct shacal2_functions;
template <>
struct shacal2_functions<32> : public basic_shacal_functions<32> {
    static word_type Sigma_0(word_type x) {
        return ROTR< 2>(x) ^ ROTR<13>(x) ^ ROTR<22>(x);
    }
    static word_type Sigma_1(word_type x) {
        return ROTR< 6>(x) ^ ROTR<11>(x) ^ ROTR<25>(x);
    }

    static word_type sigma_0(word_type x) {
        return ROTR< 7>(x) ^ ROTR<18>(x) ^ SHR< 3>(x);
    }
    static word_type sigma_1(word_type x) {
        return ROTR<17>(x) ^ ROTR<19>(x) ^ SHR<10>(x);
    }
};
template <>
struct shacal2_functions<64> : public basic_shacal_functions<64> {
    static word_type Sigma_0(word_type x) {
        return ROTR<28>(x) ^ ROTR<34>(x) ^ ROTR<39>(x);
    }
    static word_type Sigma_1(word_type x) {
        return ROTR<14>(x) ^ ROTR<18>(x) ^ ROTR<41>(x);
    }

    static word_type sigma_0(word_type x) {
        return ROTR< 1>(x) ^ ROTR< 8>(x) ^ SHR< 7>(x);
    }
    static word_type sigma_1(word_type x) {
        return ROTR<19>(x) ^ ROTR<61>(x) ^ SHR< 6>(x);
    }
};

} // namespace detail
} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_DETAIL_SHACAL_FUNCTIONS_HPP
