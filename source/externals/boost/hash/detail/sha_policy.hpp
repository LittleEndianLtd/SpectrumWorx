
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DETAIL_SHA_POLICY_HPP
#define BOOST_HASH_DETAIL_SHA_POLICY_HPP

#include <boost/hash/block_cyphers/detail/shacal_policy.hpp>
#include <boost/hash/digest.hpp>

namespace boost {
namespace hashes {
namespace detail {

struct sha_policy {

    typedef block_cyphers::detail::shacal_policy cypher_policy;
    typedef cypher_policy::block_type state_type;

    static unsigned const digest_bits = 160;
    typedef digest<digest_bits> digest_type;

    struct iv_generator {
        state_type const &
        operator()() const {
            // First 4 words are the same as MD4
            static state_type const H0 = {{
                0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476,
                0xc3d2e1f0,
            }};
            return H0;
        }
    };

};

typedef sha_policy sha0_policy;

} // namespace detail
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DETAIL_SHA_POLICY_HPP
