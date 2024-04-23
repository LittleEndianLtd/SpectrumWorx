//==============================================================================
//         Copyright 2003 - 2012   LASMEA UMR 6602 CNRS/Univ. Clermont II
//         Copyright 2009 - 2012   LRI    UMR 8623 CNRS/Univ Paris Sud XI
//
//          Distributed under the Boost Software License, Version 1.0.
//                 See accompanying file LICENSE.txt or copy at
//                     http://www.boost.org/LICENSE_1_0.txt
//==============================================================================
#define NT2_UNIT_MODULE "nt2 container runner"

#include <nt2/table.hpp>
#include <nt2/include/functions/function.hpp>
#include <nt2/include/functions/toint.hpp>
#include <nt2/include/functions/of_size.hpp>
#include <nt2/include/functions/sum.hpp>
#include <nt2/include/functions/cons.hpp>
#include <nt2/sdk/meta/as_integer.hpp>

#include <nt2/sdk/unit/module.hpp>
#include <nt2/sdk/unit/tests/basic.hpp>
#include <nt2/sdk/unit/tests/relation.hpp>
#include <nt2/sdk/unit/tests/type_expr.hpp>

template<class T>
struct extent_
{
  typedef typename T::extent_type type;
};

struct run_type
{
  template<class X>
  struct apply
       : nt2::meta::call<nt2::tag::run_(X&)>
  {
  };
};

NT2_TEST_CASE( run_type )
{
  using nt2::table;
  using nt2::tag::table_;
  using nt2::view;
  typedef double T;
  typedef nt2::meta::as_integer<T>::type U;
  typedef nt2::settings S(nt2::_4D);

  table<T, S> a0;

  typedef nt2::memory::container<table_, T, S> container;
  typedef boost::proto::basic_expr< nt2::tag::terminal_, boost::proto::term<container&> > expr;
  typedef nt2::container::expression<expr, container&> nt2_expr;

  NT2_TEST_EXPR_TYPE( a0
                    , run_type
                    , (table<T, S>&)
                    );

  NT2_TEST_EXPR_TYPE( boost::proto::child_c<0>(nt2::assign(a0, a0))
                    , run_type
                    , nt2_expr
                    );

  NT2_TEST_EXPR_TYPE( boost::proto::child_c<0>(a0 + a0)
                    , run_type
                    , (view<table_, T const, S>)
                    );

  NT2_TEST_EXPR_TYPE( a0 + a0
                    , run_type
                    , (table<T, S>)
                    );

  NT2_TEST_EXPR_TYPE( nt2::toint(a0)
                    , run_type
                    , (table<U, S>)
                    );
}

NT2_TEST_CASE( run_data )
{
  using nt2::table;
  typedef double T;
  typedef nt2::meta::as_integer<T>::type U;

  table<T> a0( nt2::of_size(5, 3) );

  for(std::size_t j=1; j!=3+1; ++j)
    for(std::size_t i=1; i!=5+1; ++i)
      a0(i, j) = T(i+j+0.5);

  table<U> a1 = nt2::toint(a0*T(2));

  for(std::size_t j=1; j!=3+1; ++j)
    for(std::size_t i=1; i!=5+1; ++i)
      NT2_TEST_EQUAL( U((i+j+0.5)*2), U(a1(i, j)) );
}

NT2_TEST_CASE( value_at )
{
  using nt2::table;
  using nt2::of_size;
  typedef float T;

  table<T> a0(of_size(4, 3));
  table<T> a1(of_size(4, 3));

  for(std::size_t j=1; j!=3+1; ++j)
    for(std::size_t i=1; i!=4+1; ++i)
      a0(i, j) = T(i+j);

  a1 = a0 + T(1);

  for(std::size_t j=1; j!=3+1; ++j)
    for(std::size_t i=1; i!=4+1; ++i)
      NT2_TEST_EQUAL( T(a1(i, j)), T(i+j+1) );

  table<T, nt2::_1D> a2(of_size(13));
  table<T, nt2::_1D> a3(of_size(13));
  for(std::size_t i=1; i!=13+1; ++i)
    a2(i) = T(i);

  a3 = a2 + T(1);

  for(std::size_t i=1; i!=13+1; ++i)
    NT2_TEST_EQUAL( T(a3(i)), T(i+1) );

/*
  table<T, nt2::_1D> xd = nt2::_(T(0),T(2),T(9));
  table<T, nt2::_1D> a2_(of_size(5));
  a2_ = a2(xd);
*/
}

