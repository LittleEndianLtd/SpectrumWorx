//==============================================================================
//         Copyright 2003 - 2012 LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2013 LRI    UMR 8623 CNRS/Univ Paris Sud XI
//         Copyright 2012        MetaScale SAS
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================

#include <nt2/sdk/magma/magma.hpp>

#include <nt2/table.hpp>
#include <nt2/include/functions/lu.hpp>
#include <nt2/include/functions/rand.hpp>
#include <nt2/include/functions/zeros.hpp>
#include <magma.h>
#include "../../flops/lu.hpp"
#include <boost/fusion/include/at.hpp>

#include <nt2/sdk/bench/benchmark.hpp>
#include <nt2/sdk/bench/metric/absolute_time.hpp>
#include <nt2/sdk/bench/metric/gflops.hpp>
#include <nt2/sdk/bench/protocol/max_duration.hpp>
#include <nt2/sdk/bench/setup/geometric.hpp>
#include <nt2/sdk/bench/setup/constant.hpp>
#include <nt2/sdk/bench/setup/combination.hpp>
#include <nt2/sdk/bench/stats/median.hpp>

using namespace nt2::bench;
using namespace nt2;

template<typename T> struct lu_nt2
{
  template<typename Setup>
  lu_nt2(Setup const& s)
              :  w(boost::fusion::at_c<1>(s))
              ,  h(boost::fusion::at_c<0>(s))
  {
    result = nt2::zeros(h,w, nt2::meta::as_<T>());
    input  = nt2::rand(h,w, nt2::meta::as_<T>());
    }

  void operator()()
  {
  result = nt2::lu(input);
  }

  friend std::ostream& operator<<(std::ostream& os, lu_nt2<T> const& p)
  {
    return os << "(" << p.size() << ")";
  }

  std::size_t size() const { return h*w; }
  std::size_t flops() const {
       return (FLOPS_GETRF(h,w));
  }

private:
  std::size_t w,h,nr;
  nt2::table<T> input, result;
};

NT2_REGISTER_BENCHMARK_TPL( lu_nt2, (float)(double) )
{
  std::size_t size_min = args("size_min", 16);
  std::size_t size_max = args("size_max", 4096);
  std::size_t size_step = args("size_step", 10);

  run_during_with< lu_nt2<T> > ( 1.
                                , and_( geometric(size_min,size_max,size_step)
                                      , geometric(size_min,size_max,size_step)
                                      )
                                , gflops<stats::median_>()
                                );
}

