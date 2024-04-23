
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_BLOCK_CYPHERS_DETAIL_BASIC_FUNCTIONS_HPP
#define BOOST_HASH_BLOCK_CYPHERS_DETAIL_BASIC_FUNCTIONS_HPP

#include <boost/integer.hpp>
#include <boost/static_assert.hpp>

namespace boost {
namespace hashes {
namespace block_cyphers {
namespace detail {

template <unsigned word_bits_N>
struct basic_functions {
    static unsigned const word_bits = word_bits_N;
    typedef typename uint_t<word_bits>::exact word_type;

    static word_type SHR(word_type x, unsigned n) {
        return x >> n;
    }
    template <unsigned n>
    static word_type SHR(word_type x) {
        BOOST_STATIC_ASSERT(n < word_bits);
        return x >> n;
    }
    static word_type SHL(word_type x, unsigned n) {
        return x << n;
    }
    template <unsigned n>
    static word_type SHL(word_type x) {
        BOOST_STATIC_ASSERT(n < word_bits);
        return x << n;
    }

    static word_type ROTR(word_type x, unsigned n) {
        return SHR(x, n) | SHL(x, word_bits-n);
    }
    template <unsigned n>
    static word_type ROTR(word_type x) {
        return SHR<n>(x) | SHL<word_bits-n>(x);
    }
    static word_type ROTL(word_type x, unsigned n) {
        return SHL(x, n) | SHR(x, word_bits-n);
    }
    template <unsigned n>
    static word_type ROTL(word_type x) {
        return SHL<n>(x) | SHR<word_bits-n>(x);
    }
};

} // namespace detail
} // namespace block_cyphers
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_BLOCK_CYPHERS_DETAIL_BASIC_FUNCTIONS_HPP
