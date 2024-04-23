
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_CRC_HPP
#define BOOST_HASH_CRC_HPP

#include <boost/crc.hpp>
#include <boost/hash/detail/primes.hpp>
#include <boost/hash/digest.hpp>
#include <boost/hash/pack.hpp>
#include <boost/static_assert.hpp>

#include <climits>

#ifdef BOOST_HASH_SHOW_PROGRESS
#include <cstdio>
#endif

namespace boost {
namespace hashes {

// Boost.CRC undefs this, so re-define it
#define BOOST_CRC_PARM_TYPE  typename ::boost::uint_t<Bits>::fast

template <unsigned Bits, BOOST_CRC_PARM_TYPE TruncPoly = 0u,
          BOOST_CRC_PARM_TYPE InitRem = 0u, BOOST_CRC_PARM_TYPE FinalXor = 0u,
          bool ReflectIn = false, bool ReflectRem = false >
class basic_crc {
  public:
    typedef crc_optimal<Bits, TruncPoly, 
                        InitRem, FinalXor, 
                        ReflectIn, ReflectRem> crc_computer;
    typedef typename crc_computer::value_type word_type;

    static unsigned const value_bits = CHAR_BIT;
    typedef uint_t<value_bits>::least value_type;

    BOOST_STATIC_ASSERT(Bits >= value_bits);

    static unsigned const digest_bits = Bits;
    typedef hashes::digest<digest_bits> digest_type;

  public:
    basic_crc() { reset(); }
    void reset() { crc_.reset(); }

    digest_type digest() const {
        word_type x = crc_.checksum();
        digest_type d;
        // TODO: Justify bit order
        pack_n<stream_endian::big_bit,
               digest_bits,
               octet_bits>(&x, 1,
                           d.data(), digest_bits/octet_bits);
        return d;
    }

    digest_type end_message() {
        digest_type d = digest();
        reset();
        return d;
    }

  public:
    basic_crc &
    update_one(value_type x) {
#ifdef BOOST_HASH_SHOW_PROGRESS
printf("%.8lx + %.2x ==> ", (long)crc_.checksum(), (int)x);
#endif
        crc_.process_byte(x);
#ifdef BOOST_HASH_SHOW_PROGRESS
printf("%.8lx\n", (long)crc_.checksum());
#endif
        return *this;
    }

    template <typename IterT>
    basic_crc &
    update_n(IterT p, size_t n) {
        while (n--) update_one(*p++);
        return *this;
    }
#ifndef BOOST_HASH_NO_OPTIMIZATION
    template <typename ValT>
    basic_crc &
    update_n(ValT const *p, size_t n) {
        if (sizeof(ValT) == 1) {
            crc_.process_bytes(p, n);
        } else {
            while (n--) update_one(*p++);
        }
        return *this;
    }
    template <typename ValT>
    basic_crc &
    update_n(ValT *p, size_t n) {
        return update_n((ValT const *)p, n);
    }
#endif

    template <typename IterT>
    basic_crc &
    update(IterT b, IterT e, std::random_access_iterator_tag) {
        return update_n(b, e-b);
    }
    template <typename IterT, typename Category>
    basic_crc &
    update(IterT b, IterT e, Category) {
        while (b != e) update_one(*b++);
        return *this;
    }
    template <typename IterT>
    basic_crc &
    update(IterT b, IterT e) {
        typedef typename std::iterator_traits<IterT>::iterator_category cat;
        return update(b, e, cat());
    }
    
  private:
     crc_computer crc_;
};

template <unsigned Bits, BOOST_CRC_PARM_TYPE TruncPoly = 0u,
          BOOST_CRC_PARM_TYPE InitRem = 0u, BOOST_CRC_PARM_TYPE FinalXor = 0u,
          bool ReflectIn = false, bool ReflectRem = false >
struct crc {
  private:
    typedef basic_crc<Bits, TruncPoly, InitRem, FinalXor, ReflectIn, ReflectRem>
            octet_hash_type;
  public:
    template <unsigned value_bits>
    struct stream_hash {
        BOOST_STATIC_ASSERT(value_bits == CHAR_BIT);
        typedef octet_hash_type type_;
#ifdef BOOST_HASH_NO_HIDE_INTERNAL_TYPES
        typedef type_ type;
#else
        struct type : type_ {};
#endif
    };
    typedef typename octet_hash_type::digest_type digest_type;
};

// http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html#CRC-algorithm
typedef crc<32, 0x04C11DB7, 0xFFFFFFFF, 0xFFFFFFFF, true, true> crc32_png;

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_CRC_HPP