NT2_TEST_CASE( scalar_size )
{
  using nt2::table;
  using nt2::of_size;
  typedef float T;

  table<T> a0 = T(42);
  NT2_TEST_EQUAL( a0.extent(), of_size(1) );
  NT2_TEST_EQUAL( T(a0(1)), T(42) );

  table<T> a1;
  NT2_TEST_EQUAL( a1.extent(), of_size(0) );

  a1 = T(42);
  NT2_TEST_EQUAL( a1.extent(), of_size(1) );
  NT2_TEST_EQUAL( T(a0(1)), T(42) );
}

NT2_TEST_CASE( element_wise )
{
  using nt2::table;
  using nt2::of_size;
  typedef float T;
  typedef boost::dispatch::meta::as_integer<T>::type iT;

  table<T> a0(of_size(1));
  table<iT> a1;
  a1 = nt2::toint(a0);

  NT2_TEST_EXPR_TYPE( nt2::run(nt2::toint(a0), 0u, nt2::meta::as_<iT>())
                    , boost::mpl::identity<boost::mpl::_>
                    , iT
                    );
}

NT2_TEST_CASE( assign_op )
{
  using nt2::table;
  using nt2::of_size;
  using nt2::cons;
  using nt2::reshape;
  using nt2::_;
  typedef float T;

  table<T> out = cons<T>(of_size(2, 2), 1, 2, 3, 4);
  T d = T(5);
  out(_, 1) *= d;

  NT2_TEST_EQUAL(out, cons<T>(of_size(2, 2), 5, 10, 3, 4));
}

NT2_TEST_CASE( reduction_size )
{
  using nt2::table;
  using nt2::of_size;
  using nt2::sum;
  typedef double T;
  using nt2::_;

  std::size_t M = 2;
  std::size_t N = 3;
  std::size_t O = 4;
  std::size_t P = 5;

  table<T,nt2::_1D> a00(of_size(M));
  table<T> a01(of_size(M));

  table<T> a1(of_size(1,M));
  table<T> a2_4(of_size(M,N,O,P));
  table<T, nt2::_3D> a2_3(of_size(M,N,O));
  table<T, nt2::_2D> a2_2(of_size(M,N));

  table<T> a3(of_size(M,N,O,1));

  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t k = 1; k <= O; ++k){
      for(std::size_t j = 1; j <= N; ++j){
        for(std::size_t i = 1; i <= M; ++i){
          a2_4(i,j,k,l) = T(1);
          a2_3(i,j,k) = T(1);
          a2_2(i,j) = T(1);
          a00(i) = T(1);
          a01(i) = T(1);
        }
      }
    }
  }


  a1 = sum(a2_4,4);
  NT2_TEST_EQUAL( a1.extent(), of_size(M,N,O) );
  NT2_TEST_EQUAL( a2_4.extent(), of_size(M,N,O,P) );

  a1 = sum(a2_4,3);
  NT2_TEST_EQUAL( a1.extent(), of_size(M,N,1,P) );

  a1 = sum(a2_3,2);
  NT2_TEST_EQUAL( a1.extent(), of_size(M,1,O) );

  a1 = sum(a2_4,2);
  NT2_TEST_EQUAL( a1.extent(), of_size(M,1,O,P) );

  a1 = sum(a2_4,1);
  NT2_TEST_EQUAL( a1.extent(), of_size(1,N,O,P) );
  a1 = sum(a2_3,1);
  NT2_TEST_EQUAL( a1.extent(), of_size(1,N,O) );
  a1 = sum(a2_2,1);
  NT2_TEST_EQUAL( a1.extent(), of_size(1,N) );

  a1 = sum(a00);
  NT2_TEST_EQUAL( a1.extent(), of_size(1) );
  a1 = sum(a01);
  NT2_TEST_EQUAL( a1.extent(), of_size(1) );

}

