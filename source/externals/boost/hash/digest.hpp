
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DIGEST_HPP
#define BOOST_HASH_DIGEST_HPP

#include <boost/array.hpp>
#include <boost/integer.hpp>
#include <boost/static_assert.hpp>

#include <string>

#include <cstring>

namespace boost {
namespace hashes {

unsigned const octet_bits = 8;
typedef uint_t<octet_bits>::least octet_type;

// Always stored internally as a sequence of octets in display order.
// This allows digests from different algorithms to have the same type,
// allowing them to be more easily stored and compared.
template <unsigned digest_bits_>
class digest : public array<octet_type, digest_bits_/octet_bits> {
  public:

    static unsigned const digest_bits = digest_bits_;
    BOOST_STATIC_ASSERT(digest_bits % octet_bits == 0);
    static unsigned const digest_octets = digest_bits/octet_bits;
    typedef array<octet_type, digest_octets> base_array_type;

    static unsigned const cstring_size = digest_bits/4 + 1;
    typedef array<char, cstring_size> cstring_type;

    digest() : base_array_type() {}
    digest(base_array_type const &a) : base_array_type(a) {}

    template <typename oit_T>
    oit_T to_ascii(oit_T it) const {
        for (unsigned j = 0; j < digest_octets; ++j) {
            octet_type b = base_array()[j];
            *it++ = "0123456789abcdef"[(b >> 4) & 0xF];
            *it++ = "0123456789abcdef"[(b >> 0) & 0xF];
        }
        return it;
    }

    std::string
    str() const {
        cstring_type cstr = cstring();
        return std::string(cstr.data(), cstr.size()-1);
    }

    cstring_type
    cstring() const {
        cstring_type s;
        char *p = to_ascii(s.data());
        *p++ = '\0';
        return s;
    }

    base_array_type const &base_array() const { return *this; }
};

template <unsigned NDB, unsigned ODB>
digest<NDB>
resize(digest<ODB> const &od) {
    digest<NDB> nd;
    unsigned bytes = sizeof(octet_type)*(NDB < ODB ? NDB : ODB)/octet_bits;
    std::memcpy(nd.data(), od.data(), bytes);
    return nd;
}

template <unsigned NDB, unsigned ODB>
digest<NDB>
truncate(digest<ODB> const &od) {
    BOOST_STATIC_ASSERT(NDB <= ODB);
    return resize<NDB>(od);
}

template <unsigned DB1, unsigned DB2>
bool operator==(digest<DB1> const &a, digest<DB2> const &b) {
    unsigned const DB = DB1 < DB2 ? DB2 : DB1;
    return resize<DB>(a).base_array() == resize<DB>(b).base_array();
}
template <unsigned DB1, unsigned DB2>
bool operator!=(digest<DB1> const &a, digest<DB2> const &b) {
    return !(a == b);
}

template <unsigned DB1, unsigned DB2>
bool operator<(digest<DB1> const &a, digest<DB2> const &b) {
    unsigned const DB = DB1 < DB2 ? DB2 : DB1;
    return resize<DB>(a).base_array() < resize<DB>(b).base_array();
}
template <unsigned DB1, unsigned DB2>
bool operator>(digest<DB1> const &a, digest<DB2> const &b) {
    return b < a;
}
template <unsigned DB1, unsigned DB2>
bool operator<=(digest<DB1> const &a, digest<DB2> const &b) {
    return !(b < a);
}
template <unsigned DB1, unsigned DB2>
bool operator>=(digest<DB1> const &a, digest<DB2> const &b) {
    return !(b > a);
}

template <unsigned DB>
bool operator!=(digest<DB> const &a, char const *b) {
    BOOST_ASSERT(std::strlen(b) == DB/4);
    return std::strcmp(a.cstring().data(), b);
}
template <unsigned DB>
bool operator==(digest<DB> const &a, char const *b) {
    return !(a != b);
}
template <unsigned DB>
bool operator!=(char const *b, digest<DB> const &a) {
    return a != b;
}
template <unsigned DB>
bool operator==(char const *b, digest<DB> const &a) {
    return a == b;
}

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DIGEST_HPP
