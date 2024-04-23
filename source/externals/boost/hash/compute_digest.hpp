
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_COMPUTE_DIGEST_HPP
#define BOOST_HASH_COMPUTE_DIGEST_HPP

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/static_assert.hpp>

#include <iterator>
#include <limits>
#include <string>

#include <cstring>
#include <cwchar>

namespace boost {
namespace hashes {

template <typename hash_T, typename iter_T>
typename hash_T::digest_type
compute_digest(iter_T b, iter_T e) {
    typedef typename std::iterator_traits<iter_T>::value_type value_type;
    BOOST_STATIC_ASSERT(std::numeric_limits<value_type>::is_specialized);
    unsigned const value_bits =
        std::numeric_limits<value_type>::digits +
        std::numeric_limits<value_type>::is_signed;
    typedef typename hash_T::template stream_hash<value_bits>::type
            stream_hash_type;
    stream_hash_type sh;
    sh.update(b, e);
    return sh.end_message();
}

template <typename hash_T, typename iter_T>
typename hash_T::digest_type
compute_digest_n(iter_T b, size_t n) {
    typedef typename std::iterator_traits<iter_T>::value_type value_type;
    BOOST_STATIC_ASSERT(std::numeric_limits<value_type>::is_specialized);
    unsigned const value_bits =
        std::numeric_limits<value_type>::digits +
        std::numeric_limits<value_type>::is_signed;
    typedef typename hash_T::template stream_hash<value_bits>::type
            stream_hash_type;
    stream_hash_type sh;
    sh.update_n(b, n);
    return sh.end_message();
}

template <typename hash_T, typename range_T>
typename hash_T::digest_type
compute_digest(range_T const &r) {
    return compute_digest<hash_T>(boost::begin(r), boost::end(r));
}

template <typename hash_T, typename container_T>
typename hash_T::digest_type
compute_digest_n(container_T const &c) {
    return compute_digest_n<hash_T>(c.begin(), c.size());
}

template <typename hash_T, typename container_T>
typename hash_T::digest_type
compute_digest_data(container_T const &c) {
    return compute_digest_n<hash_T>(c.data(), c.size());
}

template <typename hash_T,
          typename Char, typename CharTraits, typename Alloc>
typename hash_T::digest_type
compute_digest(std::basic_string<Char, CharTraits, Alloc> const &s) {
    return compute_digest_data<hash_T>(s);
}
template <typename hash_T,
          typename Char, typename CharTraits, typename Alloc>
typename hash_T::digest_type
compute_digest_n(std::basic_string<Char, CharTraits, Alloc> const &s) {
    return compute_digest_data<hash_T>(s);
}

template <typename hash_T>
typename hash_T::digest_type
compute_digest(char const *p) {
    return compute_digest_n<hash_T>(p, std::strlen(p));
}

template <typename hash_T>
typename hash_T::digest_type
compute_digest(wchar_t const *p) {
    return compute_digest_n<hash_T>(p, std::wcslen(p));
}

template <typename hash_T>
struct digest_computer {
    typedef typename hash_T::digest_type digest_type;
    template <typename T>
    digest_type operator()(T const &t) {
        return compute_digest<hash_T>(t);
    }
    template <typename T, typename U>
    digest_type operator()(T const &t, U const &u) {
        return compute_digest<hash_T>(t, u);
    }
};

template <typename hash_T>
digest_computer<hash_T>
compute_digest() {
    return digest_computer<hash_T>();
}

template <typename hash_T>
struct digest_computer_n {
    typedef typename hash_T::digest_type digest_type;
    template <typename T>
    digest_type operator()(T const &t) {
        return compute_digest_n<hash_T>(t);
    }
    template <typename T, typename U>
    digest_type operator()(T const &t, U const &u) {
        return compute_digest_n<hash_T>(t, u);
    }
};

template <typename hash_T>
digest_computer_n<hash_T>
compute_digest_n() {
    return digest_computer_n<hash_T>();
}

template <typename hash_T>
struct digest_computer_data {
    typedef typename hash_T::digest_type digest_type;
    template <typename T>
    digest_type operator()(T const &t) {
        return compute_digest_data<hash_T>(t);
    }
    template <typename T, typename U>
    digest_type operator()(T const &t, U const &u) {
        return compute_digest_data<hash_T>(t, u);
    }
};

template <typename hash_T>
digest_computer_data<hash_T>
compute_digest_data() {
    return digest_computer_data<hash_T>();
}

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_COMPUTE_DIGEST_HPP
