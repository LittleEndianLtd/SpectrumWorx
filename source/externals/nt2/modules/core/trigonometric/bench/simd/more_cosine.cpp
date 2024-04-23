//==============================================================================
//         Copyright 2003 - 2013   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#include <nt2/trigonometric/include/functions/cosine.hpp>

#include <boost/simd/sdk/simd/native.hpp>
#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/timing.hpp>
#include <boost/dispatch/meta/as_integer.hpp>
#include <nt2/include/constants/pi.hpp>

using nt2::tag::cosine_;

#define RS(T,V1,V2) (T, (V1) ,(V2))

namespace n1 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace n011 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::direct_small_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace n11 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::direct_small_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace nn011 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace nn11 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace nn0 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::direct_medium_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace nn1 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::direct_medium_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace nn0111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::clipped_medium_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace nn111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::clipped_medium_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}


namespace n11 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-60*nt2::Pi<T>(),60*nt2::Pi<T>())))
}
namespace n111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-10000*nt2::Pi<T>(),10000*nt2::Pi<T>())))
}
namespace n111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,T(-1.0e36),T(1.0e36))))
}
namespace n02 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace n2 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}

namespace n21 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-60*nt2::Pi<T>(),60*nt2::Pi<T>())))
}

namespace n211 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,-10000*nt2::Pi<T>(),10000*nt2::Pi<T>())))
}

namespace n211 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::small_>,(RS(vT,T(-1.0e15),T(1.0e15))))
}
namespace m01 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace m02 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}namespace m1 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace m2 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace m11 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-60*nt2::Pi<T>(),60*nt2::Pi<T>())))
}
namespace m21 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-60*nt2::Pi<T>(),60*nt2::Pi<T>())))
}
namespace m111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-10000*nt2::Pi<T>(),10000*nt2::Pi<T>())))
}
namespace m211 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,-10000*nt2::Pi<T>(),10000*nt2::Pi<T>())))
}
namespace m111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,T(-1.0e36),T(1.0e36))))
}
namespace m211 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::medium_>,(RS(vT,T(-1.0e36),T(1.0e36))))
}
namespace n01 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}
namespace n02 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-nt2::Pi<T>()/4,nt2::Pi<T>()/4)))
}namespace n1 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace n2 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-20*nt2::Pi<T>(),20*nt2::Pi<T>())))
}
namespace n11 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-60*nt2::Pi<T>(),60*nt2::Pi<T>())))
}
namespace n21 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-60*nt2::Pi<T>(),60*nt2::Pi<T>())))
}
namespace n111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-10000*nt2::Pi<T>(),10000*nt2::Pi<T>())))
}
namespace n211 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,-10000*nt2::Pi<T>(),10000*nt2::Pi<T>())))
}
namespace n111 {
  typedef float T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,T(-1.0e36),T(1.0e36))))
}
namespace n211 {
  typedef double T;
  typedef boost::simd::meta::vector_of<T, BOOST_SIMD_BYTES/sizeof(T)>::type vT;
  NT2_TIMING(cosine_<nt2::big_>,(RS(vT,T(-1.0e36),T(1.0e36))))
}

#undef RS
