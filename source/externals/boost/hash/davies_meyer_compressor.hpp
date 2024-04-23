
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DAVIES_MEYER_COMPRESSOR_HPP
#define BOOST_HASH_DAVIES_MEYER_COMPRESSOR_HPP

namespace boost {
namespace hashes {

//
// The Davies-Meyer construction turns a block cypher
// into a one-way compression function
//
// http://en.wikipedia.org/wiki/One-way_compression_function#Davies-Meyer
//

template <typename block_cypher_T, typename combine_F>
struct davies_meyer_compressor {
  public:
    typedef block_cypher_T block_cypher_type;

    static unsigned const word_bits = block_cypher_type::word_bits;
    typedef typename block_cypher_type::word_type word_type;

    static unsigned const state_bits = block_cypher_type::block_bits;
    static unsigned const state_words = block_cypher_type::block_words;
    typedef typename block_cypher_type::block_type state_type;

    static unsigned const block_bits = block_cypher_type::key_bits;
    static unsigned const block_words = block_cypher_type::key_words;
    typedef typename block_cypher_type::key_type block_type;

  public:
    void
    operator()(state_type &state,
               block_type const &block) {
        process_block(state, block);
    }

  private:
    static void
    process_block(state_type &state,
                  block_type const &block) {
        block_cypher_type cypher(block);
        state_type new_state = cypher.encypher((state_type const &)state);
        combine_F f;
        f(state, new_state);
    }
};

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DAVIES_MEYER_COMPRESSOR_HPP
