
//
// Copyright 2010 Scott McMurray.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HASH_PACK_HPP
#define BOOST_HASH_PACK_HPP

#include <boost/assert.hpp>
#include <boost/hash/stream_endian.hpp>
#include <boost/hash/detail/exploder.hpp>
#include <boost/hash/detail/imploder.hpp>
#include <boost/static_assert.hpp>

#ifndef BOOST_HASH_NO_OPTIMIZATION
#include <boost/detail/endian.hpp>
#include <boost/utility/enable_if.hpp>
#endif

namespace boost {
namespace hashes {

#ifndef BOOST_HASH_NO_OPTIMIZATION

template <int UnitBits,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct host_can_memcpy {
    static bool const value =
        !(UnitBits % CHAR_BIT) &&
        InputBits  >= UnitBits &&
        OutputBits >= UnitBits &&
        sizeof(InT )*CHAR_BIT == InputBits &&
        sizeof(OutT)*CHAR_BIT == OutputBits;
};

template <typename Endianness,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct can_memcpy {
    static bool const value =
        InputBits == OutputBits &&
        sizeof(InT) == sizeof(OutT);
};

template <int UnitBits,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct can_memcpy<stream_endian::host_unit<UnitBits>,
                  InputBits, OutputBits,
                  InT, OutT>
 : host_can_memcpy<UnitBits, InputBits, OutputBits, InT, OutT> {};        

#ifdef BOOST_LITTLE_ENDIAN
template <int UnitBits,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct can_memcpy<stream_endian::little_unit_big_bit<UnitBits>,
                  InputBits, OutputBits,
                  InT, OutT>
 : host_can_memcpy<UnitBits, InputBits, OutputBits, InT, OutT> {};        
template <int UnitBits,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct can_memcpy<stream_endian::little_unit_little_bit<UnitBits>,
                  InputBits, OutputBits,
                  InT, OutT>
 : host_can_memcpy<UnitBits, InputBits, OutputBits, InT, OutT> {};        
#endif

#ifdef BOOST_BIG_ENDIAN
template <int UnitBits,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct can_memcpy<stream_endian::big_unit_big_bit<UnitBits>,
                  InputBits, OutputBits,
                  InT, OutT>
 : host_can_memcpy<UnitBits, InputBits, OutputBits, InT, OutT> {};        
template <int UnitBits,
          int InputBits, int OutputBits,
          typename InT, typename OutT>
struct can_memcpy<stream_endian::big_unit_little_bit<UnitBits>,
                  InputBits, OutputBits,
                  InT, OutT>
 : host_can_memcpy<UnitBits, InputBits, OutputBits, InT, OutT> {};        
#endif

#endif


template <typename Endianness,
          int InputBits, int OutputBits,
          bool Explode = (InputBits > OutputBits),
          bool Implode = (InputBits < OutputBits)>
struct real_packer;

template <typename Endianness,
          int Bits>
struct real_packer<Endianness,
                   Bits, Bits,
                   false, false> {

    template <typename InIter, typename OutIter>
    static void pack_n(InIter in, size_t in_n,
                       OutIter out) {
        while (in_n--) *out++ = *in++;
    }

    template <typename InIter, typename OutIter>
    static void pack(InIter in, InIter in_e,
                     OutIter out) {
        while (in != in_e) *out++ = *in++;
    }

};

template <typename Endianness,
          int InputBits, int OutputBits>
struct real_packer<Endianness,
                   InputBits, OutputBits,
                   true, false> {

    BOOST_STATIC_ASSERT(InputBits % OutputBits == 0);

    template <typename InIter, typename OutIter>
    static void pack_n(InIter in, size_t in_n,
                       OutIter out) {
        while (in_n--) {
            typedef typename std::iterator_traits<InIter>::value_type InValue;
            InValue const value = *in++;
            detail::exploder<Endianness, InputBits, OutputBits>
             ::explode(value, out);
        }
    }

    template <typename InIter, typename OutIter>
    static void pack(InIter in, InIter in_e,
                     OutIter out) {
        while (in != in_e) {
            typedef typename std::iterator_traits<InIter>::value_type InValue;
            InValue const value = *in++;
            detail::exploder<Endianness, InputBits, OutputBits>
             ::explode(value, out);
        }
    }

};

template <typename Endianness,
          int InputBits, int OutputBits>
struct real_packer<Endianness,
                   InputBits, OutputBits,
                   false, true> {

    BOOST_STATIC_ASSERT(OutputBits % InputBits == 0);

    template <typename InIter, typename OutIter>
    static void pack_n(InIter in, size_t in_n,
                       OutIter out) {
        size_t out_n = in_n/(OutputBits/InputBits);
        while (out_n--) {
            typedef typename detail::outvalue_helper<OutIter, OutputBits>::type 
                    OutValue;
            OutValue value = OutValue();
            detail::imploder<Endianness, InputBits, OutputBits>
             ::implode(in, value);
            *out++ = value;
        }
    }

    template <typename InIter, typename OutIter>
    static void pack(InIter in, InIter in_e,
                     OutIter out) {
        while (in != in_e) {
            typedef typename detail::outvalue_helper<OutIter, OutputBits>::type 
                    OutValue;
            OutValue value = OutValue();
            detail::imploder<Endianness, InputBits, OutputBits>
             ::implode(in, value);
            *out++ = value;
        }
    }

};

template <typename Endianness,
          int InputBits, int OutputBits>
struct packer : real_packer<Endianness, InputBits, OutputBits> {

#ifndef BOOST_HASH_NO_OPTIMIZATION