NT2_TEST_CASE( reduction_value )
{

  using nt2::table;
  using nt2::of_size;
  using nt2::sum;
  typedef double T;
  using nt2::_;


  std::size_t M = 5;
  std::size_t N = 4;
  std::size_t O = 3;
  std::size_t P = 2;

  table<T,nt2::_1D> a00(of_size(M));
  table<T>          a01(of_size(M));

  table<T> a1;
  table<T> a2_4(of_size(M,N,O,P));
  table<T> b2_4(of_size(M,N,O,P));
  table<T, nt2::_3D> a2_3(of_size(M,N,O));
  table<T, nt2::_2D> a2_2(of_size(M,N));

  table<T> w(of_size(M,N,O,P));
  table<T> x(of_size(M,N,O,P));
  table<T> y(of_size(M,N,O,P));
  table<T> z(of_size(1,N,O,P));

  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t k = 1; k <= O; ++k){
      for(std::size_t j = 1; j <= N; ++j){
        for(std::size_t i = 1; i <= M; ++i){
          a2_4(i,j,k,l) = T(1);
          b2_4(i,j,k,l) = T(1);
          a2_3(i,j,k) = T(1);
          a2_2(i,j) = T(1);
          a00(i) = T(1);
          a01(i) = T(1);
          w(i,j,k,l) = x(i,j,k,l) = y(i,j,k,l) = z(1,j,k,l) = T(1);
        }
      }
    }
  }

  a1 = sum(a2_4,1);
  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t k = 1; k <= O; ++k){
      for(std::size_t j = 1; j <= N; ++j){
        NT2_TEST_EQUAL(T(a1(1,j,k,l)),T(M)) ;
      }
    }
  }



  a1 = sum(a2_4 + b2_4,1);
  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t k = 1; k <= O; ++k){
      for(std::size_t j = 1; j <= N; ++j){
        NT2_TEST_EQUAL(T(a1(1,j,k,l)),2*T(M)) ;
      }
    }
  }

  a1 = sum(a2_3,1);
  for(std::size_t k = 1; k <= O; ++k){
    for(std::size_t j = 1; j <= N; ++j){
      NT2_TEST_EQUAL(T(a1(1,j,k)),T(M)) ;
    }
  }

  a1 = sum(a2_2,1);
  for(std::size_t j = 1; j <= N; ++j){
    NT2_TEST_EQUAL(T(a1(1,j)),T(M)) ;
  }


  a1 = sum(a00);
  NT2_TEST_EQUAL(T(a1(1)),T(M)) ;

  a1 = sum(a2_4(nt2::_));
  NT2_TEST_EQUAL(T(a1(1)),T(nt2::numel(a2_4))) ;

  a1 = sum(a01);
  NT2_TEST_EQUAL(T(a1(1)),T(M)) ;

  a1 = sum(a2_4,4);
  for(std::size_t k = 1; k <= O; ++k){
    for(std::size_t j = 1; j <= N; ++j){
      for(std::size_t i = 1; i <= M; ++i){
        NT2_TEST_EQUAL(T(a1(i,j,k,1)),T(P)) ;
      }
    }
  }

  a1 = sum(a2_3,3);
  for(std::size_t j = 1; j <= N; ++j){
    for(std::size_t i = 1; i <= M; ++i){
      NT2_TEST_EQUAL(T(a1(i,j,1)),T(O)) ;
    }
  }

  a1 = sum(a2_2,2);
  for(std::size_t i = 1; i <= M; ++i){
    NT2_TEST_EQUAL(T(a1(i,1)),T(N)) ;
  }


  a1 = sum(a2_4,3);
  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t j = 1; j <= N; ++j){
      for(std::size_t i = 1; i <= M; ++i){
        NT2_TEST_EQUAL(T(a1(i,j,1,l)),T(O)) ;
      }
    }
  }


  a1 = sum(a2_4,2);
  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t k = 1; k <= O; ++k){
      for(std::size_t i = 1; i <= M; ++i){
        NT2_TEST_EQUAL(T(a1(i,1,k,l)),T(N)) ;
      }
    }
  }

  a1 = sum(a2_3,2);
  for(std::size_t k = 1; k <= O; ++k){
    for(std::size_t i = 1; i <= M; ++i){
      NT2_TEST_EQUAL(T(a1(i,1,k)),T(N)) ;
    }
  }

  w = sum(x+y,1) + z;

  for(std::size_t l = 1; l <= P; ++l){
    for(std::size_t k = 1; k <= O; ++k){
      for(std::size_t j = 1; j <= N; ++j){
        NT2_TEST_EQUAL(T(w(1,j,k,l)),T(2)*T(M) +T(1)) ;
      }
    }
  }
}
