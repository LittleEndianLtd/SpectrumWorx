
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DETAIL_PRIMES_HPP
#define BOOST_HASH_DETAIL_PRIMES_HPP

#include <boost/integer.hpp>

namespace boost {
namespace hashes {
namespace detail {

template <int Bits>
struct all_ones {
    typedef typename uint_t<Bits>::least type;
    static type const value = (type(all_ones<Bits-1>::value) << 1) | 1;
};
template <>
struct all_ones<0> {
    typedef uint_t<0>::least type;
    static type const value = 0;
};

template <int Bits>
struct largest_prime;

#define BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(B, D) \
    template <> \
    struct largest_prime<B> { \
        static uint_t<B>::least const value = all_ones<B>::value - D; \
    }

// http://primes.utm.edu/lists/2small/0bit.html or
// http://www.research.att.com/~njas/sequences/A013603
// Though those offets are from 2**b; This code is offsets from 2**b-1
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 2, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 3, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 4, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 5, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 6, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 7, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 8, 4);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET( 9, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(10, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(11, 8);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(12, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(13, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(14, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(15, 18);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(16, 14);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(17, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(18, 4);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(19, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(20, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(21, 8);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(22, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(23, 14);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(24, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(25, 38);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(26, 4);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(27, 38);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(28, 56);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(29, 2);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(30, 34);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(31, 0);
BOOST_HASH_DEFINE_LARGEST_PRIME_BY_OFFSET(32, 4);

} // namespace detail
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DETAIL_PRIMES_HPP