    using real_packer<Endianness, InputBits, OutputBits>::pack_n;

    template <typename InT, typename OutT>
    static typename enable_if<can_memcpy<Endianness, InputBits, OutputBits,
                                         InT, OutT> >::type
    pack_n(InT const *in, size_t n,
           OutT *out) {
        std::memcpy(out, in, n*sizeof(InT));
    }

    template <typename InT, typename OutT>
    static typename enable_if<can_memcpy<Endianness, InputBits, OutputBits,
                                         InT, OutT> >::type
    pack_n(InT *in, size_t n,
           OutT *out) {
        std::memcpy(out, in, n*sizeof(InT));
    }

#endif

};

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename IterT2>
void pack_n(IterT1 in, size_t in_n,
            IterT2 out) {
    typedef packer<Endianness, InValueBits, OutValueBits> packer_type;
    packer_type::pack_n(in, in_n, out);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename IterT2>
void pack_n(IterT1 in, size_t in_n,
            IterT2 out, size_t out_n) {
    BOOST_ASSERT(in_n*InValueBits == out_n*OutValueBits);
    pack_n<Endianness, InValueBits, OutValueBits>(in, in_n, out);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename IterT2>
void pack(IterT1 b1, IterT1 e1, std::random_access_iterator_tag,
          IterT2 b2) {
    pack_n<Endianness, InValueBits, OutValueBits>(b1, e1-b1, b2);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename CatT1,
          typename IterT2>
void pack(IterT1 b1, IterT1 e1, CatT1,
          IterT2 b2) {
    typedef packer<Endianness, InValueBits, OutValueBits> packer_type;
    packer_type::pack(b1, e1, b2);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename IterT2>
void pack(IterT1 b1, IterT1 e1,
          IterT2 b2) {
    typedef typename std::iterator_traits<IterT1>::iterator_category cat1;
    pack<Endianness, InValueBits, OutValueBits>(b1, e1, cat1(), b2);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename IterT2>
void pack(IterT1 b1, IterT1 e1, std::random_access_iterator_tag,
          IterT2 b2, IterT2 e2, std::random_access_iterator_tag) {
    pack_n<Endianness, InValueBits, OutValueBits>(b1, e1-b1, b2, e2-b2);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename CatT1,
          typename IterT2, typename CatT2>
void pack(IterT1 b1, IterT1 e1, CatT1,
          IterT2 b2, IterT2, CatT2) {
    pack<Endianness, InValueBits, OutValueBits>(b1, e1, b2);
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename IterT1, typename IterT2>
void pack(IterT1 b1, IterT1 e1,
          IterT2 b2, IterT2 e2) {
    typedef typename std::iterator_traits<IterT1>::iterator_category cat1;
    typedef typename std::iterator_traits<IterT2>::iterator_category cat2;
    pack<Endianness, InValueBits, OutValueBits>(b1, e1, cat1(), b2, e2, cat2());
}

template <typename Endianness,
          int InValueBits, int OutValueBits,
          typename InputType, typename OutputType>
void pack(InputType const &in, OutputType &out) {
    pack_n<Endianness, InValueBits, OutValueBits>(in.data(), in.size(),
                                                  out.data(), out.size());
}

} // namespace hashes
} // namespace boost

#endif // BOOST_HASH_PACK_HPP
