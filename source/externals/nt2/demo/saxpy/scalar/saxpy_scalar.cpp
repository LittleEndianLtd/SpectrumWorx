//==============================================================================
//         Copyright 2003 - 2011   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2011   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012 - 2014   MetaScale SAS
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================

#include <boost/fusion/include/at.hpp>
#include <vector>

#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/metric/gflops.hpp>
#include <nt2/sdk/bench/protocol/max_duration.hpp>
#include <nt2/sdk/bench/setup/geometric.hpp>
#include <nt2/sdk/bench/setup/constant.hpp>
#include <nt2/sdk/bench/setup/combination.hpp>
#include <nt2/sdk/bench/stats/median.hpp>

using namespace nt2::bench;
using namespace nt2;

template<typename T> struct axpy_scalar
{
  typedef void experiment_is_immutable;
  template<typename Setup>
  axpy_scalar(Setup const& s)
              :  size_(boost::fusion::at_c<0>(s))
              ,  alpha(boost::fusion::at_c<1>(s))
  {
    X.resize(size_); Y.resize(size_);
    for(std::size_t i = 0; i<size_; ++i)
      X[i] = Y[i] = T(i);
  }

  void operator()()
  {
    for(std::size_t i = 0; i<size_; i++)
      Y[i] = Y[i] + alpha*(X[i]);
  }

  friend std::ostream& operator<<(std::ostream& os, axpy_scalar<T> const& p)
  {
    return os << "(" << p.size() << ")";
  }

  std::size_t size() const { return size_; }
  double flops() const { return 2.*size_; }

  private:
    std::size_t size_;
    T alpha;
    std::vector<T> X, Y;
};

NT2_REGISTER_BENCHMARK_TPL( axpy_scalar, NT2_SIMD_REAL_TYPES )
{
  std::size_t size_min  = args("size_min", 16);
  std::size_t size_max  = args("size_max", 4096);
  std::size_t size_step = args("size_step", 2);
  T alpha = args("alpha", 1.);

  run_during_with< axpy_scalar<T> > ( 1.
                                    , and_( geometric(size_min,size_max,size_step)
                                          , constant(alpha)
                                    )
                                    , gflops<stats::median_>()
                                    );
}
