
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_SHA_HPP
#define BOOST_HASH_SHA_HPP

#include <boost/hash/block_cyphers/shacal.hpp>
#include <boost/hash/davies_meyer_compressor.hpp>
#include <boost/hash/detail/sha_policy.hpp>
#include <boost/hash/detail/state_adder.hpp>
#include <boost/hash/merkle_damgard_block_hash.hpp>
#include <boost/hash/stream_preprocessor.hpp>

namespace boost {
namespace hashes {

struct sha {
  private:
    typedef detail::sha_policy policy_type;
    typedef block_cyphers::shacal block_cypher_type;
  public:
    typedef merkle_damgard_block_hash<
                stream_endian::big_octet_big_bit,
                policy_type::digest_bits,
                policy_type::iv_generator,
                davies_meyer_compressor<block_cypher_type,
                                        detail::state_adder>
            > block_hash_type_;
#ifdef BOOST_HASH_NO_HIDE_INTERNAL_TYPES
    typedef block_hash_type_ block_hash_type;
#else
    struct block_hash_type : block_hash_type_ {};
#endif
    template <unsigned value_bits>
    struct stream_hash {
        typedef stream_preprocessor<
                    stream_endian::big_octet_big_bit,
                    value_bits,
                    block_hash_type::word_bits * 2,
                    block_hash_type
                > type_;
#ifdef BOOST_HASH_NO_HIDE_INTERNAL_TYPES
        typedef type_ type;
#else
        struct type : type_ {};
#endif
    };
    typedef block_hash_type::digest_type digest_type;
};
typedef sha sha0;

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_SHA_HPP
