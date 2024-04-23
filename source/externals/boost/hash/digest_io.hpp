
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DIGEST_IO_HPP
#define BOOST_HASH_DIGEST_IO_HPP

#include <boost/hash/digest.hpp>
#include <boost/hash/pack.hpp>

#include <istream>
#include <iterator>
#include <ostream>

#include <cctype>

namespace boost {
namespace hashes {

template <unsigned DB>
std::ostream &
operator<<(std::ostream &sink, digest<DB> const &d) {
    d.to_ascii(std::ostream_iterator<char>(sink));
    return sink;
};

template <unsigned DB>
std::istream &
operator>>(std::istream &source, digest<DB> &d) {
    boost::array<char, DB/4> a = {{}};
    for (unsigned i = 0; i < a.size(); ++i) {
        char c;
        if (!source.get(c)) {
            source.setstate(std::ios::failbit);
            break;        
        }
        if (!std::isxdigit(c, source.getloc())) {
            source.unget();
            source.setstate(std::ios::failbit);
            break;        
        }
        
        if (std::isdigit(c, source.getloc())) {
            a[i] = (c - '0');
        } else {
            a[i] = std::toupper(c, source.getloc()) - 'A' + 0xA;
        }
    }
    pack<stream_endian::big_bit, 4, 8>(a, d);
    return source;
};

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DIGEST_IO_HPP
