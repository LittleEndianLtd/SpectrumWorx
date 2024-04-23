
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_DETAIL_EXPLODER_HPP
#define BOOST_HASH_DETAIL_EXPLODER_HPP

#include <boost/hash/stream_endian.hpp>
#include <boost/hash/detail/unbounded_shift.hpp>
#include <boost/integer.hpp>
#include <boost/static_assert.hpp>

#include <iterator>

#include <climits>
#include <cstring>

namespace boost {
namespace hashes {
namespace detail {

// By definition, for all exploders, InputBits > OutputBits,
// so we're taking one value and splitting it into many smaller values

template <typename OutIter, int OutBits,
          typename T = typename std::iterator_traits<OutIter>::value_type>
struct outvalue_helper {
    typedef T type;
};
template <typename OutIter, int OutBits>
struct outvalue_helper<OutIter, OutBits, void> {
    typedef typename uint_t<OutBits>::least type;
};

template <typename Endianness,
          int InputBits, int OutputBits,
          int k>
struct exploder_step;

template <int UnitBits, int InputBits, int OutputBits, int k>
struct exploder_step<stream_endian::big_unit_big_bit<UnitBits>,
                     InputBits, OutputBits, k> {
    template <typename InputValue, typename OutIter>
    static void step(InputValue const &x, OutIter &out) {
        int const shift = InputBits - (OutputBits+k);
        typedef typename outvalue_helper<OutIter, OutputBits>::type OutValue;
        InputValue y = unbounded_shr<shift>(x);
        *out++ = OutValue(low_bits<OutputBits>(y));
    }
};

template <int UnitBits, int InputBits, int OutputBits, int k>
struct exploder_step<stream_endian::little_unit_big_bit<UnitBits>,
                     InputBits, OutputBits, k> {
    template <typename InputValue, typename OutIter>
    static void step(InputValue const &x, OutIter &out) {
        int const kb = (k % UnitBits);
        int const ku = k - kb;
        int const shift =
            OutputBits >= UnitBits ? k :
            InputBits  >= UnitBits ? ku + (UnitBits-(OutputBits+kb)) :
                                     InputBits - (OutputBits+kb);
        typedef typename outvalue_helper<OutIter, OutputBits>::type OutValue;
        InputValue y = unbounded_shr<shift>(x);
        *out++ = OutValue(low_bits<OutputBits>(y));
    }
};

template <int UnitBits, int InputBits, int OutputBits, int k>
struct exploder_step<stream_endian::big_unit_little_bit<UnitBits>,
                     InputBits, OutputBits, k> {
    template <typename InputValue, typename OutIter>
    static void step(InputValue const &x, OutIter &out) {
        int const kb = (k % UnitBits);
        int const ku = k - kb;
        int const shift =
            OutputBits >= UnitBits ? InputBits - (OutputBits+k) :
            InputBits  >= UnitBits ? InputBits - (UnitBits+ku) + kb :
                                     kb;
        typedef typename outvalue_helper<OutIter, OutputBits>::type OutValue;
        InputValue y = unbounded_shr<shift>(x);
        *out++ = OutValue(low_bits<OutputBits>(y));
    }
};

template <int UnitBits, int InputBits, int OutputBits, int k>
struct exploder_step<stream_endian::little_unit_little_bit<UnitBits>,
                     InputBits, OutputBits, k> {
    template <typename InputValue, typename OutIter>
    static void step(InputValue const &x, OutIter &out) {
        int const shift = k;
        typedef typename outvalue_helper<OutIter, OutputBits>::type OutValue;
        InputValue y = unbounded_shr<shift>(x);
        *out++ = OutValue(low_bits<OutputBits>(y));
    }
};

template <int UnitBits, int InputBits, int OutputBits, int k>
struct exploder_step<stream_endian::host_unit<UnitBits>,
                     InputBits, OutputBits, k> {
    template <typename InputValue, typename OutIter>
    static void step(InputValue const &x, OutIter &out) {
        typedef typename outvalue_helper<OutIter, OutputBits>::type OutValue;
        BOOST_STATIC_ASSERT(sizeof(InputValue)*CHAR_BIT == InputBits);
        BOOST_STATIC_ASSERT(sizeof(OutValue)*CHAR_BIT == OutputBits);
        OutValue value;
        std::memcpy(&value, (char*)&x + k/CHAR_BIT, OutputBits/CHAR_BIT);
        *out++ = value;
    }
};

template <typename Endianness,
          int InputBits, int OutputBits,
          int k = 0>
struct exploder;

template <template <int> class Endian, int UnitBits,
          int InputBits, int OutputBits,
          int k>
struct exploder<Endian<UnitBits>, InputBits, OutputBits, k> {

    // To keep the implementation managable, input and output sizes must
    // be multiples or factors of the unit size.
    // If one of these is firing, you may want a bit-only stream_endian
    // rather than one that mentions bytes or octets.
    BOOST_STATIC_ASSERT(!(InputBits  % UnitBits && UnitBits % InputBits ));
    BOOST_STATIC_ASSERT(!(OutputBits % UnitBits && UnitBits % OutputBits));

    typedef Endian<UnitBits> Endianness;
    typedef exploder_step<Endianness, InputBits, OutputBits, k> step_type;
    typedef exploder<Endianness, InputBits, OutputBits, k+OutputBits> next_type;

    template <typename InputValue, typename OutIter>
    static void explode(InputValue const &x, OutIter &out) {
        step_type::step(x, out);
        next_type::explode(x, out);
    }
    
};
template <template <int> class Endian, int UnitBits,
          int InputBits, int OutputBits>
struct exploder<Endian<UnitBits>, InputBits, OutputBits, InputBits> {
    template <typename InputValue, typename OutIter>
    static void explode(InputValue const &, OutIter &) {}
};

} // namespace detail
} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_DETAIL_EXPLODER_HPP
