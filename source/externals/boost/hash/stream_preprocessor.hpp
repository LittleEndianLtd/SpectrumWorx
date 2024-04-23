
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_STREAM_PREPROCESSOR_HPP
#define BOOST_HASH_STREAM_PREPROCESSOR_HPP

#include <boost/array.hpp>
#include <boost/hash/pack.hpp>
#include <boost/integer.hpp>
#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>

#include <iterator>

namespace boost {
namespace hashes {

//
// This will do the usual Merkle-Damgård-style strengthening, padding with
// a 1 bit, then 0 bits as needed, then, if requested, the length.
//
template <typename endian,
          unsigned value_bits_,
          unsigned length_bits_,
          typename block_hash_T>
class stream_preprocessor {
  private:
    typedef block_hash_T block_hash_type;

    static unsigned const word_bits = block_hash_type::word_bits;
    typedef typename block_hash_type::word_type word_type;

    static unsigned const block_bits = block_hash_type::block_bits;
    static unsigned const block_words = block_hash_type::block_words;
    typedef typename block_hash_type::block_type block_type;
  public:
    typedef typename block_hash_type::digest_type digest_type;

    static unsigned const value_bits = value_bits_;
    typedef typename uint_t<value_bits>::least value_type;
    BOOST_STATIC_ASSERT(word_bits % value_bits == 0);
    static unsigned const block_values = block_bits/value_bits;
    typedef array<value_type, block_values> value_array_type;

  private:

    static unsigned const length_bits = length_bits_;
    // FIXME: do something more intelligent than capping at 64
    static unsigned const length_type_bits =
        length_bits < word_bits ?
            word_bits :
        length_bits > 64 ?
            64 :
            length_bits;
    typedef typename uint_t<length_type_bits>::least length_type;
    static unsigned const length_words = length_bits/word_bits;
    BOOST_STATIC_ASSERT(!length_bits || length_bits % word_bits == 0);
    BOOST_STATIC_ASSERT(block_bits % value_bits == 0);

    BOOST_STATIC_ASSERT(!length_bits || value_bits <= length_bits);

  private:
    void
    process_block() {
        // Convert the input into words
        block_type block;
        pack<endian, value_bits, word_bits>(value_array, block);

        // Process the block
        block_hash.update(block);

        // Reset seen if we don't need to track the length
        if (!length_bits) seen = 0;
    }
    template <typename Dummy>
    typename enable_if_c<length_bits && sizeof(Dummy)>::type
    append_length(length_type length) {
        // Convert the input into words
        block_type block;
        pack<endian, value_bits, word_bits>(value_array, block);

        // Append length
        array<length_type, 1> length_array = {{length}};
        array<word_type, length_words> length_words_array;
        pack<endian, length_bits, word_bits>(length_array, length_words_array);
        for (unsigned i = length_words; i; --i) {
            block[block_words - i] = length_words_array[length_words - i];
        }

        // Process the last block
        block_hash.update(block);
    }
    template <typename Dummy>
    typename disable_if_c<length_bits && sizeof(Dummy)>::type
    append_length(length_type) {
        // No appending requested, so nothing to do
    }
  public:
    stream_preprocessor &
    update_one(value_type value) {
        unsigned i = seen % block_bits;
        unsigned j = i / value_bits;
        value_array[j] = value;
        seen += value_bits;
        if (i == block_bits - value_bits) {
            // Process the completed block
            process_block();
        }
        return *this;
    }
    template <typename IterT>
    stream_preprocessor &
    update_n(IterT p, size_t n) {
#ifndef BOOST_HASH_NO_OPTIMIZATION
        for ( ; n && (seen % block_bits); --n, ++p) {
            update_one(*p);
        }
        for ( ; n >= block_values; n -= block_values, p += block_values) {
            // Convert the input into words
            block_type block;
            pack_n<endian, value_bits, word_bits>(p, block_values,
                                                  &block[0], block_words);

            // Process the block
            block_hash.update(block);
            seen += block_bits;

            // Reset seen if we don't need to track the length
            if (!length_bits) seen = 0;
        }
#endif
        for ( ; n; --n, ++p) {
            update_one(*p);
        }
        return *this;
    }
    template <typename IterT>
    stream_preprocessor &
    update(IterT b, IterT e, std::random_access_iterator_tag) {
        return update_n(b, e-b);
    }
    template <typename IterT, typename Category>
    stream_preprocessor &
    update(IterT b, IterT e, Category) {
        while (b != e) update_one(*b++);
        return *this;
    }
    template <typename IterT>
    stream_preprocessor &
    update(IterT b, IterT e) {
        typedef typename std::iterator_traits<IterT>::iterator_category cat;
        return update(b, e, cat());
    }
    template <typename ContainerT>
    stream_preprocessor &
    update(ContainerT const &c) {
        return update_n(c.data(), c.size());
    }    
    digest_type
    end_message() {
        length_type length = seen;

        // Add a 1 bit
#ifdef BOOST_HASH_NO_OPTIMIZATION
        array<bool, value_bits> padding_bits = {{1}};
        array<value_type, 1> padding_values;
        pack<endian, 1, value_bits>(padding_bits, padding_values);
        update_one(padding_values[0]);
#else
        value_type pad = 0;
        detail::imploder_step<endian, 1, value_bits, 0>::step(1, pad);
        update_one(pad);
#endif

        // Pad with 0 bits
        while ((seen + length_bits) % block_bits != 0) {
            update_one(value_type());
        }

        // Append length
        append_length<int>(length);

        // Reset for next message
        seen = 0;

        // Calculate digest and reset block_hash
        return block_hash.end_message();
    }
    digest_type digest() const {
        return stream_preprocessor(*this).end_message();
    }

  public:
    stream_preprocessor() : value_array(), block_hash(), seen() {}
    void
    reset() {
        seen = 0;
        block_hash.reset();
    }

  private:
    value_array_type value_array;
    block_hash_type block_hash;
    length_type seen;
};

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_STREAM_PREPROCESSOR_HPP
