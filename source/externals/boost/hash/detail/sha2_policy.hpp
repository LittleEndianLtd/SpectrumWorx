
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DETAIL_SHA2_POLICY_HPP
#define BOOST_HASH_DETAIL_SHA2_POLICY_HPP

#include <boost/hash/block_cyphers/detail/shacal2_policy.hpp>
#include <boost/hash/digest.hpp>

#include <boost/cstdint.hpp>

namespace boost {
namespace hashes {
namespace detail {

template <unsigned CypherVersion>
struct basic_sha2_policy {

    static unsigned const cypher_version = CypherVersion;

    typedef block_cyphers::detail::shacal2_policy<cypher_version> cypher_policy;
    typedef typename cypher_policy::block_type state_type;

};

template <unsigned Version>
struct sha2_policy;

template <>
struct sha2_policy<224> : basic_sha2_policy<256> {

    static unsigned const digest_bits = 224;
    typedef digest<digest_bits> digest_type;

    struct iv_generator {
        state_type const &
        operator()() const {
            static state_type const H0 = {{
                0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
                0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4,
            }};
            return H0;
        }
    };

};

template <>
struct sha2_policy<256> : basic_sha2_policy<256> {

    static unsigned const digest_bits = 256;
    typedef digest<digest_bits> digest_type;

    struct iv_generator {
        state_type const &
        operator()() const {
            static state_type const H0 = {{
                0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
                0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
            }};
            return H0;
        }
    };

};

template <>
struct sha2_policy<384> : basic_sha2_policy<512> {

    static unsigned const digest_bits = 384;
    typedef digest<digest_bits> digest_type;

    struct iv_generator {
        state_type const &
        operator()() const {
            static state_type const H0 = {{
                UINT64_C(0xcbbb9d5dc1059ed8), UINT64_C(0x629a292a367cd507),
                UINT64_C(0x9159015a3070dd17), UINT64_C(0x152fecd8f70e5939),
                UINT64_C(0x67332667ffc00b31), UINT64_C(0x8eb44a8768581511),
                UINT64_C(0xdb0c2e0d64f98fa7), UINT64_C(0x47b5481dbefa4fa4),
            }};
            return H0;
        }
    };

};

template <>
struct sha2_policy<512> : basic_sha2_policy<512> {

    static unsigned const digest_bits = 512;
    typedef digest<digest_bits> digest_type;

    struct iv_generator {
        state_type const &
        operator()() const {
            static state_type const H0 = {{
                UINT64_C(0x6a09e667f3bcc908), UINT64_C(0xbb67ae8584caa73b),
                UINT64_C(0x3c6ef372fe94f82b), UINT64_C(0xa54ff53a5f1d36f1),
                UINT64_C(0x510e527fade682d1), UINT64_C(0x9b05688c2b3e6c1f),
                UINT64_C(0x1f83d9abfb41bd6b), UINT64_C(0x5be0cd19137e2179),
            }};
            return H0;
        }
    };

};

} // namespace detail
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DETAIL_SHA2_POLICY_HPP
